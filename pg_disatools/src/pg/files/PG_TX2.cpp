/*
 * TX2.cpp
 *
 *  Created on: 29.02.2016
 *      Author: ProgSys
 */

#include <pg/files/PG_TX2.h>
#include <pg/files/PG_S3Compression.h>
#include <pg/util/PG_Exception.h>
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

#include <pg/util/PG_InStream.h>
#include <pg/util/PG_ByteInFileStream.h>
#include <pg/util/PG_ByteInStream.h>

namespace PG {
namespace FILE {

bool decompressTX2(PG::UTIL::InStream* instream, PG::UTIL::RGBAImage& imageOut  ){
	if(instream == nullptr){
		PG_ERROR_STREAM("InStream is nullptr!");
		return true;
	}
	if(instream->size() < 16){
		PG_ERROR_STREAM("File is smaller then 16 byte!");
		return true;
	}


	int widthOut = instream->readUnsignedShort();
	int heightOut = instream->readUnsignedShort();

	//invalid with or height
	if(widthOut <= 0 || widthOut > 15000 || heightOut <= 0 || heightOut > 15000){
		PG_ERROR_STREAM("Invalid width or height!");
		return true;
	}

	const unsigned short compressiontype = instream->readUnsignedShort();
	const unsigned short noidear = instream->readUnsignedShort(); //seams always to be 2056?

	const unsigned short color_table_size = instream->readUnsignedShort();
	const unsigned short noidear2 = instream->readUnsignedShort();

	const unsigned int total_texture_bytes = instream->readUnsignedInt(); //without the header and colortable



	if(compressiontype == tx2Type::DXT1){
		//DXT1
		instream->skip(color_table_size*4);

		const unsigned int number_of_blocks_width = (widthOut/4);
		const unsigned int number_of_blocks_height = (heightOut/4);
		const unsigned int number_of_blocks_4x4 = number_of_blocks_width*number_of_blocks_height;

		//wrong file size?
		if( instream->size() < (number_of_blocks_4x4*8+16))
			return true;

		std::vector<PG::FILE::DXT1block> blocks(number_of_blocks_4x4);
		instream->read((char*)&blocks[0], number_of_blocks_4x4*sizeof(PG::FILE::DXT1block));

		PG::FILE::decompressS3(widthOut,heightOut, blocks, imageOut);

	}else if(compressiontype ==  tx2Type::DXT5){
		//DXT5
		instream->skip(color_table_size*4);

		const unsigned int number_of_blocks_width = (widthOut/4);
		const unsigned int number_of_blocks_height = (heightOut/4);
		const unsigned int number_of_blocks_4x4 = number_of_blocks_width*number_of_blocks_height;

		//wrong file size?
		if( instream->size() < (number_of_blocks_4x4*16+16))
			return true;

		std::vector<PG::FILE::DXT5block> blocks(number_of_blocks_4x4);
		instream->read((char*)&blocks[0], number_of_blocks_4x4*sizeof(PG::FILE::DXT5block));

		PG::FILE::decompressS3(widthOut,heightOut, blocks, imageOut);

	}else if(compressiontype == tx2Type::BGRA){
		//BGRA
		if( instream->size() < (widthOut*heightOut*4+16))
			return true;

		imageOut.resize(widthOut,heightOut);
		instream->read((char*)&imageOut[0], imageOut.size()*sizeof(PG::UTIL::rgba));

		//flip r and b
		for(PG::UTIL::rgba& rgba: imageOut){
			const unsigned char r =  rgba.r;
			rgba.r = rgba.b;
			rgba.b = r;
		}
	}else if(compressiontype == tx2Type::UNKNOWN1){
		PG_ERROR_STREAM("Format not supported!");
		return true;
	}else if(compressiontype == tx2Type::COLORTABLERGBA){
		//lookup table RGBA

		if( instream->size() < (total_texture_bytes+color_table_size+16))
			return true;

		std::vector<PG::UTIL::rgba> colortable(color_table_size);
		instream->read((char*)&colortable[0], colortable.size()*sizeof(PG::UTIL::rgba));

		imageOut.resize(widthOut,heightOut);
		for(unsigned int i = 0; i < total_texture_bytes; i++){
			imageOut[i] = colortable[instream->readUnsignedChar()];
		}

	}else{
		PG_ERROR_STREAM("Unknown format not supported!");
		return true;
	}

	return false;
}

bool decompressTX2(const PG::UTIL::File& file, PG::UTIL::RGBAImage& imageOut ){
	try {
		PG::UTIL::ByteInFileStream reader(file);
		const bool s = decompressTX2((PG::UTIL::InStream*) &reader, imageOut);
		return s;
	} catch (PG::UTIL::Exception& e) {
		PG_ERROR_STREAM(e.what());
		PG_ERROR_STREAM("Couldn't read tx2 image.")
		return true;
	} catch (...) {
		PG_ERROR_STREAM("Couldn't read tx2 image.")
		return true;
	}

	return false;

}

bool decompressTX2(const std::string& file, PG::UTIL::RGBAImage& imageOut ){
	return decompressTX2(PG::UTIL::File(file),imageOut);
}

bool decompressTX2(const std::vector<char>& bytesIn, PG::UTIL::RGBAImage& imageOut ){
	PG::UTIL::ByteInStream reader(&bytesIn[0], bytesIn.size());
	const bool s = decompressTX2((PG::UTIL::InStream*) &reader, imageOut);
	return s;
}

bool decompressTX2(const char* bytesIn, unsigned int lenghtIn, PG::UTIL::RGBAImage& imageOut ){
	PG::UTIL::ByteInStream reader(bytesIn, lenghtIn);
	const bool s = decompressTX2((PG::UTIL::InStream*) &reader, imageOut);
	return s;
}

} /* namespace FILE */
} /* namespace PG */
