#pragma once

#include <Scene/Gui/PG_GuiItem.h>
#include <Scene/Model/PG_ListModelLocation.h>

class PG_GuiText;
class PG_GuiRectangle;
class PG_GuiMouseArea;

class PG_DisaFileTableItem : public PG_GuiItem {
public:
	using PG_GuiItem::PG_GuiItem;
	void init() override;

	const PG_ListModelLocation& getLocation() const { return m_location; };
	void setLocation(const PG_ListModelLocation& location);
	void update(const PG_ListModelLocation& location, const PG_IdentitySet& identities);

	PG_GuiText* getTextItem() const { return m_text; }
private:
	PG_ListModelLocation m_location;
	PG_GuiText* m_text = nullptr;
};

class PG_DisaFileTableHeaderItem : public PG_DisaFileTableItem {
public:
	using PG_DisaFileTableItem::PG_DisaFileTableItem;
	void init() override;
};

class PG_DisaFileTableRowItem : public PG_GuiItem {
public:
	using PG_GuiItem::PG_GuiItem;
	void init() override;

	const PG_ListModelLocation& getLocation() const { return m_location; };
	void setLocation(const PG_ListModelLocation& location);
	void update(const PG_ListModelLocation& location, const PG_IdentitySet& identities);

private:
	void updateColor();

	PG_ListModelLocation m_location;
	PG_GuiRectangle* m_bg = nullptr;
	PG_GuiMouseArea* m_area = nullptr;
};