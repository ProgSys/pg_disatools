/*
 * The MIT License (MIT)
 *
 *	Copyright (c) 2016 ProgSys
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

/*
 * PG_S3File.cpp
 *
 *  Created on: 25.02.2016
 *      Author: ProgSys
 */

#include <pg/files/PG_S3File.h>
#include <algorithm>
#include <type_traits>

#include <pg/util/PG_BinaryFileTokenizer.h>
#include <pg/util/PG_BinaryFileWriter.h>
#include <pg/util/PG_Array.h>
#include <pg/util/PG_Base.h>
#include <pg/files/PG_S3Compression.h>
#include <pg/files/PG_ImageFiles.h>

namespace PG {
namespace FILE {

using namespace UTIL;

S3File::S3File(const std::string& filename): m_filename(filename) {
	m_width = 0;
	m_height = 0;

	std::string extention = filename.substr(filename.find_last_of(".") + 1);
	std::transform(extention.begin(), extention.end(), extention.begin(), ::tolower);

	if(extention == "vtf"){
		PG::UTIL::BinaryFileTokenizer reader(m_filename);
		if(reader.getString(3) != "VTF"){
			PG_ERROR_STREAM("Magic number is not VTF!");
			m_filetype = INVALID;
			return;
		}

		reader.skip(13);
		m_width = reader.getShort();
		m_height = reader.getShort();
		if(m_width == 0 || m_height == 0){
			PG_ERROR_STREAM("Image width or height is zero!");
			m_filetype = INVALID;
			return;
		}

		m_filetype = VTF;
	}else if(extention == "tx2"){
		PG::UTIL::BinaryFileTokenizer reader(m_filename);
		m_width = reader.getUnsignedShort();
		m_height = reader.getUnsignedShort();
		if(m_width == 0 || m_height == 0){
			PG_ERROR_STREAM("Image width or height is zero!");
			m_filetype = INVALID;
			return;
		}
		m_filetype = TX2;
	}else if(extention == "mpp"){
		PG_ERROR_STREAM("*.mpp is a achieve containing multiple files.")
	}else{
		PG_ERROR_STREAM("Unknown file extension'"<<extention<< "'. Only vtf and tx2 (Disgaea) is supported.")
	}

}

unsigned int S3File::getWidth() const{
	return m_width;
}

unsigned int S3File::getHeight() const{
	return m_height;
}

bool S3File::save(const std::string& outfilename, outFormat out) const{
	std::vector<RGBA> data;
	if(!m_filetype) return false;
	else if(m_filetype == VTF){
		if( !readVTF(data) )
			return false;
	}else if(m_filetype == TX2){
		if( !readTX2(data) )
			return false;
	}

	switch (out) {
		case PGM:
		{
			PG_INFO_STREAM("Writing PGM. (RGB)");
			savePGM(outfilename, m_width, m_height, data);
		}
			break;
		case TGA:
		{
			PG_INFO_STREAM("Writing TGA. (RGBA)");
			saveTGA(outfilename, m_width, m_height, data);
		}
			break;
		default:
			return false;
			break;
	}

	return false;
}

bool S3File::readVTF(std::vector<RGBA>& outRGBAData) const{
	PG::UTIL::BinaryFileTokenizer reader(m_filename);

	if(reader.getString(3) != "VTF"){
		PG_ERROR_STREAM("Magic number is not VTF!");
		return false;
	}

	reader.skip(13);
	if(m_width != reader.getShort() || m_height != reader.getShort()){
		PG_ERROR_STREAM("The image has different size then expected!");
		return false;
	}
	reader.skip(60);

	if(m_width >= 32 || m_height >= 32)
		reader.skip(4 * 4 *8 );


	const unsigned int number_of_blocks_width = (m_width/4);
	const unsigned int number_of_blocks_height =(m_height/4);
	const unsigned int number_of_blocks_4x4 = number_of_blocks_width*number_of_blocks_height;

	std::vector<DXT1block> blocks(number_of_blocks_4x4);
	reader.read((char*)&blocks[0], number_of_blocks_4x4*sizeof(DXT1block) );
	reader.close();

	uncompressDXT1(m_width,m_height, blocks, outRGBAData);

	return true;
}

bool S3File::readTX2(std::vector<RGBA>& outRGBAData) const{
	PG::UTIL::BinaryFileTokenizer reader(m_filename);

	if( m_width != reader.getUnsignedShort() || m_height != reader.getUnsignedShort()){
		PG_ERROR_STREAM("The image has different size then expected!");
		return false;
	}

	//? not shure
	//0 = no compression
	//2 = DXT5
	const unsigned short compressiontype = reader.getUnsignedShort();
	const unsigned short noidear = reader.getUnsignedShort(); //seams always to be 2056
	//well it used for a gradient?
	const unsigned short color_table_size = reader.getUnsignedShort();
	const unsigned short noidear2 = reader.getUnsignedShort();
	const unsigned int total_texture_bytes = reader.getUnsignedInt(); //without the header and midmap

	PG_INFO_STREAM(compressiontype<<" m: "<<color_table_size<<" "<<noidear2 <<" total: "<<total_texture_bytes );




	if(compressiontype == 256){

		std::vector<PG::UTIL::simpleRGBA> colortable(color_table_size);
		reader.read((char*)&colortable[0], colortable.size()*sizeof(simpleRGBA));

		outRGBAData.resize(total_texture_bytes);
		for(unsigned int i = 0; i < total_texture_bytes; i++){
			outRGBAData[i] = colortable[reader.getUnsignedChar()];
		}


	}else if(compressiontype == 2){
		reader.skip(color_table_size*4);

		const unsigned int number_of_blocks_width = (m_width/4);
		const unsigned int number_of_blocks_height = (m_height/4);
		const unsigned int number_of_blocks_4x4 = number_of_blocks_width*number_of_blocks_height;

		std::vector<DXT5block> blocks(number_of_blocks_4x4);
		reader.read((char*)&blocks[0], number_of_blocks_4x4*sizeof(DXT5block));
		reader.close();

		uncompressDXT5(m_width,m_height, blocks, outRGBAData);

	}else{
		PG_ERROR_STREAM("TX2 compression is not supported!");
	}

	return true;


}

/*
bool S3File::readMPP(std::vector<PG::UTIL::RGBA>& outRGBAData) const{
	PG::UTIL::BinaryFileTokenizer reader(m_filename);

	unsigned int unknown1 = reader.getUnsignedInt();
	unsigned int unknown2 = reader.getUnsignedInt();
	unsigned int fileSize = reader.getUnsignedInt();


	outRGBAData.resize(m_width*m_height);
	reader.skip(96);
	std::vector<unsigned char> dat(outRGBAData.size()*4);
	reader.read((char*)&dat[0], dat.size());

	for(unsigned int i = 0; i < dat.size(); i+=4){
		RGBA& rgba = outRGBAData[i/4];
		rgba.r = dat[i];
		rgba.g = dat[i+1];
		rgba.b = dat[i+2];
		rgba.a = dat[i+3];
	}

	return true;
}*/

S3File::~S3File() {
	// TODO Auto-generated destructor stub
}

} /* namespace FILE */
} /* namespace PG */
