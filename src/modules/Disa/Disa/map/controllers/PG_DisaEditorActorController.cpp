#include "PG_DisaEditorActorController.h"

#include <Scene/PG_FootlooseViewNode.h>
#include <Base/Events/PG_MouseButtonEvent.h>
#include <Base/Events/PG_MouseMotionEvent.h>
#include <Base/Events/PG_KeyEvent.h>

#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/map/PG_DisaActorSpriteNode.h>
#include <Disa/map/PG_DisaTileViewNode.h>
#include <Disa/map/PG_DisaActorModel.h>
#include <Disa/map/PG_DisaIntersection.h>
#include <Disa/map/PG_DisaActorEditSelectionNode.h>
#include <Scene/Editor/PG_EditorModule.h>
#include <Scene/Editor/PG_EditorCameraNode.h>
#include <Disa/PG_DisaMapScene.h>
#include <Util/PG_RangesTransform.h>

PG_DisaEditorActorController::PG_DisaEditorActorController() {

	connect(this, &PG_DisaEditorActorController::editorChanged, this,
		[this](PG_EditorModule* moduleOld, PG_EditorModule* moduleNew) {

			if (moduleOld) {
				disconnect(moduleOld, &PG_EditorModule::hoveredChanged, this, &PG_DisaEditorActorController::onHoverChanged);
				disconnect(moduleOld, &PG_EditorModule::selectionChanged, this, &PG_DisaEditorActorController::onSelectionChanged);
			}

			if (moduleNew) {
				connect(moduleNew, &PG_EditorModule::hoveredChanged, this, &PG_DisaEditorActorController::onHoverChanged);
				connect(moduleNew, &PG_EditorModule::selectionChanged, this, &PG_DisaEditorActorController::onSelectionChanged);
			}

		});

}

bool PG_DisaEditorActorController::isSelectable(const PG_EditorActorInfo& actor) const {
	return (bool)dynamic_cast<PG_DisaActorSpriteNode*>(*actor);
}

bool PG_DisaEditorActorController::isDragable(const PG_EditorActorInfo& actor) const {
	return (bool)dynamic_cast<PG_DisaActorSpriteNode*>(*actor);
}

void PG_DisaEditorActorController::createGizmo(PG_CameraNode* cam, PG_Scene* scene) const {

}

void PG_DisaEditorActorController::onSelectionChanged(const std::set<PG_EditorActorInfo>& deselected, const std::set<PG_EditorActorInfo>& selected) {
	PG_DisaActorEditSelectionHandlerNode* selectionNode = getScene()->getRoot()->findFirstInTree<PG_DisaActorEditSelectionHandlerNode>();

	for (const PG_EditorActorInfo& info : deselected) {
		auto actor = dynamic_cast<PG_DisaActorSpriteNode*>(info.actor.get());
		if (!actor) continue;
		auto* model = dynamic_cast<PG_DisaActorModel*>(actor->getObjectModel());
		selectionNode->setSelected(model, false);
	}

	for (const PG_EditorActorInfo& info : selected) {
		auto actor = dynamic_cast<PG_DisaActorSpriteNode*>(info.actor.get());
		if (!actor) continue;
		auto* model = dynamic_cast<PG_DisaActorModel*>(actor->getObjectModel());
		selectionNode->setSelected(model, true);
	}
}

void PG_DisaEditorActorController::onHoverChanged(const std::set<PG_EditorActorInfo>& unhovered, const std::set<PG_EditorActorInfo>& hovered) {
	PG_DisaActorEditSelectionHandlerNode* selectionNode = getScene()->getRoot()->findFirstInTree<PG_DisaActorEditSelectionHandlerNode>();

	for (const PG_EditorActorInfo& info : unhovered) {
		auto actor = dynamic_cast<PG_DisaActorSpriteNode*>(info.actor.get());
		if (!actor) continue;
		auto* model = dynamic_cast<PG_DisaActorModel*>(actor->getObjectModel());
		selectionNode->setHovered(model, false);
	}

	for (const PG_EditorActorInfo& info : hovered) {
		auto actor = dynamic_cast<PG_DisaActorSpriteNode*>(info.actor.get());
		if (!actor) continue;
		auto* model = dynamic_cast<PG_DisaActorModel*>(actor->getObjectModel());
		selectionNode->setHovered(model, true);
	}
}

static PG_Volume g_emptyVolume;
static inline const PG_Volume& extractVolume(PG_ActorNode* actor) {
	assert(actor);
	if (const auto& volume = actor->getVolumeCollision()) return volume;
	if (const auto& volume = actor->getVolumeVisibility()) return volume;
	return g_emptyVolume;
}

PG_DisaEditorActorController::Found PG_DisaEditorActorController::findObjects(PG_CameraNode* cam, const PG_Ray& ray, PG_ActorNode* root) const {
	assert(root);
	assert(!glm::any(glm::isnan(ray.dir)));
	assert(!glm::any(glm::isnan(ray.orig)));
	Found out;
	root->traverseTest([&](PG_Object* obj) {
		if (auto* actor = dynamic_cast<PG_ActorNode*>(obj)) {
			if (dynamic_cast<PG_FootlooseViewNode*>(actor) && actor->getName() != "PG_DisaActorsViewNode") return false;

			if (auto node = dynamic_cast<PG_DisaActorSpriteNode*>(actor); node && node->isVisible()) {
				if (auto intersection = extractVolume(node).intersect(ray)) {
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
PG_DisaEditorActorController::FoundItem PG_DisaEditorActorController::choose(const Found& found) const {
	if (found.empty()) return FoundItem{ PG_RayIntersection::fail(), nullptr };

	auto minElement = *std::min_element(found.begin(), found.end(), [](const FoundItem& a, const FoundItem& b)->bool {
		return  a.first.distance < b.first.distance;
		});

	return minElement;
}

void PG_DisaEditorActorController::clicked(PG_EditorCameraComponent* comp, PG_MouseButtonEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, PG_CameraNode* cam, const PG_Ray& ray) {
	if (!comp->getHovered() || editorModule->isPicking()) return;
	auto actor = dynamic_cast<PG_DisaActorSpriteNode*>(comp->getHovered().get());
	if (!actor) return;
	auto model = dynamic_cast<PG_DisaActorModel*>(actor->getObjectModel());
	if (!model) return;
	if (e->getButton() == PG_BUTTON_RIGHT) {
		if (actor->isDragged()) {
			actor->setDragged(false);
			model->setTilePosition(m_startTilePos);
		}
	}else{
		m_startTilePos = model->getTilePosition();
	}
}

PG_DisaTileViewNode* PG_DisaEditorActorController::findMinTile(const PG_Scene* scene, const PG_Ray& ray, bool includeSides) {
	//find free intersection tile
	PG_DisaTileViewNode* minTile = nullptr;
	float minDist = std::numeric_limits<float>::max();
	for (PG_DisaTileViewNode* tile : dynamic_cast<const PG_DisaMapScene*>(scene)->getMapNode()->getTileView()->getItems() | PG_CastTo<PG_DisaTileViewNode*>) {
		if (auto dist = PG_DisaFaceHit::intersection(tile, ray, includeSides); dist.first && dist.first.distance > 0.1f && minDist > dist.first.distance) {
			minDist = dist.first.distance;
			minTile = tile;
		}
	}
	if (!minTile || minDist > 100000.0F) return nullptr;
	return minTile;
}

void PG_DisaEditorActorController::drag(PG_EditorCameraComponent* comp, PG_MouseMotionEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, PG_CameraNode* cam, const PG_Ray& ray) {
	assert(comp);
	if (!comp->getHovered() || editorModule->isPicking()) return;
	auto actor = dynamic_cast<PG_DisaActorSpriteNode*>(comp->getHovered().get());
	if (!actor) return;
	auto model = dynamic_cast<PG_DisaActorModel*>(actor->getObjectModel());
	if (!model) return;

	if (auto* tile = findMinTile(scene, ray)) {
		actor->setDragged(true);
		model->setTilePosition(tile->getTilePosition());
	}
}

void PG_DisaEditorActorController::release(PG_EditorCameraComponent* comp, PG_MouseButtonEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, PG_CameraNode* cam, const PG_Ray& ray) {
	if (!comp->getHovered() || editorModule->isPicking()) return;
	auto actor = dynamic_cast<PG_DisaActorSpriteNode*>(comp->getHovered().get());
	if (!actor) return;

	if (BUTTON_BIT(e->getButton()) & PG_BUTTON_RMASK) {
		auto model = dynamic_cast<PG_DisaActorModel*>(actor->getObjectModel());
		if (!model) return;
		model->setTilePosition(m_startTilePos);
		editorModule->clearHovered();
	}
	actor->setDragged(false);
}

void PG_DisaEditorActorController::key(PG_EditorCameraComponent* comp, PG_KeyEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, PG_CameraNode* cam) {
	if (e->getKey() == PG_Key_Delete) {
		if(e->isUp()){
			auto selection = editorModule->getSelection();
			editorModule->clearHovered();
			editorModule->clearSelection();

			for (const PG_EditorActorInfo& selected : selection) {
				auto actor = dynamic_cast<PG_DisaActorSpriteNode*>(*selected);
				if (!actor) continue;
				auto model = dynamic_cast<PG_DisaActorModel*>(actor->getObjectModel());
				if (!model) return;
				model->remove();
			}
		}
		e->accept();
	}
}