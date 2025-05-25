#pragma once
#include <Scene/Gui/PG_GuiDropDown.h>

class PG_DisaItemDropDown : public PG_GuiDropDown {
	PG_OBJECT(PG_DisaItemDropDown);
	PG_PARENT(PG_GuiDropDown);
public:
	using PG_GuiDropDown::PG_GuiDropDown;

protected:
	PG_GuiItem* createDisplayElement(PG_GuiItem* parent) const override;
	void updateDisplayElement(PG_GuiItem* element, const std::string& str) const override;
	void updatePreviewElement(PG_GuiItem* element, const PG_Variety& var) const override;

};
