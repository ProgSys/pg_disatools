/*
 * PG_JavaBridge.cpp
 *
 *  Created on: 27.02.2016
 *      Author: ProgSys
 */

#include <pg/util/PG_JavaBridge.h>
#include <stdexcept>
#include <vector>
#include <pg/files/PG_S3Compression.h>
#include <pg/util/PG_ByteStream.h>

namespace PG {
namespace UTIL {

void simpleCall(){
	int x = 3;
	return;
}

int multiply(float a, float b, float & result){
	result = a * b;
	return 0;
}

char* uncompressTX2Image(const char* bytes, int length, int& width, int& height){
	if(bytes == nullptr || length == 0)
		return nullptr;

	PG::UTIL::ByteStream reader(bytes, length);
	width = reader.readUnsignedShort();
	height = reader.readUnsignedShort();

	if(width <= 0 || width > 15000 || height <= 0 || height > 15000){
		width = 0;
		height = 0;
		return nullptr;
	}

	const unsigned short compressiontype = reader.readUnsignedShort();
	const unsigned short noidear = reader.readUnsignedShort(); //seams always to be 2056
	//well it used for a gradient?
	const unsigned short color_table_size = reader.readUnsignedShort();
	const unsigned short noidear2 = reader.readUnsignedShort();
	const unsigned int total_texture_bytes = reader.readUnsignedInt(); //without the header and midmap

	char* rgbaImage = new char[width*height*4];

	if(compressiontype == 256){

		std::vector<PG::UTIL::simpleRGBA> colortable(color_table_size);
		reader.read((char*)&colortable[0], colortable.size()*sizeof(simpleRGBA));

		for(unsigned int i = 0; i < total_texture_bytes; i++){
			const unsigned int index = i*4;
			const PG::UTIL::simpleRGBA& rgba = colortable[reader.readUnsignedChar()];
			rgbaImage[i] = rgba.r;
			rgbaImage[i+1] = rgba.g;
			rgbaImage[i+2] = rgba.b;
			rgbaImage[i+3] = rgba.a;
		}


	}else if(compressiontype == 2){
		reader.skip(color_table_size*4);

		const unsigned int number_of_blocks_width = (width/4);
		const unsigned int number_of_blocks_height = (height/4);
		const unsigned int number_of_blocks_4x4 = number_of_blocks_width*number_of_blocks_height;

		std::vector<PG::FILE::DXT5block> blocks(number_of_blocks_4x4);
		reader.read((char*)&blocks[0], number_of_blocks_4x4*sizeof(PG::FILE::DXT5block));

		std::vector<PG::UTIL::RGBA> outRGBAData;
		uncompressDXT5(width,height, blocks, outRGBAData);

		for(unsigned int i = 0; i < outRGBAData.size(); i++){
			const unsigned int index = i*4;
			const PG::UTIL::RGBA& rgba = outRGBAData[i];
			rgbaImage[index] = rgba.r;
			rgbaImage[index+1] = rgba.g;
			rgbaImage[index+2] = rgba.b;
			rgbaImage[index+3] = rgba.a;
		}

	}else{
		delete[] rgbaImage;
		width = 0;
		height = 0;
		return nullptr;
	}

	return rgbaImage;
}

} /* namespace UTIL */
} /* namespace PG */
