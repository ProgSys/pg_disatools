#pragma once

#include <Base/PG_Object.h>
#include <Base/PG_Variety.h>
#include <Util/PG_Ray.h>
#include <Util/PG_iVec2Hasher.h>

class PG_EditorModule;
class PG_MouseMotionEvent;
class PG_Scene;

class PG_DisaEditMaterialOperation : public PG_Object {
	PG_OBJECT(PG_DisaEditMaterialOperation);
	PG_PARENT(PG_Object);
public:
	using PG_Object::PG_Object;

	static PG_DisaEditMaterialOperation* start(PG_Object* parent, const PG_Scene* scene, const PG_Ray& ray, float distance);
	static void drag(PG_Object* parent, PG_MouseMotionEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, const PG_Ray& ray);
	static void cancel(PG_Object* parent, const PG_Scene* scene);


	glm::vec3 dragStartPos = glm::vec3(0.f);
	std::unordered_map<glm::ivec2, PG_Variety, PG_iVec2Hasher> originalData;
};