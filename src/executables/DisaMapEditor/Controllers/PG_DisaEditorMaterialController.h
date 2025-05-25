#pragma once

#include <Scene/Editor/PG_EditorControllerBase.h>

class PG_DisaEditorMaterialController : public PG_EditorControllerBase {
public:
	PG_DisaEditorMaterialController();
	~PG_DisaEditorMaterialController();

	bool isSelectable(const PG_EditorActorInfo& actor) const override;
	bool isDragable(const PG_EditorActorInfo& actor) const override;
	void createGizmo(PG_CameraNode* cam, PG_Scene* scene) const override;

	Found findObjects(PG_CameraNode* cam, const PG_Ray& ray, PG_ActorNode* root) const override;
	FoundItem choose(const Found& found) const override;

	void clicked(PG_EditorCameraComponent* comp, PG_MouseButtonEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, PG_CameraNode* cam, const PG_Ray& ray) override;
	void drag(PG_EditorCameraComponent* comp, PG_MouseMotionEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, PG_CameraNode* cam, const PG_Ray& ray) override;
	void key(PG_EditorCameraComponent* comp, PG_KeyEvent* e, const PG_Scene* scene, PG_EditorModule* editorModule, PG_CameraNode* cam);

	bool overrideSelection(PG_EditorModule* editorModule, const PG_EditorActorInfo& toSelect, PG_MouseButtonEvent* e) const;

protected:
	void onSelectionChanged(const std::set<PG_EditorActorInfo>& deselected, const std::set<PG_EditorActorInfo>& selected);
	void onHoverChanged(const std::set<PG_EditorActorInfo>& unhovered, const std::set<PG_EditorActorInfo>& hovered);

	PG_EditorActorInfo m_lastSelected;
};