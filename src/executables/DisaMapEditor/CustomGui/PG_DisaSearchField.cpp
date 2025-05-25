#include "PG_DisaSearchField.h"
#include <Scene/Gui/PG_GuiTextEdit.h>

void PG_DisaSearchField::init() {
	PG_GuiTextField::init();

	setPlaceholderText("Search");
#ifdef _DEBUG
	getTextItem()->setSelectionMaterial(PG_Material("M_PantiesGuiTextSelection"));
#else
	getTextItem()->setSelectionMaterial(PG_Material("M_DisaGuiTextSelection"));
#endif
}