#pragma once

#include <Scene/Model/PG_ListModelFilter.h>

class PG_CategoryFilterModel : public PG_ListModelFilter {
	PG_OBJECT(PG_CategoryFilterModel);
	PG_PARENT(PG_ListModelFilter);

	PG_PROPERTY(std::string searchText GET getSearchText SET setSearchText CHANGED searchTextChanged)
	PG_PROPERTY(PG_CategoryFilterModel::ECategory filterCategory GET getFilterCategory SET setFilterCategory CHANGED filterCategoryChanged)
signals:
	void searchTextChanged();
	void filterCategoryChanged();
public:
	enum ECategory {
		None,
		Dungeons,
		Image,
		Script,
		TalkScript
	};
	using PG_ListModelFilter::PG_ListModelFilter;

	const std::string& getSearchText() const { return m_searchText; };
	void setSearchText(const std::string& str);

	ECategory getFilterCategory() const { return m_filterCategory; };
	void setFilterCategory(ECategory category);

	bool filter(const PG_ListModelLocation& loc) const final;
receivers:
	void customRemove(std::size_t index);
private:
	std::string m_searchText;
	ECategory m_filterCategory = Dungeons;
};
