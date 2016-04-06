/*
 * PG_SpriteSheet.h
 *
 *  Created on: 31.03.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_PG_FILES_PG_SPRITESHEET_FILE_H_
#define INCLUDE_PG_FILES_PG_SPRITESHEET_FILE_H_

#include <string>
#include <vector>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <pg/util/PG_File.h>
#include <pg/util/PG_Vector.h>
#include <pg/util/PG_Image.h>

#include <pg/util/PG_ApiUtil.h>

namespace PG {
namespace FILE {

typedef std::vector<PG::UTIL::rgba> ColorTable;

typedef struct
{
	unsigned short number_of_something0;//?
	unsigned short number_of_animations;
	unsigned short number_of_colortablesSets; //data between animations and sheets
	unsigned short number_of_sheets;

	unsigned short number_of_something1;
	unsigned short number_of_keyframes;
	unsigned short unknown6;
	unsigned short unknown7;

} __attribute__((packed, aligned(1))) spriteSheetHeader;

typedef struct
{
	unsigned short unknown0;
	unsigned short unknown1;
} __attribute__((packed, aligned(1))) something0;

typedef struct
{
	unsigned short unknown0;
	unsigned short unknown1;
	unsigned short unknown2;
	unsigned short unknown3;
} __attribute__((packed, aligned(1))) something1;

typedef struct
{
	unsigned short start_keyframe;
	unsigned short number_of_frames;
}__attribute__((packed, aligned(1))) animation;

typedef struct
{
	unsigned int offset;
	unsigned short width;
	unsigned short height;

	unsigned short unknown0;
	unsigned short unknown1;

}__attribute__((packed, aligned(1))) spriteSheet;

typedef struct
{
	unsigned short external_sheet; // get a sheet from different file by it's ID
	unsigned char sheet; // there is not one big sprite sheet but multiple smaller one
	unsigned char colortable; // the 16 rgba colortable which the sheet should use
	short offsetx;
	short offsety;

	//sprite info
	unsigned short x;
	unsigned short y;
	unsigned short width;
	unsigned short height;
	unsigned short scalex; // 0-100
	unsigned short scaley; // 0-100

	// only rotationx and rotationz seam to have a effect on 2D sprites
	short rotationx; //not confirmed
	short rotationy; //not confirmed
	short rotationz; //not confirmed

	unsigned short mirror; //not confirmed

} __attribute__((packed, aligned(1))) keyframe;

inline std::ostream& operator<<(std::ostream& o,const spriteSheetHeader& i){
	o <<"("<<i.number_of_something0<<", "<<i.number_of_animations<<", "<<i.number_of_colortablesSets<<", "<<i.number_of_sheets<<", ";
	o      <<i.number_of_something1<<", "<<i.number_of_keyframes<<", "<<i.unknown6<<", "<<i.unknown7<<") ";
	return o;
}

inline std::ostream& operator<<(std::ostream& o,const something0& i){
	o <<"("<<i.unknown0<<", "<<i.unknown1<<", ["<<  (  (int)(int(i.unknown1)<<16 | i.unknown0))<<"] )";
	return o;
}


inline std::ostream& operator<<(std::ostream& o,const something1& i){
	o <<"("<<i.unknown0<<", "<<i.unknown1<<", ["<<  (  (int)(int(i.unknown1)<<16 | i.unknown0)) <<"], "<<i.unknown2<<", "<<i.unknown3<<", ["<<  (  (int)(int(i.unknown2)<<16 | i.unknown3)) <<"] )";
	return o;
}

inline std::ostream& operator<<(std::ostream& o,const animation& i){
	o <<"("<<std::setw(4)<<i.start_keyframe<<", "<<std::setw(4)<<i.number_of_frames<<")";
	return o;
}

inline std::ostream& operator<<(std::ostream& o,const spriteSheet& i){
	o <<"("<<std::setw(6)<<i.offset<<", "<<std::setw(4)<<i.width<<", "<<std::setw(4)<<i.height<<", "<<std::setw(4)<<i.unknown0<<", "<<std::setw(4)<<i.unknown1<<")";
	return o;
}

inline std::ostream& operator<<(std::ostream& o,const keyframe& i){
	o <<"("<<std::setw(4)<<i.external_sheet<<", "<<std::setw(4)<<(int)i.sheet<<", "<<std::setw(4)<<(int)i.colortable<<", "<<std::setw(4)<<i.offsetx
			<<", "<<std::setw(4)<<i.offsety<<", "<<std::setw(4)<<i.x<<", "<<std::setw(4)<<i.y<<", "<<std::setw(4)<<i.width<<", "<<std::setw(4)<<i.height<<", "<<std::setw(4)<<i.scalex
			<<", "<<std::setw(4)<<i.scaley<<", "<<std::setw(4)<<i.rotationx<<", "<<std::setw(4)<<i.rotationy<<", "<<std::setw(4)<<i.rotationz<<", "<<std::setw(4)<<i.mirror<<")";
	return o;
}


class SpriteSheet {
public:
	EXPORT SpriteSheet();
	EXPORT SpriteSheet(const std::string& filename);
	EXPORT SpriteSheet(const PG::UTIL::File& file);

	EXPORT bool open(const PG::UTIL::File& file);
	EXPORT bool open(const std::string& filename);

	const PG::UTIL::File& getOpenedFile() const{
		return m_file;
	}
	EXPORT unsigned int getNumberOfAnimations() const;
	EXPORT unsigned int getNumberOfKeyframes() const;
	EXPORT unsigned int getNumberOfSpriteSheets() const;
	EXPORT unsigned int getNumberOfColorTables() const;

	EXPORT animation& getAnimation(unsigned int index);
	EXPORT const animation& getAnimation(unsigned int index) const;

	EXPORT keyframe& getKeyframe(unsigned int index);
	EXPORT const keyframe& getKeyframe(unsigned int index) const;
	const std::vector<keyframe>& getKeyframes() const{
		return m_keyframes;
	}

	EXPORT void getKeyframes(unsigned animationIndex, std::vector<keyframe>& keyframesIn) const;

	EXPORT const PG::UTIL::IDImage& getSpriteSheet(unsigned int index) const;
	const std::vector< PG::UTIL::IDImage >&  getSpriteSheets() const{
		return m_spriteSheets;
	}
	EXPORT const ColorTable& getColorTable(unsigned int index) const;
	const std::vector< ColorTable > &  getColorTables() const{
		return m_colortables;
	}

	EXPORT void filedump(std::stringstream& o) const;
	//void filedump() const;

	//friend std::ostream& operator<<(std::ostream& o,const PG::FILE::SpriteSheet& sheet);

	EXPORT bool isOpen() const;
	EXPORT void clear();

	EXPORT virtual ~SpriteSheet();

private:
	PG::UTIL::File m_file;

	spriteSheetHeader m_header;

	std::vector<something0> m_unknown0;//(header.number_of_something0);
	std::vector<animation> m_animations;//(header.number_of_animations);
	std::vector<unsigned int> m_numberOfColortables;//(header.number_of_colortablesSets);
	std::vector<spriteSheet> m_sheetsInfos;//(header.number_of_sheets);

	std::vector<something1> m_unknown1;//( ((unsigned int)addresses[1]-(unsigned int)addresses[0])/sizeof(part0));
	std::vector<keyframe> m_keyframes;//(header.number_of_keyframes);

	std::vector< ColorTable > m_colortables;
	std::vector< PG::UTIL::IDImage > m_spriteSheets;
};

inline std::ostream& operator<<(std::ostream& o,const PG::FILE::SpriteSheet& sheet){

	std::stringstream str;
	sheet.filedump(str);
	o<<str.str();
	return o;
}

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_SPRITESHEET_FILE_H_ */
