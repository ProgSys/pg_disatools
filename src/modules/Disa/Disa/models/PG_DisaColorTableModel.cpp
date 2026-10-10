#include "PG_DisaColorTableModel.h"

#include <Util/PG_ImageLoad.h>
#include <Util/Color/ColorConverters.h>


void PG_DisaColorTableModel::setTables(Tables* tables, std::size_t colorsPerTable) {
	const std::size_t oldSize = size();
	m_tables = tables;
	m_colorsPerTable = colorsPerTable;
	m_currentTable = 0;
	resetItems(oldSize);
	emit tablesChanged();
	emit currentTableChanged();
}

void PG_DisaColorTableModel::refresh() {
	const std::size_t oldSize = size();
	if (m_currentTable >= getTableCount()) m_currentTable = std::max(0, getTableCount() - 1);
	resetItems(oldSize);
	emit tablesChanged();
	emit currentTableChanged();
}

void PG_DisaColorTableModel::setTableCountEditable(bool editable) {
	if (m_tableCountEditable == editable) return;
	m_tableCountEditable = editable;
	emit tablesChanged();
}

int PG_DisaColorTableModel::getTableCount() const {
	return m_tables ? int(m_tables->size()) : 0;
}

void PG_DisaColorTableModel::setCurrentTable(int table) {
	table = glm::clamp(table, 0, std::max(0, getTableCount() - 1));
	if (table == m_currentTable) return;
	const std::size_t oldSize = size();
	m_currentTable = table;
	resetItems(oldSize);
	emit currentTableChanged();
}

glm::rgba PG_DisaColorTableModel::getColor(std::size_t index) const {
	if (index >= size()) return glm::rgba(0);
	return (*m_tables)[m_currentTable][index];
}

void PG_DisaColorTableModel::setColor(std::size_t index, const glm::rgba& color) {
	if (index >= size()) return;
	glm::rgba& target = (*m_tables)[m_currentTable][index];
	if (target == color) return;
	target = color;
	emit itemsChanged(index, index, { PG_Identity_Item, PG_Identity_Text });
	emit colorsModified();
}

void PG_DisaColorTableModel::insertTable(bool copyCurrent) {
	if (!m_tables || !m_tableCountEditable) return;
	std::vector<glm::rgba> table(m_colorsPerTable, glm::rgba(0));
	if (copyCurrent && m_currentTable < getTableCount()) table = (*m_tables)[m_currentTable];
	const int at = m_tables->empty() ? 0 : m_currentTable + 1;
	m_tables->insert(m_tables->begin() + at, std::move(table));

	const std::size_t oldSize = size();
	m_currentTable = at;
	resetItems(oldSize);
	emit tablesChanged();
	emit currentTableChanged();
	emit colorsModified();
}

void PG_DisaColorTableModel::removeCurrentTable() {
	if (!m_tables || !m_tableCountEditable || m_tables->size() <= 1) return; //an image with color tables needs at least one
	const std::size_t oldSize = size();
	m_tables->erase(m_tables->begin() + m_currentTable);
	m_currentTable = std::min(m_currentTable, getTableCount() - 1);
	resetItems(oldSize);
	emit tablesChanged();
	emit currentTableChanged();
	emit colorsModified();
}

bool PG_DisaColorTableModel::exportTable(const PG_File& file) const {
	if (empty() || file.isEmpty()) return false;
	const auto& table = (*m_tables)[m_currentTable];
	PG_RGBAImage image(unsigned(table.size()), 1);
	for (std::size_t i = 0; i < table.size(); ++i) image[i] = table[i];
	return PG_ImageLoad::save(file.getPath(), image);
}

bool PG_DisaColorTableModel::importTable(const PG_File& file) {
	if (empty() || file.isEmpty()) return false;
	PG_RGBAImage image;
	if (!PG_ImageLoad::load(file.getPath(), image) || image.empty()) return false;

	auto& table = (*m_tables)[m_currentTable];
	const std::size_t count = std::min<std::size_t>(table.size(), std::size_t(image.getWidth()) * image.getHeight());
	for (std::size_t i = 0; i < count; ++i) table[i] = image[i];
	emit itemsChanged(0, size() - 1, { PG_Identity_Item, PG_Identity_Text });
	emit colorsModified();
	return true;
}

glm::rgba PG_DisaColorTableModel::toRGBA(const glm::vec4& color) {
	return glm::rgba(glm::round(glm::clamp(color, 0.f, 1.f) * 255.f));
}

std::size_t PG_DisaColorTableModel::size() const {
	if (!m_tables || m_currentTable >= getTableCount()) return 0;
	return (*m_tables)[m_currentTable].size();
}

PG_Variety PG_DisaColorTableModel::getData(PG_Index index, Identity identity) const {
	if (!isIndexValid(index)) return PG_Variety();
	const glm::rgba color = getColor(index.get(0));
	switch (identity) {
	case PG_Identity_Location: return PG_Variety(getLocation(index));
	case PG_Identity_Item: return PG_Variety(color);
	case PG_Identity_Text:
	case PG_Identity_Edit: return PG_Variety(PG_Color::toHex(PG_Color::rgbaToF(color)));
	case PG_Identity_Tooltip: return PG_Variety(std::to_string(index.get(0)) + ": #" + PG_Color::toHex(PG_Color::rgbaToF(color)));
	default: return PG_Variety();
	}
}

void PG_DisaColorTableModel::setData(PG_Index index, const PG_Variety& data, Identity identity) {
	if (!isIndexValid(index)) return;
	if (identity == PG_Identity_Item && data.is<glm::rgba>()) {
		setColor(index.get(0), data.get<glm::rgba>());
	}
	else if (identity == PG_Identity_Text || identity == PG_Identity_Edit) {
		if (auto color = PG_Color::fromHex(data.convert<std::string>())) setColor(index.get(0), toRGBA(*color));
	}
}

void PG_DisaColorTableModel::resetItems(std::size_t oldSize) {
	if (oldSize && oldSize == size()) {
		emit itemsChanged(0, oldSize - 1, { PG_Identity_Item, PG_Identity_Text });
		return;
	}
	if (oldSize) emit itemsRemoved(0, oldSize - 1);
	if (const std::size_t newSize = size()) emit itemsAdded(0, newSize - 1);
	emit sizeChanged();
}
