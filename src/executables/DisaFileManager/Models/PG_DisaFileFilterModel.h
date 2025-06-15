#pragma once

#include <Scene/Model/PG_ListModelFilter.h>

class PG_DisaFileFilterModel : public PG_ListModelFilter {
	PG_OBJECT(PG_DisaFileFilterModel)
	PG_PARENT(PG_ListModelFilter)

	PG_PROPERTY(std::string searchText GET getSearchText SET setSearchText CHANGED searchStringChanged)
signals:
	void searchStringChanged();
public:
	using PG_ListModelFilter::PG_ListModelFilter;

	const std::string& getSearchText() const { return m_searchText; }
	void setSearchText(const std::string& str) { 
		if (m_searchText == str) return;
		m_searchText = str;
		invalidate();
		emit searchStringChanged();
	}

	bool filter(const PG_ListModelLocation& location) const override;
private:
	std::string m_searchText;
};