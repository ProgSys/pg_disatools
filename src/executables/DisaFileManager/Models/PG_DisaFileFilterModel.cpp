#include "PG_DisaFileFilterModel.h"
#include <Util/PG_File.h>
#include <Util/PG_StringUtil.h>
#include <algorithm>


bool PG_DisaFileFilterModel::filter(const PG_ListModelLocation& location) const {
	if (m_searchText.empty() && m_fileTypes.empty()) return false;

	const std::string str = location.getDataT<std::string>(PG_Identity_Text);
	if (!m_searchText.empty() && !PG_StringUtil::contains(str, m_searchText, false)) return true;
	if (m_fileTypes.empty()) return false;

	//the buttons carry the extension upper case, so ask for it in the same shape
	const std::string extension = PG_File::getFileExtensionFormated(str, false);
	return std::ranges::find(m_fileTypes, extension) == m_fileTypes.end();
}
