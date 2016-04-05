/*
 * PG_SpriteSheet.cpp
 *
 *  Created on: 31.03.2016
 *      Author: ProgSys
 */

#include <pg/files/PG_SpriteSheet.h>
#include <pg/util/PG_Base.h>
#include <pg/stream/PG_StreamInByteFile.h>

namespace PG {
namespace FILE {

SpriteSheet::SpriteSheet() {
	// TODO Auto-generated constructor stub

}

SpriteSheet::SpriteSheet(const std::string& filename): SpriteSheet(PG::UTIL::File(filename)){

}

SpriteSheet::SpriteSheet(const PG::UTIL::File& file){
	open(file);
}

bool SpriteSheet::open(const PG::UTIL::File& file){
	clear();
	if(file.isEmpty()){
		PG_ERROR_STREAM("Given filename is empty!");
		return FAILURE;
	}
	m_file = file;

	PG::STREAM::InByteFile reader(m_file);
	if(!reader.isopen()){
		clear();
		PG_ERROR_STREAM("Coudn't open file!");
		return FAILURE;
	}
	const unsigned int file_size = reader.size();
	if(file_size < 32){
		clear();
		PG_ERROR_STREAM("Given file is too small!");
		return FAILURE;
	}

	reader.read((char*)&m_header,sizeof(spriteSheetHeader) );

	std::vector<unsigned int> addresses(4);
	reader.read((char*)&addresses[0],addresses.size()*sizeof(int) );

	if(file_size <addresses[3]){
		clear();
		PG_ERROR_STREAM("Given file is too small!");
		return FAILURE;
	}

	m_unknown0.resize(m_header.number_of_something0);
	m_animations.resize(m_header.number_of_animations);
	m_numberOfColortables.resize(m_header.number_of_colortablesSets);
	m_sheetsInfos.resize(m_header.number_of_sheets);

	m_unknown1.resize(m_header.number_of_something1);
	m_keyframes.resize(m_header.number_of_keyframes);


	reader.read((char*)&m_unknown0[0],m_unknown0.size()*sizeof(something0) );
	reader.read((char*)&m_animations[0],m_animations.size()*sizeof(animation) );
	reader.read((char*)&m_numberOfColortables[0],m_numberOfColortables.size()*sizeof(unsigned int) );
	reader.read((char*)&m_sheetsInfos[0],m_sheetsInfos.size()*sizeof(spriteSheet) );
	reader.seek(addresses[0]);
	reader.read((char*)&m_unknown1[0],m_unknown1.size()*sizeof(something1) );
	reader.seek(addresses[1]);
	reader.read((char*)&m_keyframes[0],m_keyframes.size()*sizeof(keyframe) );

	//read colortables
	reader.seek(addresses[2]);
	for(unsigned int i = 0; i < m_numberOfColortables[0]; ++i){
		std::vector<PG::UTIL::rgba> colortable(16);
		reader.read((char*)&colortable[0],colortable.size()*sizeof(PG::UTIL::rgba) );

		//flip Red and Blue
		for(PG::UTIL::rgba& color: colortable){
			const char r = color.r;
			color.r = color.b;
			color.b = r;
		}

		m_colortables.push_back(colortable);
	}

	//read sheet color IDs
	for(const spriteSheet& sheet: m_sheetsInfos){
		PG::UTIL::IDImage sheetIDs(sheet.width,sheet.height);
		reader.seek(sheet.offset);
		for(unsigned int i = 0; i < sheetIDs.size(); i+=2){
			const char c = reader.readChar();
			sheetIDs[i] =  c & 0x0F;
			sheetIDs[i+1] = (c >> 4) & 0x0F ;
		}
		m_spriteSheets.push_back(sheetIDs);
	}

	return SUCCESS;
}

bool SpriteSheet::open(const std::string& filename){
	return open(PG::UTIL::File(filename));
}

bool SpriteSheet::isOpen() const{
	return !m_file.isEmpty();
}

unsigned int SpriteSheet::getNumberOfAnimations() const{
	return m_animations.size();
}
unsigned int SpriteSheet::getNumberOfKeyframes() const{
	return m_keyframes.size();
}
unsigned int SpriteSheet::getNumberOfSpriteSheets() const{
	return m_spriteSheets.size();
}
unsigned int SpriteSheet::getNumberOfColorTables() const{
	return m_colortables.size();
}

animation& SpriteSheet::getAnimation(unsigned int index){
	return m_animations[index];
}
const animation& SpriteSheet::getAnimation(unsigned int index) const{
	return m_animations[index];
}

keyframe& SpriteSheet::getKeyframe(unsigned int index){
	return m_keyframes[index];
}
const keyframe& SpriteSheet::getKeyframe(unsigned int index) const{
	return m_keyframes[index];
}

void SpriteSheet::getKeyframes(unsigned animationIndex, std::vector<keyframe>& keyframesIn) const{
	const animation& ani = getAnimation(animationIndex);
	keyframesIn.clear();
	keyframesIn.reserve(ani.number_of_frames);
	for(unsigned int i = ani.start_keyframe; i <=  ani.start_keyframe+ani.number_of_frames; ++i){
		keyframesIn.push_back(m_keyframes[i]);
	}
}

const PG::UTIL::IDImage& SpriteSheet::getSpriteSheet(unsigned int index) const{
	return m_spriteSheets[index];
}
const ColorTable& SpriteSheet::getColorTable(unsigned int index) const{
	return m_colortables[index];
}


void SpriteSheet::clear(){
	m_file.clear();

	m_unknown0.clear();
	m_animations.clear();
	m_numberOfColortables.clear();
	m_sheetsInfos.clear();

	m_unknown1.clear();
	m_keyframes.clear();

	m_colortables.clear();
	m_spriteSheets.clear();
}

SpriteSheet::~SpriteSheet() {
	// TODO Auto-generated destructor stub
}

} /* namespace FILE */
} /* namespace PG */
