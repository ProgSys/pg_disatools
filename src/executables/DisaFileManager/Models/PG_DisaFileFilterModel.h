#pragma once

#include <Scene/Model/PG_ListModelFilter.h>
#include <string>
#include <vector>

class PG_DisaFileFilterModel : public PG_ListModelFilter {
	PG_OBJECT(PG_DisaFileFilterModel)
	PG_PARENT(PG_ListModelFilter)

	PG_PROPERTY(std::string searchText GET getSearchText SET setSearchText CHANGED searchStringChanged)
	PG_PROPERTY(std::vector<std::string> fileTypes GET getFileTypes SET setFileTypes CHANGED fileTypesChanged)
signals:
	void searchStringChanged();
	void fileTypesChanged();
public:
	using PG_ListModelFilter::PG_ListModelFilter;

	const std::string& getSearchText() const { return m_searchText; }
	void setSearchText(const std::string& str) {
		if (m_searchText == str) return;
		m_searchText = str;
		invalidate();
		emit searchStringChanged();
	}

	/**
	* @brief The file extensions a row has to be one of, upper case and without the dot
	* @details Empty lets every type through, which is what no filter means.
	*/
	const std::vector<std::string>& getFileTypes() const { return m_fileTypes; }
	void setFileTypes(std::vector<std::string> types) {
		if (m_fileTypes == types) return;
		m_fileTypes = std::move(types);
		invalidate();
		emit fileTypesChanged();
	}

	bool filter(const PG_ListModelLocation& location) const override;
private:
	std::string m_searchText;
	std::vector<std::string> m_fileTypes;
};
