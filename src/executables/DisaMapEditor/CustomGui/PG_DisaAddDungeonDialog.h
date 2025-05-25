#pragma once

#include <Scene/Gui/PG_GuiPopup.h>

class PG_PropertyDynamicWrapper;
class PG_DisaFileContainer;

class PG_DisaAddDungeonDialog : public PG_GuiPopup {
	PG_OBJECT(PG_DisaAddDungeonDialog);
	PG_PARENT(PG_GuiPopup);
signals:
	void confirmClicked();
	void cancelClicked();
public:
	using PG_GuiPopup::PG_GuiPopup;

	void init();

	void setModel(PG_Object* model, const PG_ObjectFunction& targetFunc); //can only be set once

private:
	PG_PropertyDynamicWrapper* m_model = nullptr;
	PG_ObjectFunction m_targetFunc;
};