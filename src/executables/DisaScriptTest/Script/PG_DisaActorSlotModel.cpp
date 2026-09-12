#include "PG_DisaActorSlotModel.h"

#include <algorithm>

#include <Util/Debug/PG_DebugOut.h>

PG_DisaActorModel* PG_DisaActorSlotModel::getSlot(int slot) const {
	auto findIt = m_slots.find(slot);
	return findIt == m_slots.end() ? nullptr : findIt->second;
}

int PG_DisaActorSlotModel::getSlotOf(const PG_DisaActorModel* actor) const {
	auto findIt = std::ranges::find(m_slots, actor, [](const auto& pair) { return pair.second; });
	return findIt == m_slots.end() ? -1 : findIt->first;
}

PG_DisaActorModel* PG_DisaActorSlotModel::takeSlot(int slot) {
	if (auto* existing = getSlot(slot)) return existing;

	/*
	if (!isScriptSlot(slot)) {
		OUT_WARNING("Slot " << slot << " is not one a script may spawn into (" << firstSlot << " to " << lastSlot << ")!");
		return nullptr;
	}
	*/

	auto* actor = PG_new<PG_DisaActorModel>("PG_DisaActorModel");
	actor->setData(mpdfileActor{}); //a fresh one holds whatever was on the stack otherwise
	push_back(actor); //takes ownership and gives it its row
	m_slots[slot] = actor;
	return actor;
}

void PG_DisaActorSlotModel::clearSlot(int slot) {
	auto findIt = m_slots.find(slot);
	if (findIt == m_slots.end()) return;

	const int location = findIt->second->getLocation();
	m_slots.erase(findIt);
	if (location >= 0) remove(location); //deletes the actor and moves every row behind it up
}

void PG_DisaActorSlotModel::clearSlots() {
	m_slots.clear();
	clear();
}
