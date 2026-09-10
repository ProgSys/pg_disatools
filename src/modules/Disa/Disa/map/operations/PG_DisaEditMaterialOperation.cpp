#include "PG_DisaEditMaterialOperation.h"
#include <Base/Events/PG_MouseMotionEvent.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/PG_DisaMapModel.h>
#include <Disa/map/PG_DisaTileEditSelectionNode.h>
#include <Scene/Editor/PG_EditorModule.h>
#include <Scene/PG_FootlooseViewNode.h>

static inline PG_DisaEditMaterialOperation* getOperation(PG_Object* parent) {
	return parent->findFirstChild<PG_DisaEditMaterialOperation>();
}

static inline PG_DisaMapModel* getModel(const PG_Scene* scene) {
	auto* mapView = scene->getRoot()->findFirstChild<PG_DisaMapViewNode>();
	if (!mapView) return nullptr;
	return mapView->getMapModel();
}


PG_DisaEditMaterialOperation* PG_DisaEditMaterialOperation::start(PG_Object* parent, const PG_Scene* scene, const PG_Ray& ray, float distance) {
	assert(scene);
	PG_DisaEditMaterialOperation* op = getOperation(parent);
	if (!op) op = PG_new<PG_DisaEditMaterialOperation>("PG_DisaEditTerrainOperation", parent);
	auto* model = getModel(scene);
	if (!model) return op;

	op->dragStartPos = ray.getPointOnRay(distance);
	op->originalData.clear();
	const glm::ivec2 originTilePosition(std::floor(op->dragStartPos.x), std::floor(op->dragStartPos.z));

	//calc offset
	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			glm::ivec2 tilePosition(originTilePosition.x + x, originTilePosition.y + y);

			if (PG_ListModelLocation loc = model->getTiles()->getTileLocation(tilePosition)) {
				op->originalData[tilePosition] = loc.getData(PG_DisaMapTileModel::Identity_Geometry);
			}
			else {
				op->originalData[tilePosition] = PG_Variety();
			}
		}
	}


	return op;
}

void PG_DisaEditMaterialOperation::drag(PG_Object* parent, PG_MouseMotionEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, const PG_Ray& ray) {
	PG_DisaEditMaterialOperation* op = getOperation(parent);
	if (!op) return;


}

void PG_DisaEditMaterialOperation::cancel(PG_Object* parent, const PG_Scene* scene) {
	PG_DisaEditMaterialOperation* op = getOperation(parent);
	if (!op) return;
	auto* model = getModel(scene);
	if (!model) return;

	for (const auto& [pos, data] : op->originalData) {
		model->getTiles()->setTileData(pos, data);
	}

	delete op;
}