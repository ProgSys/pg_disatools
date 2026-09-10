#pragma once

#include <Base/PG_Object.h>
#include <Base/PG_Variety.h>
#include <Util/PG_Ray.h>
#include <Util/PG_iVec2Hasher.h>

class PG_EditorModule;
class PG_MouseMotionEvent;
class PG_DisaMapTileListModel;
class PG_Scene;

class PG_DisaEditTerrainOperation : public PG_Object {
	PG_OBJECT(PG_DisaEditTerrainOperation);
	PG_PARENT(PG_Object);
public:
	using PG_Object::PG_Object;

	enum EMergeMode : int {
		MergeMode_None = 0,
		MergeMode_Merge,
		MergeMode_Move
	};

	static PG_DisaEditTerrainOperation* start(PG_Object* parent, const PG_Scene* scene, const PG_Ray& ray, float distance);
	static void drag(PG_Object* parent, PG_MouseMotionEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, const PG_Ray& ray);
	static void cancel(PG_Object* parent, const PG_Scene* scene);
	static void stop(PG_Object* parent);

	static void updateTile(PG_DisaMapTileListModel* tiles, glm::ivec2 pos);


	glm::vec3 dragStartPos = glm::vec3(0.f);
	std::array<float, 4> originalTopOffsets = { 0.f };
	std::unordered_map<glm::ivec2, PG_Variety, PG_iVec2Hasher> originalData;
	EMergeMode mergeMode = MergeMode_None;
};