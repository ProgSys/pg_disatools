/*
 * The MIT License (MIT)
 *
 *	Copyright (c) 2024 ProgSys
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
#include <ostream>

#include <Util/Typedefs.h>
#include <Util/PG_File.h>
#include <Util/PG_Image.h>
#include <Disa/fileformats/PG_TX2.h>
#include <Stream/PG_StreamOut.h>


PACK(typedef struct
{
	unsigned short number_of_animations;
	unsigned short number_of_bundels;
	unsigned short number_of_colortables;
	unsigned short number_of_sheetInfos;

	unsigned short number_of_keyframes;
	unsigned short number_of_cutouts;
	unsigned short number_of_colortable_data;
	unsigned short number_of_sheet_data;

}) shfileHeader;

PACK(typedef struct
{
	unsigned short start_keyframe;
	unsigned short id;
}) shfileAnimation;

PACK(typedef struct
{
	enum Type : unsigned char {
		Default = 0,
		LoopStart = 1, //indicates the start of the loop
		LoopEnd = 2, //indicates the end of the loop
		AutoPause = 3, //indicates where the loop is exited?
	};

	unsigned short bundel_index;
	unsigned char duration;
	Type type;
	short global_x;
	short global_y;
}) shfileKeyframe;

PACK(typedef struct
{
	unsigned short start_cutout;
	unsigned short number_of_cutouts;
})  shfileLayers;

PACK(typedef struct
{
	unsigned int offset;
	unsigned short width;
	unsigned short height;

	unsigned char power_colortablesize;
	unsigned char image_swizzled;
	unsigned char power_width;
	unsigned char power_height;

}) shfileSheetInfo;

PACK(typedef struct
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

	short offsetx; // offset from the anchor
	short offsety; // offset from the anchor
	short rotation; // is degree

	unsigned char transparency; //max is 128.f
	unsigned char mic; //mirror, rendermode

}) shfileCutout;

class PG_SH {
public:
	PG_SH() = default;

	// functions
	bool open(const std::string& file);
	bool open(const PG_File& file);
	bool open(PG::STREAM::In* stream);
	bool save(const std::string& file);
	bool save(const PG_File& file);
	bool save(PG::STREAM::Out* stream);
	void clear();

	// getters
	bool isOpen() const;

	inline const shfileHeader& getHeader() const { return m_header; };
	inline const std::vector<unsigned int>& getAdresses() const { return m_addresses; };

	inline std::vector<shfileAnimation>& getAnimations() { return m_animations; };
	inline const std::vector<shfileAnimation>& getAnimations() const { return m_animations; };

	inline  std::vector<shfileLayers>& getLayers() { return m_layers; };
	inline const std::vector<shfileLayers>& getLayers() const { return m_layers; };

	inline const std::vector<unsigned int>& getNumberOfColortables() const { return m_numberOfColortables; };

	inline std::vector<shfileSheetInfo>& getSheetsInfos() { return m_sheetsInfos; };
	inline const std::vector<shfileSheetInfo>& getSheetsInfos() const { return m_sheetsInfos; };

	inline std::vector<shfileKeyframe>& getKeyframes() { return m_keyframesData; };
	inline const std::vector<shfileKeyframe>& getKeyframes() const { return m_keyframesData; };

	inline std::vector<shfileCutout>& getCutouts() { return m_cutouts; };
	inline const std::vector<shfileCutout>& getCutouts() const { return m_cutouts; };

	inline std::vector<ColorTable>& getColortables() { return m_colortables; };
	inline const std::vector<ColorTable>& getColortables() const { return m_colortables; };

	inline std::vector<PG_UCharImage>& getSpriteSheets() { return m_spriteSheets; };
	inline const std::vector<PG_UCharImage>& getSpriteSheets() const { return m_spriteSheets; };

private:
	//data
	shfileHeader m_header;
	std::vector<unsigned int> m_addresses;
	std::vector<shfileAnimation> m_animations;
	std::vector<shfileLayers> m_layers;
	std::vector<unsigned int> m_numberOfColortables;
	std::vector<shfileSheetInfo> m_sheetsInfos;

	std::vector<shfileKeyframe> m_keyframesData;
	std::vector<shfileCutout> m_cutouts;

	std::vector<ColorTable> m_colortables;
	std::vector<PG_UCharImage> m_spriteSheets;
};
