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