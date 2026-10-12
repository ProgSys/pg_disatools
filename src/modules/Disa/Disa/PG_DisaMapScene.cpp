#include "PG_DisaMapScene.h"


#include <Base/PG_Application.h>
#include <Base/PG_EventLoop.h>
#include <Base/Events/PG_TickEvent.h>
#include <Base/Events/PG_KeyEvent.h>
#include <Base/Events/PG_MouseMotionEvent.h>

#include <Scene/Editor/PG_EditorCameraNode.h>

#include <Scene/PG_ModelNode.h>
#include <Scene/PG_FootlooseViewNode.h>
#include <Disa/map/controllers/PG_DisaEditorTileController.h>
#include <Disa/map/controllers/PG_DisaEditorMaterialController.h>
#include <Disa/map/controllers/PG_DisaEditorObjectController.h>
#include <Disa/map/controllers/PG_DisaEditorActorController.h>
#include <Disa/map/controllers/PG_DisaEditorEventController.h>
#include <Disa/map/PG_DisaEditorModule.h>
#include <Disa/game/PG_DisaMainGameModule.h>


#include <Renderer/PG_SceneRenderer.h>
#include <SDLApp/PG_SDLWindow.h>
#include <Resources/PG_Shader.h>
#include <glm/gtx/rotate_vector.hpp>

#include <Disa/PG_DisaModelNode.h>
#include <Disa/PG_DisaActorNode.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/PG_DisaMapModel.h>
#include <Disa/PG_DisaAssetsModel.h>


void PG_DisaMapScene::init() {
	PG_Scene::init();

	auto editor = installModule<PG_DisaEditorModule>("PG_DisaEditorModule");
	connect(editor, &PG_DisaEditorModule::modeChanged, editor, &PG_DisaEditorModule::clearSelection);
	connect(editor, &PG_DisaEditorModule::modeChanged, this, [this](PG_DisaEditorModule::EMode mode) {
		auto comp = getActiveCamera()->findFirstChild<PG_EditorCameraComponent>();
		if (!comp) return;

		switch (mode) {
		default:
		case PG_DisaEditorModule::Mode_View: {
			comp->setInteractionController(nullptr);
			m_mapNode->setMode(PG_DisaMapViewNode::Mode_View);
		}break;
		case PG_DisaEditorModule::Mode_Terrain: {
			comp->setInteractionController(PG_new<PG_DisaEditorTileController>());
			m_mapNode->setMode(PG_DisaMapViewNode::Mode_Terrain);
		}break;
		case PG_DisaEditorModule::Mode_Material: {
			comp->setInteractionController(PG_new<PG_DisaEditorMaterialController>());
			m_mapNode->setMode(PG_DisaMapViewNode::Mode_Material);
		}break;
		case PG_DisaEditorModule::Mode_Object: {
			comp->setInteractionController(PG_new<PG_DisaEditorObjectController>());
			m_mapNode->setMode(PG_DisaMapViewNode::Mode_Object);
		}break;
		case PG_DisaEditorModule::Mode_EventTile: {
			comp->setInteractionController(PG_new<PG_DisaEditorEventController>());
			m_mapNode->setMode(PG_DisaMapViewNode::Mode_Event);
		}break;
		case PG_DisaEditorModule::Mode_Actor: {
			connect(m_mapNode, &PG_DisaMapViewNode::isActorsVisibleChanged, comp, [this] {
				if (auto mod = getModule<PG_DisaEditorModule>()) {
					mod->clearHovered();
					mod->clearSelection();
				}
				});
			comp->setInteractionController(PG_new<PG_DisaEditorActorController>());
			m_mapNode->setMode(PG_DisaMapViewNode::Mode_Actor);
		}break;
		}
	});

	{
		{
			auto cam = PG_new<PG_EditorCameraNode>("PG_EditorCameraNode", PG_CameraNode::Type_Perspective, getRoot());
			cam->setSettings({
				.selectionMouseKey = BUTTON_BIT(1),// SDL_BUTTON_LMASK;
				.panMouseKey = BUTTON_BIT(2),// SDL_BUTTON_MMASK;
				.turnMouseKey = BUTTON_BIT(3) // SDL_BUTTON_RMASK;
			});
			PG_new<PG_EditorCameraComponent>("PG_EditorCameraComponent", cam);
			cam->setTranslation(glm::vec3(3.0f, 5.0f, 3.0f));
			cam->lookAt(glm::vec3(0.0f));
			setActiveCamera(cam);
		}
		{

			auto model = PG_new<PG_ModelNode>("N_Floor", "MD_MapFloor", std::vector<PG_Model::Surface>{
				{PG_Material("M_MapGrid"), PG_Geometry("G_MapFloor", PG_Mesh::planeUVScaled(1000.f, 1000.f))}
				}, getRoot());
		}
		{
			auto axis = PG_new<PG_ModelNode>("N_Coods", "MD_Coods", std::vector<PG_Model::Surface>{ {
						PG_Material("M_SimpleColor"),
						PG_Geometry("G_Coords",
							{{
								PG_Mesh::DataWrapper("position", std::vector<glm::vec3>{
								glm::vec3(0.0f), glm::vec3(10.0f, 0.0f, 0.0f),
								glm::vec3(0.0f), glm::vec3(0.0f, 10.0f, 0.0f),
								glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 10.0f)}),

								PG_Mesh::DataWrapper("color", std::vector<glm::vec3>{
								glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
								glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
								glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)}),
							}},
							GL::LINES
						)
					} }, getRoot());
		}

		{ //test new Gird map
			//PG_new<PG_MapTileGrid>("PG_MapTileGrid", getRoot());
		}

		{
			m_mapNode = PG_new<PG_DisaMapViewNode>("PG_DisaMapViewNode", getRoot());
		}

	}
}

void PG_DisaMapScene::copy(PG_DisaMapScene* scene) {
	assert(scene);
	if (getActiveCamera() && scene->getActiveCamera())
		getActiveCamera()->setTransformation(scene->getActiveCamera()->getTransformation());
	if (auto container = scene->m_mapNode->getModel()) {
		m_mapNode->setModel(container);
	}
	else {
		m_mapNode->setMapModel(scene->m_mapNode->getMapModel());
	}
}

void PG_DisaMapScene::orbitCamera(float value) {
	if (auto cam = getActiveCamera()) {
		PG_Volume bv;
		if (m_mapNode->getTileView()) bv = m_mapNode->getTileView()->getVolumeVisibilityHierarchy();
		if (!bv) bv = m_mapNode->getVolumeVisibilityHierarchy();
		if (!bv) bv = getRoot()->getVolumeVisibilityHierarchy();
		glm::vec3 bvCenter{ 0.f };
		glm::vec3 bvPoint{ 6.f };
		if (bv) {
			bvCenter = bv.getCenter();
			bvPoint = bv.getPoint(4);
		}

		glm::vec3 baseDir = glm::normalize(glm::vec3(1.f));
		float distance = 6.f;

		distance = glm::clamp(glm::distance(bvCenter, bvPoint) * 2.f, 1.f, 6.f);

		const glm::vec3 dir = glm::rotateY(
			cam->getTranslation() - bvCenter,
			float(value) //angle (rad)
		);
			
		if (std::abs(glm::length(dir) - distance) > 0.001F) {
			cam->setTranslation(bvCenter + baseDir * distance);
		}
		else {
			cam->setTranslation(bvCenter + dir);
		}
	
		cam->lookAt(bvCenter);
		cam->updateModelMatrix();
	}
}

bool PG_DisaMapScene::event(PG_Event* e) {
	if (m_previewMode) {
		switch (e->getType())
		{
		case PG_Event_Tick: {
			auto tick = static_cast<PG_TickEvent*>(e);
			orbitCamera(0.0001 * tick->getDeltaTime());
			return PG_Scene::event(e);
		}
		case PG_Event_MouseMotion: {
			auto mouse = static_cast<PG_MouseMotionEvent*>(e);
			if (mouse->getButton() & PG_BUTTON_MMASK) {
				orbitCamera(-mouse->getPositionRelative().x / 100.0f);
			}
			return true;
		}
		case PG_Event_Key:
		case PG_Event_MouseButton:
		case PG_Event_MouseWheel:
			return true;
		}
	}

	if (e->getType() == EEventTypes::PG_Event_Render) {
		return false;
	}

	return PG_Scene::event(e);
}

void PG_DisaMapScene::setPreviewMode(bool isPreviewMode) {
	if (m_previewMode == isPreviewMode) return;
	m_previewMode = isPreviewMode;
	m_mapNode->setActorViewEnabled(!m_previewMode); //a preview shows the map, not who stands on it
	if (m_previewMode) {
		connect(m_mapNode, &PG_DisaMapViewNode::loaded, this, [this] {
			orbitCamera(0.f);
			});
	}
	else {
		disconnect(m_mapNode, &PG_DisaMapViewNode::loaded, this, nullptr);
	}
}

void PG_DisaMapScene::setAxisVisible(bool visible) {
	if (auto actor = getRoot()->findFirstChild<PG_ModelNode>("N_Coods")) {
		actor->setVisible(visible);
	}	
	if (auto actor = getRoot()->findFirstChild<PG_ModelNode>("N_Floor")) {
		actor->setVisible(visible);
	}
}

void PG_DisaMapScene::setShowHiddenModel(bool visible) {
	/*
	m_mapNode->traverseTestWithoutRoot([visible](PG_Object* obj) {
		if (auto model = dynamic_cast<PG_DisaModelNode*>(obj)) {
			if (model->isMarkedAsHidden())
				model->setVisible(visible);
		}
		return false;
		});
		*/
}

bool PG_DisaMapScene::isActorsVisible() const {
	return m_mapNode->isActorsVisible();
}

void PG_DisaMapScene::setActorsVisible(bool visible) {
	m_mapNode->setActorsVisible(visible);
}

bool PG_DisaMapScene::isGeoPanelsVisible() const {
	return m_mapNode->isGeoVisible();
}

void PG_DisaMapScene::setGeoPanelsVisible(bool visible) {
	m_mapNode->setGeoVisible(visible);
}

void PG_DisaMapScene::setEnableEditing(bool enable) {
	getModule<PG_DisaEditorModule>()->setMode(enable ? PG_DisaEditorModule::Mode_Terrain : PG_DisaEditorModule::Mode_View);
}

bool PG_DisaMapScene::isCombatSimulation() const {
	auto* combat = getModule<PG_DisaMainGameModule>();
	return combat && combat->isRunning();
}

void PG_DisaMapScene::setCombatSimulation(bool enable) {
	if (enable == isCombatSimulation()) return;

	if (!enable) {
		delete removeModule<PG_DisaMainGameModule>();
		return;
	}

	setEnableEditing(false);
	auto* combat = installModule<PG_DisaMainGameModule>("PG_DisaMainGameModule");
	if (!combat->isRunning()) delete removeModule<PG_DisaMainGameModule>(); //no map to simulate
}

bool PG_DisaMapScene::isHDMode() const {
	return m_mapNode->isHDMode();
}

void PG_DisaMapScene::setHDMode(bool state) {
	m_mapNode->setHDMode(state);
}

PG_DisaObjectModel* PG_DisaMapScene::addObject(char modelId, const std::string& resourceName, glm::vec3 pos) {
	auto* model = getMapModel();
	if (!model) return nullptr;

	auto objects = model->getObjects();
	auto object = PG_new<PG_DisaObjectModel>("PG_DisaObjectModel", objects);
	object->setChunk(model->getChunks()->back());
	mpdfileChunkObject data = mpdfileChunkObject::createNew();
	data.model_id = modelId;
	const glm::ivec3 tilePos = PG_DisaObjectModel::toObjectTranslation(pos - model->getChunks()->back()->getTileTranslation());
	data.x = tilePos.x;
	data.y = tilePos.y;
	data.z = tilePos.z;
	object->setData(data);
	object->setResourceName(resourceName);
	objects->push_back(object);
	return object;
}

PG_DisaObjectModel* PG_DisaMapScene::addModel(int index, glm::vec3 pos) {
	return addObject(static_cast<char>(index), {}, pos);
}

PG_DisaObjectModel* PG_DisaMapScene::addResourceModel(const std::string& resourceName, glm::vec3 pos) {
	return addObject(-1, resourceName, pos); //-1 is no entry of the model table, the resource is what draws
}

void PG_DisaMapScene::addModelAtViewCenter(int index) {
	if (!getActiveCamera()) return;
	const PG_Ray ray = getActiveCamera()->getRayViewDir();
	addModel(index, PG_DisaEditorTileController::findCollisionPoint(ray, getRoot()));
}

void PG_DisaMapScene::duplicateObject(int objectIndex) {
	auto* model = getMapModel();
	if (!model) return;

	auto objects = model->getObjects();
	if (objectIndex < 0 || objectIndex >= objects->size()) return;

	static int lastAdded = -1;
	static int offsetCounter = 0;
	if (objectIndex != lastAdded) {
		lastAdded = objectIndex;
		offsetCounter = 1;
	}
	else {
		++offsetCounter;
	}

	const PG_DisaObjectModel* original = objects->get(objectIndex);
	auto dublicateData = original->getData();
	dublicateData.x += 5 * offsetCounter;

	auto object = PG_new<PG_DisaObjectModel>("PG_DisaObjectModel", objects);
	object->setChunk(model->getChunks()->back());
	object->setData(dublicateData);
	object->setResourceName(original->getResourceName());
	objects->push_back(object);
}

PG_DisaMapModel* PG_DisaMapScene::getMapModel() const {
	return m_mapNode->getMapModel();
}

void PG_DisaMapScene::setModel(const std::shared_ptr<PG_DisaMPDContainer>& container) {
	setCombatSimulation(false);
	m_mapNode->setModel(container);
	if (!container) return;
	if (const auto& start = container->getCameraStartTransformation(); start && getActiveCamera()) {
		getActiveCamera()->setTransformation(*start);
	}
}

void PG_DisaMapScene::setMapModel(PG_DisaMapModel* model) {
	setCombatSimulation(false);
	m_mapNode->setMapModel(model);
}
