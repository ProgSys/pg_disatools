#pragma once

#include <Scene/Gui/PG_GuiDropDown.h>


class PG_DisaSelectorDropDown : public PG_GuiDropDown {
	PG_OBJECT(PG_DisaSelectorDropDown);
	PG_PARENT(PG_GuiDropDown);
public:
	using PG_GuiDropDown::PG_GuiDropDown;

	void init() override;
	bool event(PG_Event* e) override;

	PG_GuiItem* getTextFiled() const { return m_display; }

private:
	void updateDisplayElement(PG_GuiItem* element, const std::string& str) const override;
};