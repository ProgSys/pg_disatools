#pragma once

#include <Base/PG_Object.h>
#include <Disa/map/PG_DisaEditorModule.h>

class PG_GuiItem;
class PG_DisaMapScene;

class Inspector : public PG_Object {
	PG_OBJECT(Inspector)
	PG_PARENT(PG_Object)
public:
	using PG_Object::PG_Object;

	PG_DisaEditorModule* mod;
	PG_GuiItem* root;

	static void setup(PG_DisaMapScene* scene, PG_GuiItem* root);
	PG_DisaEditorModule::EMaterialInspectorMode getMaterialInspectorMode() const { return mod->getMaterialInspectorMode(); }
	void setMaterialInspectorMode(PG_DisaEditorModule::EMaterialInspectorMode mode) { mod->setMaterialInspectorMode(mode); }

	//connect to selection changed
private:
	void createFunctionBar();
	void createInspector();


	int m_lastMode = -1;
};