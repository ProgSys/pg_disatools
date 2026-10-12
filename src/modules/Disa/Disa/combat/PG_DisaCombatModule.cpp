#include "PG_DisaCombatModule.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <Base/Events/PG_KeyEvent.h>
#include <Base/Events/PG_MouseButtonEvent.h>
#include <Base/Events/PG_MouseMotionEvent.h>
#include <Scene/PG_Scene.h>
#include <Scene/PG_ModelNode.h>
#include <Scene/Gui/PG_GuiItem.h>
#include <Disa/PG_DisaCamera.h>
#include <Disa/PG_DisaMapModel.h>
#include <Disa/PG_DisaMapScene.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/map/PG_DisaActorListModel.h>
#include <Disa/map/PG_DisaActorsViewNode.h>
#include <Disa/map/PG_DisaSpriteModel.h>
#include <Disa/map/PG_DisaTileViewNode.h>
#include <Disa/map/PG_DisaActorEditSelectionNode.h>
#include <Disa/map/controllers/PG_DisaEditorActorController.h>
#include <Disa/combat/PG_DisaCombatCursorNode.h>
#include <Disa/combat/PG_DisaMoveRange.h>

PG_DisaCombatModule::~PG_DisaCombatModule() {
	/* The scene may already be on its way out, so only what this module owns goes. The nodes are
	 * children of the scene and leave with it. */
	if (auto* actors = m_actors.get()) delete actors;
}

void PG_DisaCombatModule::start(PG_DisaMapViewNode* mapView) {
	if (m_running || !mapView || !mapView->getMapModel() || !getScene()) return;
	PG_Scene* scene = getScene();
	PG_DisaMapModel* map = mapView->getMapModel();
	m_mapView = mapView;

	m_tileMin = glm::ivec2(std::numeric_limits<int>::max());
	m_tileMax = glm::ivec2(std::numeric_limits<int>::min());
	for (const PG_DisaMapTileModel* tile : *map->getTiles()) {
		m_tileMin = glm::min(m_tileMin, tile->getTilePosition());
		m_tileMax = glm::max(m_tileMax, tile->getTilePosition());
	}
	if (m_tileMin.x > m_tileMax.x) m_tileMin = m_tileMax = glm::ivec2(0);

	PG_CameraNode* previous = scene->getActiveCamera();
	m_previousCamera = previous;
	glm::vec3 lookAt = glm::vec3(m_tileMin.x + m_tileMax.x, 0.f, m_tileMin.y + m_tileMax.y) * 0.5f;
	float orbit = -45.f;
	if (previous) {
		const PG_Ray view = previous->getRayViewDir();
		if (auto ground = view.intersectPlane(glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f))) lookAt = view.getPointOnRay(*ground);
		orbit = glm::degrees(std::atan2(-view.dir.z, view.dir.x));
	}

	auto* camera = PG_new<PG_DisaCamera>("N_CombatCamera", PG_CameraNode::Type_Perspective, scene->getRoot());
	camera->setSettings({
		.selectionMouseKey = 0,
		.panMouseKey = PG_BUTTON_MMASK,
		.turnMouseKey = 0,
		.cancelMouseKey = 0
	});
	m_camera = camera;
	//the game starts a battle 750 units away at a 30 degree tilt, see PG_DisaScriptTestModel::resetGameCamera
	//the game only looks along the diagonals, 45 degree plus quarter turns
	orbit = 45.f + 90.f * std::round((orbit - 45.f) / 90.f);
	camera->setOrbitAngles(-30.f, orbit);
	camera->setOrbitDistance(750.f / 12.f);
	scene->setActiveCamera(camera);
	camera->addEventFilter(this);

	m_wasActorsVisible = mapView->isActorsVisible();
	mapView->setActorsVisible(false);
	copyActors();

	auto* root = PG_new<PG_ActorNode>("N_Combat", mapView);
	m_root = root;
	m_actorView = PG_new<PG_DisaActorsViewNode>("PG_DisaCombatActorsViewNode", root);
	m_actorView->setModel(m_actors.get());
	m_rangeNode = PG_new<PG_ModelNode>("N_MoveRange", root);

	auto* cursor = PG_new<PG_DisaCombatCursorNode>("N_CombatCursor", root);
	m_cursor = cursor;
	connect(cursor, &PG_DisaCombatCursorNode::tileChanged, this, &PG_DisaCombatModule::updateCursorPointer);

	//the battle starts on a player unit, without one where the editor camera looked
	auto isPlayer = [](const PG_DisaActorModel* actor) {
		auto* sprite = dynamic_cast<const PG_DisaSpriteModel*>(actor);
		return sprite && sprite->isVisible() && sprite->getCharacterAlliance() == 0;
		};
	if (auto player = std::ranges::find_if(*m_actors.get(), isPlayer); player != m_actors->end()) lookAt = (*player)->getObjectTranslation();

	m_running = true;
	moveCursorTo(glm::ivec2(std::floor(lookAt.x), std::floor(lookAt.z)));
	camera->setOrbitPoint(cursor->getTranslation());
	emit runningChanged(true);
}

void PG_DisaCombatModule::stop() {
	if (!m_running) return;
	m_running = false;
	clearSelection();

	if (auto* camera = m_camera.get()) {
		camera->removeEventFilter(this);
		if (auto* scene = getScene(); scene && scene->getActiveCamera() == camera) scene->setActiveCamera(m_previousCamera.get());
		delete camera;
	}
	m_camera.reset();
	m_previousCamera.reset();

	if (auto* root = m_root.get()) delete root;
	m_root.reset();
	m_actorView = nullptr;
	m_rangeNode = nullptr;
	m_cursor.reset();

	if (auto* actors = m_actors.get()) delete actors;
	m_actors.reset();

	if (auto* mapView = m_mapView.get()) mapView->setActorsVisible(m_wasActorsVisible);
	m_mapView.reset();
	emit runningChanged(false);
}

void PG_DisaCombatModule::moveCursorTo(glm::ivec2 tile, float frames) {
	auto* cursor = m_cursor.get();
	if (!cursor) return;
	cursor->moveToTile(glm::clamp(tile, m_tileMin, m_tileMax), frames);
}

PG_DisaActorModel* PG_DisaCombatModule::findUnit(glm::ivec2 tile) const {
	if (!m_actors) return nullptr;
	auto found = std::ranges::find_if(*m_actors.get(), [tile](const PG_DisaActorModel* actor) {
		return dynamic_cast<const PG_DisaSpriteModel*>(actor) && actor->isVisible() && actor->getTilePosition() == tile;
		});
	return found != m_actors->end() ? *found : nullptr;
}

void PG_DisaCombatModule::selectAtCursor() {
	if (!m_cursor) return;
	if (auto* unit = findUnit(m_cursor->getTile())) select(unit);
}

void PG_DisaCombatModule::select(PG_DisaActorModel* unit) {
	auto* sprite = dynamic_cast<PG_DisaSpriteModel*>(unit);
	auto* map = getMapModel();
	if (!sprite || !map || !m_root) return;

	m_selected = unit;
	PG_DisaTileViewNode::updateFloorColorGeometry(map->getTiles(), PG_DisaMoveRange::find(*map, *m_actors.get(), *sprite), 0, m_rangeNode);

	if (!m_selectedMarker) {
		m_selectedMarker = PG_new<PG_DisaActorEditSelectionNode>("N_SelectedMarker", m_root.get());
		m_selectedMarker->setSelected(true);
	}
	//a geo construct stands lower than a unit, like the edit mode marker
	m_selectedMarker->setBaseHeight(sprite->getCharacterId() == 2800 ? 0.7f : 1.4f);
	m_selectedMarker->setTranslation(unit->getObjectTranslation());
	m_selectedMarker->updateModelMatrix();
	m_selectedMarker->updateModelMatrixWorld();
	m_selectedMarker->rotateTowardsCamera();

	updateCursorPointer();
	emit selectionChanged();
}

void PG_DisaCombatModule::clearSelection() {
	if (!m_selected && !m_selectedMarker) return;
	m_selected.reset();
	if (auto* map = getMapModel(); map && m_rangeNode) PG_DisaTileViewNode::updateFloorColorGeometry(map->getTiles(), {}, 0, m_rangeNode);
	delete m_selectedMarker;
	m_selectedMarker = nullptr;
	updateCursorPointer();
	emit selectionChanged();
}

bool PG_DisaCombatModule::filterEvent(PG_Event* e) {
	if (!m_running) return false;
	switch (e->getType()) {
	case PG_Event_MouseMotion: return onMouseMotion(static_cast<PG_MouseMotionEvent*>(e));
	case PG_Event_MouseButton: return onMouseButton(static_cast<PG_MouseButtonEvent*>(e));
	case PG_Event_Key: return onKey(static_cast<PG_KeyEvent*>(e));
	default: return false;
	}
}

bool PG_DisaCombatModule::onMouseMotion(PG_MouseMotionEvent* e) {
	if (auto ray = rayOf(glm::vec2(e->getPosition()))) {
		if (auto tile = pickTile(*ray); tile && m_cursor && *tile != m_cursor->getTile()) moveCursorTo(*tile);
	}
	return false; //the camera still pans
}

bool PG_DisaCombatModule::onMouseButton(PG_MouseButtonEvent* e) {
	if (!e->isDown()) return false;
	if (e->getButton() == PG_BUTTON_LEFT) {
		if (auto ray = rayOf(glm::vec2(e->getPosition()))) {
			if (auto tile = pickTile(*ray)) moveCursorTo(*tile);
		}
		selectAtCursor();
	}
	else if (e->getButton() == PG_BUTTON_RIGHT) {
		clearSelection();
	}
	return false;
}

bool PG_DisaCombatModule::onKey(PG_KeyEvent* e) {
	glm::vec2 move(0.f); //x right, y forward, relative to the camera
	switch (e->getKey()) {
	case PG_Key_Up: move.y = 1.f; break;
	case PG_Key_Down: move.y = -1.f; break;
	case PG_Key_Right: move.x = 1.f; break;
	case PG_Key_Left: move.x = -1.f; break;
	case PG_Key_Escape:
		if (e->isDown()) clearSelection();
		e->accept();
		return true;
	default: return false;
	}
	e->accept();
	if (e->isUp() || !m_camera || !m_cursor) return true;

	//the closest axis to the camera direction, like the game does it in FUN_004d0db0
	const glm::vec3 forward = m_camera->getForwardOrbit();
	const glm::vec2 forwardGround = glm::normalize(glm::vec2(forward.x, forward.z));
	const glm::vec2 rightGround(-forwardGround.y, forwardGround.x);
	const glm::vec2 direction = forwardGround * move.y + rightGround * move.x;
	const glm::ivec2 step = std::abs(direction.x) > std::abs(direction.y)
		? glm::ivec2(direction.x > 0.f ? 1 : -1, 0) : glm::ivec2(0, direction.y > 0.f ? 1 : -1);

	moveCursorTo(m_cursor->getTile() + step);
	m_camera->setOrbitPoint(m_cursor->getTranslation(), 150.f);
	return true;
}

std::optional<glm::ivec2> PG_DisaCombatModule::pickTile(const PG_Ray& ray) const {
	if (auto* tile = PG_DisaEditorActorController::findMinTile(getScene(), ray, true)) return tile->getTilePosition();

	if (auto ground = ray.intersectPlane(glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f))) {
		const glm::vec3 point = ray.getPointOnRay(*ground);
		const glm::ivec2 tile(std::floor(point.x), std::floor(point.z));
		if (glm::all(glm::greaterThanEqual(tile, m_tileMin)) && glm::all(glm::lessThanEqual(tile, m_tileMax))) return tile;
	}
	return std::nullopt;
}

std::optional<PG_Ray> PG_DisaCombatModule::rayOf(glm::vec2 screenPosition) const {
	auto* scene = getScene();
	if (!scene || !m_camera) return std::nullopt;
	auto* item = scene->findParent<PG_GuiItem>(10);
	if (!item) return std::nullopt;
	return m_camera->getRay(item->toLocalTranslation(screenPosition), item->getSize());
}

PG_DisaMapModel* PG_DisaCombatModule::getMapModel() const {
	auto* mapView = m_mapView.get();
	return mapView ? mapView->getMapModel() : nullptr;
}

void PG_DisaCombatModule::copyActors() {
	auto* map = getMapModel();
	if (!map) return;

	//a child of the map, so the copies find the ground they stand on
	auto* actors = PG_new<PG_DisaActorListModel>("PG_DisaCombatActorListModel", map);
	m_actors = actors;

	std::vector<PG_DisaActorModel*> copies;
	std::vector<const PG_DisaActorModel*> originals;
	for (PG_DisaActorModel* actor : *map->getActors()) {
		auto* sprite = dynamic_cast<PG_DisaSpriteModel*>(actor);
		if (!sprite) continue; //meshes only come from scripts, which do not run here
		auto* copy = PG_new<PG_DisaSpriteModel>("PG_DisaSpriteModel");
		copy->setData(sprite->getData());
		copies.push_back(copy);
		originals.push_back(sprite);
	}
	actors->set(std::move(copies));

	for (std::size_t i = 0; i < originals.size(); ++i) {
		PG_DisaActorModel* copy = actors->get(static_cast<unsigned int>(i));
		copy->setObjectTranslation(originals[i]->getObjectTranslation());
		copy->setRotation(originals[i]->getRotation());
		copy->setVisible(originals[i]->isVisible());
	}
	actors->resetMapSlots();
}

void PG_DisaCombatModule::updateCursorPointer() {
	auto* cursor = m_cursor.get();
	if (!cursor) return;
	auto* selected = m_selected.get();
	cursor->setPointerVisible(!selected || selected->getTilePosition() != cursor->getTile());
}
