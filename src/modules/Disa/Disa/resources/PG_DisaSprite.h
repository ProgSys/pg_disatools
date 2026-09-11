#pragma once

#include <memory>

#include <resources/PG_Sprite.h>
#include <Stream/PG_StreamIn.h>
#include <Disa/fileformats/PG_SH.h>

class PG_DisaSprite : public PG_Sprite{
public:
	using PG_Sprite::PG_Sprite;

	PG_DisaSprite(unsigned short spriteID, const PG_SH& sh);
	PG_DisaSprite(unsigned short spriteID, PG_SH&& sh);

	void load(unsigned short spriteID, const PG_SH& sh);
	void load(unsigned short spriteID, PG_SH&& sh);
	/**
	* @brief Loads a sprite sheet from a stream.
	* @details The sheet is read on the loading thread, so nothing waits on the file here.
	* @param name Resource name, sprites asked for under the same name are shared.
	*/
	void load(const std::string& name, std::shared_ptr<PG::STREAM::In> reader);
};

/**
* @brief The sprite sheets a Disgaea sprite is drawn out of, resolved to colour.
* @details A sheet holds colour table indices rather than colours, and a cutout picks the 16 colour block
* its part of the sheet is read through, so one sheet can carry several palettes at once.
*/
namespace PG_DisaSpriteSheets {
	/**
	* @brief How many sprite sheets the given sprite is made of.
	* @returns 0 when the sprite is not a Disgaea sprite sheet, or is not loaded yet.
	*/
	int getSize(const PG_Sprite& sprite);
	/**
	* @brief One sprite sheet of the given sprite, resolved to colour.
	* @details The sheet is read back out of graphics memory, so this blocks until the render thread has
	* it. Fine for a preview or an export, never for something that runs every frame.
	* @param activeColorTable Which of the sheet's colour tables to resolve against. A character uses this
	* to be recoloured, a plain look at the sheet wants 0.
	* @returns An empty image when the index names no sheet, or when the read back failed.
	*/
	PG_RGBAImage buildImage(const PG_Sprite& sprite, int sheetIndex, int activeColorTable = 0);
	/// @see buildImage, for every sheet at once
	std::vector<PG_RGBAImage> buildImages(const PG_Sprite& sprite, int activeColorTable = 0);
}