/*
 * Exports for java JNI.
 *
 *  Copyright (C) 2016  ProgSys
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

	const unsigned short color_table_size = reader.readUnsignedShort();
	const unsigned short noidear2 = reader.readUnsignedShort();
	const unsigned int total_texture_bytes = reader.readUnsignedInt(); //without the header and midmap

	char* rgbaImage = new char[width*height*4];

	if(compressiontype == 256){
		//lookup table RGBA
		std::vector<PG::UTIL::rgba> colortable(color_table_size);
		reader.read((char*)&colortable[0], colortable.size()*sizeof(PG::UTIL::rgba));

		for(unsigned int i = 0; i < total_texture_bytes; i++){
			const unsigned int index = i*4;
			const PG::UTIL::rgba& rgba = colortable[reader.readUnsignedChar()];
			rgbaImage[index] = rgba.r;
			rgbaImage[index+1] = rgba.g;
			rgbaImage[index+2] = rgba.b;
			rgbaImage[index+3] = rgba.a;
		}

	}else if(compressiontype == 3){
			//BGRA
			std::vector<PG::UTIL::rgba> bgra(width*height);
			reader.read((char*)&bgra[0], bgra.size()*sizeof(PG::UTIL::rgba));

			for(unsigned int i = 0; i < bgra.size(); i++){
				const unsigned int index = i*4;
				const PG::UTIL::rgba& bgrac = bgra[i];
				rgbaImage[index] = bgrac.b;
				rgbaImage[index+1] = bgrac.g;
				rgbaImage[index+2] = bgrac.r;
				rgbaImage[index+3] = bgrac.a;
			}

	}else if(compressiontype == 0 || compressiontype == 16){
			//DXT1
			reader.skip(color_table_size*4);

			const unsigned int number_of_blocks_width = (width/4);
			const unsigned int number_of_blocks_height = (height/4);
			const unsigned int number_of_blocks_4x4 = number_of_blocks_width*number_of_blocks_height;

			std::vector<PG::FILE::DXT1block> blocks(number_of_blocks_4x4);
			reader.read((char*)&blocks[0], number_of_blocks_4x4*sizeof(PG::FILE::DXT1block));

			uncompressDXT1(width,height, blocks, rgbaImage);

	}else if(compressiontype == 2){
		//DXT5
		reader.skip(color_table_size*4);

		const unsigned int number_of_blocks_width = (width/4);
		const unsigned int number_of_blocks_height = (height/4);
		const unsigned int number_of_blocks_4x4 = number_of_blocks_width*number_of_blocks_height;

		std::vector<PG::FILE::DXT5block> blocks(number_of_blocks_4x4);
		reader.read((char*)&blocks[0], number_of_blocks_4x4*sizeof(PG::FILE::DXT5block));

		uncompressDXT5(width,height, blocks, rgbaImage);


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
