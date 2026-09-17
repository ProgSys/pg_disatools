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
	if (getSlotItem(slot) == item) return;

	clearSlot(slot);
	if (!item) return;

	if (item->getLocation() < 0) push_back(item); //takes ownership and gives it its row
	m_slots[slot] = item;
}

PG_ListModelLocation PG_DisaActorListModel::takeActorSlot(int slot) {
	if (auto location = getSlot(slot)) return location;

	auto* actor = PG_new<PG_DisaSpriteModel>("PG_DisaSpriteModel", this);
	actor->setData(mpdfileActor{}); //a fresh one holds whatever was on the stack otherwise
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
	if (!item) return glm::vec3(0.f); //slot 201 and every number nobody filled count from the origin

	if (isItemSlot(slot)) return item->getObjectTranslation();

	/* A slot that is not an item is a named origin, and the game adds only x and z for those: an
	 * actor parented to the panel keeps the height its own command gave it. */
	const glm::vec3 position = item->getObjectTranslation();
	return glm::vec3(position.x, 0.f, position.z);
}

void PG_DisaActorListModel::resetMapSlots() {
	for (int slot = firstMapSlot; slot <= lastMapSlot; ++slot) m_slots.erase(slot);

	//the game fills its actor array from the file in order, so row N is slot N
	const int count = std::min(static_cast<int>(size()), lastMapSlot + 1);
	for (int row = 0; row < count; ++row) m_slots[row] = &getRef(row);
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

	//the panel sits in the middle of its tile, not on the corner the tile is counted from
	panel->setObjectTranslation(glm::vec3(pos.x + 0.5f, height, pos.y + 0.5f));
	panel->setVisible(true);
}
