#include "PG_DisaContext.h"

#include <algorithm>
#include <cmath>

#include <Disa/PG_DisaMapModel.h>
#include <Util/Debug/PG_DebugOut.h>

#include <Disa/map/PG_DisaActorListModel.h>

PG_DisaActorListModel* PG_DisaContext::getMapActors() const {
	return m_mapModel ? m_mapModel->getActors() : nullptr;
}

void PG_DisaContext::setMapModel(PG_DisaMapModel* model) {
	m_mapModel = model;
}

//==== Actors ====

PG_DisaSpriteModel* PG_DisaContext::resolveSlot(int slot, const PG_DisaScriptSlots* scriptSlots) const {
	//64 to 95 belong to the chain that is running, every other number is the map's own
	if (scriptSlots && PG_DisaScriptSlots::isScriptSlot(slot)) return scriptSlots->getSlot(slot);
	auto* map = getMapActors();
	return map ? dynamic_cast<PG_DisaSpriteModel*>(map->getSlotItem(slot)) : nullptr;
}

std::vector<PG_DisaSpriteModel*> PG_DisaContext::resolveSlots(int slot, const PG_DisaScriptSlots* scriptSlots) const {
	/* A group slot stands for everything taking part in the current skill, which needs a battle
	 * the tool does not run. Every actor the chain spawned is the closest it can get. */
	if (slot == Slot_AllTargets) {
		if (scriptSlots) return scriptSlots->getActors();

		auto* map = getMapActors();
		if (!map) return {};

		//the list holds the base, only the ones drawn as a sprite are what a script means here
		std::vector<PG_DisaSpriteModel*> out;
		for (auto* item : *map) {
			if (auto* sprite = dynamic_cast<PG_DisaSpriteModel*>(item)) out.push_back(sprite);
		}
		return out;
	}

	if (auto* actor = resolveSlot(slot, scriptSlots)) return { actor };
	return {};
}

PG_DisaSpriteModel* PG_DisaContext::takeSlot(int slot, PG_DisaScriptSlots* scriptSlots) {
	if (scriptSlots && PG_DisaScriptSlots::isScriptSlot(slot)) return scriptSlots->takeActorSlot(slot);
	auto* map = getMapActors();
	if (!map) return nullptr;

	auto location = map->takeActorSlot(slot);
	return dynamic_cast<PG_DisaSpriteModel*>(location.getData(PG_Identity_Item).convert<PG_DisaActorModel*>());
}

glm::vec3 PG_DisaContext::getSlotOrigin(int slot, const PG_DisaScriptSlots* scriptSlots) const {
	//the chain's own actors are the only thing the map's table does not know about
	if (scriptSlots && PG_DisaScriptSlots::isScriptSlot(slot)) {
		auto* actor = scriptSlots->getSlot(slot);
		return actor ? actor->getObjectTranslation() : glm::vec3(0.f);
	}

	if (slot == Slot_EventPosition) {
		//the game counts from where the event was started, which needs a battle the tool does not run
		OUT_WARNING("Slot 210 is the position of a running event, the map origin is used instead.");
		return glm::vec3(0.f);
	}

	auto* map = getMapActors();
	return map ? map->getSlotOrigin(slot) : glm::vec3(0.f);
}

//==== Ground ====

float PG_DisaContext::getGroundHeight(const glm::vec3& position) const {
	if (!m_mapModel) return 0.f;

	//a position between two tiles belongs to the one it stands on, so it is floored not rounded
	const glm::ivec2 tile(static_cast<int>(std::floor(position.x)), static_cast<int>(std::floor(position.z)));
	auto location = m_mapModel->getTiles()->getTileLocation(tile);
	if (!location) return 0.f;
	return location.getDataT<float>(PG_DisaMapTileModel::Identity_TopHeight);
}

void PG_DisaContext::snapToGround(PG_DisaSpriteModel* actor) const {
	if (!actor) return;
	glm::vec3 position = actor->getObjectTranslation();
	position.y = getGroundHeight(position);
	actor->setObjectTranslation(position);
}

//==== Animations ====

PG_DisaAnimationComponent* PG_DisaContext::findAnimation(const PG_DisaSpriteModel* actor,
	PG_DisaAnimationComponent::EChannel channel) const {
	for (const auto& line : m_animations) {
		auto* animation = line.get();
		if (animation && !animation->isFinished() && animation->getActorModel() == actor
			&& animation->getChannel() == channel) {
			return animation;
		}
	}
	return nullptr;
}

PG_DisaAnimationComponent* PG_DisaContext::takeAnimation(PG_DisaSpriteModel* actor,
	PG_DisaAnimationComponent::EChannel channel, const std::string& name) {
	if (!actor) return nullptr;
	if (auto* running = findAnimation(actor, channel)) return running;

	//the actor owns it, so it goes away with the actor and this only holds a line to it
	auto* animation = PG_new<PG_DisaAnimationComponent>(name, actor);
	animation->setChannel(channel);
	m_animations.emplace_back(animation);
	return animation;
}

void PG_DisaContext::clearAnimation(PG_DisaSpriteModel* actor, PG_DisaAnimationComponent::EChannel channel) {
	delete findAnimation(actor, channel);
	std::erase_if(m_animations, [](const PG_Lifeline_T<PG_DisaAnimationComponent>& line) { return line.expired(); });
}

bool PG_DisaContext::hasAnimation(const PG_DisaSpriteModel* actor) const {
	return std::ranges::any_of(m_animations, [actor](const PG_Lifeline_T<PG_DisaAnimationComponent>& line) {
		auto* animation = line.get();
		return animation && !animation->isFinished() && animation->getActorModel() == actor;
		});
}

bool PG_DisaContext::hasAnimation(const PG_DisaSpriteModel* actor, PG_DisaAnimationComponent::EChannel channel) const {
	return findAnimation(actor, channel) != nullptr;
}

void PG_DisaContext::stopAnimations(const PG_DisaSpriteModel* actor) {
	for (const auto& line : m_animations) {
		auto* animation = line.get();
		if (animation && (!actor || animation->getActorModel() == actor)) animation->stop();
	}
}

void PG_DisaContext::tick(float deltaTicks) {
	if (m_animations.empty()) return;

	/* An animation may start another one, so only the ones that were already there run this tick,
	 * a new one starts on the next. */
	const std::size_t count = m_animations.size();
	for (std::size_t i = 0; i < count && i < m_animations.size(); ++i) {
		if (auto* animation = m_animations[i].get()) animation->tick(deltaTicks);
	}

	std::erase_if(m_animations, [](const PG_Lifeline_T<PG_DisaAnimationComponent>& line) {
		auto* animation = line.get();
		if (!animation) return true; //its actor already took it down
		if (!animation->isFinished()) return false;
		delete animation;
		return true;
		});
}

void PG_DisaContext::reset() {
	for (const auto& line : m_animations) delete line.get();
	m_animations.clear();

	/* What a chain spawned goes when the chain's PG_DisaScriptSlots does, which is when the
	 * executor drops its lanes. This only clears the map's own script range. */
	if (auto* map = getMapActors()) map->clearScriptSlots();
}

bool PG_DisaContext::getGameFlag(unsigned int flagIndex) const {
	const std::size_t byteIndex = flagIndex >> 3;
	if (byteIndex >= m_gameState.size()) return false;
	return (m_gameState[byteIndex] & (1 << (flagIndex & 7))) != 0;
}

void PG_DisaContext::setGameFlag(unsigned int flagIndex, bool value) {
	const std::size_t byteIndex = flagIndex >> 3;
	if (byteIndex >= m_gameState.size()) return;
	const unsigned char mask = 1 << (flagIndex & 7);
	if (value) m_gameState[byteIndex] |= mask;
	else m_gameState[byteIndex] &= ~mask;
	emit gameFlagChanged(flagIndex, value);
}