#include "PG_DisaEditorMaterialController.h"
#include <Base/Events/PG_MouseButtonEvent.h>
#include <Base/Events/PG_MouseMotionEvent.h>
#include <Base/Events/PG_KeyEvent.h>
#include <Scene/PG_ActorNode.h>
#include <Scene/PG_ModelNode.h>
#include <Scene/Editor/PG_EditorModule.h>
#include <Disa/map/PG_DisaMaterialEditSelectionNode.h>
#include <Disa/map/PG_DisaIntersection.h>
#include <Disa/map/PG_DisaTileViewNode.h>
#include <Disa/PG_DisaMapModel.h>
#include <Operation/PG_DisaEditMaterialOperation.h>
#include <Controllers/PG_DisaEditorModule.h>
#include <unordered_map>

PG_DisaEditorMaterialController::PG_DisaEditorMaterialController() {

	connect(this, &PG_DisaEditorMaterialController::editorChanged, this,
		[this](PG_EditorModule* moduleOld, PG_EditorModule* moduleNew) {

			if (moduleOld) {
				disconnect(moduleOld, &PG_EditorModule::hoveredChanged, this, &PG_DisaEditorMaterialController::onHoverChanged);
				disconnect(moduleOld, &PG_EditorModule::selectionChanged, this, &PG_DisaEditorMaterialController::onSelectionChanged);
			}

			if (moduleNew) {
				connect(moduleNew, &PG_EditorModule::hoveredChanged, this, &PG_DisaEditorMaterialController::onHoverChanged);
				connect(moduleNew, &PG_EditorModule::selectionChanged, this, &PG_DisaEditorMaterialController::onSelectionChanged);
			}

		});
}

PG_DisaEditorMaterialController::~PG_DisaEditorMaterialController() {
	if (auto* editor = getEditor()) {
		editor->clearHovered();
		editor->clearSelection();
	}
}


bool PG_DisaEditorMaterialController::isSelectable(const PG_EditorActorInfo& actor) const {
	return (bool)dynamic_cast<PG_DisaTileViewNode*>(*actor);
}
bool PG_DisaEditorMaterialController::isDragable(const PG_EditorActorInfo& actor) const {
	return false; // (bool)dynamic_cast<PG_DisaTileViewNode*>(*actor) || (bool)dynamic_cast<PG_DisaMapViewNode*>(*actor);
}
void PG_DisaEditorMaterialController::createGizmo(PG_CameraNode* cam, PG_Scene* scene) const {}


PG_DisaEditorMaterialController::Found PG_DisaEditorMaterialController::findObjects(PG_CameraNode* cam, const PG_Ray& ray, PG_ActorNode* root) const {
	assert(root);
	assert(!glm::any(glm::isnan(ray.dir)));
	assert(!glm::any(glm::isnan(ray.orig)));
	constexpr float maxDistance = 50.f;
	Found out;
	root->traverseTest([&](PG_Object* obj) {
		if (auto* actor = dynamic_cast<PG_ActorNode*>(obj)) {
			if (auto* tile = dynamic_cast<PG_DisaTileViewNode*>(actor); tile && tile->getLocation()) {
				if (const auto& volume = tile->getVolumeCollision(); volume && volume.intersect(ray)) {
					auto tileInfo = PG_DisaFaceHit::intersection(tile, ray, true, false);
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

PG_DisaEditorMaterialController::FoundItem PG_DisaEditorMaterialController::choose(const Found& found) const {
	if (found.empty()) return FoundItem{ PG_RayIntersection::fail(), nullptr };

	return *std::min_element(found.begin(), found.end(), [](const FoundItem& a, const FoundItem& b)->bool {
		return  a.first.distance < b.first.distance;
		});
}

using FaceMap = std::unordered_map<PG_DisaFaceHit::Face, std::pair<bool, PG_DisaFaceHit>>;
static inline std::unordered_map<glm::ivec2, FaceMap, PG_iVec2Hasher> assignToMap(const std::set<PG_EditorActorInfo>& deselected, const std::set<PG_EditorActorInfo>& selected) {
	std::unordered_map<glm::ivec2, FaceMap, PG_iVec2Hasher> faceHovers; //tile, hovered, hit

	for (const PG_EditorActorInfo& info : deselected) {
		if (!info.additionalData) continue;

		if (auto tile = dynamic_cast<PG_DisaTileViewNode*>(*info)) {
			auto hit = info.additionalData.get<PG_DisaFaceHit>();
			faceHovers[tile->getTilePosition()][hit.face] = { false, hit };
		}
	}

	for (const PG_EditorActorInfo& info : selected) {
		if (!info.additionalData) continue;

		if (auto tile = dynamic_cast<PG_DisaTileViewNode*>(*info)) {
			auto hit = info.additionalData.get<PG_DisaFaceHit>();
			faceHovers[tile->getTilePosition()][hit.face] = { true, hit };
		}
	}

	return faceHovers;
}

void PG_DisaEditorMaterialController::onSelectionChanged(const std::set<PG_EditorActorInfo>& deselected, const std::set<PG_EditorActorInfo>& selected) {
	PG_DisaMaterialEditSelectionHandlerNode* selectionNode = getScene()->getRoot()->findFirstInTree<PG_DisaMaterialEditSelectionHandlerNode>();
	if (!selectionNode) return;

	for (const auto& [tile, faces] : assignToMap(deselected, selected)) {
		for (const auto& [face, pair] : faces) {
			selectionNode->setSelected(tile, pair.first, pair.second);
		}
	}
}

void PG_DisaEditorMaterialController::onHoverChanged(const std::set<PG_EditorActorInfo>& unhovered, const std::set<PG_EditorActorInfo>& hovered) {
	PG_DisaMaterialEditSelectionHandlerNode* selectionNode = getScene()->getRoot()->findFirstInTree<PG_DisaMaterialEditSelectionHandlerNode>();
	if (!selectionNode) return;

	for (const auto& [tile, faces] : assignToMap(unhovered, hovered)) {
		for (const auto& [face, pair] : faces) {
			selectionNode->setHovered(tile, pair.first, pair.second);
		}
	}
}

void PG_DisaEditorMaterialController::key(PG_EditorCameraComponent* comp, PG_KeyEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, PG_CameraNode* cam) {
	if (e->getKey() == PG_Key_Delete) {
		if (e->isUp()) {
			const auto& selection = editorModule->getSelection();
			std::unordered_map<PG_DisaMapTileModel*, FaceMap> faceHovers; //tile, hovered, hit
			
			for (const PG_EditorActorInfo& info : selection) {
				if (!info.additionalData) continue;

				if (auto tile = dynamic_cast<PG_DisaTileViewNode*>(*info)) {
					auto hit = info.additionalData.get<PG_DisaFaceHit>();
					faceHovers[dynamic_cast<PG_DisaMapTileModel*>(tile->getTileModel())][hit.face] = { true, hit };
				}
			}
			auto* disaEditor = dynamic_cast<PG_DisaEditorModule*>(getEditor());
			if (!disaEditor) return;

			for (const auto& [tile, faces] : faceHovers) {
				for (const auto& [face, pair] : faces) {
					switch (disaEditor->getMaterialInspectorMode())
					{
					case PG_DisaEditorModule::MaterialInspectorMode_Base: {
						switch (face)
						{
							case PG_DisaFaceHit::Face::Face_Top: tile->clearTexture(4); break;
							case PG_DisaFaceHit::Face::Face_West: tile->clearTexture(3); break;
							case PG_DisaFaceHit::Face::Face_East: tile->clearTexture(1); break;
							case PG_DisaFaceHit::Face::Face_South: tile->clearTexture(2); break;
							case PG_DisaFaceHit::Face::Face_North: tile->clearTexture(0); break;
						default:
							break;
						}
					}break;
					case PG_DisaEditorModule::MaterialInspectorMode_Decal:{
						if (face != PG_DisaFaceHit::Face::Face_Top) continue;
						tile->clearTexture(9);
					}break;
					case PG_DisaEditorModule::MaterialInspectorMode_Shadow: {
						switch (face)
						{
						case PG_DisaFaceHit::Face::Face_Top: tile->clearTexture(10); break;
						case PG_DisaFaceHit::Face::Face_West: tile->clearTexture(8); break;
						case PG_DisaFaceHit::Face::Face_East: tile->clearTexture(6); break;
						case PG_DisaFaceHit::Face::Face_South: tile->clearTexture(7); break;
						case PG_DisaFaceHit::Face::Face_North: tile->clearTexture(5); break;
						default:
							break;
						}
					}break;
					default:
						break;
					}
					
				}
			}

		}
		e->accept();
	}
}

void PG_DisaEditorMaterialController::clicked(PG_EditorCameraComponent* comp, PG_MouseButtonEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, PG_CameraNode* cam, const PG_Ray& ray) {}

bool PG_DisaEditorMaterialController::overrideSelection(PG_EditorModule* editorModule, const PG_EditorActorInfo& toSelect, PG_MouseButtonEvent* e) const {
	if (e->getButton() == PG_BUTTON_LEFT && e->getKeyMod() & PG_KMod_LAlt && !editorModule->getSelection().empty()) {

		auto hitEnd = toSelect.additionalData.get<PG_DisaFaceHit>();
		auto startIt = std::ranges::max_element(editorModule->getSelection(), {}, &PG_EditorActorInfo::index);
		auto hitStart = startIt->additionalData.get<PG_DisaFaceHit>();

		if (hitEnd.face != hitStart.face) return true;

		//select AABB
		PG_AABBT<glm::ivec2> aabb;
		PG_ActorNode* viewNode = nullptr;
		if (auto tile = dynamic_cast<PG_DisaTileViewNode*>(*toSelect)) {
			aabb.add(tile->getTilePosition());
			viewNode = tile->getParent<PG_ActorNode>();
		}
		else return true;

		if (auto tile = dynamic_cast<PG_DisaTileViewNode*>(**startIt)) {
			aabb.add(tile->getTilePosition());
		}
		else return true;

		assert(viewNode);
		if (!viewNode) return true;

		std::unordered_map<glm::ivec2, PG_DisaTileViewNode*, PG_iVec2Hasher> mapping;
		viewNode->traverseTestWithoutRoot([&mapping](PG_Object* obj) {
			if (auto* tile = dynamic_cast<PG_DisaTileViewNode*>(obj); tile && tile->getLocation()) {
				mapping[tile->getTilePosition()] = tile;
			}
			return false;
			});

		for (int x = aabb.min.x; x <= aabb.max.x; ++x) {
			for (int y = aabb.min.y; y <= aabb.max.y; ++y) {
				auto findIt = mapping.find(glm::ivec2(x, y));
				if (findIt == mapping.end()) continue;
				editorModule->select(PG_EditorActorInfo(findIt->second, PG_Variety{ PG_DisaFaceHit{hitEnd.face, static_cast<PG_DisaFaceHit::HitBits>(0)} }));
			}
		}

		return true;
	}

	return false;
}

void PG_DisaEditorMaterialController::drag(PG_EditorCameraComponent* comp, PG_MouseMotionEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, PG_CameraNode* cam, const PG_Ray& ray) {
	//PG_DisaEditTerrainOperation::drag(this, e, scene, editorModule, ray);
}