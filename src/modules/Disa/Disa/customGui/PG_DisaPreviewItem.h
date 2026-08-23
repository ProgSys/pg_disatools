#pragma once

#include <Scene/Gui/PG_GuiItem.h>
#include <Util/PG_File.h>
#include <Resources/PG_Texture.h>
#include <Disa/fileformats/PG_DisaMPDPack.h>

class PG_GuiRenderItem;
class PG_GuiText;
class PG_DisaMapModel;
class PG_GuiRectangle;

class PG_DisaPreviewItem : public PG_GuiItem
{
	PG_OBJECT(PG_DisaPreviewItem);
	PG_PROPERTY(PG_ListModelLocation location GET getLocation SET setLocation CHANGED locationChanged)
signals:
	void locationChanged();
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

	/**
	* @brief If disabled then no preview will be loaded at all
	*/
	inline bool isPreviewEnabled() const { return m_previewEnabled; }
	void setPreviewEnabled(bool enabled);
	/**
	* @returns true if any kind of preview is open
	*/
	inline bool hasPreview() const { return m_container && m_container->hasGuiChildren(); }
	/**
	* @brief Asks the user for a target file and saves the currently open preview as png
	*/
	void exportImage();

private:
	void clearImage();
	void clear();
	void createGameScene();
	void updateBorder();

	PG_ListModelLocation m_location;
	PG_GuiText* m_text = nullptr;
	PG_GuiRectangle* m_border = nullptr;
	PG_GuiItem* m_container = nullptr;
	bool m_previewEnabled = true;
};
