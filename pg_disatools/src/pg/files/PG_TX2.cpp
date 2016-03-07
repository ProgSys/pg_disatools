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
#include <pg/files/PG_TX2.h>
#include <pg/files/PG_S3Compression.h>
#include <pg/util/PG_Exception.h>


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
	}else if(compressiontype == tx2Type::COLORTABLERGBA16){
		//lookup table RGBA with max 16 values

		if(color_table_size > 16){
			PG_ERROR_STREAM("Color table is too big!");
			return true;
		}

		const unsigned int total_number_of_bytes = (widthOut*heightOut)/2; // every byte holds two table values
		if( instream->size() < (total_number_of_bytes+color_table_size+16)){
			PG_ERROR_STREAM("File too small! ("<<instream->size()<<" < " << (total_number_of_bytes+color_table_size+16)<<") ");
			return true;
		}

		std::vector<PG::UTIL::rgba> colortable(16);
		instream->read((char*)&colortable[0], color_table_size*sizeof(PG::UTIL::rgba));

		imageOut.resize(widthOut,heightOut);


		for(unsigned int i = 0; i < total_number_of_bytes; ++i){
			const char c = instream->readUnsignedChar();

			const unsigned int pos = i*2;
			imageOut[pos] = colortable[ c & 0x0F];
			imageOut[pos+1] = colortable[ (c >> 4) & 0x0F ];
		}
	}else if(compressiontype == tx2Type::COLORTABLEBGRA16){
			//lookup table RGBA with max 16 values

			if(color_table_size > 16){
				PG_ERROR_STREAM("Color table is too big!");
				return true;
			}

			const unsigned int total_number_of_bytes = (widthOut*heightOut)/2; // every byte holds two table values
			if( instream->size() < (total_number_of_bytes+color_table_size+16)){
				PG_ERROR_STREAM("File too small! ("<<instream->size()<<" < " << (total_number_of_bytes+color_table_size+16)<<") ");
				return true;
			}

			std::vector<PG::UTIL::rgba> colortable(16);
			instream->read((char*)&colortable[0], color_table_size*sizeof(PG::UTIL::rgba));

			//flip R and B channel
			for(PG::UTIL::rgba& color: colortable){
				const char r = color.r;
				color.r = color.b;
				color.b = r;
			}

			imageOut.resize(widthOut,heightOut);


			for(unsigned int i = 0; i < total_number_of_bytes; ++i){
				const char c = instream->readUnsignedChar();

				const unsigned int pos = i*2;
				imageOut[pos] = colortable[ c & 0x0F];
				imageOut[pos+1] = colortable[ (c >> 4) & 0x0F ];
			}

	}else if(compressiontype == tx2Type::COLORTABLERGBA256){
		//lookup table RGBA

		if(color_table_size > 256){
			PG_ERROR_STREAM("Color table is too big!");
			return true;
		}

		const unsigned int total_number_of_bytes = (widthOut*heightOut);
		if( instream->size() < (total_number_of_bytes+color_table_size+16)){
			PG_ERROR_STREAM("File too small! ("<<instream->size()<<" < " << (total_number_of_bytes+color_table_size+16)<<") ");
			return true;
		}

		std::vector<PG::UTIL::rgba> colortable(256);
		instream->read((char*)&colortable[0], color_table_size*sizeof(PG::UTIL::rgba));

		imageOut.resize(widthOut,heightOut);
		for(unsigned int i = 0; i < total_number_of_bytes; i++){
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






bool compressTX2(PG::UTIL::RGBAImage& imageIn, tx2Type compressionTypeIn, std::vector<char>& bytesOut){

	if(compressionTypeIn == DXT1){
		const unsigned short width =  imageIn.getWidth();
		const unsigned short height =  imageIn.getHeight();
		const unsigned int number_of_blocks_width = (width/4);
		const unsigned int number_of_blocks_height = (height/4);
		const unsigned int number_of_blocks_4x4 = number_of_blocks_width*number_of_blocks_height;

		bytesOut.resize(16+number_of_blocks_4x4*8);

		memcpy(&bytesOut[0], &width, 2);
		memcpy(&bytesOut[2], &height, 2);

		bytesOut[6] = 0x08;
		bytesOut[7] = 0x08;
		const unsigned int size = width*height;
		memcpy(&bytesOut[12], &size, 4);
		std::vector<DXT1block> blocks;

		if(PG::FILE::compressS3(imageIn,blocks)){
			PG_ERROR_STREAM("Couldn't compress image.");
			return true;
		}
		memcpy(&bytesOut[16], &blocks[0], number_of_blocks_4x4*8);

	}else{
		PG_ERROR_STREAM("Compression type not supported, please choose another one.");
		return true;
	}

	return false;
}

} /* namespace FILE */
} /* namespace PG */
