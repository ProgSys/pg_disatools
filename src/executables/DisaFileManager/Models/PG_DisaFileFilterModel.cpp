#include "PG_DisaFileFilterModel.h"
#include <Util/PG_StringUtil.h>


bool PG_DisaFileFilterModel::filter(const PG_ListModelLocation& location) const {
	if (m_searchText.empty()) return false;
	std::string str = location.getDataT<std::string>(Identity_Text);
	return !PG_StringUtil::contains(str, m_searchText, false);
}