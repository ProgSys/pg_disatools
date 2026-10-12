#include "PG_DisaActorLogicComponentModel.h"

#include <algorithm>
#include <cmath>
#include <glm/gtc/constants.hpp>
#include <Base/Events/PG_TickEvent.h>
#include <Disa/PG_DisaMapModel.h>
#include <Disa/map/PG_DisaSpriteModel.h>
#include <Disa/game/PG_DisaCursorNode.h>

namespace {

constexpr float g_framesPerTile = 15.f; /// FUN_004475c0 walks a unit one tile in 15 frames

} //namespace

void PG_DisaActorLogicComponentModel::init() {
	if (auto* unit = getUnit()) {
		m_tile = unit->getTilePosition();
		m_stats = PG_DisaMoveRange::statsOf(*unit);
		m_movementLeft = m_stats.movement;
	}
}

bool PG_DisaActorLogicComponentModel::event(PG_Event* e) {
	if (e->getType() == PG_Event_Tick && isWalking()) {
		walk(static_cast<float>(static_cast<PG_TickEvent*>(e)->getDeltaTime()) * 60.f / 1000.f);
	}
	return PG_Object::event(e);
}

PG_DisaSpriteModel* PG_DisaActorLogicComponentModel::getUnit() const {
	return getParent<PG_DisaSpriteModel>();
}

bool PG_DisaActorLogicComponentModel::isMovable() const {
	auto* unit = getUnit();
	return unit && unit->getCharacterAlliance() == 0;
}

void PG_DisaActorLogicComponentModel::moveAlong(const std::vector<glm::ivec2>& path) {
	if (path.size() < 2) return;
	m_walk.insert(m_walk.end(), std::next(path.begin()), path.end());
	m_movementLeft -= static_cast<int>(path.size()) - 1;
	m_tile = path.back();
}

void PG_DisaActorLogicComponentModel::resetTurn() {
	m_movementLeft = m_stats.movement;
}

glm::vec3 PG_DisaActorLogicComponentModel::standPosition(PG_DisaMapModel* map, glm::ivec2 tile) {
	glm::vec3 position = PG_DisaActorModel::tileCenter(tile);
	position.y = PG_DisaCursorNode::groundHeight(map, tile);
	return position;
}

void PG_DisaActorLogicComponentModel::walk(float frames) {
	auto* unit = getUnit();
	if (!unit) return;
	auto* map = unit->findParent<PG_DisaMapModel>(5);

	while (frames > 0.f && !m_walk.empty()) {
		const glm::vec3 target = standPosition(map, m_walk.front());
		if (m_stepFrames == 0.f) startStep(target);

		const float used = std::min(frames, g_framesPerTile - m_stepFrames);
		m_stepFrames += used;
		frames -= used;

		const float progress = m_stepFrames / g_framesPerTile;
		glm::vec3 position = glm::mix(m_stepStart, target, progress);
		if (m_isStepJump) {
			//an arc that clears the higher of both tiles, like the gravity arc of FUN_0046c9b0
			const float height = std::abs(target.y - m_stepStart.y) * 0.5f + 0.2f;
			position.y += 4.f * height * progress * (1.f - progress);
		}
		unit->setObjectTranslation(position);

		if (m_stepFrames >= g_framesPerTile) {
			m_walk.pop_front();
			endStep();
		}
	}
	if (m_walk.empty()) unit->setAnimationSet(PG_DisaSpriteModel::IdleStanding);
}

void PG_DisaActorLogicComponentModel::startStep(const glm::vec3& target) {
	auto* unit = getUnit();
	m_stepStart = unit->getObjectTranslation();

	//0 degree looks along +z and -90 along +x, see PG_DisaActorModel::getRotation
	const glm::vec2 direction(target.x - m_stepStart.x, target.z - m_stepStart.z);
	if (direction != glm::vec2(0.f)) unit->setRotation(glm::degrees(std::atan2(-direction.x, direction.y)));

	//like FUN_004475c0 a step of more than half a tile up or down is a jump
	m_isStepJump = std::abs(target.y - m_stepStart.y) > 0.5f;
	if (m_isStepJump) {
		//the arc puts the height itself, gravity would pull the unit down early
		m_gravityBeforeJump = unit->getGravity();
		unit->setGravity(0.f);
	}
	unit->setAnimationSet(m_isStepJump ? PG_DisaSpriteModel::Jump : PG_DisaSpriteModel::Walking);
}

void PG_DisaActorLogicComponentModel::endStep() {
	if (m_isStepJump) getUnit()->setGravity(m_gravityBeforeJump);
	m_isStepJump = false;
	m_stepFrames = 0.f;
}
