#pragma once

#include <Scene/Editor/PG_EditorControllerBase.h>

class PG_DisaTileViewNode;

class PG_DisaEditorActorController : public PG_EditorControllerBase {
public:
	PG_DisaEditorActorController();

	bool isSelectable(const PG_EditorActorInfo& actor) const override;
	bool isDragable(const PG_EditorActorInfo& actor) const override;

	void createGizmo(PG_CameraNode* cam, PG_Scene* scene) const override;

	Found findObjects(PG_CameraNode* cam, const PG_Ray& ray, PG_ActorNode* root) const override;
	FoundItem choose(const Found& found) const override;

	void clicked(PG_EditorCameraComponent* comp, PG_MouseButtonEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, PG_CameraNode* cam, const PG_Ray& ray) override;
	void drag(PG_EditorCameraComponent* comp, PG_MouseMotionEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, PG_CameraNode* cam, const PG_Ray& ray) override;
	void release(PG_EditorCameraComponent* comp, PG_MouseButtonEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, PG_CameraNode* cam, const PG_Ray& ray) override;
	void key(PG_EditorCameraComponent* comp, PG_KeyEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, PG_CameraNode* cam) override;

	static PG_DisaTileViewNode* findMinTile(const PG_Scene* scene, const PG_Ray& ray, bool includeSides = false);

protected:
	void onSelectionChanged(const std::set<PG_EditorActorInfo>& deselected, const std::set<PG_EditorActorInfo>& selected);
	void onHoverChanged(const std::set<PG_EditorActorInfo>& unhovered, const std::set<PG_EditorActorInfo>& hovered);

	glm::ivec2 m_startTilePos;
};