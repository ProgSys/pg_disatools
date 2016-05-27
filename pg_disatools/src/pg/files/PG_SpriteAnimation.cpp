/*
 * PG_SpriteAnimation.cpp
 *
 *  Created on: 17.04.2016
 *      Author: ProgSys
 */

#include <pg/files/PG_SpriteAnimation.h>
#include <algorithm>
#include <pg/util/PG_Base.h>
#include <pg/stream/PG_StreamInByteFile.h>
#include <pg/stream/PG_StreamOutByteFile.h>
#include <pg/util/PG_Exception.h>

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

} __attribute__((packed, aligned(1))) shHeader;

typedef struct
{
	unsigned short start_keyframe;
	unsigned short id;
} __attribute__((packed, aligned(1))) shAnimation;

typedef struct
{
	unsigned short bundel_index;
	unsigned char delay;
	unsigned char type;
	unsigned short unknown2;
	unsigned short unknown3;
} __attribute__((packed, aligned(1))) shKeyframe;

typedef struct
{
	unsigned short start_cutout;
	unsigned short number_of_cutouts;
}__attribute__((packed, aligned(1))) shBundel;

typedef struct
{
	unsigned int offset;
	unsigned short width;
	unsigned short height;

	unsigned short unknown0;
	unsigned short unknown1;

}__attribute__((packed, aligned(1))) shSheetInfo;

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

	bool operator== (const cutout& cut) const{
		if(cut.isExternalSheet){
			return cut.sheetID == external_sheet && cut.x == x && cut.y == y && cut.width == width && cut.height == height;
		}else{
			return cut.sheetID == sheet && cut.x == x && cut.y == y && cut.width == width && cut.height == height;
		}

	}

} __attribute__((packed, aligned(1))) shCutout;



animation::animation(unsigned int _ID, const std::string _name):
	name(_name),ID(_ID)
{}


const keyframe& animation::getKeyframeByFrame(unsigned int frame) const{
	unsigned int lenght = 0;
	for(const keyframe& key: keyframes){
		lenght += key.durration;
		if(frame < lenght) return key;
	}
	return keyframes.back();
}

keyframe& animation::getKeyframeByFrame(unsigned int frame){
	return const_cast<keyframe&>(getKeyframeByFrame(frame));
}

unsigned int animation::getKeyframeIDByFrame(unsigned int frame) const{
	unsigned int lenght = 0;
	unsigned int index = 0;
	for(const keyframe& key: keyframes){
		lenght += key.durration;
		if(frame < lenght) return index;
		index++;
	}
	return keyframes.size()-1;
}

const unsigned int animation::getLenght() const{
	unsigned int lenght = 0;
	for(const keyframe& key: keyframes){
		lenght += key.durration;
	}
	return lenght;
}

SpriteAnimation::SpriteAnimation() {
	// TODO Auto-generated constructor stub

}

bool SpriteAnimation::isOpen() const{
	return !m_file.isEmpty();
}

bool SpriteAnimation::open(const std::string& file){
	if(file.empty()){
		PG_ERROR_STREAM("Given filename is empty!");
		return FAILURE;
	}
	try{
		PG::STREAM::InByteFile reader(file);
		if(!reader.isopen()){
			PG_ERROR_STREAM("Coudn't open file!");
			return FAILURE;
		}

		if(reader.readString(8) != "PG_SHANI"){
			PG_ERROR_STREAM("Magic number is wrong!");
			return FAILURE;
		}

		if(reader.readInt() != 1){
			PG_ERROR_STREAM("Invalid version number! (Should be '1')");
			return FAILURE;
		}

		reader.seek(reader.readUnsignedInt());
		std::vector<unsigned int> addresses(4);
		reader.read((char*)&addresses[0],addresses.size()*sizeof(int) );
		clear();
		m_file = file;

		reader.seek(addresses[0]);
		m_cutouts.resize(reader.readUnsignedShort());
		reader.read((char*)&m_cutouts[0], sizeof(cutout)*m_cutouts.size());

		reader.seek(addresses[1]);
		const unsigned int animationsSize = reader.readUnsignedInt();
		m_animations.reserve(animationsSize);
		for(unsigned int i = 0; i < animationsSize; ++i){
			animation ani;
			const unsigned short strSize = reader.readUnsignedShort();
			if(strSize > 0) ani.name = reader.readString(strSize);
			ani.ID = reader.readUnsignedInt();

			const unsigned int keyframesSize = reader.readUnsignedShort();
			ani.keyframes.reserve(keyframesSize);
			for(unsigned int k = 0; k < keyframesSize; ++k){
				keyframe key;
				key.durration = reader.readUnsignedInt();
				key.layers.resize(reader.readUnsignedShort());
				reader.read((char*)&key.layers[0], sizeof(layer)*key.layers.size());
				ani.keyframes.push_back(key);
			}
			m_animations.push_back(ani);
		}

		reader.seek(addresses[2]);
		m_colortables.resize(reader.readUnsignedShort());
		reader.read((char*)&(m_colortables[0].x), sizeof(PG::UTIL::rgba)*m_colortables.size());


		reader.seek(addresses[3]);
		const unsigned int sheetsSize = reader.readUnsignedShort();
		for(unsigned int i = 0; i < sheetsSize; ++i){
			const unsigned short width = reader.readUnsignedShort();
			const unsigned short height = reader.readUnsignedShort();
			m_spriteSheets.push_back(PG::UTIL::IDImage(width,height));
			reader.read((char*)&m_spriteSheets.back()[0], m_spriteSheets.back().size());
		}

	}catch (PG::UTIL::Exception& e) {
		 PG_ERROR_STREAM("Couldn't import PG sprite animation! : "<<e.what());
		 return FAILURE;
	}catch (...) {
		 PG_ERROR_STREAM("Couldn't import PG sprite animation!");
		 return FAILURE;
	}


	return SUCCESS;
}

bool SpriteAnimation::save(const std::string& file){
	if(file.empty()){
		PG_ERROR_STREAM("Given filename is empty!");
		return FAILURE;
	}

	try{
		PG::STREAM::OutByteFile writer(file);
		if(!writer.isOpen()){
			PG_ERROR_STREAM("Coudn't save file!");
			return FAILURE;
		}
		m_file = file;

		writer.writeString("PG_SHANI");
		writer.writeInt(1);
		writer.writeInt(16);

		writer.writeInt(0);
		writer.writeInt(0);
		writer.writeInt(0);
		writer.writeInt(0);


		const unsigned int startCutouts = writer.getPosition();
		writer.writeInt(m_cutouts.size());
		writer.write((char*)&m_cutouts[0], sizeof(cutout)*m_cutouts.size());

		const unsigned int startAnimations = writer.getPosition();
		writer.writeInt(m_animations.size());
		for(const animation& ani: m_animations){
			writer.writeShort(ani.name.size());
			writer.writeString(ani.name);
			writer.writeInt(ani.ID);
			writer.writeShort(ani.keyframes.size());
			for(const keyframe& key: ani.keyframes){
				writer.writeInt(key.durration);
				writer.writeShort(key.layers.size());
				writer.write((char*)&key.layers[0], sizeof(layer)*key.layers.size());
			}
		}

		const unsigned int startColortables = writer.getPosition();
		writer.writeShort(m_colortables.size());
		writer.write((char*)&(m_colortables[0].x), sizeof(PG::UTIL::rgba)*m_colortables.size());

		const unsigned int startImages = writer.getPosition();
		writer.writeShort(m_spriteSheets.size());
		for(const PG::UTIL::IDImage& sheet: m_spriteSheets){
			writer.writeShort(sheet.getWidth());
			writer.writeShort(sheet.getHeight());
			writer.write((char*)&sheet[0], sheet.size());
		}

		writer.writeString("FileInfo: cutouts - animations infos (-> keyframes -> layers) - colortables - char images");

		writer.seek(16);
		writer.writeInt(startCutouts);
		writer.writeInt(startAnimations);
		writer.writeInt(startColortables);
		writer.writeInt(startImages);

		writer.close();
	}catch (PG::UTIL::Exception& e) {
		 PG_ERROR_STREAM("Couldn't save sprite animation! : "<<e.what());
		 return FAILURE;
	}catch (...) {
		 PG_ERROR_STREAM("Couldn't save sprite animation!");
		 return FAILURE;
	}


	return SUCCESS;
}

bool SpriteAnimation::importSH(const std::string& file){
	if(file.empty()){
		PG_ERROR_STREAM("Given filename is empty!");
		return FAILURE;
	}

	try{
		PG::STREAM::InByteFile reader(file);
		if(!reader.isopen()){
			PG_ERROR_STREAM("Coudn't open file!");
			return FAILURE;
		}

		const unsigned int file_size = reader.size();
		if(file_size < 32){
			clear();
			PG_ERROR_STREAM("Given file is too small!");
			return FAILURE;
		}

		shHeader header;
		reader.read((char*)&header,sizeof(shHeader));

		std::vector<unsigned int> addresses(4);
		reader.read((char*)&addresses[0],addresses.size()*sizeof(int) );

		if(file_size < addresses[3]){
			PG_ERROR_STREAM("Given file is too small!");
			return FAILURE;
		}

		std::vector<shAnimation> animations(header.number_of_animations);
		std::vector<shBundel> layers(header.number_of_bundels);
		std::vector<unsigned int> numberOfColortables(header.number_of_colortablesSets);
		std::vector<shSheetInfo> sheetsInfos(header.number_of_sheets);

		std::vector<shKeyframe> keyframesData(header.number_of_keyframes);
		std::vector<shCutout> cutouts(header.number_of_cutouts);

		reader.read((char*)&animations[0],animations.size()*sizeof(shAnimation) );
		reader.read((char*)&layers[0],layers.size()*sizeof(shBundel) );
		reader.read((char*)&numberOfColortables[0],numberOfColortables.size()*sizeof(unsigned int) );
		reader.read((char*)&sheetsInfos[0],sheetsInfos.size()*sizeof(shSheetInfo) );
		reader.seek(addresses[0]);
		reader.read((char*)&keyframesData[0],keyframesData.size()*sizeof(shKeyframe) );
		reader.seek(addresses[1]);
		reader.read((char*)&cutouts[0],cutouts.size()*sizeof(shCutout) );

		clear();
		m_file = file;
		//read colortables
		reader.seek(addresses[2]);

		m_colortables.resize(16*numberOfColortables[0]);
		reader.read((char*)&m_colortables[0],m_colortables.size()*sizeof(PG::UTIL::rgba) );
		//flip Red and Blue
		for(PG::UTIL::rgba& color: m_colortables){
			const char r = color.r;
			color.r = color.b;
			color.b = r;
		}

		//read sheet color IDs
		for(const shSheetInfo& sheet: sheetsInfos){
			PG::UTIL::IDImage sheetIDs(sheet.width,sheet.height);
			reader.seek(sheet.offset);
			if(sheet.unknown0 == 8)
				//color table 256
				reader.read((char*) &sheetIDs[0], sheetIDs.size());
			else
				//color table 16
				for(unsigned int i = 0; i < sheetIDs.size(); i+=2){
					const char c = reader.readChar();
					sheetIDs[i] =  c & 0x0F;
					sheetIDs[i+1] = (c >> 4) & 0x0F ;
				}


			m_spriteSheets.push_back(sheetIDs);
		}
		reader.close();

		//reformat
		m_animations.reserve(animations.size());
		for(const shAnimation& ani: animations){
			m_animations.push_back(animation(ani.id));
			std::vector<keyframe>& keys = m_animations.back().keyframes;

			//go over all keyframes
			unsigned int currKey = ani.start_keyframe;
			while(currKey < keyframesData.size()){
				if(keyframesData[currKey].type == 2) break;

				keyframe key;
				key.durration = keyframesData[currKey].delay;

				//go over all layers
				const shBundel& bundle = layers[keyframesData[currKey].bundel_index];
				for(unsigned int shCut = bundle.start_cutout; shCut < bundle.start_cutout+bundle.number_of_cutouts; ++shCut){
					const shCutout& shCutData = cutouts[shCut];

					layer lay = {0, shCutData.colortable,
							shCutData.anchorx, shCutData.anchory,
							shCutData.scalex, shCutData.scaley,
							shCutData.offsetx, shCutData.offsety,
							shCutData.rotation, shCutData.mirror};

					//search if there is already a cutout with the same values
					auto it = std::find_if(m_cutouts.begin(), m_cutouts.end(), [shCutData](const cutout& cutP){
						return shCutData == cutP;
					});
					if(it != m_cutouts.end()){
						lay.cutoutID = std::distance(m_cutouts.begin(), it);
					}else{
						//insert new cutout
						cutout myCut;
						if(shCutData.external_sheet){
							myCut = {1, shCutData.external_sheet, shCutData.x, shCutData.y, shCutData.width, shCutData.height};
						}else{
							myCut = {0, shCutData.sheet, shCutData.x, shCutData.y, shCutData.width, shCutData.height};
						}
						m_cutouts.push_back(myCut);
						lay.cutoutID = m_cutouts.size()-1;
					}

					key.layers.push_back(lay);

				}
				currKey++;
				keys.push_back(key);
			}//while end


		}

	}catch (PG::UTIL::Exception& e) {
		 PG_ERROR_STREAM("Couldn't import SH sprite animation! : "<<e.what());
		 return FAILURE;
	}catch (...) {
		 PG_ERROR_STREAM("Couldn't import SH sprite animation!");
		 return FAILURE;
	}


	return SUCCESS;
}

bool SpriteAnimation::exportSH(const std::string& file){
	PG_ERROR_STREAM("Not implemented!");
	return FAILURE;
}

const PG::UTIL::File&  SpriteAnimation::getOpenedFile() const{
	return m_file;
}

unsigned int SpriteAnimation::getNumberOfAnimations() const{
	return m_animations.size();
}

const animation& SpriteAnimation::getAnimation(unsigned int index) const{
	return m_animations[index];
}

animation& SpriteAnimation::getAnimation(unsigned int index){
	return m_animations[index];
}

const std::vector<animation>& SpriteAnimation::getAnimations() const{
	return m_animations;
}

std::vector<animation>& SpriteAnimation::getAnimations(){
	return m_animations;
}

unsigned int SpriteAnimation::getNumberOfCutouts() const{
	return m_cutouts.size();
}

const cutout& SpriteAnimation::getCutout(unsigned int index) const{
	return m_cutouts[index];
}

cutout& SpriteAnimation::getCutout(unsigned int index){
	return m_cutouts[index];
}

const std::vector<cutout>& SpriteAnimation::getCutouts() const{
	return m_cutouts;
}

std::vector<cutout>& SpriteAnimation::getCutouts(){
	return m_cutouts;
}

unsigned int SpriteAnimation::getNumberOfColortables() const{
	assert_Test("Each colortable must be size of 16!", m_colortables.size()%16 != 0);
	return  m_colortables.size()/16;
}

unsigned int SpriteAnimation::getColortableSize() const{
	assert_Test("Each colortable must be size of 16!", m_colortables.size()%16 != 0);
	return  m_colortables.size();
}

const ColorTable& SpriteAnimation::getColortable() const{
	return  m_colortables;
}

ColorTable& SpriteAnimation::getColortable(){
	return  m_colortables;
}

unsigned int SpriteAnimation::getNumberOfSheets() const{
	return  m_spriteSheets.size();
}

const PG::UTIL::IDImage& SpriteAnimation::getSheet(unsigned int index) const{
	return  m_spriteSheets[index];
}

PG::UTIL::IDImage& SpriteAnimation::getSheet(unsigned int index){
	return  m_spriteSheets[index];
}

const std::vector< PG::UTIL::IDImage >& SpriteAnimation::getSheets() const{
	return m_spriteSheets;
}

std::vector< PG::UTIL::IDImage >& SpriteAnimation::getSheets(){
	return m_spriteSheets;
}

void SpriteAnimation::clear(){
	m_file.clear();

	m_animations.clear();
	m_cutouts.clear();

	m_colortables.clear();
	m_spriteSheets.clear();
}

void SpriteAnimation::dump(std::ostream& o) const{
	o <<"TODO!";
}

SpriteAnimation::~SpriteAnimation() {
	// TODO Auto-generated destructor stub
}

} /* namespace FILE */
} /* namespace PG */
