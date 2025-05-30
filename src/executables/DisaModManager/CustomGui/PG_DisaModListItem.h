#pragma once

#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Model/PG_ListModelLocation.h>

class PG_GuiImage;
class PG_GuiText;
class PG_GuiToggleButton;

class PG_DisaModListItem : public PG_GuiRectangle
{
public:
	using PG_GuiRectangle::PG_GuiRectangle;
	void init() override;

	const PG_ListModelLocation& getLocation() const { return m_location; };
	void setLocation(const PG_ListModelLocation& location);
	void update(const PG_ListModelLocation& location, const PG_IdentitySet& identities);
private:
	void updateAutor();
	PG_ListModelLocation m_location;
	PG_GuiImage* m_img = nullptr;
	PG_GuiText* m_title = nullptr;
	PG_GuiText* m_author = nullptr;
	PG_GuiText* m_decription = nullptr;
	PG_GuiToggleButton* m_disableBtn = nullptr;
};