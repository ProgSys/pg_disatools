#pragma once

#include <Scene/Gui/PG_GuiItem.h>
#include <Scene/Model/PG_ListModelLocation.h>


class PG_GuiSpinBox;
class PG_GuiButton;

class PG_DisaEventTileListItem : public PG_GuiItem
{
public:
	using PG_GuiItem::PG_GuiItem;
	void init() override;

	const PG_ListModelLocation& getLocation() const { return m_location; };
	void setLocation(const PG_ListModelLocation& location);
	void update(const PG_ListModelLocation& location, const PG_IdentitySet& identities);
private:

	PG_ListModelLocation m_location;
	PG_GuiSpinBox* m_spinbox = nullptr;
	PG_GuiButton* m_removeBtn = nullptr;
};