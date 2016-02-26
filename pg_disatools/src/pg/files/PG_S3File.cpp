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
#include <sstream>
#include <algorithm>
#include <type_traits>

#include <pg/util/PG_BinaryFileTokenizer.h>
#include <pg/util/PG_BinaryFileWriter.h>
#include <pg/util/PG_Array.h>
#include <pg/util/PG_Base.h>

namespace PG {
namespace FILE {

using namespace UTIL;


void DXT1block::getColors(std::vector<PG::UTIL::RGBA>& outRGBAData) const{
	Array<RGBA,4> c; // the 4 interpolated colors
	const int c0Val = (color[1] << 8) + color[0];
	c[0].r = (color[1] >> 3) & 0x1F;
	c[0].g = ((color[1] << 3) & 0x38) + ((color[0] >> 5) & 0x07);
	c[0].b = (color[0]) & 0x1F;
	c[0].scaleRGB565();

	const int c1Val = (color[3] << 8) + color[2];
	c[1].r = (color[3] >> 3) & 0x1F;
	c[1].g = ((color[3] << 3) & 0x38) + ((color[2] >> 5) & 0x07);
	c[1].b = (color[2]) & 0x1F;
	c[1].scaleRGB565();


	if(c0Val > c1Val){
		c[2] = interpolate(c[0],c[1], 1.0/3.0 );
		c[3] = interpolate(c[0],c[1], 2.0/3.0 );
	}else{
		c[2] = interpolate(c[0],c[1], 0.5 );
		c[3].a = 0;
	}

	/*
	PG_INFO_STREAM("c0Val: "<<c0Val);
	PG_INFO_STREAM("c1Val: "<<c1Val);

	PG_INFO_STREAM("c0: "<<c[0]);
	PG_INFO_STREAM("c1: "<<c[1]);
	PG_INFO_STREAM("c2: "<<c[2]);
	PG_INFO_STREAM("c3: "<<c[3]);
	*/

	//read lookup table and insert RGBA
	outRGBAData.resize(16); // block size 4x4
	for(unsigned int i = 4; i < 8; ++i){
		const unsigned int pos = (i-4)*4;
		outRGBAData[pos] = c[(color[i]) & 0x03];
		outRGBAData[pos+1] = c[(color[i]>>2) & 0x03];
		outRGBAData[pos+2] = c[(color[i]>>4) & 0x03];
		outRGBAData[pos+3] = c[(color[i]>>6) & 0x03];
	}


}

void DXT5block::getColors(std::vector<PG::UTIL::RGBA>& outRGBAData) const{
	Array<RGBA,4> c; // the 4 interpolated colors
	const int c0Val = (color[1] << 8) + color[0];
	c[0].r = (color[1] >> 3) & 0x1F;
	c[0].g = ((color[1] << 3) & 0x38) + ((color[0] >> 5) & 0x07);
	c[0].b = (color[0]) & 0x1F;
	c[0].scaleRGB565();

	const int c1Val = (color[3] << 8) + color[2];
	c[1].r = (color[3] >> 3) & 0x1F;
	c[1].g = ((color[3] << 3) & 0x38) + ((color[2] >> 5) & 0x07);
	c[1].b = (color[2]) & 0x1F;
	c[1].scaleRGB565();

	c[2] = interpolate(c[0],c[1], 1.0/3.0 );
	c[3] = interpolate(c[0],c[1], 2.0/3.0 );

	/*
	PG_INFO_STREAM("c0Val: "<<c0Val);
	PG_INFO_STREAM("c1Val: "<<c1Val);

	PG_INFO_STREAM("c0: "<<c[0]);
	PG_INFO_STREAM("c1: "<<c[1]);
	PG_INFO_STREAM("c2: "<<c[2]);
	PG_INFO_STREAM("c3: "<<c[3]);
	*/

	//read lookup table and insert RGBA
	outRGBAData.resize(16); // block size 4x4
	for(unsigned int i = 4; i < 8; ++i){
		const unsigned int pos = (i-4)*4;
		outRGBAData[pos] = c[(color[i]) & 0x03];
		outRGBAData[pos+1] = c[(color[i]>>2) & 0x03];
		outRGBAData[pos+2] = c[(color[i]>>4) & 0x03];
		outRGBAData[pos+3] = c[(color[i]>>6) & 0x03];
	}


	// the 8 interpolated alpha values
	PG::UTIL::Array<unsigned char,8> a;
	a[0] = alpha[0];//( (alpha[0] >> 4) & 0x0F )*(255/15.0);
	a[1] = alpha[1];//(alpha[0] & 0x0F)*(255/15.0);

	if(a[0] > a[1] ){
		a[2] =  interpolate(a[0], a[1], 1.0/7.0);
		a[3] =  interpolate(a[0], a[1], 2.0/7.0);
		a[4] =  interpolate(a[0], a[1], 3.0/7.0);
		a[5] =  interpolate(a[0], a[1], 4.0/7.0);
		a[6] =  interpolate(a[0], a[1], 5.0/7.0);
		a[7] =  interpolate(a[0], a[1], 6.0/7.0);
	}else{
		a[2] =  interpolate(a[0], a[1], 1.0/5.0);
		a[3] =  interpolate(a[0], a[1], 2.0/5.0);
		a[4] =  interpolate(a[0], a[1], 3.0/5.0);
		a[5] =  interpolate(a[0], a[1], 4.0/5.0);
		a[6] = 0;
		a[7] = 255;
	}

	/*
	for(unsigned char c: a){
		std::cout << (int)c <<" ";
	}
	std::cout << std::endl;
	*/

	//read alpha lookup table and set A
	unsigned int start_bit = 16; //(offset to the start of lookup)
	//TODO: find something easyer?
	for(unsigned int i = 0; i < 16; ++i){
		const unsigned int t1 = start_bit/8;
		const unsigned int end_bit = start_bit+2;
		const unsigned int t2 = (end_bit)/8;
		const unsigned int start_bit_rel = start_bit - int(start_bit/8.0)*8;

		if(t1 == t2){ //is the value between two chars?
			const unsigned int u = (alpha[t1]>>start_bit_rel) & 0x07;
			outRGBAData[i].a = a[u];
		}else{
			if(start_bit_rel == 6){
				const unsigned int u = (alpha[t1]>>6) | ( (alpha[t2]<<2) & 0x01);
				outRGBAData[i].a = a[u];
			}else{
				const unsigned int u = (alpha[t1]>>7) | (alpha[t2]<<1 & 0x06);
				outRGBAData[i].a = a[u];
			}

		}
		start_bit+=3;
	}

}

template<class blockType>
bool uncompressS3(unsigned int width, unsigned int height, const std::vector<blockType>& inData, std::vector<PG::UTIL::RGBA>& outRGBAData){
	static_assert(std::is_base_of<PG::FILE::S3Base, blockType>::value, "blockType must inherit from S3Base");

	if(width == 0 || height == 0){
		PG_ERROR_STREAM("Image width or height is zero!");
		return true;
	}

	const unsigned int block_width = width/4.0;
	const unsigned int block_height = height/4.0;

	if(inData.size() < block_width*block_height){
		PG_ERROR_STREAM("Not enough blocks given! Expected "<<block_width*block_height<<" given "<<inData.size()<<" .");
		return true;
	}

	outRGBAData.resize(width*height);
	for(unsigned int y = 0; y < block_height; ++y)
		for(unsigned int x = 0; x < block_width; ++x){
			const unsigned int current_block = y*block_width+x;
			assert_Test("current_block is out if bound!", current_block > block_width*block_height);
			const unsigned int current_start_index = y*block_width*4*4+x*4;
			assert_Test("current_start_index is out if bound!", current_start_index > width*height);
			std::vector<PG::UTIL::RGBA> RGBAData;
			const blockType& block = inData[current_block];
			block.getColors(RGBAData);

			for(unsigned int by = 0; by < 4; ++by)
				for(unsigned int bx = 0; bx < 4; ++bx){
					const unsigned int current_index = current_start_index+ ((by*width)+bx);
					//PG_INFO_STREAM("s: "<<outRGBAData.size()<<" current_index: "<<current_index<<" current_block: "<<current_start_index);
					outRGBAData[current_index] = RGBAData[by*4+bx];
				}

		}

	return false;
}

S3File::S3File(const std::string& filename): m_filename(filename) {
	//PG::UTIL::BinaryFileTokenizer reader(m_filename);
	//m_width = reader.getUnsignedShort();
	//m_height = reader.getUnsignedShort();

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

			//return savePPM(outfilename);
			PG::UTIL::BinaryFileWriter writer(outfilename);
			std::stringstream o;
			o<<"P6\n"<<m_width<<"\n"<<m_height<<"\n255\n";
			writer.writeString(o.str());

			for(RGBA& rgba: data){
				writer.writeChar(rgba.r);
				writer.writeChar(rgba.g);
				writer.writeChar(rgba.b);
			}
		}
			break;
		case TGA:
		{
			PG_INFO_STREAM("Writing TGA. (RGBA)");
			PG::UTIL::BinaryFileWriter writer(outfilename);
			writer.writeInt(131072);
			writer.writeInt(0);
			writer.writeInt(0);
			writer.writeShort(m_width);
			writer.writeShort(m_height);
			writer.writeShort(2080);
			//writer.writeShort(m_width*4);


			for(unsigned int y = 0; y < m_height; ++y ){
				for(unsigned int x = 0; x < m_width; ++x ){
					const unsigned int index = ((m_height-y-1)*m_width+x); //flip
					const RGBA& rgba = data[index];

					writer.writeChar(rgba.b);
					writer.writeChar(rgba.g);
					writer.writeChar(rgba.r);
					writer.writeChar(rgba.a);
				}
			}

			writer.writeLongLong(0);
			writer.writeLongLong(0);
			writer.writeString("TRUEVISION-XFILE.");

			//writer.write(&data[0], data.size());

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
	if(m_width != reader.getUnsignedShort() || m_height != reader.getUnsignedShort()){
		PG_ERROR_STREAM("The image has different size then expected!");
		return false;
	}

	reader.skip(12);

	const unsigned int number_of_blocks_width = (m_width/4);
	const unsigned int number_of_blocks_height = (m_height/4);
	const unsigned int number_of_blocks_4x4 = number_of_blocks_width*number_of_blocks_height;

	std::vector<DXT5block> blocks(number_of_blocks_4x4);
	reader.read((char*)&blocks[0], number_of_blocks_4x4*sizeof(DXT5block));
	reader.close();

	uncompressDXT5(m_width,m_height, blocks, outRGBAData);

	return true;


}


S3File::~S3File() {
	// TODO Auto-generated destructor stub
}

} /* namespace FILE */
} /* namespace PG */
