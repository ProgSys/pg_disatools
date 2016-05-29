/*
 * PG_SH.h
 *
 *  Created on: 29.05.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_PG_FILES_PG_SH_H_
#define INCLUDE_PG_FILES_PG_SH_H_

#include <string>
#include <vector>
#include <ostream>
#include <pg/util/PG_File.h>
#include <pg/util/PG_Vector.h>
#include <pg/util/PG_Image.h>

namespace PG {
namespace FILE {

typedef struct
{
	unsigned short number_of_animations;
	unsigned short number_of_bundels;
	unsigned short number_of_colortablesSets; //data between animations and sheets
	unsigned short number_of_sheets;

	unsigned short number_of_keyframes;
	unsigned short number_of_cutouts;
	unsigned short unknown6;
	unsigned short unknown7;

} __attribute__((packed, aligned(1))) shfileHeader;

typedef struct
{
	unsigned short start_keyframe;
	unsigned short id;
} __attribute__((packed, aligned(1))) shfileAnimation;

typedef struct
{
	unsigned short bundel_index;
	unsigned char duration;
	unsigned char type;
	unsigned short unknown2;
	unsigned short unknown3;
} __attribute__((packed, aligned(1))) shfileKeyframe;

typedef struct
{
	unsigned short start_cutout;
	unsigned short number_of_cutouts;
}__attribute__((packed, aligned(1))) shfileLayers;

typedef struct
{
	unsigned int offset;
	unsigned short width;
	unsigned short height;

	unsigned short unknown0;
	unsigned short unknown1;

}__attribute__((packed, aligned(1))) shfileSheetInfo;

typedef struct
{
	unsigned short external_sheet; // get a sheet from different file by it's ID
	unsigned char sheet; // there is not one big sprite sheet but multiple smaller one
	unsigned char colortable; // the 16 rgba colortable which the sheet should use
	short anchorx; // rotation and mirror point
	short anchory; // rotation and mirror point

	//sprite info
	unsigned short x;
	unsigned short y;
	unsigned short width;
	unsigned short height;
	unsigned short scalex; // 0-100
	unsigned short scaley; // 0-100

	// only rotationx and rotationz seam to have a effect on 2D sprites
	short offsetx; // offset from the anchor
	short offsety; // offset from the anchor
	short rotation; // is degree

	unsigned char mirror; //not confirmed
	unsigned char unkown0; //not confirmed

} __attribute__((packed, aligned(1))) shfileCutout;

typedef std::vector<PG::UTIL::rgba> ColorTable;

class SH {
public:
	SH();
	virtual ~SH();

	// functions
	bool open(const std::string& file);
	bool open(const PG::UTIL::File& file);
	void clear();

	// getters
	bool isOpen() const;
	inline const PG::UTIL::File& getOpenedFile() const { return m_openedFile;};
	inline const shfileHeader& getHeader() const { return m_header;};
	inline const std::vector<unsigned int>& getAdresses() const { return m_addresses;};
	inline const std::vector<shfileAnimation>& getAnimations() const { return m_animations;};
	inline const std::vector<shfileLayers>& getLayers() const { return m_layers;};
	inline const std::vector<unsigned int>& getNumberOfColortables() const { return m_numberOfColortables;};
	inline const std::vector<shfileSheetInfo>& getSheetsInfos() const { return m_sheetsInfos;};
	inline const std::vector<shfileKeyframe>& getKeyframes() const { return m_keyframesData;};
	inline const std::vector<shfileCutout>& getCutouts() const { return m_cutouts;};

	inline const ColorTable& getColortables() const { return m_colortables;};
	inline const std::vector< PG::UTIL::IDImage >& getSprtieSheets() const { return m_spriteSheets;};

private:
	PG::UTIL::File m_openedFile;

	//data
	shfileHeader m_header;
	std::vector<unsigned int> m_addresses;
	std::vector<shfileAnimation> m_animations;
	std::vector<shfileLayers> m_layers;
	std::vector<unsigned int> m_numberOfColortables;
	std::vector<shfileSheetInfo> m_sheetsInfos;

	std::vector<shfileKeyframe> m_keyframesData;
	std::vector<shfileCutout> m_cutouts;

	ColorTable m_colortables;
	std::vector< PG::UTIL::IDImage > m_spriteSheets;
};

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_SH_H_ */
