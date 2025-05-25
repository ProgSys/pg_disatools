#pragma once

#include <Scene/Editor/PG_EditorModule.h>

class PG_DisaEditorModule : public PG_EditorModule {
	PG_OBJECT(PG_DisaEditorModule);
	PG_PARENT(PG_EditorModule);
public:
	enum EMode: int {
		Mode_View,
		Mode_Terrain,
		Mode_Material,
		Mode_Object,
		Mode_ShadowMapping,
		Mode_EventTile,
		Mode_Actor,
	};

	enum EMaterialInspectorMode {
		MaterialInspectorMode_Base,
		MaterialInspectorMode_Decal,
		MaterialInspectorMode_Shadow,
	};

signals:
	void modeChanged(EMode mode);
	void materialInspectorModeChanged(EMaterialInspectorMode mode);
public:

	using PG_EditorModule::PG_EditorModule;

	static std::type_index getModuleIndex() { return typeid(PG_EditorModule); }

	EMode getMode() const { return m_mode; }
	void setMode(EMode mode);

	EMaterialInspectorMode getMaterialInspectorMode() const { return m_materialInspectorMode; }
	void setMaterialInspectorMode(EMaterialInspectorMode mode);

private:
	EMode m_mode = Mode_View;
	EMaterialInspectorMode m_materialInspectorMode = MaterialInspectorMode_Base;
};