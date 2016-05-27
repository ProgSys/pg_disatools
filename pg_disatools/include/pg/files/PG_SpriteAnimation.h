/*
 * PG_SpriteAnimation.h
 *
 *  Created on: 17.04.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_PG_FILES_PG_SPRITEANIMATION_H_
#define INCLUDE_PG_FILES_PG_SPRITEANIMATION_H_

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
	unsigned char isExternalSheet; // get a sheet from different file by it's ID
	unsigned short sheetID; // there is not one big sprite sheet but multiple smaller one
	unsigned short x;
	unsigned short y;
	unsigned short width;
	unsigned short height;

} __attribute__((packed, aligned(1))) cutout;

typedef struct
{
	unsigned int cutoutID;
	unsigned char colortableID; // the 16 rgba colortable which the sheet should use

	short anchorx; // rotation and mirror point
	short anchory; // rotation and mirror point
	unsigned short scalex; // 0-100
	unsigned short scaley; // 0-100
	short offsetx; // offset from the anchor
	short offsety; // offset from the anchor
	short rotation; // is degree

	unsigned char mirror; //not confirmed

} __attribute__((packed, aligned(1))) layer;

struct keyframe
{
	unsigned int durration = 1;
	std::vector<layer> layers;
};

struct animation{
	animation(unsigned int _ID = 0, const std::string _name = "undefined");

	std::string name = "undefined";
	unsigned int ID;
	std::vector<keyframe> keyframes;

	const keyframe& getKeyframeByFrame(unsigned int frame) const;
	keyframe& getKeyframeByFrame(unsigned int frame);
	unsigned int getKeyframeIDByFrame(unsigned int frame) const;

	const unsigned int getLenght() const;
};

typedef std::vector<PG::UTIL::rgba> ColorTable;

class SpriteAnimation {
public:
	SpriteAnimation();

	bool isOpen() const;
	bool open(const std::string& file);
	bool save(const std::string& file);

	bool importSH(const std::string& file);
	bool exportSH(const std::string& file);

	const PG::UTIL::File& getOpenedFile() const;

	//getters
	unsigned int getNumberOfAnimations() const;
	const animation& getAnimation(unsigned int index) const;
	animation& getAnimation(unsigned int index);
	const std::vector<animation>& getAnimations() const;
	std::vector<animation>& getAnimations();

	unsigned int getNumberOfCutouts() const;
	const cutout& getCutout(unsigned int index) const;
	cutout& getCutout(unsigned int index);
	const std::vector<cutout>& getCutouts() const;
	std::vector<cutout>& getCutouts();

	unsigned int getNumberOfColortables() const;
	unsigned int getColortableSize() const;
	const ColorTable& getColortable() const;
	ColorTable& getColortable();

	unsigned int getNumberOfSheets() const;
	const PG::UTIL::IDImage& getSheet(unsigned int index) const;
	PG::UTIL::IDImage& getSheet(unsigned int index);
	const std::vector< PG::UTIL::IDImage >& getSheets() const;
	std::vector< PG::UTIL::IDImage >& getSheets();

	void clear();

	void dump(std::ostream& o) const;

	friend std::ostream& operator<<(std::ostream& o,const PG::FILE::SpriteAnimation& pgani){
		pgani.dump(o);
		return o;
	}

	virtual ~SpriteAnimation();
private:
	PG::UTIL::File m_file;

	std::vector<animation> m_animations;
	std::vector<cutout> m_cutouts;

	ColorTable m_colortables;
	std::vector< PG::UTIL::IDImage > m_spriteSheets;
};

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_SPRITEANIMATION_H_ */
