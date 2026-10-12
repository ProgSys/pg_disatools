#include "PG_DisaMainGameModule.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <Base/Events/PG_KeyEvent.h>
#include <Base/Events/PG_MouseButtonEvent.h>
#include <Base/Events/PG_MouseMotionEvent.h>
#include <Scene/PG_Scene.h>
#include <Scene/PG_ModelNode.h>
#include <Scene/Gui/PG_GuiItem.h>
#include <Disa/PG_DisaMapModel.h>
#include <Disa/PG_DisaMapScene.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/map/PG_DisaSpriteModel.h>
#include <Disa/map/PG_DisaTileViewNode.h>
#include <Disa/map/PG_DisaActorEditSelectionNode.h>
#include <Disa/map/controllers/PG_DisaEditorActorController.h>
#include <Disa/game/PG_DisaCursorNode.h>

PG_DisaMainGameModule::~PG_DisaMainGameModule() {
	/* The scene may already be on its way out, so only what this module owns goes. The nodes are
	 * children of the scene and leave with it. */
	if (auto* actors = m_actors.get()) delete actors;
}

void PG_DisaMainGameModule::added(PG_Scene* scene) {
	auto* mapScene = dynamic_cast<PG_DisaMapScene*>(scene);
	PG_DisaMapViewNode* mapView = mapScene ? mapScene->getMapNode() : nullptr;
	if (m_running || !mapView || !mapView->getMapModel()) return;
	PG_DisaMapModel* map = mapView->getMapModel();

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

	auto* camera = PG_new<PG_DisaCamera>("N_GameCamera", PG_CameraNode::Type_Perspective, scene->getRoot());
	camera->setSettings({
		.selectionMouseKey = 0,
		.panMouseKey = PG_BUTTON_MMASK,
		.turnMouseKey = 0,
		.cancelMouseKey = 0
	});
	m_camera = camera;
	//the game only looks along the diagonals, 45 degree plus quarter turns
	orbit = 45.f + 90.f * std::round((orbit - 45.f) / 90.f);
	//the game starts a battle 750 units away at a 30 degree tilt, see PG_DisaScriptTestModel::resetGameCamera
	camera->setOrbitAngles(-30.f, orbit);
	camera->setOrbitDistance(750.f / 12.f);
	scene->setActiveCamera(camera);
	camera->addEventFilter(this);

	copyActors();
	m_previousActors = mapView->getActorList();
	mapView->setActorList(m_actors.get());

	auto* root = PG_new<PG_ActorNode>("N_Game", mapView);
	m_root = root;
	m_rangeNode = PG_new<PG_ModelNode>("N_MoveRange", root);
	m_pathNode = PG_new<PG_ModelNode>("N_MovePath", root);
	m_cursor = PG_new<PG_DisaCursorNode>("N_GameCursor", root);
	connect(m_cursor, &PG_DisaCursorNode::tileChanged, this, [this] {
		updatePath();
		updateCursorPointer();
		});

	//the battle starts on a player unit, without one where the editor camera looked
	auto isPlayer = [](const PG_DisaActorModel* actor) {
		auto* logic = actor->findFirstChild<PG_DisaActorLogicComponentModel>();
		return logic && actor->isVisible() && logic->isMovable();
		};
	if (auto player = std::ranges::find_if(*m_actors.get(), isPlayer); player != m_actors->end()) lookAt = (*player)->getObjectTranslation();

	m_running = true;
	m_turn = 1;
	moveCursorTo(glm::ivec2(std::floor(lookAt.x), std::floor(lookAt.z)));
	camera->setOrbitPoint(m_cursor->getTranslation());
	emit runningChanged(true);
}

void PG_DisaMainGameModule::removed(PG_Scene* scene) {
	if (!m_running) return;
	m_running = false;
	clearSelection();

	if (auto* camera = m_camera.get()) {
		camera->removeEventFilter(this);
		if (scene && scene->getActiveCamera() == camera) scene->setActiveCamera(m_previousCamera.get());
		delete camera;
	}
	m_camera.reset();
	m_previousCamera.reset();

	if (auto* root = m_root.get()) delete root;
	m_root.reset();
	m_cursor = nullptr;
	m_rangeNode = nullptr;
	m_pathNode = nullptr;

	if (auto* mapView = getMapView()) mapView->setActorList(m_previousActors.get());
	m_previousActors.reset();
	if (auto* actors = m_actors.get()) delete actors;
	m_actors.reset();
	emit runningChanged(false);
}

void PG_DisaMainGameModule::nextTurn() {
	++m_turn;
	emit turnChanged(m_turn); //every unit resets itself on it
	if (m_selected) updateRange();
}

bool PG_DisaMainGameModule::event(PG_Event* e) {
	//TODO forward is suboptimal, reason is that PG_DisaMapContainerModel is stored in sptr thus has no parent
	if (e->getType() == PG_Event_Tick && m_running && m_actors) {
		for (PG_DisaActorModel* actor : *m_actors.get()) {
			if (auto* logic = actor->findFirstChild<PG_DisaActorLogicComponentModel>()) logic->event(e);
		}
	}
	return PG_SceneModule::event(e);
}

bool PG_DisaMainGameModule::filterEvent(PG_Event* e) {
	if (!m_running) return false;
	switch (e->getType()) {
	case PG_Event_MouseMotion: return onMouseMotion(static_cast<PG_MouseMotionEvent*>(e));
	case PG_Event_MouseButton: return onMouseButton(static_cast<PG_MouseButtonEvent*>(e));
	case PG_Event_Key: return onKey(static_cast<PG_KeyEvent*>(e));
	default: return false;
	}
}

bool PG_DisaMainGameModule::onMouseMotion(PG_MouseMotionEvent* e) {
	if (auto ray = rayOf(glm::vec2(e->getPosition()))) {
		if (auto tile = pickTile(*ray); tile && m_cursor && *tile != m_cursor->getTile()) moveCursorTo(*tile);
	}
	return false; //the camera still pans
}

bool PG_DisaMainGameModule::onMouseButton(PG_MouseButtonEvent* e) {
	if (!e->isDown()) return false;
	if (e->getButton() == PG_BUTTON_LEFT) {
		if (auto ray = rayOf(glm::vec2(e->getPosition()))) {
			if (auto tile = pickTile(*ray)) moveCursorTo(*tile);
		}
		confirmAtCursor();
	}
	else if (e->getButton() == PG_BUTTON_RIGHT) {
		clearSelection();
	}
	return false;
}

bool PG_DisaMainGameModule::onKey(PG_KeyEvent* e) {
	glm::vec2 move(0.f); //x right, y forward, relative to the camera
	switch (e->getKey()) {
	case PG_Key_Up: move.x = -1.f; break;
	case PG_Key_Down: move.x = 1.f; break;
	case PG_Key_Right: move.y = 1.f; break;
	case PG_Key_Left: move.y = -1.f; break;
	case PG_Key_Return:
		if (e->isDown() && !e->isRepeat()) confirmAtCursor();
		e->accept();
		return true;
	case PG_Key_Backspace:
	case PG_Key_Escape:
		if (e->isDown()) clearSelection();
		e->accept();
		return true;
	case PG_Key_T:
		if (e->isDown() && !e->isRepeat()) nextTurn();
		e->accept();
		return true;
	default: return false;
	}
	e->accept();
	if (e->isUp() || !m_camera || !m_cursor) return true;

	const glm::vec3 forward = m_camera->getForwardOrbit();
	const glm::vec2 forwardGround = glm::normalize(glm::vec2(forward.x, forward.z));
	const glm::vec2 rightGround(-forwardGround.y, forwardGround.x);
	const glm::vec2 direction = forwardGround * move.y + rightGround * move.x;

	/* The quarter of the direction picks the axis. The camera looks along a diagonal, right on the
	 * border of two quarters, so like FUN_004d0db0, which adds 30 to the yaw before it divides by 90,
	 * the borders are shifted to keep the four keys on four different axes. */
	const float quarters = glm::degrees(std::atan2(direction.y, direction.x) + glm::radians(15.f)) / 90.f;
	constexpr glm::ivec2 steps[] = { glm::ivec2(1, 0), glm::ivec2(0, 1), glm::ivec2(-1, 0), glm::ivec2(0, -1) };
	const glm::ivec2 step = steps[static_cast<int>(std::floor(quarters + 0.5f)) & 3];

	moveCursorTo(m_cursor->getTile() + step);
	m_camera->setOrbitPoint(m_cursor->getTranslation(), 150.f);
	return true;
}

void PG_DisaMainGameModule::confirmAtCursor() {
	if (!m_cursor) return;
	const glm::ivec2 tile = m_cursor->getTile();
	auto* unit = findUnit(tile);
	if (unit && unit != m_selected.get()) {
		select(unit);
		return;
	}
	if (!moveSelected(tile) && unit) select(unit);
}

void PG_DisaMainGameModule::moveCursorTo(glm::ivec2 tile, float frames) {
	if (m_cursor) m_cursor->moveToTile(glm::clamp(tile, m_tileMin, m_tileMax), frames);
}

std::optional<glm::ivec2> PG_DisaMainGameModule::pickTile(const PG_Ray& ray) const {
	if (auto* tile = PG_DisaEditorActorController::findMinTile(getScene(), ray, true)) return tile->getTilePosition();

	if (auto ground = ray.intersectPlane(glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f))) {
		const glm::vec3 point = ray.getPointOnRay(*ground);
		const glm::ivec2 tile(std::floor(point.x), std::floor(point.z));
		if (glm::all(glm::greaterThanEqual(tile, m_tileMin)) && glm::all(glm::lessThanEqual(tile, m_tileMax))) return tile;
	}
	return std::nullopt;
}

std::optional<PG_Ray> PG_DisaMainGameModule::rayOf(glm::vec2 screenPosition) const {
	auto* scene = getScene();
	if (!scene || !m_camera) return std::nullopt;
	auto* item = scene->findParent<PG_GuiItem>(10);
	if (!item) return std::nullopt;
	return m_camera->getRay(item->toLocalTranslation(screenPosition), item->getSize());
}

PG_DisaMapViewNode* PG_DisaMainGameModule::getMapView() const {
	auto* mapScene = dynamic_cast<PG_DisaMapScene*>(getScene());
	return mapScene ? mapScene->getMapNode() : nullptr;
}

PG_DisaMapModel* PG_DisaMainGameModule::getMapModel() const {
	auto* mapView = getMapView();
	return mapView ? mapView->getMapModel() : nullptr;
}

void PG_DisaMainGameModule::copyActors() {
	auto* map = getMapModel();
	if (!map) return;

	auto* actors = PG_new<PG_DisaActorListModel>("PG_DisaGameActorListModel", map);
	m_actors = actors;
	//a unit that spawns later gets its game state the same way
	connect(actors, &PG_ListModelBase::itemsAdded, this, [this](PG_Index startIndex, PG_Index endIndex) {
		auto* list = m_actors.get();
		for (PG_Index i = startIndex; list && i <= endIndex && i < list->size(); ++i) attachLogic(list->get(i));
		});

	const PG_DisaActorListModel* originals = map->getActors();
	std::vector<PG_DisaActorModel*> copies;
	copies.reserve(originals->size());
	for (const PG_DisaActorModel* actor : *originals) copies.push_back(actor->clone());
	actors->set(std::vector<PG_DisaActorModel*>(copies));

	for (std::size_t i = 0; i < copies.size(); ++i) {
		if (const int slot = originals->getSlotOf(originals->get(static_cast<PG_Index>(i))); slot >= 0) actors->setSlot(slot, copies[i]);
	}
}

void PG_DisaMainGameModule::attachLogic(PG_DisaActorModel* actor) {
	if (!dynamic_cast<PG_DisaSpriteModel*>(actor) || actor->findFirstChild<PG_DisaActorLogicComponentModel>()) return;
	auto* logic = PG_new<PG_DisaActorLogicComponentModel>("PG_DisaActorLogicComponentModel", actor);
	connect(this, &PG_DisaMainGameModule::turnChanged, logic, &PG_DisaActorLogicComponentModel::resetTurn);
}

PG_DisaActorLogicComponentModel* PG_DisaMainGameModule::findUnit(glm::ivec2 tile) const {
	if (!m_actors) return nullptr;
	for (const PG_DisaActorModel* actor : *m_actors.get()) {
		auto* logic = actor->findFirstChild<PG_DisaActorLogicComponentModel>();
		if (logic && actor->isVisible() && logic->getTile() == tile) return logic;
	}
	return nullptr;
}

void PG_DisaMainGameModule::select(PG_DisaActorLogicComponentModel* unit) {
	if (!unit || !m_root) return;

	m_selected = unit;
	if (!m_selectedMarker) {
		m_selectedMarker = PG_new<PG_DisaActorEditSelectionNode>("N_SelectedMarker", m_root.get());
		m_selectedMarker->setSelected(true);
	}
	//a geo construct stands lower than a unit, like the edit mode marker
	m_selectedMarker->setBaseHeight(unit->getUnit()->getCharacterId() == 2800 ? 0.7f : 1.4f);
	updateRange();
}

void PG_DisaMainGameModule::clearSelection() {
	if (!m_selected && !m_selectedMarker) return;
	m_selected.reset();
	m_range.reset();
	if (auto* map = getMapModel()) {
		if (m_rangeNode) PG_DisaTileViewNode::updateFloorColorGeometry(map->getTiles(), {}, 0, m_rangeNode);
		if (m_pathNode) PG_DisaTileViewNode::updateFloorPathGeometry(map->getTiles(), {}, 0.f, m_pathNode);
	}
	delete m_selectedMarker;
	m_selectedMarker = nullptr;
	updateCursorPointer();
}

bool PG_DisaMainGameModule::moveSelected(glm::ivec2 tile) {
	auto* unit = m_selected.get();
	if (!unit || !unit->isMovable() || !m_range || tile == m_range->getStart()) return false;

	const std::vector<glm::ivec2> path = m_range->buildPath(m_range->nearestDestination(tile));
	if (path.size() < 2) return false;
	unit->moveAlong(path);
	updateRange();
	return true;
}

void PG_DisaMainGameModule::updateRange() {
	auto* unit = m_selected.get();
	auto* map = getMapModel();
	if (!unit || !map || !m_actors) return;

	std::vector<PG_DisaMoveRange::Occupant> occupants;
	for (const PG_DisaActorModel* actor : *m_actors.get()) {
		auto* other = actor->findFirstChild<PG_DisaActorLogicComponentModel>();
		if (!other || other == unit || !actor->isVisible()) continue;
		occupants.push_back({ other->getTile(), other->getUnit()->getCharacterAlliance() });
	}
	const PG_DisaMoveRange::Stats stats{ unit->getMovementLeft(), unit->getStats().jumpPower };
	m_range.emplace(*map, unit->getTile(), unit->getUnit()->getCharacterAlliance(), stats, occupants);
	PG_DisaTileViewNode::updateFloorColorGeometry(map->getTiles(), m_range->getTiles(), 0, m_rangeNode);

	//the marker shows where the unit stands for the rules, its sprite may still be walking there
	m_selectedMarker->setTranslation(PG_DisaActorLogicComponentModel::standPosition(map, unit->getTile()));
	m_selectedMarker->updateModelMatrix();
	m_selectedMarker->updateModelMatrixWorld();
	m_selectedMarker->rotateTowardsCamera();

	updatePath();
	updateCursorPointer();
}

void PG_DisaMainGameModule::updatePath() {
	auto* map = getMapModel();
	if (!map || !m_pathNode) return;

	std::vector<glm::ivec2> path;
	auto* unit = m_selected.get();
	if (m_range && m_cursor && unit && unit->isMovable() && m_cursor->getTile() != m_range->getStart()) {
		path = m_range->buildPath(m_range->nearestDestination(m_cursor->getTile()));
	}
	PG_DisaTileViewNode::updateFloorPathGeometry(map->getTiles(), path, 0.15f, m_pathNode);
}

void PG_DisaMainGameModule::updateCursorPointer() {
	if (!m_cursor) return;
	auto* selected = m_selected.get();
	m_cursor->setPointerVisible(!selected || selected->getTile() != m_cursor->getTile());
}
