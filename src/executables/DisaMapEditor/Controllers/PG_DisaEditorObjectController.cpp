#include "PG_DisaEditorObjectController.h"

#include <Base/Events/PG_MouseButtonEvent.h>
#include <Base/Events/PG_MouseMotionEvent.h>
#include <Base/Events/PG_KeyEvent.h>
#include <Disa/map/PG_DisaObjectViewNode.h>
#include <Disa/map/PG_DisaObjectModel.h>
#include <Disa/PG_DisaModelNode.h>

#include <Scene/PG_NodeTypes.h>
#include <Scene/Editor/PG_EditorModule.h>
#include <Scene/Editor/PG_EditorCameraNode.h>
#include <Scene/Editor/Handles/PG_HandleAxis.h>
#include <Scene/Editor/Handles/PG_HandleGizmo.h>

#include <Models/PG_DisaMapEditorMainModel.h>
#include <Views/PG_DisaMapSceneView.h>
#include <Controllers/PG_DisaEditorTileController.h>

PG_DisaEditorObjectController::PG_DisaEditorObjectController() {

	connect(this, &PG_DisaEditorObjectController::editorChanged, this,
		[this](PG_EditorModule* moduleOld, PG_EditorModule* moduleNew) {

			if (moduleOld) {
				disconnect(moduleOld, &PG_EditorModule::hoveredChanged, this, &PG_DisaEditorObjectController::onHoverChanged);
				disconnect(moduleOld, &PG_EditorModule::selectionChanged, this, &PG_DisaEditorObjectController::onSelectionChanged);
			}

			if (moduleNew) {
				connect(moduleNew, &PG_EditorModule::hoveredChanged, this, &PG_DisaEditorObjectController::onHoverChanged);
				connect(moduleNew, &PG_EditorModule::selectionChanged, this, &PG_DisaEditorObjectController::onSelectionChanged);
			}

		});

	connect(PG_DisaMapEditorMainModel::get(), &PG_DisaMapEditorMainModel::addModelPressed, this, [this](int index) {
		if (auto scene = dynamic_cast<PG_DisaMapSceneView*>(getEditor()->getScene())) {
			auto ray = getEditor()->getScene()->getActiveCamera()->getRayViewDir();
			const glm::vec3 point = PG_DisaEditorTileController::findCollisionPoint(ray, getEditor()->getScene()->getRoot());
			scene->addModelClicked(index, point);
		}
	});

	connect(PG_DisaMapEditorMainModel::get(), &PG_DisaMapEditorMainModel::dublicateModelPressed, this, [this](int modelIndex) {
		if (auto scene = dynamic_cast<PG_DisaMapSceneView*>(getEditor()->getScene())) {
			scene->addDublicateClicked(modelIndex);
		}
		});
}

PG_DisaEditorObjectController::~PG_DisaEditorObjectController() {
	if (auto* editor = getEditor()) {
		editor->clearHovered();
		editor->clearSelection();
		editor->getScene()->getRoot()->clearChildrenByType({ PG_Type::Node_HandleAxis });
	}
	for (auto* model : m_selectedBoxs) {
		model->deleteLater();
	}
}

bool PG_DisaEditorObjectController::isSelectable(const PG_EditorActorInfo& actor) const {
	return (bool)dynamic_cast<PG_DisaObjectViewNode*>(*actor);
}
bool PG_DisaEditorObjectController::isDragable(const PG_EditorActorInfo& actor) const {
	return (bool)dynamic_cast<PG_HandleGizmo*>(*actor);
}
void PG_DisaEditorObjectController::createGizmo(PG_CameraNode* cam, PG_Scene* scene) const {
	if (!scene || !getEditor()) return;

	auto root = scene->getRoot();

	auto editor = getEditor();
	if (cam && editor->hasSelection()) {
		PG_HandleAxis* axis = root->findFirstChild<PG_HandleAxis>();
		if (!axis) axis = PG_new<PG_HandleAxis>("N_Axis", root);
		axis->setScale(glm::vec3(0.3f));
		axis->setEditType(editor->getEditMode());
		axis->center(editor);
	}
	else {
		root->clearChildrenByType({ PG_Type::Node_HandleAxis });
	}
}

static PG_Volume g_emptyVolume;
static inline const PG_Volume& extractVolume(PG_ActorNode* actor) {
	assert(actor);
	if (const auto& volume = actor->getVolumeCollision()) return volume;
	if (const auto& volume = actor->getVolumeVisibility()) return volume;
	return g_emptyVolume;
}

PG_DisaEditorObjectController::Found PG_DisaEditorObjectController::findObjects(PG_CameraNode* cam, const PG_Ray& ray, PG_ActorNode* root) const {
	assert(root);
	assert(!glm::any(glm::isnan(ray.dir)));
	assert(!glm::any(glm::isnan(ray.orig)));
	Found out;
	root->traverseTest([&](PG_Object* obj) {
		if (auto* actor = dynamic_cast<PG_ActorNode*>(obj)) {

			if (auto node = dynamic_cast<PG_DisaObjectViewNode*>(actor); node && node->isVisible()) {
				auto model = node->getModelNode();
				if(!model) return false;
				if (auto intersection = extractVolume(model).intersect(ray)) {
					out.emplace_back(intersection, actor);
				}
				return false;
			} else if (auto gizmo = dynamic_cast<PG_HandleGizmo*>(actor)) {
				if (auto intersection = extractVolume(actor).intersect(ray); intersection && !gizmo->filterSelection(nullptr, cam, ray)) {
					out.emplace_back(intersection, actor);
				}
				return false;
			}
			return true;
		}
		return false;
		});
	return out;
}

PG_DisaEditorObjectController::FoundItem PG_DisaEditorObjectController::choose(const Found& found) const {
	if (found.empty()) return FoundItem{ PG_RayIntersection::fail(), nullptr };

	return *std::min_element(found.begin(), found.end(), [](const FoundItem& a, const FoundItem& b)->bool {
		const bool isAGizmo = dynamic_cast<PG_HandleGizmo*>(a.second.actor.get());
		const bool isBGizmo = dynamic_cast<PG_HandleGizmo*>(b.second.actor.get());
		if (isAGizmo != isBGizmo) {
			return isAGizmo > isBGizmo;
		}
		return  a.first.distance < b.first.distance;
		});
}

void PG_DisaEditorObjectController::onSelectionChanged(const std::set<PG_EditorActorInfo>& deselected, const std::set<PG_EditorActorInfo>& selected) {
	updateSelectionBox();
}

void PG_DisaEditorObjectController::onHoverChanged(const std::set<PG_EditorActorInfo>& unhovered, const std::set<PG_EditorActorInfo>& hovered) {
	updateHoverBox();
}

void PG_DisaEditorObjectController::clicked(PG_EditorCameraComponent* comp, PG_MouseButtonEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, PG_CameraNode* cam, const PG_Ray& ray) {
	if (comp->getHovered() && !editorModule->isPicking()) {
		if (auto gizmo = dynamic_cast<PG_HandleGizmo*>(comp->getHovered().get()))
			gizmo->click(editorModule, cam, ray);
	}
}

void PG_DisaEditorObjectController::drag(PG_EditorCameraComponent* comp, PG_MouseMotionEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, PG_CameraNode* cam, const PG_Ray& ray) {
	assert(comp);
	if (comp->getHovered() && !editorModule->isPicking()) {
		if (auto gizmo = dynamic_cast<PG_HandleGizmo*>(comp->getHovered().get()))
			gizmo->interact(editorModule, cam, ray, comp->getDragOffset());
	}
}

void PG_DisaEditorObjectController::key(PG_EditorCameraComponent* comp, PG_KeyEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, PG_CameraNode* cam) {
	if (e->getKey() == PG_Key_Delete) {
		if (e->isUp()) {
			auto selection = editorModule->getSelection();
			editorModule->clearHovered();
			editorModule->clearSelection();

			for (const PG_EditorActorInfo& selected : selection) {
				auto actor = dynamic_cast<PG_DisaObjectViewNode*>(*selected);
				if (!actor) continue;
				auto model = dynamic_cast<PG_DisaObjectModel*>(actor->getObjectModel());
				if (!model) return;
				model->remove();
			}
		}
		e->accept();
	}
}

static inline void addLines(std::vector<glm::vec4>& vertices, const std::vector<glm::vec3>& points) {
	if (points.size() != 8) return;

	vertices.insert(vertices.end(), {
		glm::vec4(points[0],1.f),glm::vec4(points[1],1.f),
		glm::vec4(points[1],1.f),glm::vec4(points[2],1.f),
		glm::vec4(points[2],1.f),glm::vec4(points[3],1.f),
		glm::vec4(points[3],1.f),glm::vec4(points[0],1.f),

		glm::vec4(points[4],1.f),glm::vec4(points[5],1.f),
		glm::vec4(points[5],1.f),glm::vec4(points[6],1.f),
		glm::vec4(points[6],1.f),glm::vec4(points[7],1.f),
		glm::vec4(points[7],1.f),glm::vec4(points[4],1.f),

		glm::vec4(points[0],1.f),glm::vec4(points[4],1.f),
		glm::vec4(points[1],1.f),glm::vec4(points[5],1.f),
		glm::vec4(points[2],1.f),glm::vec4(points[6],1.f),
		glm::vec4(points[3],1.f),glm::vec4(points[7],1.f),
		});
}

static inline void getLines(std::vector<glm::vec4>& vertices, const PG_EditorActorInfo& actor, bool local = false) {

	if (!actor) return;
	PG_ModelNode* model = nullptr;
	if (auto disaObj = dynamic_cast<PG_DisaObjectViewNode*>(actor.actor.get())) {
		model = disaObj->getModelNode();
	}
	else {
		model = dynamic_cast<PG_ModelNode*>(actor.actor.get());
	}
	if (!model) return;

	if (const auto& bv = model->getVolumeCollision()) {
		auto points = bv.getPoints();
		if (local) {
			const glm::mat4 mat = glm::inverse(actor.actor->getModelMatrixWorld());
			for (glm::vec3& v : points) v = mat * glm::vec4(v, 1.f);
		}
		addLines(vertices, points);
	}

}

static inline std::vector<glm::vec4> getLines(const std::set<PG_EditorActorInfo>& actors) {
	std::vector<glm::vec4> vertices;
	for (const PG_EditorActorInfo& actor : actors) {
		if (dynamic_cast<PG_HandleGizmo*>(actor.actor.get())) continue;
		getLines(vertices, actor);
	}
	return vertices;
}

void PG_DisaEditorObjectController::updateHoverBox() {
	if (!getEditor()) {
		if (m_hoverBox) delete m_hoverBox;
		m_hoverBox = nullptr;
		return;
	}


	std::vector<glm::vec4> vertices = getLines(getEditor()->getHovered());
	if (vertices.empty()) {
		if (m_hoverBox) m_hoverBox->setVisible(false);
		return;
	}

	if (!m_hoverBox)
		m_hoverBox = PG_new<PG_ModelNode>("N_HoverLine", this);

	m_hoverBox->setModel(PG_Model(PG_UNIQUE_RES("D_HoverLine"), { {PG_Material("M_HoverLine"), PG_Geometry(PG_UNIQUE_RES("G_HoverLine"), PG_Mesh::lines(std::move(vertices)), GL::LINES)} }));
	m_hoverBox->setVisible(true);
}

void PG_DisaEditorObjectController::updateSelectionBox() {
	if (!getEditor()) {
		for (auto* box : m_selectedBoxs) {
			disconnect(box, nullptr, this, nullptr);
			box->deleteLater();
		}
		m_selectedBoxs.clear();
		return;
	}

	std::vector<PG_ModelNode*> newSelectionBoxes;
	for (const auto& actor : getEditor()->getSelection()) {
		PG_ModelNode* selection = actor->findFirstChild<PG_ModelNode>("N_DisaSelectionLine");
		if (!selection) {
			selection = PG_new<PG_ModelNode>("N_SelectionLine", actor.actor.get());
			connect(selection, &PG_Object::onDeleted, this, [this]() {
				auto findIt = std::ranges::find(m_selectedBoxs, PG_Object::caller());
				if(findIt != m_selectedBoxs.end()) m_selectedBoxs.erase(findIt);
			});
			std::vector<glm::vec4> vertices;
			getLines(vertices, actor, true);

			selection->setModel(PG_Model(PG_UNIQUE_RES("D_SelectionLine"), { {PG_Material("M_SelectionLine"), PG_Geometry(PG_UNIQUE_RES("G_SelectionLine"), PG_Mesh::lines(std::move(vertices)), GL::LINES)} }));
		}
		else {
			auto findIt = std::ranges::find(m_selectedBoxs, selection);
			m_selectedBoxs.erase(findIt);
		}
		newSelectionBoxes.push_back(selection);
	}

	for (auto* box : m_selectedBoxs) {
		disconnect(box, nullptr, this, nullptr);
		box->deleteLater();
	}
	m_selectedBoxs = std::move(newSelectionBoxes);

}

std::vector<PG_DisaEditorObjectController::RenderTarget> PG_DisaEditorObjectController::getInteractionRender() const {
	std::vector<PG_DisaEditorObjectController::RenderTarget> out;
	if (m_hoverBox && m_hoverBox->isVisible())
		out.push_back({ m_hoverBox, m_hoverBox->getModel().getMaterial(0) });
	//if (m_selectedBox && m_selectedBox->isVisible())
		//out.push_back({ m_selectedBox, m_selectedBox->getModel().getMaterial(0) });
	return out;
}
