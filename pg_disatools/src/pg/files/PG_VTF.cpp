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
#include <pg/files/PG_VTF.h>
#include <pg/files/PG_S3Compression.h>
#include <pg/util/PG_Exception.h>
#include <pg/stream/PG_StreamInByteFile.h>
#include <pg/stream/PG_StreamOutByteFile.h>

namespace PG {
namespace FILE {


bool decompressVTF(PG::STREAM::In* instream, PG::UTIL::RGBAImage& imageOut ){
	if(instream == nullptr || instream->size() < 16) return true;

	if(instream->readString(3) != "VTF"){
		PG_ERROR_STREAM("Magic number is not VTF!");
		return true;
	}

	instream->skip(13);
	unsigned int width = instream->readShort();
	unsigned int height = instream->readShort();
	//DXT1 768 //DXT5 8960
	unsigned short type = instream->readShort();

	if(width == 0 || width > 15000 || height == 0 || height > 15000)
		return true;

	instream->skip(58);

	if(width >= 32 || height >= 32)
		instream->skip(4 * 4 *8 );

	if(type == 768){
		//DXT1
		const unsigned int number_of_blocks_width = (width/4);
		const unsigned int number_of_blocks_height =(height/4);
		const unsigned int number_of_blocks_4x4 = number_of_blocks_width*number_of_blocks_height;

		std::vector<DXT1block> blocks(number_of_blocks_4x4);
		instream->read((char*)&blocks[0], number_of_blocks_4x4*sizeof(DXT1block) );
		instream->close();

		PG::FILE::decompressS3(width,height, blocks, imageOut);

	}else if(type ==  8960){
		//DXT5
		const unsigned int number_of_blocks_width = (width/4);
		const unsigned int number_of_blocks_height = (height/4);
		const unsigned int number_of_blocks_4x4 = number_of_blocks_width*number_of_blocks_height;

		//wrong file size?
		if( instream->size() < (number_of_blocks_4x4*16+16))
			return true;

		std::vector<PG::FILE::DXT5block> blocks(number_of_blocks_4x4);
		instream->read((char*)&blocks[0], number_of_blocks_4x4*sizeof(PG::FILE::DXT5block));

		PG::FILE::decompressS3(width,height, blocks, imageOut);
	}else{
		PG_ERROR_STREAM("Unsupported VTF format!");
		return true;
	}

	return false;
}


bool decompressVTF(const PG::UTIL::File& file, PG::UTIL::RGBAImage& imageOut ){
	try {
		PG::STREAM::InByteFile reader(file);
		const bool s = decompressVTF((PG::STREAM::In*) &reader, imageOut);
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

bool decompressVTF(const std::string& file, PG::UTIL::RGBAImage& imageOut ){
	return decompressVTF(PG::UTIL::File(file),imageOut);
}


} /* namespace FILE */
} /* namespace PG */
