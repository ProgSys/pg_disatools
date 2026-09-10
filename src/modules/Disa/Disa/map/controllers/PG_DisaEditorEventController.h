#pragma once

#include <Scene/Editor/PG_EditorControllerBase.h>

class PG_DisaEditorEventController : public PG_EditorControllerBase {
public:
	PG_DisaEditorEventController();
	~PG_DisaEditorEventController();

	bool isSelectable(const PG_EditorActorInfo& actor) const override;
	bool isDragable(const PG_EditorActorInfo& actor) const override;
	void createGizmo(PG_CameraNode* cam, PG_Scene* scene) const override;

	Found findObjects(PG_CameraNode* cam, const PG_Ray& ray, PG_ActorNode* root) const override;
	FoundItem choose(const Found& found) const override;

protected:
	void onSelectionChanged(const std::set<PG_EditorActorInfo>& deselected, const std::set<PG_EditorActorInfo>& selected);
	void onHoverChanged(const std::set<PG_EditorActorInfo>& unhovered, const std::set<PG_EditorActorInfo>& hovered);
};