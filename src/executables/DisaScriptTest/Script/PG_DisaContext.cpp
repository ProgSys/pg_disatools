#include "PG_DisaContext.h"

#include <algorithm>
#include <cmath>

#include <Disa/PG_DisaMapModel.h>
#include <Util/Debug/PG_DebugOut.h>

#include "PG_DisaActorSlotModel.h"

void PG_DisaContext::init() {
	m_actors = PG_new<PG_DisaActorSlotModel>("PG_DisaActorSlotModel", this);
}

//==== Actors ====

PG_DisaActorModel* PG_DisaContext::resolveSlot(int slot) const {
	return m_actors ? m_actors->getSlot(slot) : nullptr;
}

std::vector<PG_DisaActorModel*> PG_DisaContext::resolveSlots(int slot) const {
	/* A group slot stands for everything taking part in the current skill, which needs a battle
	 * the tool does not run. Every actor a script spawned is the closest it can get. */
	if (slot == Slot_AllTargets) {
		if (!m_actors) return {};
		return std::vector<PG_DisaActorModel*>(m_actors->begin(), m_actors->end());
	}

	if (auto* actor = resolveSlot(slot)) return { actor };
	return {};
}

PG_DisaActorModel* PG_DisaContext::takeSlot(int slot) {
	return m_actors ? m_actors->takeSlot(slot) : nullptr;
}

glm::vec3 PG_DisaContext::getSlotOrigin(int slot) const {
	if (auto* actor = resolveSlot(slot)) return actor->getObjectTranslation();
	return glm::vec3(0.f); //everything else counts from the map origin, the teleporter included
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

void PG_DisaContext::snapToGround(PG_DisaActorModel* actor) const {
	if (!actor) return;
	glm::vec3 position = actor->getObjectTranslation();
	position.y = getGroundHeight(position);
	actor->setObjectTranslation(position);
}

//==== Animations ====

void PG_DisaContext::addAnimation(PG_DisaAnimationComponent* animation) {
	if (!animation) return;

	//the newer one wins, two animations of the same kind on one actor cannot both be right
	for (auto* running : m_animations) {
		if (running->getActor() == animation->getActor() && running->getChannel() == animation->getChannel()) {
			running->stop();
		}
	}
	animation->setParent(this);
	m_animations.push_back(animation);
}

bool PG_DisaContext::hasAnimation(const PG_DisaActorModel* actor) const {
	return std::ranges::any_of(m_animations, [actor](const PG_DisaAnimationComponent* animation) {
		return !animation->isFinished() && animation->getActor() == actor;
		});
}

bool PG_DisaContext::hasAnimation(const PG_DisaActorModel* actor, PG_DisaAnimationComponent::EChannel channel) const {
	return std::ranges::any_of(m_animations, [actor, channel](const PG_DisaAnimationComponent* animation) {
		return !animation->isFinished() && animation->getActor() == actor && animation->getChannel() == channel;
		});
}

void PG_DisaContext::stopAnimations(const PG_DisaActorModel* actor) {
	for (auto* animation : m_animations) {
		if (!actor || animation->getActor() == actor) animation->stop();
	}
}

void PG_DisaContext::tick(float deltaTicks) {
	if (m_animations.empty()) return;

	/* An animation may start another one, so only the ones that were already there run this tick,
	 * a new one starts on the next. */
	const std::size_t count = m_animations.size();
	for (std::size_t i = 0; i < count && i < m_animations.size(); ++i) {
		m_animations[i]->tick(deltaTicks);
	}

	std::erase_if(m_animations, [](PG_DisaAnimationComponent* animation) {
		if (!animation->isFinished()) return false;
		delete animation;
		return true;
		});
}

void PG_DisaContext::reset() {
	for (auto* animation : m_animations) delete animation;
	m_animations.clear();
	if (m_actors) m_actors->clearSlots();
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