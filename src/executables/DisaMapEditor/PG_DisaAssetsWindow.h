#pragma once
#include <Scene/Gui/PG_GuiItem.h>
#include <Scene/Model/PG_ListModelBase.h>

class PG_GuiListView;
class PG_GuiButton;
class PG_DisaAssetsModel;

class PG_DisaAssetsListItem : public PG_GuiItem {
	PG_OBJECT(PG_DisaAssetsListItem)
	PG_PARENT(PG_GuiItem)
public:
	using PG_GuiItem::PG_GuiItem;
	void init() override;

	PG_ListModelLocation getLocation() const { return m_location; }
	void setLocation(const PG_ListModelLocation& location);

	void update(const PG_ListModelLocation& location, const PG_IdentitySet& identities);

private:
	void updateCenter();

	bool m_isTexture = true;

	PG_ListModelLocation m_location;
	PG_GuiItem* m_rightItem = nullptr;
	PG_GuiButton* m_topBtn = nullptr;
};

class PG_DisaAssetsWindow : public PG_GuiItem {
	PG_OBJECT(PG_DisaAssetsWindow)
		PG_PARENT(PG_GuiItem)
signals:
	void toggleModeChanged(bool isTexturesMode);
public:
	using PG_GuiItem::PG_GuiItem;
	void init() override;

	bool isShowTextures() { return m_isShowTextures; }
	void setShowTextures(bool isTexture) {
		if (m_isShowTextures != isTexture) {
			m_isShowTextures = isTexture;
			emit toggleModeChanged(m_isShowTextures);
		}
	}

	void setModel(PG_DisaAssetsModel* model);

private:
	bool m_isShowTextures = true;
	PG_DisaAssetsModel* m_model = nullptr;
	PG_GuiListView* m_listView = nullptr;
};