#pragma once

#include <Scene/Gui/PG_GuiItem.h>
#include <Scene/Model/PG_ListModelLocation.h>


class PG_GuiText;
class PG_GuiImage;
class PG_GuiMouseArea;

class PG_DisaCharacterPortraitGridTile : public PG_GuiItem
{
public:
	using PG_GuiItem::PG_GuiItem;
	void init() override;

	const PG_ListModelLocation& getLocation() const { return m_location; };
	void setLocation(const PG_ListModelLocation& location);
	void update(const PG_ListModelLocation& location, const PG_IdentitySet& identities);
private:
	void reload();

	PG_ListModelLocation m_location;
	PG_GuiText* m_text = nullptr;
	PG_GuiImage* m_image = nullptr;
	PG_GuiMouseArea* m_area = nullptr;
};