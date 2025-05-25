#include "PG_DisaMapSceneView.h"


#include <Base/PG_Application.h>
#include <Base/PG_EventLoop.h>
#include <Base/Events/PG_TickEvent.h>
#include <Base/Events/PG_KeyEvent.h>
#include <Base/Events/PG_MouseMotionEvent.h>

#include <Scene/Editor/PG_EditorCameraNode.h>

#include <Scene/PG_ModelNode.h>
#include <Scene/PG_FootlooseViewNode.h>
#include <PG_DisaDefines.h>
#include <Controllers/PG_DisaEditorTileController.h>
#include <Controllers/PG_DisaEditorMaterialController.h>
#include <Controllers/PG_DisaEditorObjectController.h>
#include <Controllers/PG_DisaEditorActorController.h>
#include <Controllers/PG_DisaEditorEventController.h>
#include <Controllers/PG_DisaEditorModule.h>


#include <Renderer/PG_SceneRenderer.h>
#include <SDLApp/PG_SDLWindow.h>
#include <Resources/PG_Shader.h>
#include <nfd.h>
#include <glm/gtx/rotate_vector.hpp>

#include <Disa/PG_DisaModelNode.h>
#include <Disa/PG_DisaActorNode.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/PG_DisaMapModel.h>
#include <Disa/PG_DisaAssetsModel.h>
#include <Models/PG_DisaMapContainerModel.h>


void PG_DisaMapSceneView::init() {
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
			m_mapNodeV2->setMode(PG_DisaMapViewNode::Mode_View);
		}break;
		case PG_DisaEditorModule::Mode_Terrain: {
			comp->setInteractionController(PG_new<PG_DisaEditorTileController>());
			m_mapNodeV2->setMode(PG_DisaMapViewNode::Mode_Terrain);
		}break;
		case PG_DisaEditorModule::Mode_Material: {
			comp->setInteractionController(PG_new<PG_DisaEditorMaterialController>());
			m_mapNodeV2->setMode(PG_DisaMapViewNode::Mode_Material);
		}break;
		case PG_DisaEditorModule::Mode_Object: {
			comp->setInteractionController(PG_new<PG_DisaEditorObjectController>());
			m_mapNodeV2->setMode(PG_DisaMapViewNode::Mode_Object);
		}break;
		case PG_DisaEditorModule::Mode_EventTile: {
			comp->setInteractionController(PG_new<PG_DisaEditorEventController>());
			m_mapNodeV2->setMode(PG_DisaMapViewNode::Mode_Event);
		}break;
		case PG_DisaEditorModule::Mode_Actor: {
			connect(m_mapNodeV2, &PG_DisaMapViewNode::isActorsVisibleChanged, comp, [this] {
				if (auto mod = getModule<PG_DisaEditorModule>()) {
					mod->clearHovered();
					mod->clearSelection();
				}
				});
			comp->setInteractionController(PG_new<PG_DisaEditorActorController>());
			m_mapNodeV2->setMode(PG_DisaMapViewNode::Mode_Actor);
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
				{PG_Material("M_MapGrid"), PG_Geometry("G_MapFloor", GL::Geometry::planeUVScaled(1000.f, 1000.f))}
				}, getRoot());
		}
		{
			auto axis = PG_new<PG_ModelNode>("N_Coods", "MD_Coods", std::vector<PG_Model::Surface>{ {
						PG_Material("M_SimpleColor"),
						PG_Geometry("G_Coords",
							{{
								GL::DataWrapper("position", std::vector<glm::vec3>{
								glm::vec3(0.0f), glm::vec3(10.0f, 0.0f, 0.0f),
								glm::vec3(0.0f), glm::vec3(0.0f, 10.0f, 0.0f),
								glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 10.0f)}),

								GL::DataWrapper("color", std::vector<glm::vec3>{
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
			m_mapNodeV2 = PG_new<PG_DisaMapViewNode>("PG_DisaMapViewNode", getRoot());

			connect(m_mapNodeV2, &PG_DisaMapViewNode::loaded, this, [this] {
				orbitCamera(0.f);
				});
		}

	}
}

void PG_DisaMapSceneView::copy(PG_DisaMapSceneView* scene) {
	assert(scene);
	if (getActiveCamera() && scene->getActiveCamera())
		getActiveCamera()->setTransformation(scene->getActiveCamera()->getTransformation());
	if (auto model = scene->m_mapNodeV2->getModel()) {
		m_mapNodeV2->setModel(model);
	}
}

void PG_DisaMapSceneView::orbitCamera(float value) {
	if (auto cam = getActiveCamera()) {
		PG_Volume bv;
		if (m_mapNodeV2->getTileView()) bv = m_mapNodeV2->getTileView()->getVolumeVisibilityHierarchy();
		if (!bv) bv = m_mapNodeV2->getVolumeVisibilityHierarchy();
		if (!bv) bv = getRoot()->getVolumeVisibilityHierarchy();
		if (!bv) return;

		glm::vec3 baseDir = glm::normalize(glm::vec3(1.f));
		float distance = 6.f;

		const glm::vec3 center = bv.getCenter();
		distance = glm::clamp(glm::distance(center, bv.getPoint(4)) * 2.f, 1.f, 6.f);

		const glm::vec3 dir = glm::rotateY(
			cam->getTranslation() - center,
			float(value) //angle (rad)
		);
			
		if (std::abs(glm::length(dir) - distance) > 0.001F) {
			cam->setTranslation(center + baseDir * distance);
		}
		else {
			cam->setTranslation(center + dir);
		}
	
		cam->lookAt(center);
		cam->updateModelMatrix();
	}
}

bool PG_DisaMapSceneView::event(PG_Event* e) {
	if (m_previewMode) {
		switch (e->getType())
		{
		case Event_Tick: {
			auto tick = static_cast<PG_TickEvent*>(e);
			orbitCamera(0.0001 * tick->getDeltaTime());
			return PG_Scene::event(e);
		}
		case Event_MouseMotion: {
			auto mouse = static_cast<PG_MouseMotionEvent*>(e);
			if (mouse->getButton() & PG_BUTTON_MMASK) {
				orbitCamera(-mouse->getPositionRelative().x / 100.0f);
			}
			return true;
		}
		case Event_Key:
		case Event_MouseButton:
		case Event_MouseWheel:
			return true;
		}
	}

	if (e->getType() == EEventTypes::Event_Render) {
		return false;
	}

	return PG_Scene::event(e);
}

void PG_DisaMapSceneView::setAxisVisible(bool visible) {
	if (auto actor = getRoot()->findFirstChild<PG_ModelNode>("N_Coods")) {
		actor->setVisible(visible);
	}	
	if (auto actor = getRoot()->findFirstChild<PG_ModelNode>("N_Floor")) {
		actor->setVisible(visible);
	}
}

bool hasEnding(const std::string& fullString, const std::string& ending) {
	if (fullString.length() >= ending.length()) 
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	else 
		return false;
}

void PG_DisaMapSceneView::setShowHiddenModel(bool visible) {
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

bool PG_DisaMapSceneView::isActorsVisible() const {
	return m_mapNodeV2->isActorsVisible();
}

void PG_DisaMapSceneView::setActorsVisible(bool visible) {
	m_mapNodeV2->setActorsVisible(visible);
}

bool PG_DisaMapSceneView::isGeoPanelsVisible() const {
	return m_mapNodeV2->isGeoVisible();
}

void PG_DisaMapSceneView::setGeoPanelsVisible(bool visible) {
	m_mapNodeV2->setGeoVisible(visible);
}

void PG_DisaMapSceneView::setEnableEditing(bool enable) {
	getModule<PG_DisaEditorModule>()->setMode(enable ? PG_DisaEditorModule::Mode_Terrain : PG_DisaEditorModule::Mode_View);
}

bool PG_DisaMapSceneView::isHDMode() const {
	return m_mapNodeV2->isHDMode();
}

void PG_DisaMapSceneView::setHDMode(bool state) {
	m_mapNodeV2->setHDMode(state);
}

void PG_DisaMapSceneView::exportGLTFClicked() {
	/*
	PG_DisaMapNode* map = getMapNode();
	if (!map || map->getCurretlyOpenedFile().isEmpty()) return;
	std::string filename = map->getCurretlyOpenedFile().getName() + ".gltf";
	nfdchar_t* outPath = nullptr;
	nfdresult_t result = NFD_SaveDialog("gltf", nullptr, filename.c_str(), &outPath);
	std::string path;
	if (outPath) {
		path = outPath;
		delete[] outPath;
	}
	if (result == NFD_OKAY) {
		PG_File file(path);
		const std::string ext = file.getFileExtensionFormated();
		if (ext.empty()) file += ".gltf";
		map->exportGLTF(file);
	}
	else if (result != NFD_CANCEL) {
		OUT_ERROR("SaveDialog error: " << NFD_GetError());
	}
	*/
}

void PG_DisaMapSceneView::addModelClicked(int index, glm::vec3 pos) {
	if (!m_mapNodeV2) return;

	auto model = m_mapNodeV2->getModel()->getModel();
	auto objects = model->getObjects();
	auto object = PG_new<PG_DisaObjectModel>("PG_DisaObjectModel", objects);
	object->setChunk(model->getChunks()->back());
	mpdfileChunkObject data = mpdfileChunkObject::createEmpty();
	data.model_id = index;
	const glm::ivec3 tilePos = PG_DisaObjectModel::toObjectTranslation(pos - model->getChunks()->back()->getTileTranslation());
	data.x = tilePos.x;
	data.y = tilePos.y;
	data.z = tilePos.z;
	object->setData(data);
	objects->push_back(object);
}

PG_DisaMapModel* PG_DisaMapSceneView::getMapModel() const {
	return m_mapNodeV2->getMapModel();
}

void PG_DisaMapSceneView::setModel(const std::shared_ptr<PG_UiStateBaseModel>& container) {
	m_mapNodeV2->setModel(std::dynamic_pointer_cast<PG_DisaMPDContainer>(container));
	if (auto mapModel = std::dynamic_pointer_cast<PG_DisaMapContainerModel>(container)) {
		m_mapNodeV2->getScene()->getActiveCamera()->setTransformation(mapModel->getCameraStartTransformation());
	}
}
