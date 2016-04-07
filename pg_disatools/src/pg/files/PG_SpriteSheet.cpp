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

	std::vector<animationData> animations(m_header.number_of_animations);
	std::vector<bundelData> layers(m_header.number_of_bundels);
	std::vector<unsigned int> numberOfColortables(m_header.number_of_colortablesSets);
	std::vector<spriteSheet> sheetsInfos(m_header.number_of_sheets);

	std::vector<keyframeData> keyframesData(m_header.number_of_keyframes);
	std::vector<cutout> cutouts(m_header.number_of_cutouts);



	reader.read((char*)&animations[0],animations.size()*sizeof(animationData) );
	reader.read((char*)&layers[0],layers.size()*sizeof(bundelData) );
	reader.read((char*)&numberOfColortables[0],numberOfColortables.size()*sizeof(unsigned int) );
	reader.read((char*)&sheetsInfos[0],sheetsInfos.size()*sizeof(spriteSheet) );
	reader.seek(addresses[0]);
	reader.read((char*)&keyframesData[0],keyframesData.size()*sizeof(keyframeData) );
	reader.seek(addresses[1]);
	reader.read((char*)&cutouts[0],cutouts.size()*sizeof(cutout) );

	//read colortables
	reader.seek(addresses[2]);
	for(unsigned int i = 0; i < numberOfColortables[0]; ++i){
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
	for(const spriteSheet& sheet: sheetsInfos){
		PG::UTIL::IDImage sheetIDs(sheet.width,sheet.height);
		reader.seek(sheet.offset);
		for(unsigned int i = 0; i < sheetIDs.size(); i+=2){
			const char c = reader.readChar();
			sheetIDs[i] =  c & 0x0F;
			sheetIDs[i+1] = (c >> 4) & 0x0F ;
		}
		m_spriteSheets.push_back(sheetIDs);
	}
	reader.close();

	//set animation data
	m_animations.reserve(animations.size());
	for(const animationData& ani: animations){
		if(ani.start_keyframe >= keyframesData.size()) continue;

		m_animations.push_back(animation2D(ani.unknown1));
		animation2D& ani2D = m_animations.back();

		unsigned int currKey = ani.start_keyframe;
		while(currKey < keyframesData.size() && keyframesData[currKey].type != 2 ){
			ani2D.keyframes.push_back(animation2D::keyframe(keyframesData[currKey],layers, cutouts));
			currKey++;
		}

	}

	return SUCCESS;
}

bool SpriteSheet::open(const std::string& filename){
	return open(PG::UTIL::File(filename));
}

bool SpriteSheet::isOpen() const{
	return !m_file.isEmpty();
}

/*
const PG::UTIL::File& SpriteSheet::getOpenedFile() const{
	return m_file;
}*/

unsigned int SpriteSheet::getNumberOfAnimations() const{
	return m_animations.size();
}

unsigned int SpriteSheet::getNumberOfSpriteSheets() const{
	return m_spriteSheets.size();
}
unsigned int SpriteSheet::getNumberOfColorTables() const{
	return m_colortables.size();
}

animation2D& SpriteSheet::getAnimation(unsigned int index){
	return m_animations[index];
}
const animation2D& SpriteSheet::getAnimation(unsigned int index) const{
	return m_animations[index];
}

const std::vector<animation2D>& SpriteSheet::getAnimations() const{
	return m_animations;
}

const PG::UTIL::IDImage& SpriteSheet::getSpriteSheet(unsigned int index) const{
	return m_spriteSheets[index];
}

const std::vector< PG::UTIL::IDImage >&  SpriteSheet::getSpriteSheets() const{
	return m_spriteSheets;
}


const ColorTable& SpriteSheet::getColorTable(unsigned int index) const{
	return m_colortables[index];
}


const std::vector< ColorTable >&  SpriteSheet::getColorTables() const{
	return m_colortables;
}


template <typename T>
inline void printInt(const std::vector<T>& arr, std::stringstream& o){
	unsigned int count = 0;
	for(const T& i: arr){
		 o<<i<<", ";
		count++;
		if(count == 4){
			 o<<"\n";
			count = 0;
		}
	}
	o<<"\n";
}

void SpriteSheet::filedump(std::stringstream& o) const{
	if(!isOpen()){
		o<<"No file open!";
		return;
	}

	o<<"File: '"<<m_file<<"'\n";
	o<<"Header: \n";
	o<<m_header<<"\n";

	/*
	reader.read((char*)&m_unknown0[0],m_unknown0.size()*sizeof(something0) );
	reader.read((char*)&m_animations[0],m_animations.size()*sizeof(animation) );
	reader.read((char*)&m_numberOfColortables[0],m_numberOfColortables.size()*sizeof(unsigned int) );
	reader.read((char*)&m_sheetsInfos[0],m_sheetsInfos.size()*sizeof(spriteSheet) );
	reader.seek(addresses[0]);
	reader.read((char*)&m_unknown1[0],m_unknown1.size()*sizeof(something1) );
	reader.seek(addresses[1]);
	reader.read((char*)&m_keyframes[0],m_keyframes.size()*sizeof(keyframe) );
	*/

	o<<"Animations: "<< m_animations.size()<<"\n";
	unsigned int i = 0;
	for(const animation2D& ani: m_animations){
		o<<" Animation "<<i<<", with "<<ani.keyframes.size()<< " keyframes: \n";
		unsigned int ikey = 0;
		for(const animation2D::keyframe& key: ani.keyframes){
			o<<"  * Key "<<ikey<<" type: "<<(int)key.type<< " delay: "<< (int)key.delay<<", with "<<key.layers.size() <<" layers:\n";
			unsigned int ilayer = 0;
			for(const cutout& cut: key.layers){
				o<<"    - Layer "<<ilayer<<": "<< cut <<"\n";
				ilayer++;
			}

			ikey++;
		}
		i++;
	}

}


void SpriteSheet::clear(){
	m_file.clear();

	m_animations.clear();

	m_colortables.clear();
	m_spriteSheets.clear();
}


SpriteSheet::~SpriteSheet() {
	// TODO Auto-generated destructor stub
}

} /* namespace FILE */
} /* namespace PG */
