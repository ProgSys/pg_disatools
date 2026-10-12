#include "PG_DisaCursorNode.h"

#include <Base/Events/PG_TickEvent.h>
#include <Scene/PG_ModelNode.h>
#include <Disa/PG_DisaMapModel.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/map/PG_DisaActorModel.h>
#include <Disa/map/PG_DisaTileViewNode.h>
#include <Disa/map/PG_DisaActorEditSelectionNode.h>

void PG_DisaCursorNode::init() {
	PG_ActorNode::init();

	m_marker = PG_new<PG_ModelNode>("N_CursorMarker", this);
	m_pointer = PG_new<PG_DisaActorEditSelectionNode>("N_CursorPointer", this);
	m_pointer->setHovered(true);
}

bool PG_DisaCursorNode::event(PG_Event* e) {
	if (e->getType() == PG_Event_Tick && isMoving()) {
		constexpr double millisecondsPerFrame = 1000.0 / 60.0;
		const float frames = std::min(m_framesLeft, static_cast<float>(static_cast<PG_TickEvent*>(e)->getDeltaTime() / millisecondsPerFrame));
		m_framesLeft -= frames;
		setTranslation(m_framesLeft > 0.f ? getTranslation() + m_step * frames : m_target);
		m_marker->setTranslation(glm::vec3(m_tile.x, 0.f, m_tile.y) - getTranslation());
	}
	return PG_ActorNode::event(e);
}

void PG_DisaCursorNode::moveToTile(glm::ivec2 tile, float frames) {
	glm::vec3 target = PG_DisaActorModel::tileCenter(tile);
	target.y = groundHeight(getMapModel(), tile);

	const bool tileChange = tile != m_tile;
	m_tile = tile;
	translateTo(target, frames);
	if (tileChange) {
		updateMarker();
		emit tileChanged();
	}
}

void PG_DisaCursorNode::translateTo(const glm::vec3& target, float frames) {
	m_target = target;
	if (frames <= 0.f) {
		m_framesLeft = 0.f;
		m_step = glm::vec3(0.f);
		setTranslation(target);
	}
	else {
		m_framesLeft = frames;
		m_step = (target - getTranslation()) / frames;
	}
	m_marker->setTranslation(glm::vec3(m_tile.x, 0.f, m_tile.y) - getTranslation());
}

void PG_DisaCursorNode::setPointerVisible(bool visible) {
	m_pointer->setVisible(visible);
}

void PG_DisaCursorNode::updateMarker() {
	if (auto* map = getMapModel()) PG_DisaTileViewNode::updateFloorSelectionGeometry(m_tile, map, m_marker, "M_DisaCombatCursorTile");
}

float PG_DisaCursorNode::groundHeight(PG_DisaMapModel* map, glm::ivec2 tile) {
	if (!map) return 0.f;
	if (auto location = map->getTiles()->getTileLocation(tile)) return location.getDataT<float>(PG_DisaMapTileModel::Identity_TopHeight);
	return 0.f;
}

PG_DisaMapModel* PG_DisaCursorNode::getMapModel() const {
	auto* view = findParent<PG_DisaMapViewNode>(4);
	return view ? view->getMapModel() : nullptr;
}
