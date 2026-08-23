#include "PG_DisaModListModel.h"
#include <Base/PG_Timer.h>
#include <Disa/Models/PG_DisaModLoadOrderListModel.h>
#include <Disa/fileformats/PG_DisaFileContainer.h>

PG_DisaModListModel::~PG_DisaModListModel() {
	if (PG_Timer* timer = findFirstChild<PG_Timer>("timer")) {
		saveOrder(getItems());
	}

}

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
	case PG_Identity_PreviewImage: return m_items[index].previewImage();
	case Identity_Description: return m_items[index].description();
	case Identity_Author: return m_items[index].author();
	case Identity_Disabled: return m_items[index].disabled;
	}
	return PG_ListModel<PG_DisaModListModelItem>::getData(index, identity);
}

void PG_DisaModListModel::moveUp(PG_ListModelLocation loc) {
	if (!loc.isValid() || loc.getIndex() == 0) return;
	swap(loc.getIndex(), loc.getIndex() - 1);
	startSaveOrder();
}

void PG_DisaModListModel::moveDown(PG_ListModelLocation loc) {
	if (!loc.isValid() || loc.getIndex() == (size() - 1)) return;
	swap(loc.getIndex(), loc.getIndex() + 1);
	startSaveOrder();
}

void PG_DisaModListModel::disableMod(PG_ListModelLocation loc) {
	if (!loc.isValid()) return;
	bool& b = m_items[loc.getIndex()].disabled;
	b = !b;
	emit itemsChanged(loc.getIndex(), loc.getIndex(), { Identity_Disabled });
	startSaveOrder();
}

void PG_DisaModListModel::deleteMod(PG_ListModelLocation loc) {
	if (!loc.isValid()) return;
	m_items[loc.getIndex()].model->getFile().remove();
	remove(loc.getIndex());
	startSaveOrder();
}

void PG_DisaModListModel::loadAndApplyOrder(std::vector<PG_DisaModListModelItem>& items) {
	if (m_orderFile.isEmpty()) return;
	PG_DisaModLoadOrderListModel order;
	order.open(m_orderFile);
	
	if (order.empty()) {
		if (!items.empty()) {
			saveOrder(items);
		}
		return;
	}

	std::unordered_map<std::string, size_t> idOrder = order.getOrderMapping();
	//sort by order
	std::ranges::sort(items, {}, [&idOrder](const PG_DisaModListModelItem& i) {
		auto findIt = idOrder.find(i.id());
		return findIt != idOrder.end() ? findIt->second : std::numeric_limits<size_t>::max();
	});
	//apply disabled
	for (auto& item : items) {
		auto findIt = idOrder.find(item.id());
		if (findIt == idOrder.end()) continue;
		item.disabled = order.get(findIt->second).disabled;
	}
}

void PG_DisaModListModel::saveOrder(const std::vector<PG_DisaModListModelItem>& items) const {
	if (m_orderFile.isEmpty()) return;
	std::vector<PG_DisaModLoadOrderListModelItem> orderItems;
	orderItems.reserve(items.size());
	for (const auto& item : items) {
		orderItems.emplace_back(item.id(), item.title(), item.disabled);
	}
	PG_DisaModLoadOrderListModel order;
	order.set(std::move(orderItems));
	order.save(m_orderFile);
}

void PG_DisaModListModel::startSaveOrder() {
	PG_Timer* timer = findFirstChild<PG_Timer>("timer");
	if (!timer) {
		timer = PG_new<PG_Timer>("timer", this);
		timer->setDuration(std::chrono::seconds(7));
		connect(timer, &PG_Timer::passed, this, [this, timer]() {
			saveOrder(getItems());
			timer->setParent(nullptr);
			timer->deleteLater();
			});
	}
	timer->start(true);
}