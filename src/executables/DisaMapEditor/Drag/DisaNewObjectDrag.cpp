#include "DisaNewObjectDrag.h"
#include "DisaDragUtil.h"
#include <Base/PG_Application.h>
#include <Base/PG_Lifeline.h>
#include <Base/Events/PG_MouseButtonEvent.h>
#include <Base/Events/PG_MouseMotionEvent.h>
#include <Scene/PG_ActorNode.h>
#include <Scene/PG_CameraNode.h>
#include <Scene/PG_FootlooseViewNode.h>
#include <Scene/Gui/PG_GuiItem.h>
#include <Disa/PG_DisaMapScene.h>
#include <Disa/PG_DisaMapModel.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/PG_DisaModelNode.h>
#include <Disa/map/PG_DisaEditorModule.h>
#include <Disa/map/PG_DisaObjectModel.h>
#include <Disa/map/PG_DisaObjectViewNode.h>
#include <Disa/map/controllers/PG_DisaEditorTileController.h>
#include <Models/PG_DisaObjectCatalogModel.h>
#include <Util/PG_RangesTransform.h>
#include <RootScene.h>

DisaNewObjectDrag::~DisaNewObjectDrag() {
	reset();
}

PG_DisaMapScene* DisaNewObjectDrag::findGameScene() const {
	auto root = findParent<RootScene>();
	if (!root) return nullptr;
	auto render = root->getRoot()->findFirstInTree(PG_Search::byName{ "GridRenderItem" });
	if (!render) return nullptr;
	return render->findFirstInTree<PG_DisaMapScene>();
}

void DisaNewObjectDrag::start(const PG_ListModelLocation& location) {
	if (m_dragging || !location) return;

	PG_DisaMapScene* gameScene = findGameScene();
	if (!gameScene) return;
	auto* mapModel = gameScene->getMapModel();
	if (!mapModel || mapModel->getChunks()->empty()) return;

	m_model = location.getDataT<PG_Model>(PG_Identity_PreviewImage);
	m_geometryIndex = location.getDataT<int>(PG_DisaObjectCatalogModel::Identity_GeometryIndex);
	m_resourceName = location.getDataT<std::string>(PG_DisaObjectCatalogModel::Identity_ResourceName);
	if (!m_model.isValid()) return;

	//new objects go into the last chunk, see PG_DisaMapScene::addModel
	m_objectMoc = PG_new<PG_DisaObjectModel>("ObjectMoc", this);
	m_objectMoc->setData(mpdfileChunkObject::createEmpty());
	m_objectMoc->setChunk(mapModel->getChunks()->back());

	m_indicator = PG_new<PG_ActorNode>("ObjectIndicator", gameScene->getRoot());
	m_indicator->setScale(glm::vec3(PG_DisaObjectModel::getBaseScale()));
	m_indicator->setVisible(false); //stays hidden until the first mouse move places it
	connect(m_indicator, &PG_Object::onDeleted, this, &DisaNewObjectDrag::onIndicatorDeleted);

	m_indicatorModel = PG_new<PG_DisaModelNode>("PG_DisaModelNode", m_indicator);
	//a resource may still be loading, and only a loaded model can be made see through
	connect(m_indicatorModel, &PG_ModelNode::loaded, m_indicatorModel, &PG_DisaModelNode::makeTransparent);
	m_indicatorModel->setModel(m_model);
	m_indicatorModel->makeTransparent();

	simulateMouseRelease();

	PG_Application::grabMouse(this);
	m_dragging = true;
}

/**
* If returns true, then event will not be propagate farther in tree.
*/
bool DisaNewObjectDrag::event(PG_Event* e) {
	if (m_dragging) {
		switch (e->getType()) {
		case PG_Event_MouseMotion:
			mouseMoved(static_cast<PG_MouseMotionEvent*>(e));
			e->accept();
			return true;
		case PG_Event_MouseButton: {
			auto key = static_cast<PG_MouseButtonEvent*>(e);
			if (key->getButton() == PG_BUTTON_LEFT) {
				m_dragging = false;
				postCallEvent(&DisaNewObjectDrag::released);
			}
			else if (key->getButton() == PG_BUTTON_RIGHT) { //cancel
				m_dragging = false;
				postCallEvent(&DisaNewObjectDrag::reset);
			}
			e->accept();
			return true;
		}
		case PG_Event_Key:
			e->accept();
			return true;
		default: break;
		}
	}
	return PG_Object::event(e);
}

void DisaNewObjectDrag::mouseMoved(PG_MouseMotionEvent* e) {
	if (!m_indicator || !m_objectMoc) return;
	auto root = findParent<RootScene>();
	if (!root) return;
	auto render = dynamic_cast<PG_GuiItem*>(root->getRoot()->findFirstInTree(PG_Search::byName{ "GridRenderItem" }));
	if (!render) return;
	PG_DisaMapScene* gameScene = render->findFirstInTree<PG_DisaMapScene>();
	if (!gameScene || !gameScene->getActiveCamera()) return;

	const PG_Ray ray = gameScene->getActiveCamera()->getRay(render->toLocalTranslation(e->getPosition()), render->getSize());
	const glm::vec3 hit = PG_DisaEditorTileController::findCollisionPoint(ray, gameScene->getRoot());
	m_position = hit;

	//round trip through the object grid, so the indicator sits where the object will
	m_objectMoc->setObjectDisaTranslation(PG_DisaObjectModel::toObjectTranslation(hit - m_objectMoc->getChunkTranslation()));
	m_indicator->setTranslation(m_objectMoc->getObjectTranslation());
	m_indicator->setVisible(true);
}

void DisaNewObjectDrag::released() {
	PG_DisaMapScene* scene = findGameScene();
	const std::optional<glm::vec3> position = m_position;
	const std::string resourceName = m_resourceName;
	const int geometryIndex = m_geometryIndex;
	reset();
	if (!scene || !position) return;

	PG_DisaObjectModel* object = resourceName.empty() ? scene->addModel(geometryIndex, *position) : scene->addResourceModel(resourceName, *position);
	if (!object) return;

	auto* view = scene->getMapNode()->getObjectsView();
	view->flush();
	PG_DisaObjectViewNode* node = nullptr;
	for (auto* item : view->getItems() | PG_CastTo<PG_DisaObjectViewNode*>) {
		if (item->getObjectModel() == object) {
			node = item;
			break;
		}
	}
	if (!node) return;

	/* Selecting rebuilds the inspector, which deletes the grid this drag lives under. So it happens
	 * after this call returned, from the scene, which outlives the grid. */
	scene->postCallEvent([scene, target = PG_Lifeline_T<PG_ActorNode>(node)] {
		auto* editor = scene->getModule<PG_DisaEditorModule>();
		if (editor && target) editor->setSelection(target.get());
	});
}

void DisaNewObjectDrag::onIndicatorDeleted() {
	m_indicator = nullptr;
	m_indicatorModel = nullptr;
}

void DisaNewObjectDrag::reset() {
	if (m_indicatorModel) m_indicatorModel->reverseTransparent(); //the materials belong to the shared model
	if (m_indicator) delete m_indicator;
	m_indicator = nullptr;
	m_indicatorModel = nullptr;
	if (m_objectMoc) delete m_objectMoc;
	m_objectMoc = nullptr;

	m_model = PG_Model();
	m_resourceName.clear();
	m_geometryIndex = -1;
	m_position.reset();

	if (PG_Application::grabbedMouse() == this) PG_Application::grabMouse(nullptr);
	m_dragging = false;
}
