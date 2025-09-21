/*
 * The MIT License (MIT)
 *
 *	Copyright (c) 2020 ProgSys
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */

#pragma once

#include <string>
#include <vector>
#include <array>
#include <ostream>
#include <glm/glm.hpp>
#include <Util/Typedefs.h>
#include <Util/PG_File.h>
#include <Stream/PG_StreamIn.h>
#include <Stream/PG_StreamOut.h>
#include <Util/PG_DataSerializer.h>

PACK(struct mpdfileHeader// 16 bytes
{
	unsigned short number_of_chunks;
	unsigned short number_of_actors;
	unsigned short field_0x4;
	unsigned short field_0x6; //the rest is kinda 0

	unsigned short field_0x8;
	unsigned short field_0xA;
	unsigned short field_0xC;
	unsigned short field_0xE;
} );

PACK(struct mpdfileChunkHeader // 64 bytes
{
	float map_offset_x, map_offset_y, map_offset_z, map_offset_w; //0 - 12
	float xRot, yRot, zRot, wRot; //16,20,24,28; works but totaly broken ingame
	float map_offset2_x, map_offset2_y, map_offset2_z; //32, 36, 40 ; why there are twp offsets I don't know

	unsigned short number_of_tiles; // 44
	unsigned short unknown2;
	unsigned short index;

	unsigned char unknown_data2[14];

} );

PACK(struct mpdfileChunkObject //36 bytes
{
    short x, y, z; // y is negative for some reason?
    short xRot, yRot, zRot; // in degree
    short xScale, yScale, zScale; // 0-100
    short unk10;
    unsigned char  unk11, model_id, visible, unk12;
    char textureScrollX, textureScrollY; //this defines the speed and direction of texture scroll for flaged textures
	unsigned char  pad[10];

	static mpdfileChunkObject createEmpty() {
		return mpdfileChunkObject{
			0, 0, 0,
			0, 0 ,0,
			100, 100, 100,
			0,
			0, 0, 1, 0,
			0, 0,
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
		};
	}
} ) ;

PACK(struct mpdfileChunkEventTile // 4 byte
{
	  char  x, z;
	  unsigned short index;

} );


PACK( struct mpdfileTexdata{ //8 bytes
  unsigned char u;
  unsigned char v; 
  unsigned char vRight;
  unsigned char width;
  unsigned char height; 
  unsigned char heightRight;
  unsigned char textureIndex;
  unsigned char transform; //rotation(0b11) - mirrorX(0b100) - mirrorY(0b1000) - decalmode(0b110000)
} );

PACK( struct mpdfileTile{ //128 bytes
  mpdfileTexdata textures[12];  // Base color: N, E, S, W, Top; Decal layer: N, E, S, W, Top; Shadowmap: Top, (is always zero?) 
	/*
	*  NW2-----NE3
	*  |       |
	*  |       |
	*  SW0-----SE1
	*/
  std::array<char, 4> cornersTop;   // Height in each corner of the tile, negative = up; (SW, SE, NW, NE) 
  glm::i8vec2 sideN; //North bottom points, are usualy 0 if side face is hidden or starts from ground
  glm::i8vec2 sideE; //East bottom points, are usualy 0 if side face is hidden or starts from ground
  glm::i8vec2 sideS; //South bottom points, are usualy 0 if side face is hidden or starts from ground
  glm::i8vec2 sideW; //West bottom points, are usualy 0 if side face is hidden or starts from ground
  unsigned char  unk2[4];      // TBD
  char visible;
  unsigned char  x, z; //position of tile
  unsigned char  unk3[4]; //rotation?
  unsigned char  mobility;     // 0 = Unlimited, 1 = Fly only, 2 = Impassable
  unsigned char  geo_color;
  unsigned char  geo_mark;     // 0 = No geopanel, 100 = Has geopanel
  unsigned char  pad2[6];
});

PACK(struct mpdfileChunkDef{
	mpdfileChunkHeader header;
	std::array<mpdfileChunkObject, 32> objects; //80 - (80 + 1152 bytes)
	std::array<mpdfileChunkEventTile, 16> eventTiles; // 1232 - (1232+ 64)
	mpdfileTile baseTile; // 1296

});

struct mpdfileChunk{
	mpdfileChunkDef info;
	std::vector<mpdfileTile> tiles;
};

PACK( struct mpdfileActor
{
  unsigned short id;          // Class ID
  unsigned short level;
  unsigned char  alliance; //0 = User controlled, 1 = Enemy, 2 = Enemy, third party (will have turn first), 3 = Passive (will do nothing), 4 = Neutral (will do nothing, and not agro enemies)
  char  x, z;
  char  rotation;    // -1 = W, 0 = N, 1 = E, 2 = S
  unsigned char  visible;
  unsigned char  ai;          // 0 = Disabled, 1 = Guard, 2 = Attack
  unsigned char  unk5, unk6; //health multiplyer? speed?
  unsigned short items[4];
  unsigned char  appearance;  // 0 = Appear Normal, 1 = Absent 2nd, 2 = Appear 2nd
  unsigned char  unk7;
  unsigned short geo_effect;  // Only(?) used for geosymbols, val%10 = color, val/10 = effect
  unsigned short unk8;
  unsigned short magic[4];
  char  unk[30];     // TBD stat modifiers?
} );


class PG_MPD {
public:

	bool open(const std::string& file);
	bool open(const PG_File& file);
    bool open(PG::STREAM::In* reader);

	bool openJson(const std::string& file);
	bool openJson(const PG_File& file);
    bool openJson(PG::STREAM::In* reader);

	void saveJson(const std::string& file) const;

	bool save(const std::string& file);
	bool save(const PG_File& file);
	bool save(PG::STREAM::Out* writer);

	void clear();


	mpdfileHeader& getHeader();
	const mpdfileHeader& getHeader() const;

	std::vector<mpdfileChunk>& getChunks();
	const std::vector<mpdfileChunk>& getChunks() const;

	std::vector<mpdfileActor>& getActors();
	const std::vector<mpdfileActor>& getActors() const;

	PG_DataSerializer& getCustomMetaData() { return m_customMetaData; }
	const PG_DataSerializer& getCustomMetaData() const { return m_customMetaData; }

private:
	mpdfileHeader m_header;
	std::vector<mpdfileChunk> m_chunks;
	std::vector<mpdfileActor> m_actors;
	PG_DataSerializer m_customMetaData;
};

