/*
 * PG_MPD.h
 *
 *  Created on: 09.09.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_PG_FILES_PG_MPD_H_
#define INCLUDE_PG_FILES_PG_MPD_H_

#include <string>
#include <vector>
#include <ostream>
#include <Util/PG_File.h>
#include <Util/PG_Vector.h>

namespace PG {
namespace FILE {

PACK(struct mpdfileHeader// 16 bytes
{
	unsigned short number_of_chunks;
	unsigned short number_of_actors;
	unsigned short unknown1;

	unsigned short unknown2;
	unsigned short unknown3;
	unsigned short unknown4;
	unsigned short unknown5;
	unsigned short unknown6;
} );

PACK(struct mpdfileChunkHeader // 64 bytes
{
	float map_offset_x;
	float map_offset_y; //?
	float map_offset_z; //?

	float unknown1; //?

	float unknown_data[7];

	unsigned short number_of_tiles;
	unsigned short unknown2;
	unsigned short index;

	unsigned short unknown_data2[7];

} );

PACK(struct mpdfileChunkObject //36 bytes
{
	short unk1, unk2, unk3, unk4, unk5;
	short unk6, unk7, unk8, unk9, unk10;
	unsigned char  unk11, unk12, unk13, unk14, unk15, unk16;
	unsigned char  pad[10];

} ) ;

PACK(struct mpdfileChunkEventTile // 4 byte
{
	  char  x, z;
	  unsigned short index;

} );


PACK( struct mpdfileTexdata{ //8 bytes
  unsigned char u;
  unsigned char stretch; //?
  unsigned char v;
  unsigned char width;
  unsigned char size_stretch; //?
  unsigned char height;
  unsigned char textureIndex;
  unsigned char mirror;
} );

PACK( struct mpdfileTile{
  mpdfileTexdata textures[12];  // N, W, S, E, Top; same repeated with unknown meaning (repeated texture?); 2 unknown textures
  char  corners[4];   // Height in each corner of the tile, negative = up; (SW, SE, NW, NE) (needs verification!)
  char  corners2[4];  // \ Related to bottom corners of tile, somehow. All zero except for "floating platforms" (e.g. mp1302)
  char  corners3[4];  // /
  unsigned char  unk2[4];      // TBD
  unsigned char  unk3;
  char  x, z;
  unsigned char  unk4, unk5, unk6, unk7;
  unsigned char  mobility;     // 0 = Unlimited, 1 = Fly only, 2 = Impassable
  unsigned char  geo_color;
  unsigned char  geo_mark;     // 0 = No geopanel, 100 = Has geopanel
  unsigned char  pad2[6];
});

PACK(struct mpdfileChunkDef{
	mpdfileChunkHeader header;
	mpdfileChunkObject objects[32]; //80 - (80 + 1152 bytes)
	mpdfileChunkEventTile eventTiles[16]; // 1232 - (1232+ 64)
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
  unsigned char  unk2;
  char  x, z;
  char  rotation;    // -1 = W, 0 = N, 1 = E, 2 = S
  unsigned char  unk4;
  unsigned char  ai;          // ? verify
  unsigned char  unk5, unk6;
  unsigned short items[4];
  unsigned char  appearance;  // 0 = Appear Normal, 1 = Absent 2nd, 2 = Appear 2nd
  unsigned char  unk7;
  unsigned short geo_effect;  // Only(?) used for geosymbols, val%10 = color, val/10 = effect
  unsigned short unk8;
  unsigned short magic[4];
  char  unk[30];     // TBD
} );

class MPD {
public:
	MPD();
	virtual ~MPD();

	// functions
	bool open(const std::string& file);
	bool open(const PG::UTIL::File& file);
	void clear();

	void dump(const std::string& file) const;

	mpdfileHeader& getHeader();
	const mpdfileHeader& getHeader() const;

	std::vector<mpdfileChunk>& getChunks();
	const std::vector<mpdfileChunk>& getChunks() const;

	std::vector<mpdfileActor>& getActors();
	const std::vector<mpdfileActor>& getActors() const;

private:
	mpdfileHeader m_header;
	std::vector<mpdfileChunk> m_chunks;
	std::vector<mpdfileActor> m_actors;
};

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_MPD_H_ */
