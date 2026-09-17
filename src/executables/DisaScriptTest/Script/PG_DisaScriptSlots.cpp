#include "PG_DisaScriptSlots.h"

PG_DisaSpriteModel* PG_DisaScriptSlots::getSlot(int slot) const {
	auto findIt = m_slots.find(slot);
	return findIt == m_slots.end() ? nullptr : findIt->second.get();
}

PG_DisaSpriteModel* PG_DisaScriptSlots::takeActorSlot(int slot) {
	if (auto* existing = getSlot(slot)) return existing;

	auto* actors = m_actors.get();
	if (!actors) return nullptr;

	auto* actor = PG_new<PG_DisaSpriteModel>("PG_DisaSpriteModel", actors);
	actor->setData(mpdfileActor{}); //a fresh one holds whatever was on the stack otherwise
	actors->push_back(actor); //takes ownership and gives it its row
	m_slots[slot] = actor;
	return actor;
}

void PG_DisaScriptSlots::clearSlot(int slot) {
	auto findIt = m_slots.find(slot);
	if (findIt == m_slots.end()) return;

	auto* actor = findIt->second.get();
	m_slots.erase(findIt);

	auto* actors = m_actors.get();
	if (actors && actor && actor->getLocation() >= 0) actors->remove(actor->getLocation());
}

void PG_DisaScriptSlots::clear() {
	auto slots = m_slots;
	m_slots.clear();

	auto* actors = m_actors.get();
	if (!actors) return;
	for (const auto& [slot, line] : slots) {
		auto* actor = line.get();
		if (actor && actor->getLocation() >= 0) actors->remove(actor->getLocation());
	}
}

std::vector<PG_DisaSpriteModel*> PG_DisaScriptSlots::getActors() const {
	std::vector<PG_DisaSpriteModel*> out;
	out.reserve(m_slots.size());
	for (const auto& [slot, line] : m_slots) {
		if (auto* actor = line.get()) out.push_back(actor);
	}
	return out;
}
