#pragma once

#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Model/PG_ListModelLocation.h>


class PG_ListModelBase;
class PG_DisaGridTile;
class PG_GuiPopup;
class PG_GuiPopupInstance;
class PG_MouseButtonEvent;

/**
* @brief Image tile to select an item form the model
*/
class PG_DisaItemSelector : public PG_GuiRectangle {
	PG_OBJECT(PG_DisaItemSelector);
	PG_PARENT(PG_GuiRectangle);

	PG_PROPERTY(std::string name GET getName SET setSelectedName CHANGED selectedChanged);
signals:
	void selectedChanged();
	void rightClicked(PG_MouseButtonEvent* e); ///e is only valid during the emit
public:
	using PG_GuiRectangle::PG_GuiRectangle;

	void init() override;

	PG_ListModelBase* getModel() const { return m_model; }
	void setModel(PG_ListModelBase* model);

	PG_ListModelLocation getSelected() const;

	void open();
	void close();


	std::string getName() const;
	void setSelectedName(const std::string& name);
	/**
	* @brief Will clear current selection
	*/
	void clear();

	static PG_DisaItemSelector* createImageSelector(const std::string& name, PG_GuiItem* root, PG_ListModelBase* model);
	static PG_DisaItemSelector* createDungeonSelector(const std::string& name, PG_GuiItem* root, PG_ListModelBase* model);
private:
	void initPopupMenu(PG_GuiPopupInstance* parent);
	void selected(PG_ListModelLocation loc);
	void findSelection();

	PG_ListModelBase* m_model = nullptr;
	PG_DisaGridTile* m_tile = nullptr;
	PG_GuiPopup* m_popup = nullptr;
	std::string m_selectedName;
};