#pragma once

#include <Scene/Gui/PG_GuiItem.h>
#include <Scene/Model/PG_ListModelLocation.h>

class PG_GuiText;
class PG_GuiImage;
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

/**
* @brief A table cell that puts the icon of the file's extension in front of its name.
* @details Only the first column names a file, so that is the only one the icon shows in. The view
* hands the same item every column it scrolls past, which is why the icon is decided per location
* rather than once at build time.
*/
class PG_DisaFileTableFileItem : public PG_DisaFileTableItem {
public:
	using PG_DisaFileTableItem::PG_DisaFileTableItem;
	void init() override;

	void setLocation(const PG_ListModelLocation& location);
	void update(const PG_ListModelLocation& location, const PG_IdentitySet& identities);

private:
	void updateIcon(const PG_ListModelLocation& location);

	PG_GuiImage* m_icon = nullptr;
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