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

#include <pg/stream/PG_StreamOut.h>
#include <pg/stream/PG_StreamOutByteFile.h>

namespace PG {
namespace FILE {

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
		reader.close();
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

tx2Image::~tx2Image(){}

bool tx2Image::save(const std::string& file) const{
	if(file.empty()) return FAILURE;
	try {
		PG::STREAM::OutByteFile writer(file);
		if(!writer.isOpen()) return FAILURE;

		writer.writeShort(header.width);
		writer.writeShort(header.height);
		writer.writeShort(header.type);

		writer.writeChar(log10(header.width)/0.301029995);
		writer.writeChar(log10(header.height)/0.301029995);
		writer.writeShort(header.colortableSize);
		writer.writeShort(header.colortables.size());
		writer.writeInt(header.width*header.height);

		writer.write((char*)&data[0], data.size());

		writer.close();
	} catch (PG::UTIL::Exception& e) {
		PG_ERROR_STREAM(e.what());
		PG_ERROR_STREAM("Couldn't write tx2 image.")
		return FAILURE;
	} catch (...) {
		PG_ERROR_STREAM("Couldn't write tx2 image.")
		return FAILURE;
	}


	return SUCCESS;
}
bool tx2Image::open(const std::string& file){
	if(file.empty()) return FAILURE;


	try {
		PG::STREAM::InByteFile reader(file);
		if(!reader.isopen()) return FAILURE;
		readTX2Header(&reader,header);

		switch (header.type) {
			case tx2Type::DXT1:
			{
				const unsigned int number_of_blocks_width = (header.width/4);
				const unsigned int number_of_blocks_height = (header.height/4);
				const unsigned int number_of_blocks_4x4 = number_of_blocks_width*number_of_blocks_height;

				//wrong file size?
				if( reader.size() < (number_of_blocks_4x4*8+16)){
					PG_ERROR_STREAM("File too small! ("<<reader.size()<<" < "<<(number_of_blocks_4x4*8+16)<<")");
					return true;
				}

				data.resize(number_of_blocks_4x4*sizeof(PG::FILE::DXT1block));
			}
				break;
			case tx2Type::DXT5:
			{
				const unsigned int number_of_blocks_width = (header.width/4);
				const unsigned int number_of_blocks_height = (header.height/4);
				const unsigned int number_of_blocks_4x4 = number_of_blocks_width*number_of_blocks_height;

				//wrong file size?
				if( reader.size() < (number_of_blocks_4x4*16+16)){
					PG_ERROR_STREAM("File too small! ("<<reader.size()<<" < "<<(number_of_blocks_4x4*16+16)<<")");
					return true;
				}

				data.resize( number_of_blocks_4x4*sizeof(PG::FILE::DXT5block));

			}
				break;
			case tx2Type::BGRA:
			{
				data.resize( header.width*header.height*sizeof(PG::FILE::rgba));
			}
				break;
			case tx2Type::COLORTABLE_BGRA16:
			case tx2Type::COLORTABLE_RGBA16:
			{
				data.resize( header.width*header.height*0.5);
			}
				break;
			case tx2Type::COLORTABLE_BGRA256:
			case tx2Type::COLORTABLE_RGBA256:
			{
				data.resize( header.width*header.height);
			}
				break;
			default:
			{
				reader.close();
				return FAILURE;
			}
				break;
		}

		reader.read((char*)&data[0], data.size());

		reader.close();
	} catch (PG::UTIL::Exception& e) {
		PG_ERROR_STREAM(e.what());
		PG_ERROR_STREAM("Couldn't read tx2 image.")
		return FAILURE;
	} catch (...) {
		PG_ERROR_STREAM("Couldn't read tx2 image.")
		return FAILURE;
	}

	return SUCCESS;
}

unsigned int tx2Image::getRGBADataSize() const{
	return header.width*header.height*sizeof(PG::UTIL::rgba);
}

void tx2Image::getRGBAData(char* rgbaOut, unsigned short colortable) const{
	switch (header.type) {
		case tx2Type::DXT1:
		{
			const unsigned int number_of_blocks_width = (header.width/4);
			const unsigned int number_of_blocks_height = (header.height/4);
			const unsigned int number_of_blocks_4x4 = number_of_blocks_width*number_of_blocks_height;

			PG::FILE::decompressS3<PG::FILE::DXT1block>(header.width,header.height, data, rgbaOut);
		}
			break;
		case tx2Type::DXT5:
		{

			const unsigned int number_of_blocks_width = (header.width/4);
			const unsigned int number_of_blocks_height = (header.height/4);
			const unsigned int number_of_blocks_4x4 = number_of_blocks_width*number_of_blocks_height;

			PG::FILE::decompressS3<PG::FILE::DXT5block>(header.width,header.height, data, rgbaOut);
		}
			break;
		case tx2Type::BGRA:
		{
			for(unsigned int i = 0; i < data.size();i += 4){
				rgbaOut[i] = data[i+2];
				rgbaOut[i+1] = data[i+1];
				rgbaOut[i+2] = data[i];
				rgbaOut[i+3] = data[i+3];
			}
		}
			break;
		case tx2Type::COLORTABLE_BGRA16:
		{
			assert_Test("Color table ID out of bound!", colortable >= header.colortables.size());
			const ColorTable& table =  header.colortables[colortable];
			for(unsigned int i = 0; i < data.size(); ++i){
				const char c = data[i];

				const unsigned int pos1 = i*2*sizeof(PG::UTIL::rgba);
				const unsigned int pos2 = pos1+sizeof(PG::UTIL::rgba);
				const PG::UTIL::rgba& co1 = table[ c & 0x0F];
				const PG::UTIL::rgba& co2 = table[ (c >> 4) & 0x0F ];
				rgbaOut[pos1] = co1.b;
				rgbaOut[pos1+1] = co1.g;
				rgbaOut[pos1+2] = co1.r;
				rgbaOut[pos1+3] = co1.a;

				rgbaOut[pos2] = co2.b;
				rgbaOut[pos2+1] = co2.g;
				rgbaOut[pos2+2] = co2.r;
				rgbaOut[pos2+3] = co2.a;
			}
		}
			break;
		case tx2Type::COLORTABLE_RGBA16:
		{
			assert_Test("Color table ID out of bound!", colortable >= header.colortables.size());
			const ColorTable& table =  header.colortables[colortable];
			for(unsigned int i = 0; i < data.size(); ++i){
				const char c = data[i];

				const unsigned int pos1 = i*2*sizeof(PG::UTIL::rgba);
				const unsigned int pos2 = pos1+sizeof(PG::UTIL::rgba);
				const PG::UTIL::rgba& co1 = table[ c & 0x0F];
				const PG::UTIL::rgba& co2 = table[ (c >> 4) & 0x0F ];
				rgbaOut[pos1] = co1.r;
				rgbaOut[pos1+1] = co1.g;
				rgbaOut[pos1+2] = co1.b;
				rgbaOut[pos1+3] = co1.a;

				rgbaOut[pos2] = co2.r;
				rgbaOut[pos2+1] = co2.g;
				rgbaOut[pos2+2] = co2.b;
				rgbaOut[pos2+3] = co2.a;
			}
		}
			break;
		case tx2Type::COLORTABLE_BGRA256:
		{
			assert_Test("Color table ID out of bound!", colortable >= header.colortables.size());
			const ColorTable& table =  header.colortables[colortable];
			for(unsigned int i = 0; i < data.size(); ++i){
				const unsigned int pos = i*sizeof(PG::UTIL::rgba);

				const PG::UTIL::rgba& co = table[ data[i]];
				rgbaOut[pos] = co.b;
				rgbaOut[pos+1] = co.g;
				rgbaOut[pos+2] = co.r;
				rgbaOut[pos+3] = co.a;
			}
		}
			break;
		case tx2Type::COLORTABLE_RGBA256:
		{
			assert_Test("Color table ID out of bound!", colortable >= header.colortables.size());
			const ColorTable& table =  header.colortables[colortable];
			for(unsigned int i = 0; i < data.size(); ++i){
				const unsigned int pos = i*sizeof(PG::UTIL::rgba);

				const PG::UTIL::rgba& co = table[ data[i]];
				rgbaOut[pos] = co.r;
				rgbaOut[pos+1] = co.g;
				rgbaOut[pos+2] = co.b;
				rgbaOut[pos+3] = co.a;
			}
		}
			break;
		default:
		break;
	}
}


void tx2Image::setWithRGBA(unsigned short width, unsigned short height, const char* rgbaIn ){
	clear(tx2Type::BGRA);
	header.width = width;
	header.height = height;

	data.resize(width*height*4);
	for(unsigned int i = 0; i < data.size(); i+=4){
		data[i] = rgbaIn[i+2];
		data[i+1] = rgbaIn[i+1];
		data[i+2] = rgbaIn[i];
		data[i+3] = rgbaIn[i+3];
	}

}

void tx2Image::convertTo(tx2Type compressionTypeIn){

}

void tx2Image::clear(tx2Type type){
	data.clear();
	header.colortableSize = 0;
	header.colortables.clear();
	header.height = 0;
	header.width = 0;
	header.type = type;
}


} /* namespace FILE */
} /* namespace PG */
