#include "PG_DisaCombatModule.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <ranges>
#include <glm/gtc/constants.hpp>
#include <Base/Events/PG_KeyEvent.h>
#include <Base/Events/PG_MouseButtonEvent.h>
#include <Base/Events/PG_MouseMotionEvent.h>
#include <Base/Events/PG_TickEvent.h>
#include <Scene/PG_Scene.h>
#include <Scene/PG_ModelNode.h>
#include <Scene/PG_TickComponent.h>
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

PG_DisaCombatModule::~PG_DisaCombatModule() {
	/* The scene may already be on its way out, so only what this module owns goes. The nodes are
	 * children of the scene and leave with it. */
	if (auto* actors = m_actors.get()) delete actors;
}

void PG_DisaCombatModule::added(PG_Scene* scene) {
	auto* mapScene = dynamic_cast<PG_DisaMapScene*>(scene);
	PG_DisaMapViewNode* mapView = mapScene ? mapScene->getMapNode() : nullptr;
	if (m_running || !mapView || !mapView->getMapModel()) return;
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
	//the game only looks along the diagonals, 45 degree plus quarter turns
	orbit = 45.f + 90.f * std::round((orbit - 45.f) / 90.f);
	//the game starts a battle 750 units away at a 30 degree tilt, see PG_DisaScriptTestModel::resetGameCamera
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
	m_pathNode = PG_new<PG_ModelNode>("N_MovePath", root);
	PG_new<PG_TickComponent>(PG_TickComponent::TickFunction([this](PG_ActorNode*, PG_TickEvent* tick) {
		updateWalks(static_cast<float>(tick->getDeltaTime()));
		}), "N_CombatWalks", root);

	auto* cursor = PG_new<PG_DisaCombatCursorNode>("N_CombatCursor", root);
	m_cursor = cursor;
	connect(cursor, &PG_DisaCombatCursorNode::tileChanged, this, [this] {
		updatePath();
		updateCursorPointer();
		});

	//the battle starts on a player unit, without one where the editor camera looked
	auto isPlayer = [](const PG_DisaActorModel* actor) {
		auto* sprite = dynamic_cast<const PG_DisaSpriteModel*>(actor);
		return sprite && sprite->isVisible() && sprite->getCharacterAlliance() == 0;
		};
	if (auto player = std::ranges::find_if(*m_actors.get(), isPlayer); player != m_actors->end()) lookAt = (*player)->getObjectTranslation();

	m_running = true;
	m_turn = 1;
	moveCursorTo(glm::ivec2(std::floor(lookAt.x), std::floor(lookAt.z)));
	camera->setOrbitPoint(cursor->getTranslation());
	emit runningChanged(true);
}

void PG_DisaCombatModule::removed(PG_Scene* scene) {
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
	m_actorView = nullptr;
	m_rangeNode = nullptr;
	m_pathNode = nullptr;
	m_cursor.reset();

	m_units.clear();
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
	auto found = std::ranges::find_if(m_units, [tile](const auto& unit) { return unit.first->isVisible() && unit.second.tile == tile; });
	return found != m_units.end() ? found->first : nullptr;
}

glm::ivec2 PG_DisaCombatModule::getUnitTile(const PG_DisaActorModel* unit) const {
	auto findIt = m_units.find(const_cast<PG_DisaActorModel*>(unit));
	return findIt != m_units.end() ? findIt->second.tile : unit->getTilePosition();
}

int PG_DisaCombatModule::getMovementLeft(const PG_DisaActorModel* unit) const {
	auto findIt = m_units.find(const_cast<PG_DisaActorModel*>(unit));
	return findIt != m_units.end() ? findIt->second.movementLeft : 0;
}

bool PG_DisaCombatModule::isMovable(const PG_DisaActorModel* unit) const {
	auto* sprite = dynamic_cast<const PG_DisaSpriteModel*>(unit);
	return sprite && sprite->getCharacterAlliance() == 0;
}

void PG_DisaCombatModule::selectAtCursor() {
	if (!m_cursor) return;
	if (auto* unit = findUnit(m_cursor->getTile())) select(unit);
}

void PG_DisaCombatModule::select(PG_DisaActorModel* unit) {
	auto* sprite = dynamic_cast<PG_DisaSpriteModel*>(unit);
	if (!sprite || !m_root || !m_units.contains(unit)) return;

	m_selected = unit;
	if (!m_selectedMarker) {
		m_selectedMarker = PG_new<PG_DisaActorEditSelectionNode>("N_SelectedMarker", m_root.get());
		m_selectedMarker->setSelected(true);
	}
	//a geo construct stands lower than a unit, like the edit mode marker
	m_selectedMarker->setBaseHeight(sprite->getCharacterId() == 2800 ? 0.7f : 1.4f);
	updateRange();
	emit selectionChanged();
}

void PG_DisaCombatModule::clearSelection() {
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
	emit selectionChanged();
}

bool PG_DisaCombatModule::moveSelected(glm::ivec2 tile) {
	auto* unit = m_selected.get();
	auto stateIt = m_units.find(unit);
	if (!unit || !isMovable(unit) || !m_range || stateIt == m_units.end() || tile == m_range->getStart()) return false;

	const std::vector<glm::ivec2> path = m_range->buildPath(m_range->nearestDestination(tile));
	if (path.size() < 2) return false;

	UnitState& state = stateIt->second;
	state.walk.insert(state.walk.end(), std::next(path.begin()), path.end());
	state.movementLeft -= static_cast<int>(path.size()) - 1;
	state.tile = path.back();
	updateRange();
	return true;
}

void PG_DisaCombatModule::nextTurn() {
	++m_turn;
	for (UnitState& state : m_units | std::views::values) state.movementLeft = state.stats.movement;
	if (m_selected) updateRange();
	emit turnChanged(m_turn);
	
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
		confirmAtCursor();
	}
	else if (e->getButton() == PG_BUTTON_RIGHT) {
		clearSelection();
	}
	return false;
}

bool PG_DisaCombatModule::onKey(PG_KeyEvent* e) {
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

void PG_DisaCombatModule::confirmAtCursor() {
	if (!m_cursor) return;
	const glm::ivec2 tile = m_cursor->getTile();
	auto* unit = findUnit(tile);
	if (unit && unit != m_selected.get()) {
		select(unit);
		return;
	}
	if (!moveSelected(tile) && unit) select(unit);
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

	m_units.clear();
	for (std::size_t i = 0; i < originals.size(); ++i) {
		PG_DisaActorModel* copy = actors->get(static_cast<unsigned int>(i));
		copy->setObjectTranslation(originals[i]->getObjectTranslation());
		copy->setRotation(originals[i]->getRotation());
		copy->setVisible(originals[i]->isVisible());
		copy->setGravity(0.f); //the walk puts the height itself, a jump would otherwise drop early

		UnitState& state = m_units[copy];
		state.tile = copy->getTilePosition();
		state.stats = PG_DisaMoveRange::statsOf(*static_cast<PG_DisaSpriteModel*>(copy));
		state.movementLeft = state.stats.movement;
	}
	actors->resetMapSlots();
}

glm::vec3 PG_DisaCombatModule::standPosition(glm::ivec2 tile) const {
	glm::vec3 position = PG_DisaActorModel::tileCenter(tile);
	position.y = PG_DisaCombatCursorNode::groundHeight(getMapModel(), tile);
	return position;
}

void PG_DisaCombatModule::updateRange() {
	auto* unit = dynamic_cast<PG_DisaSpriteModel*>(m_selected.get());
	auto* map = getMapModel();
	auto stateIt = m_units.find(unit);
	if (!unit || !map || stateIt == m_units.end()) return;
	const UnitState& state = stateIt->second;

	std::vector<PG_DisaMoveRange::Occupant> occupants;
	for (const auto& [actor, other] : m_units) {
		if (actor == unit || !actor->isVisible()) continue;
		occupants.push_back({ other.tile, static_cast<const PG_DisaSpriteModel*>(actor)->getCharacterAlliance() });
	}
	m_range.emplace(*map, state.tile, unit->getCharacterAlliance(), PG_DisaMoveRange::Stats{ state.movementLeft, state.stats.jumpPower }, occupants);
	PG_DisaTileViewNode::updateFloorColorGeometry(map->getTiles(), m_range->getTiles(), 0, m_rangeNode);

	//the marker shows where the unit stands for the rules, its sprite may still be walking there
	m_selectedMarker->setTranslation(standPosition(state.tile));
	m_selectedMarker->updateModelMatrix();
	m_selectedMarker->updateModelMatrixWorld();
	m_selectedMarker->rotateTowardsCamera();

	updatePath();
	updateCursorPointer();
}

void PG_DisaCombatModule::updatePath() {
	auto* map = getMapModel();
	if (!map || !m_pathNode) return;

	std::vector<glm::ivec2> path;
	if (m_range && m_cursor && isMovable(m_selected.get()) && m_cursor->getTile() != m_range->getStart()) {
		path = m_range->buildPath(m_range->nearestDestination(m_cursor->getTile()));
	}
	PG_DisaTileViewNode::updateFloorPathGeometry(map->getTiles(), path, 0.15f, m_pathNode);
}

void PG_DisaCombatModule::updateCursorPointer() {
	auto* cursor = m_cursor.get();
	if (!cursor) return;
	auto* selected = m_selected.get();
	cursor->setPointerVisible(!selected || getUnitTile(selected) != cursor->getTile());
}

void PG_DisaCombatModule::updateWalks(float milliseconds) {
	constexpr float framesPerTile = 15.f; //FUN_004475c0 moves a walking unit one tile in 15 frames
	const float frames = milliseconds * 60.f / 1000.f;

	for (auto& [actor, state] : m_units) {
		if (state.walk.empty()) continue;
		auto* sprite = static_cast<PG_DisaSpriteModel*>(actor);

		float framesLeft = frames;
		while (framesLeft > 0.f && !state.walk.empty()) {
			const glm::vec3 target = standPosition(state.walk.front());
			if (state.stepFrames == 0.f) {
				state.stepStart = actor->getObjectTranslation();
				//0 degree looks along +z, -90 along +x, see PG_DisaActorModel::getRotation
				const glm::vec2 direction(target.x - state.stepStart.x, target.z - state.stepStart.z);
				if (direction != glm::vec2(0.f)) actor->setRotation(glm::degrees(std::atan2(-direction.x, direction.y)));
				sprite->setAnimationSet(PG_DisaSpriteModel::Walking);
			}

			const float used = std::min(framesLeft, framesPerTile - state.stepFrames);
			state.stepFrames += used;
			framesLeft -= used;

			const float progress = state.stepFrames / framesPerTile;
			glm::vec3 position = glm::mix(state.stepStart, target, progress);
			//the game jumps a step of more than half a tile up or down
			if (std::abs(target.y - state.stepStart.y) > 0.5f) position.y += 0.5f * std::sin(progress * glm::pi<float>());
			actor->setObjectTranslation(position);

			if (state.stepFrames >= framesPerTile) {
				state.walk.pop_front();
				state.stepFrames = 0.f;
			}
		}
		if (state.walk.empty()) sprite->setAnimationSet(PG_DisaSpriteModel::IdleStanding);
	}
}
