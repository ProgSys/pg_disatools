#include "PG_CategoryFilterModel.h"
#include <util/PG_StringUtil.h>
#include <Disa/fileformats/PG_DisaFileContainer.h>

void PG_CategoryFilterModel::setSearchText(const std::string& str) {
	if (m_searchText == str) return;
	m_searchText = str;
	invalidate();
}

void PG_CategoryFilterModel::setFilterCategory(ECategory category) {
	if (m_filterCategory == category) return;
	m_filterCategory = category;
	invalidate();
}

bool PG_CategoryFilterModel::filter(const PG_ListModelLocation& loc) const {
	if (m_filterCategory) {
		auto item = loc.getData(PG_Identity_Item);
		if (item.is<PG_DisaFileEntry>()) {
			auto type = item.get<PG_DisaFileEntry>().getInfo().getType();
			switch (m_filterCategory)
			{
			default:
			case PG_CategoryFilterModel::None: break;
			case PG_CategoryFilterModel::Dungeons: 
				if (type != PG_DisaFileInfo::MPD) return true;
			break;
			case PG_CategoryFilterModel::Image:
				if (type != PG_DisaFileInfo::PNG && type != PG_DisaFileInfo::TX2) return true;
			break;
			case PG_CategoryFilterModel::Script: 
				if (type != PG_DisaFileInfo::TALK_SCRIPT && type != PG_DisaFileInfo::SCRIPT) return true;
			break;
			case PG_CategoryFilterModel::TalkScript:
				if (type != PG_DisaFileInfo::TALK_SCRIPT) return true;
				break;
			}
		}
	}

	if(!m_searchText.empty()){
		auto name = loc.getDataT<std::string>(PG_Identity_Text);
		if (!PG_StringUtil::contains(name, m_searchText, false)) {
			return true;
		}
	}

	return false;
}

void PG_CategoryFilterModel::customRemove(std::size_t index) {
	if (!m_model || index >= m_mappingItems.size()) return;
	if (auto func = m_model->getFunction("void customRemove(std::size_t)")) {
		func.callArgs(m_model, std::size_t(m_mappingItems[index]));
		return;
	}
	remove(index);
}