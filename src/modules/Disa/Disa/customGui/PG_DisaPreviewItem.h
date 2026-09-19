#pragma once

#include <Scene/Gui/PG_GuiItem.h>
#include <Util/PG_File.h>
#include <Resources/PG_Sprite.h>
#include <Resources/PG_Texture.h>
#include <Disa/fileformats/PG_DisaMPDPack.h>

class PG_Audio;
class PG_GuiRenderItem;
class PG_GuiSprite;
class PG_GuiText;
class PG_DisaMapScene;
class PG_GuiImage;
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
	/// Draws the audio as a waveform, once the samples are in
	void open(const PG_Audio& audio);
	/// Plays the sprite, with a spin box to walk through its animations
	void open(const PG_Sprite& sprite);
	void open(const PG_DisaMPDPack& mapFiles);
	void open(const PG_DisaMPPPack& mppPack);
	/// Shows the geometry on its own, as the only object of a map with no tiles
	void open(const PG_DisaGEOPack& geoPack);

	const PG_ListModelLocation& getLocation() const { return m_location; };
	void setLocation(const PG_ListModelLocation& location);
	void update(const PG_ListModelLocation& location, const PG_IdentitySet& identities);

	/**
	* @brief If disabled then no preview will be loaded at all
	*/
	inline bool isPreviewEnabled() const { return m_previewEnabled; }
	void setPreviewEnabled(bool enabled);
	/**
	* @brief The file name drawn along the bottom edge
	* @details Turn it off where the surrounding gui already names the file.
	*/
	bool isNameVisible() const;
	void setNameVisible(bool visible);
	/**
	* @returns true if any kind of preview is open
	*/
	inline bool hasPreview() const { return m_container && m_container->hasGuiChildren(); }
	/**
	* @brief Asks the user for a target file and saves the currently open preview as png
	*/
	void exportImage();
	/**
	* @brief Asks the user for a target folder and writes every sheet of the open sprite into it as a png
	*/
	void exportSpriteSheets();
	/**
	* @returns The picture the open preview is made of, null when it is not made of one
	* @details A map preview is a rendered scene and an audio preview has its image before it has
	* anything in it, so neither has a texture to hand over.
	*/
	const PG_Texture* getPreviewTexture() const;
	/**
	* @returns The sprite the open preview plays, null when the preview is not a sprite or still loading
	*/
	const PG_Sprite* getPreviewSprite() const;

private:
	/**
	* @brief The image the preview draws into, built on the first call.
	* @details Also puts the item back on the material its source picks for itself, so a preview never
	* inherits the shader the one before it needed.
	*/
	PG_GuiImage* getOrCreateImage();
	/**
	* @brief The scene a map preview is drawn in, built on the first call
	* @details Comes up in preview mode, so it turns the map in front of the camera and takes no input.
	*/
	PG_DisaMapScene* getOrCreateGameScene();
	/**
	* @brief The node a sprite preview plays in, built on the first call.
	* @details Comes up with the flips that carry a sprite from the scene's y up world into gui space.
	*/
	PG_GuiSprite* getOrCreateSprite();

	void clearImage();
	void clear();
	void updateBorder();

	PG_ListModelLocation m_location;
	PG_GuiText* m_text = nullptr;
	PG_GuiImage* m_checker = nullptr;
	PG_GuiRectangle* m_border = nullptr;
	PG_GuiItem* m_container = nullptr;
	PG_DisaMapScene* m_gameScene = nullptr;
	bool m_previewEnabled = true;
};
