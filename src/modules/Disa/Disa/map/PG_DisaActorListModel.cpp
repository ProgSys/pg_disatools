#include "PG_DisaActorListModel.h"

#include <algorithm>

#include <Disa/PG_DisaMapModel.h>
#include <Disa/map/PG_DisaEventTileModel.h>
#include <Disa/map/PG_DisaGeometryModel.h>
#include <Disa/map/PG_DisaSpriteModel.h>

PG_DisaActorModel* PG_DisaActorListModel::getSlotItem(int slot) const {
	auto findIt = m_slots.find(slot);
	return findIt == m_slots.end() ? nullptr : findIt->second.get();
}

PG_ListModelLocation PG_DisaActorListModel::getSlot(int slot) const {
	auto* item = getSlotItem(slot);
	if (!item || item->getLocation() < 0) return PG_ListModelLocation();
	return PG_ListModelLocation(const_cast<PG_DisaActorListModel*>(this), item->getLocation());
}

int PG_DisaActorListModel::getSlotOf(const PG_DisaActorModel* item) const {
	if (!item) return -1;
	for (const auto& [slot, line] : m_slots) {
		if (line.get() == item) return slot;
	}
	return -1;
}

void PG_DisaActorListModel::setSlot(int slot, PG_DisaActorModel* item) {
	auto* current = getSlotItem(slot);
	if (current == item) return;

	/* Swapping one item for another reuses the row it sits in. Removing first would move every row
	 * behind it up and the push back would move them all again. */
	if (current && current->getLocation() >= 0 && item && item->getLocation() < 0) {
		set(current->getLocation(), item); //deletes what was there and takes the new one over
		m_slots[slot] = item;
		return;
	}

	clearSlot(slot);
	if (!item) return;

	if (item->getLocation() < 0) push_back(item); //takes ownership and gives it its row
	m_slots[slot] = item;
}

PG_ListModelLocation PG_DisaActorListModel::takeActorSlot(int slot) {
	if (auto location = getSlot(slot)) return location;

	auto* actor = PG_new<PG_DisaSpriteModel>("PG_DisaSpriteModel", this);
	actor->setData(mpdfileActor::create());
	setSlot(slot, actor);
	return getSlot(slot);
}

void PG_DisaActorListModel::clearSlot(int slot) {
	auto findIt = m_slots.find(slot);
	if (findIt == m_slots.end()) return;

	auto* item = findIt->second.get();
	m_slots.erase(findIt);
	//removing moves every row behind it up, which is why the slots are kept beside the rows
	if (item && item->getLocation() >= 0) remove(item->getLocation());
}

void PG_DisaActorListModel::clearScriptSlots() {
	for (int slot = firstScriptSlot; slot <= lastScriptSlot; ++slot) clearSlot(slot);
}

glm::vec3 PG_DisaActorListModel::getSlotOrigin(int slot) const {
	auto* item = getSlotItem(slot);
	//slot 201 and every number nobody filled count from the origin
	if (!item) return mapOrigin;

	if (isItemSlot(slot)) return item->getObjectTranslation();

	/* A slot that is not an item is a named origin, and the game adds only x and z for those: an
	 * actor parented to the panel keeps the height its own command gave it. */
	const glm::vec3 position = item->getObjectTranslation();
	return glm::vec3(position.x, 0.f, position.z);
}

int PG_DisaActorListModel::findFreeMapSlot() const {
	for (int slot = firstMapSlot; slot <= lastMapSlot; ++slot) {
		if (isSlotFree(slot)) return slot;
	}
	return -1;
}

void PG_DisaActorListModel::resetMapSlots() {
	for (int slot = firstMapSlot; slot <= lastMapSlot; ++slot) m_slots.erase(slot);

	//the game fills its actor array from the file in order, so row N is slot N
	const int count = std::min(static_cast<int>(size()), lastMapSlot + 1);
	for (int row = 0; row < count; ++row) m_slots[row] = &getRef(row);
}

void PG_DisaActorListModel::setVisible(bool value) {
	if (m_visible == value) return;
	m_visible = value;
	emit visibleChanged(m_visible);
}

void PG_DisaActorListModel::setupTeleporter() {
	auto* map = findParent<PG_DisaMapModel>(5);
	if (!map) return;

	connect(map->getEvents(), &PG_ListModelBase::itemsAdded, this, &PG_DisaActorListModel::updateTeleporter);
	connect(map->getEvents(), &PG_ListModelBase::itemsRemoved, this, &PG_DisaActorListModel::updateTeleporter);
	connect(map->getEvents(), &PG_ListModelBase::itemsChanged, this,
		[this](unsigned int, unsigned int, const PG_IdentitySet& identities) {
			if (contains<PG_DisaEventTileModel::Identity_Value, PG_DisaEventTileModel::Identity_TilePosition,
				PG_Identity_Location, PG_Identity_Item>(identities)) {
				updateTeleporter();
			}
		});

	connect(map->getTiles(), &PG_DisaMapTileListModel::tilesGeometryChanged, this,
		[this](const std::unordered_set<glm::ivec2, PG_iVec2Hasher>& tiles) {
			auto* panel = getSlotItem(slotTeleporter);
			if (!panel) return;

			//the mesh hangs over the tile to the west of the panel, so that one counts too
			const glm::ivec2 tile = panel->getTilePosition();
			if (tiles.contains(tile) || tiles.contains(tile - glm::ivec2(1, 0))) updateTeleporter();
		}, PG_ConnectionType_Single);

	updateTeleporter();
}

void PG_DisaActorListModel::updateTeleporter() {
	postCallEvent(PG_Event_Behaviour_Singleton, &PG_DisaActorListModel::doUpdateTeleporter);
}

void PG_DisaActorListModel::doUpdateTeleporter() {
	auto* map = findParent<PG_DisaMapModel>(5);
	if (!map) return;

	auto* panel = getSlotItem(slotTeleporter);
	if (!panel) {
		auto* mesh = PG_new<PG_DisaGeometryModel>("PG_DisaTeleporter", this);
		mesh->setResource("R_DisaTeleporterBase");
		mesh->setGravity(0.f); //the panel sits where the tile is, it does not fall onto it
		setSlot(slotTeleporter, mesh);
		panel = mesh;
	}

	PG_DisaEventTileModel* found = nullptr;
	if (auto* events = map->getEvents()) {
		auto findIt = std::ranges::find(*events, 1, &PG_DisaEventTileModel::getValue);
		if (findIt != events->end()) found = *findIt;
	}
	if (!found) {
		panel->setVisible(false); //a map without a panel keeps the slot, it just shows nothing
		return;
	}

	const glm::ivec2 pos = found->getTilePosition();
	float height = 0.f;
	if (auto tile = map->getTiles()->getTileLocation(pos)) {
		height = tile.getDataT<float>(PG_DisaMapTileModel::Identity_RefHeight);
	}

	const glm::vec3 center = PG_DisaActorModel::tileCenter(pos);
	panel->setObjectTranslation(glm::vec3(center.x, height, center.z));
	panel->setVisible(true);
}


PG_DisaActorScriptViewModel* PG_DisaActorListModel::createScriptViewModel() {
	//no parent: the chain that asked for it owns it, and its actors go when it does
	auto* view = PG_new<PG_DisaActorScriptViewModel>("PG_DisaActorScriptViewModel");
	view->m_originModel = this;
	return view;
}

//==== PG_DisaActorScriptViewModel ====

PG_DisaActorScriptViewModel::~PG_DisaActorScriptViewModel() {
	resetScriptSlots();
}

int PG_DisaActorScriptViewModel::findRow(int slot) const {
	const auto& rows = getItems();
	auto findIt = std::ranges::find(rows, slot, &SlotActorPair::first);
	return findIt == rows.end() ? -1 : static_cast<int>(std::distance(rows.begin(), findIt));
}

PG_DisaActorModel* PG_DisaActorScriptViewModel::getSlotItem(int slot) const {
	if (!PG_DisaActorListModel::isScriptSlot(slot)) {
		return m_originModel ? m_originModel->getSlotItem(slot) : nullptr;
	}

	const int row = findRow(slot);
	return row < 0 ? nullptr : get(row).second.get();
}

PG_ListModelLocation PG_DisaActorScriptViewModel::getSlot(int slot) const {
	//the actor is a row of the list, which is where a view node reads its data from
	auto* item = getSlotItem(slot);
	if (!item || !m_originModel || item->getLocation() < 0) return PG_ListModelLocation();
	return PG_ListModelLocation(m_originModel.get(), item->getLocation());
}

int PG_DisaActorScriptViewModel::getSlotOf(const PG_DisaActorModel* item) const {
	if (!item) return -1;

	for (const auto& [slot, line] : getItems()) {
		if (line.get() == item) return slot;
	}
	return m_originModel ? m_originModel->getSlotOf(item) : -1;
}

void PG_DisaActorScriptViewModel::setSlot(int slot, PG_DisaActorModel* item) {
	if (!PG_DisaActorListModel::isScriptSlot(slot)) {
		if (m_originModel) m_originModel->setSlot(slot, item);
		return;
	}
	if (getSlotItem(slot) == item) return;

	const int row = findRow(slot);
	auto* current = row < 0 ? nullptr : get(row).second.get();

	if (!item || !m_originModel) {
		clearSlot(slot);
		return;
	}

	//the list owns the actor, this only says which slot of the chain it answers to
	if (current && current->getLocation() >= 0 && item->getLocation() < 0) {
		m_originModel->set(current->getLocation(), item); //deletes what was there
	}
	else {
		clearSlot(slot);
		if (item->getLocation() < 0) m_originModel->push_back(item);
	}

	const int at = findRow(slot);
	if (at < 0) push_back(SlotActorPair{ slot, item });
	else set(at, SlotActorPair{ slot, item });
}

PG_ListModelLocation PG_DisaActorScriptViewModel::takeActorSlot(int slot) {
	if (!PG_DisaActorListModel::isScriptSlot(slot)) {
		return m_originModel ? m_originModel->takeActorSlot(slot) : PG_ListModelLocation();
	}
	if (auto location = getSlot(slot)) return location;
	if (!m_originModel) return PG_ListModelLocation();

	auto* actor = PG_new<PG_DisaSpriteModel>("PG_DisaSpriteModel", m_originModel.get());
	actor->setData(mpdfileActor::create());
	setSlot(slot, actor);
	return getSlot(slot);
}

void PG_DisaActorScriptViewModel::clearSlot(int slot) {
	if (!PG_DisaActorListModel::isScriptSlot(slot)) {
		if (m_originModel) m_originModel->clearSlot(slot);
		return;
	}

	const int row = findRow(slot);
	if (row < 0) return;

	auto* item = get(row).second.get();
	remove(row); //only the row of this view, the actor belongs to the list

	if (m_originModel && item && item->getLocation() >= 0) m_originModel->remove(item->getLocation());
}

void PG_DisaActorScriptViewModel::resetScriptSlots() {
	while (!empty()) clearSlot(get(0).first);
}

glm::vec3 PG_DisaActorScriptViewModel::getSlotOrigin(int slot) const {
	if (!PG_DisaActorListModel::isScriptSlot(slot)) {
		return m_originModel ? m_originModel->getSlotOrigin(slot) : PG_DisaActorListModel::mapOrigin;
	}

	auto* item = getSlotItem(slot);
	return item ? item->getObjectTranslation() : PG_DisaActorListModel::mapOrigin;
}

std::vector<PG_DisaActorModel*> PG_DisaActorScriptViewModel::getActors() const {
	std::vector<PG_DisaActorModel*> out;
	out.reserve(size());
	for (const auto& [slot, line] : getItems()) {
		if (auto* item = line.get()) out.push_back(item);
	}
	return out;
}
