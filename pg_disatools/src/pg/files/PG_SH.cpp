/*
 * PG_SH.cpp
 *
 *  Created on: 29.05.2016
 *      Author: ProgSys
 */

#include <pg/files/PG_SH.h>
#include <algorithm>
#include <pg/util/PG_Base.h>
#include <pg/stream/PG_StreamInByteFile.h>
#include <pg/stream/PG_StreamOutByteFile.h>
#include <pg/util/PG_Exception.h>

namespace PG {
namespace FILE {

SH::SH() {
	// TODO Auto-generated constructor stub

}

bool SH::isOpen() const{
	return !m_openedFile.isEmpty();
}
bool SH::open(const std::string& file){
	if(file.empty()){
		PG_ERROR_STREAM("Given filename is empty!");
		return FAILURE;
	}

	return open(PG::UTIL::File(file));
}
bool SH::open(const PG::UTIL::File& file){
	clear();
	if(!file.exists()){
		PG_ERROR_STREAM("Target file doesn't exist!");
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

		reader.read((char*)&m_header,sizeof(shfileHeader));

		m_addresses.resize(4);
		reader.read((char*)&m_addresses[0],m_addresses.size()*sizeof(int) );

		if(file_size < m_addresses[3]){
			PG_ERROR_STREAM("Given file is too small!");
			return FAILURE;
		}

		m_openedFile = file;

		m_animations.resize(m_header.number_of_animations);
		m_layers.resize(m_header.number_of_bundels);
		m_numberOfColortables.resize(m_header.number_of_colortablesSets);
		m_sheetsInfos.resize(m_header.number_of_sheets);

		m_keyframesData.resize(m_header.number_of_keyframes);
		m_cutouts.resize(m_header.number_of_cutouts);

		reader.read((char*)&m_animations[0],m_animations.size()*sizeof(shfileAnimation) );
		reader.read((char*)&m_layers[0],m_layers.size()*sizeof(shfileLayers) );
		reader.read((char*)&m_numberOfColortables[0],m_numberOfColortables.size()*sizeof(unsigned int) );
		reader.read((char*)&m_sheetsInfos[0],m_sheetsInfos.size()*sizeof(shfileSheetInfo) );
		reader.seek(m_addresses[0]);
		reader.read((char*)&m_keyframesData[0],m_keyframesData.size()*sizeof(shfileKeyframe) );
		reader.seek(m_addresses[1]);
		reader.read((char*)&m_cutouts[0],m_cutouts.size()*sizeof(shfileCutout) );


		//read colortables
		reader.seek(m_addresses[2]);

		m_colortables.resize(16*m_numberOfColortables[0]);
		reader.read((char*)&m_colortables[0],m_colortables.size()*sizeof(PG::UTIL::rgba) );
		//flip Red and Blue
		for(PG::UTIL::rgba& color: m_colortables){
			const char r = color.r;
			color.r = color.b;
			color.b = r;
		}

		//read sheet color IDs
		for(const shfileSheetInfo& sheet: m_sheetsInfos){
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

	}catch (PG::UTIL::Exception& e) {
		 PG_ERROR_STREAM("Couldn't import SH sprite animation! : "<<e.what());
		 clear();
		 return FAILURE;
	}catch (...) {
		 PG_ERROR_STREAM("Couldn't import SH sprite animation!");
		 clear();
		 return FAILURE;
	}


	return SUCCESS;
}

void SH::clear(){
	m_openedFile.clear();
	m_addresses.clear();
	m_animations.clear();
	m_layers.clear();
	m_numberOfColortables.clear();
	m_sheetsInfos.clear();

	m_keyframesData.clear();
	m_cutouts.clear();

	m_colortables.clear();
	m_spriteSheets.clear();
}

SH::~SH() {
	// TODO Auto-generated destructor stub
}

} /* namespace FILE */
} /* namespace PG */
