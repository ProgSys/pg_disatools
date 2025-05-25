#include "PG_TableDAT.h"
#include <Stream/PG_StreamInByteFile.h>
#include <Disa/fileformats/PG_ShiftJisEncoder.h>
#include <Stream/PG_StreamInByteWindow.h>
#include <Util/PG_StringUtil.h>

using namespace PG_DefDefinition;

void PG_TableDAT::setDefFile(const PG_File& file) {
	m_defFile = file;
}

inline bool readFormat(const RowFormat& format, PG::STREAM::In& in, std::vector<PG_TebleDATColumn>& dataOut) {
	switch (format.type)
	{

	case RowFormat::INT: {
		switch (format.byteSize) {
		case 0:
		case 1:
			dataOut.emplace_back(RowFormat::INT, PG_Variety(in.readChar()));
		break;
		case 2:
		case 16:
			dataOut.emplace_back(RowFormat::INT, PG_Variety(in.readShort()));
		break;
		case 4:
		case 32:
			dataOut.emplace_back(RowFormat::INT, PG_Variety(in.readInt()));
		break;
		case 8:
		case 64:
		{
			dataOut.emplace_back(RowFormat::INT, PG_Variety(in.readLongLong()));
		}
		break;
		default:
		{
			OUT_ERROR("(Int) Invalid byteSize: " << format.byteSize);
			dataOut.emplace_back(RowFormat::INT, PG_Variety(in.readChar()));
		}
		break;
		}
	}break;
	case RowFormat::UINT: {
		switch (format.byteSize) {
		case 0:
		case 1:
			dataOut.emplace_back(RowFormat::INT, PG_Variety(in.readUnsignedChar()));
		break;
		case 2:
		case 16:
			dataOut.emplace_back(RowFormat::INT, PG_Variety(in.readUnsignedShort()));
		break;
		case 4:
		case 32:
			dataOut.emplace_back(RowFormat::INT, PG_Variety(in.readUnsignedInt()));
		break;
		case 8:
		case 64:
			dataOut.emplace_back(RowFormat::INT, PG_Variety(in.readUnsignedLongLong()));
		break;
		default:
		{
			OUT_ERROR("(UInt) Invalid byteSize: " << format.byteSize);
			dataOut.emplace_back(RowFormat::INT, PG_Variety(in.readUnsignedChar()));
		}
		break;
		}
	}break;
	case RowFormat::ZERO: {
		if (format.byteSize > 0)
			in.skip(format.byteSize);
		return false;
	}break;
	case RowFormat::SHIFT_JIS: {
		int size = 0;
		if (format.dynamicLength > 0) {
			char* c = (char*)&size;
			in.read(c, format.dynamicLength);
		}
		else {
			size = format.byteSize;
		}

		std::string shiftJis = in.readString(size);

		//crop string if not dynamic
		if (format.dynamicLength <= 0) {
			int find_zero = 0;
			for (; find_zero < size; find_zero++)
				if (shiftJis[find_zero] == 0) break;
			size = std::min(size, find_zero);
			shiftJis.erase(shiftJis.begin() + size, shiftJis.end());
		}
		dataOut.emplace_back(RowFormat::SHIFT_JIS, PG_Variety(shiftJisToUtf8(shiftJis)));
	} break;
	case RowFormat::HEX:
	case RowFormat::UNICODE: {
		int size = 0;
		if (format.dynamicLength > 0) {
			char* c = (char*)&size;
			in.read(c, format.dynamicLength);
		}
		else {
			size = format.byteSize;
		}

		std::string string = in.readString(size);

		//crop string if not dynamic
		if (format.dynamicLength <= 0) {
			int find_zero = 0;
			for (; find_zero + 1 < format.byteSize; find_zero += 2)
				if (string[find_zero] == 0 && string[find_zero + 1] == 0) break;
			size = std::min(size, find_zero);
		}

		if (format.type == RowFormat::HEX) {
			//TODO
		}


		dataOut.emplace_back(format.type, string);
	} break;

	default: assert(false); break;
	}
	return true;
}

bool PG_TableDAT::open(std::shared_ptr<PG::STREAM::In> reader) {
	assert(reader);
	if (m_defFile.isEmpty()) return FAILURE;
	
	{
		PG::STREAM::InByteFile defReader(m_defFile);
		if (!defReader.isOpen()) {
			OUT_ERROR("Failed to open \"" << m_defFile << "\"!");
			return FAILURE;
		}
		m_dataStructure = PG_Def::parse(&defReader);
	}
	if (m_dataStructure.header.empty() || m_dataStructure.formats.empty()) {
		OUT_ERROR("Header or format is empty in \"" << m_defFile << "\"!");
		return FAILURE;
	}

	unsigned int size = 0;
	std::vector<PG_TebleDATColumn> headerData;
	for (const auto& format : m_dataStructure.headerFormats) {
		readFormat(format.format, *reader, headerData);
		if (format.headerType == HeaderFormat::ROW_SIZE && format.format.type != RowFormat::ZERO) {
			unsigned int testSize = headerData.back().data.convert<unsigned int>();
			if (size == 0) {
				size = testSize;
			}
			else if (size != testSize) {
				OUT_ERROR("Both file counters in table header must be the same! (" << size << " != " << testSize << ")");
				return FAILURE;
			}
		}
	}

	if (size > 9000) {
		OUT_ERROR("Size is too big! (" << size << ")");
		return FAILURE;
	}
	else if (size == 0) {
		return SUCCESS;
	}

	std::vector<PG_TebleDATRow> data(size);
	for (PG_TebleDATRow& entry: data) {
		for (const RowFormat& format : m_dataStructure.formats) {
			readFormat(format, *reader, entry.columns);
		}
	}
	set(std::move(data));

	return SUCCESS;
}

bool PG_TableDAT::save(std::shared_ptr<PG::STREAM::Out> writer) const {
	//TODO
	return FAILURE;
}

int PG_TableDAT::getHeaderIndex(const std::string& headerName) const {
	auto findIt = std::ranges::find_if(m_dataStructure.header, [&headerName](const ColumnFormat& f) {
		return PG_StringUtil::isEqualCaseless(headerName, f.name);
		});
	return findIt == m_dataStructure.header.end()? -1: std::distance(m_dataStructure.header.begin(), findIt);
}

std::shared_ptr<PG::STREAM::In> PG_TableDAT::createReader() const {
	//TODO
	if (auto entry = getParentEntry()) {
		return entry.createReader();
	}
	return nullptr;
}
std::shared_ptr<PG::STREAM::In> PG_TableDAT::createReaderOriginal() const {
	//TODO
	if (auto entry = getParentEntry()) {
		auto containerReader = entry.createReaderOriginal();
		if (!containerReader) return nullptr;
		return std::make_shared<PG::STREAM::PG_StreamInByteWindow>(containerReader, entry.getInfo().getOffset(), entry.getInfo().getSize());
	}
	return nullptr;
}