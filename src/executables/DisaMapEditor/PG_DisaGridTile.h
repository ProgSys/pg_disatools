#pragma once

#include <Scene/Gui/PG_GuiItem.h>
#include <Util/PG_File.h>
#include <Resources/PG_Texture.h>
#include <PG_DisaMapListModel.h>

class PG_GuiRenderItem;
class PG_GuiText;
class PG_DisaMapSceneView;
class PG_DisaMapModel;
class PG_GuiRectangle;

class PG_DisaGridTile : public PG_GuiItem
{
	PG_OBJECT(PG_DisaGridTile);
signals:
	void clicked();
	void clickedLocation(PG_ListModelLocation loc);
	void rightClicked();
	void selectionChanged();
public:
	using PG_GuiItem::PG_GuiItem;
	void init() override;
	void openPreview(const PG_Variety& var);
	void open(const PG_Texture& img);
	void open(const std::string& imgPath);
	void open(const PG_DisaMPDPack& mapFiles);
	void open(const PG_DisaMPPPack& mppPack);

	const PG_ListModelLocation& getLocation() const { return m_location; };
	void setLocation(const PG_ListModelLocation& location);
	void update(const PG_ListModelLocation& location, const PG_IdentitySet& identities);

	PG_DisaMPDPack getMapFiles() const;
	inline PG_DisaMapSceneView* getGameScene() const { return m_gameScene; }

private:
	void createGameScene();
	void updateBorder();

	PG_ListModelLocation m_location;
	PG_GuiText* m_text = nullptr;
	PG_GuiRectangle* m_border = nullptr;
	PG_GuiItem* m_container = nullptr;
	PG_DisaMapSceneView* m_gameScene = nullptr;
};