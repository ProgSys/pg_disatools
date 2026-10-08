#include "PG_DisaDataTableModel.h"

#include <Resources/PG_ResourceManager.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Disa/fileformats/PG_ShiftJisEncoder.h>
#include <Util/PG_StringUtil.h>
#include <Util/Debug/PG_DebugOut.h>

#include <charconv>
#include <cstring>
#include <fstream>
#include <iterator>

using namespace PG_DefDefinition;

namespace {
	constexpr std::size_t g_maxRows = 9000;

	struct ByteReader {
		const std::vector<char>& data;
		std::size_t pos = 0;

		bool read(void* out, std::size_t bytes) {
			if (pos + bytes > data.size()) return false;
			std::memcpy(out, data.data() + pos, bytes);
			pos += bytes;
			return true;
		}
		bool read(std::string& out, std::size_t bytes) {
			if (pos + bytes > data.size()) return false;
			out.assign(data.data() + pos, bytes);
			pos += bytes;
			return true;
		}
	};

	/// Byte size of a number, the definition files also allow the size in bits
	std::size_t numberByteSize(const RowFormat& format) {
		switch (format.byteSize) {
		case 2: case 16: return 2;
		case 4: case 32: return 4;
		case 8: case 64: return 8;
		default: return 1;
		}
	}

	bool isShiftJisLeadByte(unsigned char c) {
		return (c >= 0x81 && c <= 0x9F) || (c >= 0xE0 && c <= 0xFC);
	}

	/// Cuts the string to at most maxBytes, without splitting a double byte char
	void clampShiftJis(std::string& sjis, std::size_t maxBytes) {
		std::size_t i = 0;
		while (i < sjis.size()) {
			const std::size_t charSize = isShiftJisLeadByte(sjis[i]) ? 2 : 1;
			if (i + charSize > maxBytes) break;
			i += charSize;
		}
		sjis.resize(std::min(i, sjis.size()));
	}

	std::string utf16ToUtf8(const std::string& utf16) {
		std::string out;
		for (std::size_t i = 0; i + 1 < utf16.size(); i += 2) {
			uint32_t c = (unsigned char)utf16[i] | ((unsigned char)utf16[i + 1] << 8);
			if (c >= 0xD800 && c <= 0xDBFF && i + 3 < utf16.size()) {
				const uint32_t low = (unsigned char)utf16[i + 2] | ((unsigned char)utf16[i + 3] << 8);
				c = 0x10000 + ((c - 0xD800) << 10) + (low - 0xDC00);
				i += 2;
			}
			if (c < 0x80) out.push_back(char(c));
			else if (c < 0x800) {
				out.push_back(char(0xC0 | (c >> 6)));
				out.push_back(char(0x80 | (c & 0x3F)));
			}
			else if (c < 0x10000) {
				out.push_back(char(0xE0 | (c >> 12)));
				out.push_back(char(0x80 | ((c >> 6) & 0x3F)));
				out.push_back(char(0x80 | (c & 0x3F)));
			}
			else {
				out.push_back(char(0xF0 | (c >> 18)));
				out.push_back(char(0x80 | ((c >> 12) & 0x3F)));
				out.push_back(char(0x80 | ((c >> 6) & 0x3F)));
				out.push_back(char(0x80 | (c & 0x3F)));
			}
		}
		return out;
	}

	std::string utf8ToUtf16(const std::string& utf8) {
		std::string out;
		const auto push = [&out](uint32_t c) {
			out.push_back(char(c & 0xFF));
			out.push_back(char((c >> 8) & 0xFF));
		};
		for (std::size_t i = 0; i < utf8.size();) {
			const unsigned char c = utf8[i];
			uint32_t code = c;
			std::size_t length = 1;
			if (c >= 0xF0) { code = c & 0x07; length = 4; }
			else if (c >= 0xE0) { code = c & 0x0F; length = 3; }
			else if (c >= 0xC0) { code = c & 0x1F; length = 2; }
			for (std::size_t a = 1; a < length && i + a < utf8.size(); ++a)
				code = (code << 6) | (utf8[i + a] & 0x3F);
			i += length;

			if (code >= 0x10000) {
				code -= 0x10000;
				push(0xD800 + (code >> 10));
				push(0xDC00 + (code & 0x3FF));
			}
			else push(code);
		}
		return out;
	}

	std::string toHex(const std::string& bytes) {
		constexpr char digits[] = "0123456789abcdef";
		std::string out;
		out.reserve(bytes.size() * 2);
		for (unsigned char c : bytes) {
			out.push_back(digits[c >> 4]);
			out.push_back(digits[c & 0xF]);
		}
		return out;
	}

	bool fromHex(const std::string& hex, std::string& bytesOut) {
		if (hex.size() % 2 != 0) return false;
		bytesOut.clear();
		for (std::size_t i = 0; i < hex.size(); i += 2) {
			unsigned int value = 0;
			auto result = std::from_chars(hex.data() + i, hex.data() + i + 2, value, 16);
			if (result.ec != std::errc() || result.ptr != hex.data() + i + 2) return false;
			bytesOut.push_back(char(value));
		}
		return true;
	}

	/// Reads the length prefix of a dynamic string or returns the fixed size
	bool readStringSize(const RowFormat& format, ByteReader& reader, std::size_t& sizeOut) {
		if (format.dynamicLength == 0) {
			sizeOut = format.byteSize;
			return true;
		}
		uint64_t size = 0;
		if (format.dynamicLength > sizeof(size) || !reader.read(&size, format.dynamicLength)) return false;
		sizeOut = size;
		return true;
	}

	/// @returns false when the data ended too early
	bool readCell(const RowFormat& format, ByteReader& reader, std::string& out) {
		switch (format.type) {
		case RowFormat::ZERO: {
			std::string skip;
			return reader.read(skip, format.byteSize);
		}
		case RowFormat::INT:
		case RowFormat::UINT: {
			const std::size_t bytes = numberByteSize(format);
			uint64_t raw = 0;
			if (!reader.read(&raw, bytes)) return false;
			if (format.type == RowFormat::INT && bytes < 8 && (raw >> (bytes * 8 - 1)) & 1) {
				raw |= ~uint64_t(0) << (bytes * 8); //sign extend
			}
			out = format.type == RowFormat::INT ? std::to_string(int64_t(raw)) : std::to_string(raw);
			return true;
		}
		case RowFormat::SHIFT_JIS: {
			std::size_t size = 0;
			std::string sjis;
			if (!readStringSize(format, reader, size) || !reader.read(sjis, size)) return false;
			if (format.dynamicLength == 0) sjis.resize(std::strlen(sjis.c_str()));
			out = shiftJisToUtf8(sjis);
			return true;
		}
		case RowFormat::UNICODE: {
			std::size_t size = 0;
			std::string utf16;
			if (!readStringSize(format, reader, size) || !reader.read(utf16, size)) return false;
			if (format.dynamicLength == 0) {
				std::size_t end = 0;
				while (end + 1 < utf16.size() && (utf16[end] != 0 || utf16[end + 1] != 0)) end += 2;
				utf16.resize(end);
			}
			out = utf16ToUtf8(utf16);
			return true;
		}
		case RowFormat::HEX: {
			std::size_t size = 0;
			std::string bytes;
			if (!readStringSize(format, reader, size) || !reader.read(bytes, size)) return false;
			out = toHex(bytes);
			return true;
		}
		default: return false;
		}
	}

	void writeNumber(std::vector<char>& out, uint64_t value, std::size_t bytes) {
		for (std::size_t i = 0; i < bytes; ++i) out.push_back(char((value >> (i * 8)) & 0xFF));
	}

	void writeString(const RowFormat& format, std::vector<char>& out, std::string bytes) {
		if (format.dynamicLength > 0) {
			writeNumber(out, bytes.size(), format.dynamicLength);
			out.insert(out.end(), bytes.begin(), bytes.end());
			return;
		}
		bytes.resize(format.byteSize, 0);
		out.insert(out.end(), bytes.begin(), bytes.end());
	}

	void writeCell(const RowFormat& format, std::vector<char>& out, const std::string& value) {
		switch (format.type) {
		case RowFormat::ZERO: out.insert(out.end(), format.byteSize, 0); break;
		case RowFormat::INT: {
			int64_t number = 0;
			std::from_chars(value.data(), value.data() + value.size(), number);
			writeNumber(out, uint64_t(number), numberByteSize(format));
		}break;
		case RowFormat::UINT: {
			uint64_t number = 0;
			std::from_chars(value.data(), value.data() + value.size(), number);
			writeNumber(out, number, numberByteSize(format));
		}break;
		case RowFormat::SHIFT_JIS: {
			std::string sjis = utf8ToShiftJis(value, false);
			if (format.dynamicLength == 0) clampShiftJis(sjis, format.byteSize);
			writeString(format, out, std::move(sjis));
		}break;
		case RowFormat::UNICODE: {
			std::string utf16 = utf8ToUtf16(value);
			if (format.dynamicLength == 0 && utf16.size() > format.byteSize) utf16.resize(format.byteSize & ~1u);
			writeString(format, out, std::move(utf16));
		}break;
		case RowFormat::HEX: {
			std::string bytes;
			fromHex(value, bytes);
			writeString(format, out, std::move(bytes));
		}break;
		default: break;
		}
	}

	std::string csvEscape(const std::string& value) {
		if (value.find_first_of(",\"\n\r") == std::string::npos) return value;
		std::string out = "\"";
		for (char c : value) {
			if (c == '"') out.push_back('"');
			out.push_back(c);
		}
		out.push_back('"');
		return out;
	}
}

PG_Dir PG_DisaDataTableModel::getDefDir() {
	return PG_Dir(PG_ResourceManager::resourceLocation().getDir() + "/script/disa/defFiles");
}

PG_File PG_DisaDataTableModel::findDefFile(const PG_File& dataFile) {
	std::string dataName = dataFile.getName();
	PG_StringUtil::toUpper(dataName);

	PG_File found;
	std::size_t foundLength = 0;
	for (const PG_File& defFile : getDefDir().files()) {
		if (!PG_StringUtil::isEqualCaseless(defFile.getFileExtension(), "DEF")) continue;
		std::string defName = defFile.getName();
		PG_StringUtil::toUpper(defName);
		if (defName.size() > foundLength && dataName.starts_with(defName)) {
			found = defFile;
			foundLength = defName.size();
		}
	}
	return found;
}

bool PG_DisaDataTableModel::open(const PG_File& file, std::string& errorOut) {
	const PG_File defFile = findDefFile(file);
	if (defFile.isEmpty()) {
		errorOut = "There is no definition for \"" + file.getFile() + "\". The file name must start with the name of a definition in \""
			+ getDefDir().getDir() + "\"!";
		return false;
	}

	PG_Def def;
	{
		PG::STREAM::InByteFile defReader(defFile);
		if (!defReader.isOpen()) {
			errorOut = "Failed to open definition \"" + defFile.getPath() + "\"!";
			return false;
		}
		try {
			def = PG_Def::parse(&defReader);
		}
		catch (const std::exception& e) {
			errorOut = "Failed to parse definition \"" + defFile.getFile() + "\": " + e.what();
			return false;
		}
	}
	if (def.header.empty() || def.formats.empty()) {
		errorOut = "The definition \"" + defFile.getFile() + "\" has no columns or no row format!";
		return false;
	}

	std::vector<char> data;
	{
		std::ifstream stream(file.getPath(), std::ios::binary);
		if (!stream) {
			errorOut = "Failed to open \"" + file.getPath() + "\"!";
			return false;
		}
		data.assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
	}

	ByteReader reader{ data };
	std::vector<std::string> headerValues;
	std::size_t rowCount = 0;
	bool hasRowCount = false;
	for (const HeaderFormat& format : def.headerFormats) {
		std::string value;
		if (!readCell(format.format, reader, value)) {
			errorOut = "The file is too small for the header of \"" + defFile.getFile() + "\"!";
			return false;
		}
		if (format.headerType == HeaderFormat::ROW_SIZE && format.format.type != RowFormat::ZERO) {
			const std::size_t count = std::stoull(value);
			if (hasRowCount && count != rowCount) {
				errorOut = "The row counters in the header don't match (" + std::to_string(rowCount) + " != " + value + ")!";
				return false;
			}
			rowCount = count;
			hasRowCount = true;
		}
		headerValues.push_back(std::move(value));
	}

	if (rowCount > g_maxRows) {
		errorOut = "The file claims to have " + std::to_string(rowCount) + " rows, that is too many! Is it the right definition?";
		return false;
	}

	std::vector<Row> rows(rowCount);
	for (Row& row : rows) {
		for (const RowFormat& format : def.formats) {
			std::string value;
			if (!readCell(format, reader, value)) {
				errorOut = "The file ended before all " + std::to_string(rowCount) + " rows were read! Is it the right definition?";
				return false;
			}
			if (format.type != RowFormat::ZERO) row.push_back(std::move(value));
		}
	}

	//the column header[a] shows the a-th none zero format
	for (std::size_t i = 0, a = 0; i < def.formats.size() && a < def.header.size(); ++i) {
		if (def.formats[i].type == RowFormat::ZERO) continue;
		def.header[a++].formatIndex = int(i);
	}

	if (!m_rows.empty()) {
		const std::size_t oldSize = m_rows.size();
		m_rows.clear();
		emit itemsRemoved(0, oldSize - 1);
	}
	m_def = std::move(def);
	m_headerValues = std::move(headerValues);
	m_rows = std::move(rows);
	m_file = file;
	m_modified = false;

	if (!m_rows.empty()) emit itemsAdded(0, m_rows.size() - 1);
	emit sizeChanged();
	emit fileChanged();
	emit modifiedChanged();
	return true;
}

bool PG_DisaDataTableModel::save(const PG_File& file) {
	if (!isOpen() || file.isEmpty()) return false;

	std::vector<char> data;
	for (std::size_t i = 0; i < m_def.headerFormats.size(); ++i) {
		const HeaderFormat& format = m_def.headerFormats[i];
		const bool isRowSize = format.headerType == HeaderFormat::ROW_SIZE && format.format.type != RowFormat::ZERO;
		writeCell(format.format, data, isRowSize ? std::to_string(m_rows.size()) : m_headerValues[i]);
	}

	for (const Row& row : m_rows) {
		std::size_t cell = 0;
		for (const RowFormat& format : m_def.formats) {
			if (format.type == RowFormat::ZERO) writeCell(format, data, {});
			else writeCell(format, data, row[cell++]);
		}
	}

	//the old editor did end every file like this
	data.insert(data.end(), 16, 0);

	std::ofstream stream(file.getPath(), std::ios::binary | std::ios::trunc);
	if (!stream) return false;
	stream.write(data.data(), data.size());
	if (!stream) return false;

	m_file = file;
	setModified(false);
	emit fileChanged();
	return true;
}

bool PG_DisaDataTableModel::exportCSV(const PG_File& file) const {
	if (!isOpen() || file.isEmpty()) return false;
	std::ofstream stream(file.getPath(), std::ios::binary | std::ios::trunc);
	if (!stream) return false;

	for (std::size_t c = 0; c < getColumnCount(); ++c) {
		if (c) stream << ',';
		stream << csvEscape(getColumnName(c));
	}
	stream << '\n';
	for (const Row& row : m_rows) {
		for (std::size_t c = 0; c < getColumnCount(); ++c) {
			if (c) stream << ',';
			if (c < row.size()) stream << csvEscape(row[c]);
		}
		stream << '\n';
	}
	return bool(stream);
}

std::string PG_DisaDataTableModel::getColumnName(std::size_t column) const {
	if (column >= m_def.header.size()) return {};
	std::string name = m_def.header[column].name;
	//PG_Def marks names that start with '$' with 0x80, so they don't collide with column ranges
	if (!name.empty() && (unsigned char)name.front() == 0x80) name.front() = '$';
	return name;
}

float PG_DisaDataTableModel::getColumnWidth(std::size_t column) const {
	constexpr float charWidth = 9.f; //roughly, at the font size of the table
	const float nameWidth = getColumnName(column).size() * charWidth + 20.f;
	const RowFormat* format = getFormat(column);
	if (!format) return nameWidth;

	float dataWidth = 60.f;
	switch (format->type) {
	case RowFormat::SHIFT_JIS:
	case RowFormat::UNICODE:
		dataWidth = format->dynamicLength ? 300.f : glm::clamp(format->byteSize * 7.f, 80.f, 600.f);
		break;
	case RowFormat::HEX:
		dataWidth = format->dynamicLength ? 300.f : glm::clamp(format->byteSize * 2.f * charWidth, 60.f, 600.f);
		break;
	case RowFormat::INT:
	case RowFormat::UINT:
		dataWidth = numberByteSize(*format) >= 4 ? 110.f : 60.f;
		break;
	default: break;
	}
	return std::max(nameWidth, dataWidth);
}

void PG_DisaDataTableModel::insertRow(std::size_t at) {
	if (!isOpen() || m_rows.size() >= g_maxRows) return;
	at = std::min(at, m_rows.size());

	Row row;
	if (m_rows.empty()) {
		for (std::size_t c = 0; c < getColumnCount(); ++c) {
			const RowFormat* format = getFormat(c);
			row.push_back(format && (format->type == RowFormat::INT || format->type == RowFormat::UINT) ? "0" : "");
		}
	}
	else row = m_rows[std::min(at, m_rows.size() - 1)];

	m_rows.insert(m_rows.begin() + at, std::move(row));
	emit itemsAdded(at, at);
	if (at + 1 < m_rows.size()) {
		shiftIndexColumns(at + 1, 1);
		emit itemsChanged(at + 1, m_rows.size() - 1, { PG_Identity_Location, PG_Identity_Text });
	}
	emit sizeChanged();
	setModified(true);
}

void PG_DisaDataTableModel::removeRow(std::size_t at) {
	if (at >= m_rows.size()) return;
	m_rows.erase(m_rows.begin() + at);
	emit itemsRemoved(at, at);
	if (at < m_rows.size()) {
		shiftIndexColumns(at, -1);
		emit itemsChanged(at, m_rows.size() - 1, { PG_Identity_Location, PG_Identity_Text });
	}
	emit sizeChanged();
	setModified(true);
}

void PG_DisaDataTableModel::moveRow(std::size_t from, std::size_t to) {
	if (from >= m_rows.size() || to >= m_rows.size() || from == to) return;
	std::swap(m_rows[from], m_rows[to]);
	for (std::size_t c = 0; c < m_def.header.size() && c < m_rows[from].size(); ++c) {
		if (m_def.header[c].type == ColumnFormat::INDEX) std::swap(m_rows[from][c], m_rows[to][c]);
	}
	emit itemsChanged(std::min(from, to), std::max(from, to), { PG_Identity_Item, PG_Identity_Text });
	setModified(true);
}

std::size_t PG_DisaDataTableModel::sizeOfDimension(std::size_t dim) const {
	switch (dim) {
	case 0: return m_rows.size();
	case 1: return m_def.header.size();
	default: return 0;
	}
}

bool PG_DisaDataTableModel::isIndexValid(const PG_Index& index) const {
	if (index.size() == 0 || index.size() > 2) return false;
	const auto row = index.get(0);
	if (row < 0 || row >= int(m_rows.size())) return false;
	if (index.size() == 1) return true;
	const auto column = index.get(1);
	return column >= 0 && column < int(m_def.header.size());
}

PG_Variety PG_DisaDataTableModel::getData(PG_Index index, Identity identity) const {
	if (!isIndexValid(index) || index.size() < 2) return PG_Variety();
	const std::size_t row = index.get(0);
	const std::size_t column = index.get(1);

	switch (identity) {
	case PG_Identity_Location: return PG_Variety(getLocation(index));
	case PG_Identity_Text:
	case PG_Identity_Edit:
		return PG_Variety(column < m_rows[row].size() ? m_rows[row][column] : std::string());
	case PG_Identity_Tooltip: return PG_Variety(getColumnName(column));
	case Identity_Editable: return PG_Variety(isEditable(column));
	case Identity_Background: return PG_Variety(getBackground(row, column));
	default: return PG_Variety();
	}
}

void PG_DisaDataTableModel::setData(PG_Index index, const PG_Variety& data, Identity identity) {
	if (identity != PG_Identity_Text && identity != PG_Identity_Edit) return;
	if (!isIndexValid(index) || index.size() < 2) return;
	const std::size_t row = index.get(0);
	const std::size_t column = index.get(1);
	if (!isEditable(column) || column >= m_rows[row].size()) return;

	std::string value = data.convert<std::string>();
	PG_StringUtil::trim(value);
	const bool valid = validate(*getFormat(column), value);
	if (valid && value != m_rows[row][column]) {
		m_rows[row][column] = std::move(value);
		setModified(true);
	}
	//also when rejected, so the view shows the old value again
	emit itemsChanged(row, row, { PG_Identity_Text });
}

const RowFormat* PG_DisaDataTableModel::getFormat(std::size_t column) const {
	if (column >= m_def.header.size()) return nullptr;
	const int formatIndex = m_def.header[column].formatIndex;
	if (formatIndex < 0 || formatIndex >= int(m_def.formats.size())) return nullptr;
	return &m_def.formats[formatIndex];
}

bool PG_DisaDataTableModel::isEditable(std::size_t column) const {
	return getFormat(column) && m_def.header[column].flag != ColumnFormat::NOEDIT;
}

glm::vec4 PG_DisaDataTableModel::getBackground(std::size_t row, std::size_t column) const {
	const ColumnFormat& col = m_def.header[column];
	const RowFormat* format = getFormat(column);
	if (!col.hasColor || !format || (format->type != RowFormat::INT && format->type != RowFormat::UINT)) return glm::vec4(0.f);

	constexpr float alpha = 0.35f; //the colors are meant for a white table, toned down for the dark one
	const glm::vec3 minColor = glm::vec3(col.colorMinBackground) / 255.f;
	const glm::vec3 maxColor = glm::vec3(col.colorMaxBackground) / 255.f;
	if (col.colorMin == col.colorMax) return glm::vec4(minColor, alpha);

	long long value = 0;
	const std::string& text = m_rows[row][column];
	std::from_chars(text.data(), text.data() + text.size(), value);
	const float t = glm::clamp(float(value - col.colorMin) / float(col.colorMax - col.colorMin), 0.f, 1.f);
	return glm::vec4(glm::mix(minColor, maxColor, t), alpha);
}

bool PG_DisaDataTableModel::validate(const RowFormat& format, std::string& value) const {
	switch (format.type) {
	case RowFormat::INT: {
		long long number = 0;
		auto result = std::from_chars(value.data(), value.data() + value.size(), number);
		if (result.ec != std::errc() || result.ptr != value.data() + value.size()) return false;
		const std::size_t bits = numberByteSize(format) * 8;
		if (bits < 64) {
			const long long max = (1ll << (bits - 1)) - 1;
			if (number > max || number < -max - 1) return false;
		}
		value = std::to_string(number);
		return true;
	}
	case RowFormat::UINT: {
		unsigned long long number = 0;
		auto result = std::from_chars(value.data(), value.data() + value.size(), number);
		if (result.ec != std::errc() || result.ptr != value.data() + value.size()) return false;
		const std::size_t bits = numberByteSize(format) * 8;
		if (bits < 64 && number > (1ull << bits) - 1) return false;
		value = std::to_string(number);
		return true;
	}
	case RowFormat::SHIFT_JIS: {
		if (format.dynamicLength) return true;
		std::string sjis = utf8ToShiftJis(value, false);
		if (sjis.size() <= format.byteSize) return true;
		clampShiftJis(sjis, format.byteSize);
		value = shiftJisToUtf8(sjis);
		return true;
	}
	case RowFormat::UNICODE: {
		if (format.dynamicLength) return true;
		std::string utf16 = utf8ToUtf16(value);
		if (utf16.size() <= format.byteSize) return true;
		utf16.resize(format.byteSize & ~1u);
		value = utf16ToUtf8(utf16);
		return true;
	}
	case RowFormat::HEX: {
		PG_StringUtil::toLower(value);
		std::string bytes;
		if (!fromHex(value, bytes)) return false;
		return format.dynamicLength || bytes.size() <= format.byteSize;
	}
	default: return false;
	}
}

void PG_DisaDataTableModel::shiftIndexColumns(std::size_t startRow, long long amount) {
	for (std::size_t c = 0; c < m_def.header.size(); ++c) {
		if (m_def.header[c].type != ColumnFormat::INDEX) continue;
		for (std::size_t r = startRow; r < m_rows.size(); ++r) {
			if (c >= m_rows[r].size()) continue;
			long long value = 0;
			std::from_chars(m_rows[r][c].data(), m_rows[r][c].data() + m_rows[r][c].size(), value);
			m_rows[r][c] = std::to_string(value + amount);
		}
	}
}

void PG_DisaDataTableModel::setModified(bool modified) {
	if (m_modified == modified) return;
	m_modified = modified;
	emit modifiedChanged();
}
