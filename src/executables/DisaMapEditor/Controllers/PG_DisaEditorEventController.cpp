#include "PG_DisaEditorEventController.h"
#include <Base/Events/PG_MouseButtonEvent.h>
#include <Base/Events/PG_MouseMotionEvent.h>
#include <Scene/PG_ActorNode.h>
#include <Scene/PG_ModelNode.h>
#include <Scene/Editor/PG_EditorModule.h>
#include <Disa/map/PG_DisaEventEditSelectionNode.h>
#include <Disa/map/PG_DisaIntersection.h>
#include <Disa/map/PG_DisaTileViewNode.h>
#include <Disa/PG_DisaMapViewNode.h>

#include <Operation/PG_DisaEditMaterialOperation.h>
#include <unordered_map>

constexpr glm::vec3 g_floorNormal = glm::vec3(0.f, 1.f, 0.f);

PG_DisaEditorEventController::PG_DisaEditorEventController() {

	connect(this, &PG_DisaEditorEventController::editorChanged, this,
		[this](PG_EditorModule* moduleOld, PG_EditorModule* moduleNew) {

			if (moduleOld) {
				disconnect(moduleOld, &PG_EditorModule::hoveredChanged, this, &PG_DisaEditorEventController::onHoverChanged);
				disconnect(moduleOld, &PG_EditorModule::selectionChanged, this, &PG_DisaEditorEventController::onSelectionChanged);
			}

			if (moduleNew) {
				connect(moduleNew, &PG_EditorModule::hoveredChanged, this, &PG_DisaEditorEventController::onHoverChanged);
				connect(moduleNew, &PG_EditorModule::selectionChanged, this, &PG_DisaEditorEventController::onSelectionChanged);
			}

		});
}

PG_DisaEditorEventController::~PG_DisaEditorEventController() {
	if (auto* editor = getEditor()) {
		editor->clearHovered();
		editor->clearSelection();
	}
}


bool PG_DisaEditorEventController::isSelectable(const PG_EditorActorInfo& actor) const {
	return (bool)dynamic_cast<PG_DisaTileViewNode*>(*actor) || (bool)dynamic_cast<PG_DisaMapViewNode*>(*actor);
}
bool PG_DisaEditorEventController::isDragable(const PG_EditorActorInfo& actor) const {
	return false; // (bool)dynamic_cast<PG_DisaTileViewNode*>(*actor) || (bool)dynamic_cast<PG_DisaMapViewNode*>(*actor);
}
void PG_DisaEditorEventController::createGizmo(PG_CameraNode* cam, PG_Scene* scene) const {}


PG_DisaEditorEventController::Found PG_DisaEditorEventController::findObjects(PG_CameraNode* cam, const PG_Ray& ray, PG_ActorNode* root) const {
	assert(root);
	assert(!glm::any(glm::isnan(ray.dir)));
	assert(!glm::any(glm::isnan(ray.orig)));
	constexpr float maxDistance = 50.f;
	Found out;
	root->traverseTest([&](PG_Object* obj) {
		if (auto* actor = dynamic_cast<PG_ActorNode*>(obj)) {
			if (dynamic_cast<PG_DisaMapViewNode*>(actor)) {
				if (auto intersection = ray.intersectPlane(glm::vec3(0.f), g_floorNormal); intersection && intersection.value() < maxDistance) {
					//add floor intersetion so we can insert new tiles
					glm::vec3 intersectionPoint = ray.getPointOnRay(intersection.value());
					PG_DisaEmptyTileHit hit{ glm::ivec2(std::floor(intersectionPoint.x), std::floor(intersectionPoint.z)) };
					if (hit.tilePos.x < -128 || hit.tilePos.x > 128 || hit.tilePos.y < -128 || hit.tilePos.y > 128) return true; //out of map bounds
					out.emplace_back(PG_RayIntersection{ intersection.value(), g_floorNormal }, PG_EditorActorInfo(actor, PG_Variety(hit)));
				}

			}
			else if (auto* tile = dynamic_cast<PG_DisaTileViewNode*>(actor); tile && tile->getLocation()) {
				if (const auto& volume = tile->getVolumeCollision(); volume && volume.intersect(ray)) {
					auto tileInfo = PG_DisaFaceHit::intersection(tile, ray, false);
					if (tileInfo.first.distance < maxDistance && tileInfo.second) {
						out.push_back(std::move(tileInfo));
					}
				}
				return false;
			}
			return true;
		}
		return false;
		});
	return out;
}

PG_DisaEditorEventController::FoundItem PG_DisaEditorEventController::choose(const Found& found) const {
	if (found.empty()) return FoundItem{ PG_RayIntersection::fail(), nullptr };

	return *std::min_element(found.begin(), found.end(), [](const FoundItem& a, const FoundItem& b)->bool {
		return  a.first.distance < b.first.distance;
		});
}

static inline std::unordered_map<glm::ivec2, bool, PG_iVec2Hasher> assignToMap(const std::set<PG_EditorActorInfo>& deselected, const std::set<PG_EditorActorInfo>& selected) {
	std::unordered_map<glm::ivec2, bool, PG_iVec2Hasher> faceHovers; //tile, hovered, hit

	for (const PG_EditorActorInfo& info : deselected) {
		if (!info.additionalData) continue;

		if (auto tile = dynamic_cast<PG_DisaTileViewNode*>(*info)) {
			auto hit = info.additionalData.get<PG_DisaFaceHit>();
			faceHovers[tile->getTilePosition()] =  false;
		}else if (dynamic_cast<PG_DisaMapViewNode*>(*info)) {
			auto hit = info.additionalData.get<PG_DisaEmptyTileHit>();
			faceHovers[hit.tilePos] = false;
		}
	}

	for (const PG_EditorActorInfo& info : selected) {
		if (!info.additionalData) continue;

		if (auto tile = dynamic_cast<PG_DisaTileViewNode*>(*info)) {
			auto hit = info.additionalData.get<PG_DisaFaceHit>();
			faceHovers[tile->getTilePosition()] = true;
		}
		else if (dynamic_cast<PG_DisaMapViewNode*>(*info)) {
			auto hit = info.additionalData.get<PG_DisaEmptyTileHit>();
			faceHovers[hit.tilePos] = true;
		}
	}

	return faceHovers;
}

void PG_DisaEditorEventController::onSelectionChanged(const std::set<PG_EditorActorInfo>& deselected, const std::set<PG_EditorActorInfo>& selected) {
	PG_DisaEventEditSelectionHandlerNode* selectionNode = getScene()->getRoot()->findFirstInTree<PG_DisaEventEditSelectionHandlerNode>();
	if (!selectionNode) return;

	for (const auto& [tile, value] : assignToMap(deselected, selected)) {
		selectionNode->setSelected(tile, value, PG_DisaFaceHit{ PG_DisaFaceHit::Face_Top, PG_DisaFaceHit::Center});
	}
}

void PG_DisaEditorEventController::onHoverChanged(const std::set<PG_EditorActorInfo>& unhovered, const std::set<PG_EditorActorInfo>& hovered) {
	PG_DisaEventEditSelectionHandlerNode* selectionNode = getScene()->getRoot()->findFirstInTree<PG_DisaEventEditSelectionHandlerNode>();
	if (!selectionNode) return;

	for (const auto& [tile, value] : assignToMap(unhovered, hovered)) {
		selectionNode->setHovered(tile, value, PG_DisaFaceHit{ PG_DisaFaceHit::Face_Top, PG_DisaFaceHit::Center });
	}

}
