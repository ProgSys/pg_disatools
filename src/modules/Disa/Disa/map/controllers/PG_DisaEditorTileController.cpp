#include "PG_DisaEditorTileController.h"
#include <Base/Events/PG_MouseButtonEvent.h>
#include <Base/Events/PG_MouseMotionEvent.h>
#include <Base/Events/PG_KeyEvent.h>
#include <Scene/PG_ActorNode.h>
#include <Scene/PG_ModelNode.h>
#include <Scene/Editor/PG_EditorModule.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/PG_DisaMapModel.h>
#include <Disa/map/PG_DisaTileViewNode.h>
#include <Disa/map/PG_DisaTileEditSelectionNode.h>
#include <Disa/map/PG_DisaIntersection.h>

#include <Disa/map/operations/PG_DisaEditTerrainOperation.h>
#include <unordered_map>

constexpr glm::vec3 g_floorNormal = glm::vec3(0.f, 1.f, 0.f);

PG_DisaEditorTileController::PG_DisaEditorTileController(){

	connect(this, &PG_DisaEditorTileController::editorChanged, this,
		[this](PG_EditorModule* moduleOld, PG_EditorModule* moduleNew) {

			if (moduleOld) {
				disconnect(moduleOld, &PG_EditorModule::hoveredChanged, this, &PG_DisaEditorTileController::onHoverChanged);
				disconnect(moduleOld, &PG_EditorModule::selectionChanged, this, &PG_DisaEditorTileController::onSelectionChanged);
			}

			if (moduleNew) {
				connect(moduleNew, &PG_EditorModule::hoveredChanged, this, &PG_DisaEditorTileController::onHoverChanged);
				connect(moduleNew, &PG_EditorModule::selectionChanged, this, &PG_DisaEditorTileController::onSelectionChanged);
			}

		});
}

PG_DisaEditorTileController::~PG_DisaEditorTileController() {
	if (auto* editor = getEditor()) {
		editor->clearHovered();
		editor->clearSelection();
	}
}

bool PG_DisaEditorTileController::isSelectable(const PG_EditorActorInfo& actor) const {
	return (bool)dynamic_cast<PG_DisaTileViewNode*>(*actor);
}
bool PG_DisaEditorTileController::isDragable(const PG_EditorActorInfo& actor) const {
	return (bool)dynamic_cast<PG_DisaTileViewNode*>(*actor) || (bool)dynamic_cast<PG_DisaMapViewNode*>(*actor);
}
void PG_DisaEditorTileController::createGizmo( PG_CameraNode* cam, PG_Scene* scene) const {}


glm::vec3 PG_DisaEditorTileController::findCollisionPoint(const PG_Ray& ray, PG_ActorNode* root) {
	float minDistance = std::numeric_limits<float>::max();

	root->traverseTest([&](PG_Object* obj) {
		if (auto* actor = dynamic_cast<PG_ActorNode*>(obj)) {
			if (dynamic_cast<PG_DisaMapViewNode*>(actor)) {
				if (auto intersection = ray.intersectPlane(glm::vec3(0.f), g_floorNormal); intersection && intersection.value() < minDistance) {
					minDistance = intersection.value();
				}

			}
			else if (auto* tile = dynamic_cast<PG_DisaTileViewNode*>(actor); tile && tile->getLocation()) {
				if (const auto& volume = tile->getVolumeCollision(); volume && volume.intersect(ray)) {
					auto tileInfo = PG_DisaFaceHit::intersection(tile, ray, true);
					if (tileInfo.first.distance < minDistance && tileInfo.second) {
						minDistance = tileInfo.first.distance;
					}
				}
				return false;
			}
			return true;
		}
		return false;
		});
	minDistance = std::min(minDistance, 300.f);
	return ray.getPointOnRay(minDistance);
}

PG_DisaEditorTileController::Found PG_DisaEditorTileController::findObjects(PG_CameraNode* cam, const PG_Ray& ray, PG_ActorNode* root) const {
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
				
			}else if (auto* tile = dynamic_cast<PG_DisaTileViewNode*>(actor); tile && tile->getLocation()) {
				if (const auto& volume = tile->getVolumeCollision(); volume && volume.intersect(ray)) {
					auto tileInfo = PG_DisaFaceHit::intersection(tile, ray, true);
					if (tileInfo.first.distance < maxDistance &&  tileInfo.second) {
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

PG_DisaEditorTileController::FoundItem PG_DisaEditorTileController::choose(const Found& found) const {
	if(found.empty()) return FoundItem{ PG_RayIntersection::fail(), nullptr };

	auto minElement = *std::min_element(found.begin(), found.end(), [](const FoundItem& a, const FoundItem& b)->bool {
		return  a.first.distance < b.first.distance;
	});

	if (minElement.second.additionalData.is<PG_DisaFaceHit>() && minElement.second.additionalData.get<PG_DisaFaceHit>().face != PG_DisaFaceHit::Face_Top) {
		return FoundItem{ PG_RayIntersection::fail(), nullptr }; //we hit a side
	}

	return minElement;
}

void PG_DisaEditorTileController::onSelectionChanged(const std::set<PG_EditorActorInfo>& deselected, const std::set<PG_EditorActorInfo>& selected) {
	std::unordered_map<glm::ivec2, bool, PG_iVec2Hasher> faceSelected; //tile, hovered, hit
	PG_DisaMapSelectionHandlerNode* selectionNode = getScene()->getRoot()->findFirstInTree<PG_DisaMapSelectionHandlerNode>();
	bool performCleanUp = false; //some tile was deleted

	for (const PG_EditorActorInfo& info : deselected) {
		if (auto tile = dynamic_cast<PG_DisaTileViewNode*>(*info)) {
			faceSelected[tile->getTilePosition()] = false;
		}
		else if (info.actor.expired()) {
			performCleanUp = true;
		}
	}
	for (const PG_EditorActorInfo& info : selected) {
		if (auto tile = dynamic_cast<PG_DisaTileViewNode*>(*info); tile && info.additionalData) {
			faceSelected[tile->getTilePosition()] = true;
		}
	}

	for (const auto& [tile, selected] : faceSelected) {
		selectionNode->setSelected(tile, selected);
	}
	if (performCleanUp) selectionNode->cleanUp();
}

void PG_DisaEditorTileController::onHoverChanged(const std::set<PG_EditorActorInfo>& unhovered, const std::set<PG_EditorActorInfo>& hovered) {
	std::unordered_map<glm::ivec2, std::pair<bool, PG_DisaFaceHit>, PG_iVec2Hasher> faceHovers; //tile, hovered, hit
	
	PG_DisaMapSelectionHandlerNode* selectionNode = getScene()->getRoot()->findFirstInTree<PG_DisaMapSelectionHandlerNode>();
	if (!selectionNode) return;

	for (const PG_EditorActorInfo& info : unhovered) {
		if (auto tile = dynamic_cast<PG_DisaTileViewNode*>(*info)) {
			faceHovers[tile->getTilePosition()] = { false, {} };
		}
		else if (auto tile = dynamic_cast<PG_DisaMapViewNode*>(*info); tile && info.additionalData) {
			faceHovers[info.additionalData.get<PG_DisaEmptyTileHit>().tilePos] = {false, {}};
		}
	}
	for (const PG_EditorActorInfo& info : hovered) {
		if (!info.additionalData) continue;

		if (auto tile = dynamic_cast<PG_DisaTileViewNode*>(*info)) {
			faceHovers[tile->getTilePosition()] = { true, info.additionalData.get<PG_DisaFaceHit>() };
		}
		else if (auto tile = dynamic_cast<PG_DisaMapViewNode*>(*info)) {
			faceHovers[info.additionalData.get<PG_DisaEmptyTileHit>().tilePos] = { true, {} };
		}
	}

	for (const auto& [tile, pair] : faceHovers) {
		selectionNode->setHovered(tile, pair.first, pair.second);
	}

}

void PG_DisaEditorTileController::clicked(PG_EditorCameraComponent* comp, PG_MouseButtonEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, PG_CameraNode* cam, const PG_Ray& ray) {
	if (e->getButton() == PG_BUTTON_RIGHT) {
		PG_DisaEditTerrainOperation::cancel(this, scene);
		return;
	}

	PG_DisaEditorTileController::FoundItem minDist{ PG_RayIntersection{std::numeric_limits<float>::max()}, PG_EditorActorInfo() };
	for (const auto& [actor, additonalData, index] : editorModule->getHovered()) {
		if (auto tile = dynamic_cast<PG_DisaTileViewNode*>(actor.get())) {
			if (auto dist = PG_DisaFaceHit::intersection(tile, ray); dist.first && minDist.first.distance > dist.first.distance) {
				minDist = dist;
			}
		}
		else if (auto* map = dynamic_cast<PG_DisaMapViewNode*>(actor.get())) {
			if (auto intersection = ray.intersectPlane(glm::vec3(0.f), g_floorNormal); intersection && minDist.first.distance  > *intersection) {
				minDist.first.distance = *intersection;
			}
		}
	}
	if (minDist.first.distance > 100000.0F) return;

	PG_DisaEditTerrainOperation::start(this, scene, ray, minDist.first.distance);
}

void PG_DisaEditorTileController::drag(PG_EditorCameraComponent* comp, PG_MouseMotionEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, PG_CameraNode* cam, const PG_Ray& ray) {
	PG_DisaEditTerrainOperation::drag(this, e, scene, editorModule, ray);
}

void PG_DisaEditorTileController::release(PG_EditorCameraComponent*, PG_MouseButtonEvent* , const PG_Scene* , PG_EditorModule* , PG_CameraNode* , const PG_Ray& ) {
	PG_DisaEditTerrainOperation::stop(this);
}

void PG_DisaEditorTileController::key(PG_EditorCameraComponent* comp, PG_KeyEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, PG_CameraNode* cam) {
	if (e->getKey() == PG_Key_Delete) {
		if (e->isUp()) {

			struct TileInfo{
				int location;
				glm::ivec2 pos;

				bool operator<(const TileInfo& i) const { return location < i.location; }
			};

			PG_DisaMapTileListModel* listModel = nullptr;
			std::set<TileInfo> selectedIndeces;
			for (const PG_EditorActorInfo& selected : editorModule->getSelection()) {
				auto actor = dynamic_cast<PG_DisaTileViewNode*>(*selected);
				if (!actor) continue;
				auto model = dynamic_cast<PG_DisaMapTileModel*>(actor->getTileModel());
				if (!model) continue;
				if (auto* _listModel = model->getParent<PG_DisaMapTileListModel>()) {
					if (!listModel) listModel = _listModel;
					else if (_listModel != listModel) { //something is very wrong
						listModel = nullptr;
						break;
					}
				}
				selectedIndeces.insert({ model->getLocation(), model->getTilePosition() });
			}

			PG_DisaEditTerrainOperation::stop(this);
			editorModule->clearHovered();
			editorModule->clearSelection();

			if (listModel) {
				std::unordered_set<glm::ivec2, PG_iVec2Hasher> surounding;
				{ //remove selected
					std::set<PG_ListModelLocation> locations;
					for (auto i : selectedIndeces) {
						locations.insert(listModel->getLocation(i.location));
						for (const glm::ivec2 offset : {glm::ivec2(1, 0), glm::ivec2(-1, 0), glm::ivec2(0, 1), glm::ivec2(0, -1)}) {
							surounding.insert(i.pos + offset);
						}
					}
					listModel->remove(locations);
				}

				for (glm::ivec2 s : surounding) {
					PG_DisaEditTerrainOperation::updateTile(listModel, s);
				}
			}

		}
		e->accept();
	}
}