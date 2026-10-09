#include "PG_DisaDefListModel.h"

#include <Models/PG_DisaDataTableModel.h>
#include <Util/PG_StringUtil.h>

#include <algorithm>
#include <map>
#include <sstream>

void PG_DisaDefListModel::reload() {
	const PG_Dir defDir = PG_DisaDataTableModel::getDefDir();

	std::map<std::string, std::string> descriptions;
	{
		std::istringstream stream(PG_File(defDir.toFilePath("descriptions.txt")).readAllAsString());
		std::string line;
		while (std::getline(stream, line)) {
			const auto separator = line.find(':');
			if (separator == std::string::npos) continue;
			std::string name = line.substr(0, separator);
			std::string description = line.substr(separator + 1);
			PG_StringUtil::trim(name);
			PG_StringUtil::trim(description);
			PG_StringUtil::toUpper(name);
			descriptions[name] = description;
		}
	}

	std::vector<Row> rows;
	for (const PG_File& file : defDir.files()) {
		if (!PG_StringUtil::isEqualCaseless(file.getFileExtension(), "DEF")) continue;
		std::string key = file.getName();
		PG_StringUtil::toUpper(key);
		auto findIt = descriptions.find(key);
		rows.push_back(Row{ file.getFile(), findIt != descriptions.end() ? findIt->second : "No description available", file.getName() });
	}
	std::ranges::sort(rows, {}, &Row::fileName);

	if (!m_rows.empty()) {
		const std::size_t oldSize = m_rows.size();
		m_rows.clear();
		emit itemsRemoved(0, oldSize - 1);
	}
	m_rows = std::move(rows);
	if (!m_rows.empty()) emit itemsAdded(0, m_rows.size() - 1);
	emit sizeChanged();
}

std::string PG_DisaDefListModel::getColumnName(std::size_t column) const {
	switch (column) {
	case 0: return "Definition file";
	case 1: return "Description";
	case 2: return "File starting name";
	default: return {};
	}
}

float PG_DisaDefListModel::getColumnWidth(std::size_t column) const {
	switch (column) {
	case 0: return 180.f;
	case 1: return 650.f;
	default: return 180.f;
	}
}

std::size_t PG_DisaDefListModel::sizeOfDimension(std::size_t dim) const {
	switch (dim) {
	case 0: return m_rows.size();
	case 1: return getColumnCount();
	default: return 0;
	}
}

bool PG_DisaDefListModel::isIndexValid(const PG_Index& index) const {
	if (index.size() == 0 || index.size() > 2) return false;
	const auto row = index.get(0);
	if (row < 0 || row >= int(m_rows.size())) return false;
	if (index.size() == 1) return true;
	const auto column = index.get(1);
	return column >= 0 && column < int(getColumnCount());
}

PG_Variety PG_DisaDefListModel::getData(PG_Index index, Identity identity) const {
	if (!isIndexValid(index) || index.size() < 2) return PG_Variety();
	const Row& row = m_rows[index.get(0)];

	switch (identity) {
	case PG_Identity_Location: return PG_Variety(getLocation(index));
	case PG_Identity_Text:
	case PG_Identity_Tooltip:
		switch (index.get(1)) {
		case 0: return PG_Variety(row.fileName);
		case 1: return PG_Variety(row.description);
		default: return PG_Variety(row.startName);
		}
	case PG_DisaDataTableModel::Identity_Editable: return PG_Variety(false);
	case PG_DisaDataTableModel::Identity_Background: return PG_Variety(glm::vec4(0.f));
	default: return PG_Variety();
	}
}
