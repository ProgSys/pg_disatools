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


#include <pg/stream/PG_StreamIn.h>
#include <pg/stream/PG_StreamInByteFile.h>
#include <pg/stream/PG_StreamInByteArray.h>

namespace PG {
namespace FILE {



bool decompressTX2(PG::STREAM::In* instream,const tx2Image::tx2header& img, PG::UTIL::RGBAImage& imageOut ){
	if(img.type == tx2Type::DXT1){
			//DXT1
			instream->skip(img.colortableSize*4);

			const unsigned int number_of_blocks_width = (img.width/4);
			const unsigned int number_of_blocks_height = (img.height/4);
			const unsigned int number_of_blocks_4x4 = number_of_blocks_width*number_of_blocks_height;

			//wrong file size?
			if( instream->size() < (number_of_blocks_4x4*8+16)){
				PG_ERROR_STREAM("File too small! ("<<instream->size()<<" < "<<(number_of_blocks_4x4*8+16)<<")");
				return true;
			}

			std::vector<PG::FILE::DXT1block> blocks(number_of_blocks_4x4);
			instream->read((char*)&blocks[0], number_of_blocks_4x4*sizeof(PG::FILE::DXT1block));

			PG::FILE::decompressS3(img.width,img.height, blocks, imageOut);

		}else if(img.type ==  tx2Type::DXT5){
			//DXT5
			instream->skip(img.colortableSize*4);

			const unsigned int number_of_blocks_width = (img.width/4);
			const unsigned int number_of_blocks_height = (img.height/4);
			const unsigned int number_of_blocks_4x4 = number_of_blocks_width*number_of_blocks_height;

			//wrong file size?
			if( instream->size() < (number_of_blocks_4x4*16+16)){
				PG_ERROR_STREAM("File too small! ("<<instream->size()<<" < "<<(number_of_blocks_4x4*16+16)<<")");
				return true;
			}

			std::vector<PG::FILE::DXT5block> blocks(number_of_blocks_4x4);
			instream->read((char*)&blocks[0], number_of_blocks_4x4*sizeof(PG::FILE::DXT5block));


			PG::FILE::decompressS3(img.width,img.height, blocks, imageOut);

		}else if(img.type == tx2Type::BGRA){
			//BGRA
			if( instream->size() < (img.width*img.height*4+16)){
				PG_ERROR_STREAM("File too small! ("<<instream->size()<<" < "<<(img.width*img.height*4+16)<<")");
				return true;
			}

			imageOut.resize(img.width,img.height);
			instream->read((char*)&imageOut[0], imageOut.size()*sizeof(PG::UTIL::rgba));

			//flip r and b
			for(PG::UTIL::rgba& rgba: imageOut){
				const unsigned char r =  rgba.r;
				rgba.r = rgba.b;
				rgba.b = r;
			}
		}else if(img.type == tx2Type::COLORTABLE_BGRA16 || img.type == tx2Type::COLORTABLE_RGBA16){
				//lookup table RGBA with max 16 values

				if(img.colortableSize > 16){
					PG_WARN_STREAM("Color table is too big!  ("<<img.colortableSize<<" but should be 16)");
				}

				const unsigned int total_number_of_bytes = (img.width*img.height)/2; // every byte holds two table values
				if( instream->size() < (total_number_of_bytes+img.colortableSize+16)){
					PG_ERROR_STREAM("File too small! ("<<instream->size()<<" < " << (total_number_of_bytes+img.colortableSize+16)<<") ");
					return true;
				}

				imageOut.resize(img.width,img.height);


				for(unsigned int i = 0; i < total_number_of_bytes; ++i){
					const char c = instream->readUnsignedChar();

					const unsigned int pos = i*2;
					imageOut[pos] = img.colortables[0][ c & 0x0F];
					imageOut[pos+1] = img.colortables[0][ (c >> 4) & 0x0F ];
				}

		}else if(img.type == tx2Type::COLORTABLE_BGRA256 || img.type == tx2Type::COLORTABLE_RGBA256){
			//lookup table RGBA

			if(img.colortableSize > 256){
				PG_ERROR_STREAM("Color table is too big!");
				return true;
			}

			const unsigned int total_number_of_bytes = (img.width*img.height);
			if( instream->size() < (total_number_of_bytes+img.colortableSize+16)){
				PG_ERROR_STREAM("File too small! ("<<instream->size()<<" < " << (total_number_of_bytes+img.colortableSize+16)<<") ");
				return true;
			}


			imageOut.resize(img.width,img.height);

			for(unsigned int i = 0; i < total_number_of_bytes; i++){
				imageOut[i] = img.colortables[0][instream->readUnsignedChar()];
			}


		}else{
			PG_ERROR_STREAM("Unknown format not supported!");
			return true;
		}

		return false;
}

bool readTX2Header(PG::STREAM::In* instream, tx2Image::tx2header& header){
	header.width = instream->readUnsignedShort();
	header.height = instream->readUnsignedShort();

	//invalid with or height
	if(header.width <= 0 || header.width > 15000 || header.height <= 0 || header.height > 15000){
		PG_ERROR_STREAM("Invalid width or height!");
		return true;
	}

	header.type = static_cast<tx2Type>(instream->readUnsignedShort());
	instream->skip(2);
	header.colortableSize = instream->readUnsignedShort();
	const unsigned short numberOfColortables = instream->readUnsignedShort();

	header.colortables.resize((numberOfColortables)? numberOfColortables: (header.type == tx2Type::BGRA || header.type == tx2Type::DXT1 || header.type == tx2Type::DXT5 || header.type == tx2Type::TX2ERROR)? 0: 1);

	instream->skip(4);


	for(ColorTable& colortable: header.colortables ){
		colortable.resize(header.colortableSize);
		instream->read((char*)&colortable[0], header.colortableSize*sizeof(PG::UTIL::rgba));


		if(header.type == tx2Type::COLORTABLE_BGRA16 || header.type == tx2Type::COLORTABLE_BGRA256)
			//flip R and B channel
			for(PG::UTIL::rgba& color: colortable){
				const char r = color.r;
				color.r = color.b;
				color.b = r;
			}
	}



	return false;
}

bool decompressTX2(PG::STREAM::In* instream, PG::UTIL::RGBAImage& imageOut  ){
	if(instream == nullptr){
		PG_ERROR_STREAM("InStream is nullptr!");
		return true;
	}
	if(instream->size() < 16){
		PG_ERROR_STREAM("File is smaller then 16 byte!");
		return true;
	}

	tx2Image::tx2header header;
	if(readTX2Header(instream, header))
		return true;


	return decompressTX2(instream, header, imageOut);
}

bool decompressTX2(const tx2Image& img, PG::UTIL::RGBAImage& imageOut ){

	PG::STREAM::InByteArray reader(&img.data[0], img.data.size());
	const bool s = decompressTX2((PG::STREAM::In*) &reader, img.header, imageOut);
	return s;
}

bool decompressTX2(const PG::UTIL::File& file, PG::UTIL::RGBAImage& imageOut ){
	try {
		PG::STREAM::InByteFile reader(file);
		const bool s = decompressTX2((PG::STREAM::In*) &reader, imageOut);
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
	PG::STREAM::InByteArray reader(&bytesIn[0], bytesIn.size());
	const bool s = decompressTX2((PG::STREAM::In*) &reader, imageOut);
	return s;
}

bool decompressTX2(const char* bytesIn, unsigned int lenghtIn, PG::UTIL::RGBAImage& imageOut ){
	PG::STREAM::InByteArray reader(bytesIn, lenghtIn);
	const bool s = decompressTX2((PG::STREAM::In*) &reader, imageOut);
	return s;
}






bool compressTX2(const PG::UTIL::RGBAImage& imageIn, tx2Type compressionTypeIn, std::vector<char>& bytesOut){

	if(compressionTypeIn == DXT1){
		const unsigned short width =  imageIn.getWidth();
		const unsigned short height =  imageIn.getHeight();
		const unsigned int number_of_blocks_width = (width/4);
		const unsigned int number_of_blocks_height = (height/4);
		const unsigned int number_of_blocks_4x4 = number_of_blocks_width*number_of_blocks_height;

		bytesOut.resize(16+number_of_blocks_4x4*8);

		memcpy(&bytesOut[0], &width, 2);
		memcpy(&bytesOut[2], &height, 2);

		bytesOut[6] = log10(width)/0.301029995;
		bytesOut[7] = log10(height)/0.301029995;
		const unsigned int size = width*height;
		memcpy(&bytesOut[12], &size, 4);
		std::vector<DXT1block> blocks;

		if(PG::FILE::compressS3(imageIn,blocks)){
			PG_ERROR_STREAM("Couldn't compress image.");
			return true;
		}
		memcpy(&bytesOut[16], &blocks[0], number_of_blocks_4x4*8);
	}else if(compressionTypeIn == DXT5){
			const unsigned short width =  imageIn.getWidth();
			const unsigned short height =  imageIn.getHeight();
			const unsigned int number_of_blocks_width = (width/4);
			const unsigned int number_of_blocks_height = (height/4);
			const unsigned int number_of_blocks_4x4 = number_of_blocks_width*number_of_blocks_height;

			bytesOut.resize(16+number_of_blocks_4x4*16);

			memcpy(&bytesOut[0], &width, 2);
			memcpy(&bytesOut[2], &height, 2);
			bytesOut[4] = 0x02;
			bytesOut[6] = log10(width)/0.301029995;
			bytesOut[7] = log10(height)/0.301029995;
			const unsigned int dimantion = width*4*16;
			memcpy(&bytesOut[12], &dimantion, 4);
			std::vector<DXT5block> blocks;

			if(PG::FILE::compressS3(imageIn,blocks)){
				PG_ERROR_STREAM("Couldn't compress image.");
				return true;
			}
			memcpy(&bytesOut[16], &blocks[0], number_of_blocks_4x4*16);
	}else if(compressionTypeIn == BGRA){
			const unsigned short width =  imageIn.getWidth();
			const unsigned short height =  imageIn.getHeight();
			const unsigned int size = width*height*4;
			bytesOut.resize(16+size);

			memcpy(&bytesOut[0], &width, 2);
			memcpy(&bytesOut[2], &height, 2);
			bytesOut[4] = 0x03;
			bytesOut[6] = log10(width)/0.301029995;
			bytesOut[7] = log10(height)/0.301029995;
			const unsigned int dimantion = width*height;
			memcpy(&bytesOut[12], &dimantion, 4);

			unsigned int i = 16;
			for(const PG::UTIL::rgba& color: imageIn){
				bytesOut[i] = color.b;
				bytesOut[i+1] = color.g;
				bytesOut[i+2] = color.r;
				bytesOut[i+3] = color.a;
				i += 4;
			}

	}else{
		PG_ERROR_STREAM("Compression type not supported, please choose another one.");
		return true;
	}

	return false;
}

} /* namespace FILE */
} /* namespace PG */
