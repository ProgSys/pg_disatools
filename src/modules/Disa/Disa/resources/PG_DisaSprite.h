#pragma once

#include <resources/PG_Sprite.h>
#include <Disa/fileformats/PG_SH.h>

class PG_DisaSprite : public PG_Sprite{
public:
	using PG_Sprite::PG_Sprite;

	PG_DisaSprite(unsigned short spriteID, const PG_SH& sh);
	PG_DisaSprite(unsigned short spriteID, PG_SH&& sh);

	void load(unsigned short spriteID, const PG_SH& sh);
	void load(unsigned short spriteID, PG_SH&& sh);
};