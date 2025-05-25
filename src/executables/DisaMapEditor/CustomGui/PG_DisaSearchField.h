#pragma once

#include <Scene/Gui/PG_GuiTextField.h>

class PG_DisaSearchField : public PG_GuiTextField {
	PG_OBJECT(PG_DisaSearchField);
	PG_PARENT(PG_GuiTextField);
public:
	using PG_GuiTextField::PG_GuiTextField;

	void init() override;
};