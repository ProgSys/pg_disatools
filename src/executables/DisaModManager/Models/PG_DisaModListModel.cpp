#include "PG_DisaModListModel.h"

void PG_DisaModListModel::reload() {
	{ //clean up
		auto oldItems = std::move(m_items);
		emit itemsRemoved(0, oldItems.size() - 1);
		emit sizeChanged();
	}

	loadData();
}

PG_Variety PG_DisaModListModel::getData(PG_Index index, Identity identity) const {
	switch (identity) {
	case Identity_PreviewImage: return m_items[index].previewImage();
	case Identity_Description: return m_items[index].description();
	case Identity_Author: return m_items[index].author();
	case Identity_Order: return m_items[index].order;
	}
	return PG_ListModel<PG_DisaModListModelItem>::getData(index, identity);
}
