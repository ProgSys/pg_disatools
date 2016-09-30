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
#ifndef INCLUDE_PG_FILES_PG_S3COMPRESSION_H_
#define INCLUDE_PG_FILES_PG_S3COMPRESSION_H_

#include <vector>
#include <sstream>
#include <algorithm>
#include <type_traits>

#include <pg/util/PG_Vector.h>
#include <pg/util/PG_VectorUtil.h>
#include <pg/util/PG_Array.h>
#include <pg/util/PG_Image.h>

namespace PG {
namespace FILE {

using namespace UTIL;

enum S3CompressionMethod{
	MIN_MAX, // find the minimal and maximal color
	FIND_BEST // take two colors from set and test them if they fit, repeat until best match found
};


struct S3Base{};

struct DXT1block: public S3Base{
	unsigned char color[8];
	/*!
	 * @brief Returns a block of 4x4 RGBA pixels.
	 */
	void decompress(std::vector<PG::UTIL::rgba>& outRGBAData) const;
	void compress(const PG::UTIL::RGBAImage& window, S3CompressionMethod method);

	unsigned int setColorA(const rgba& color888A);
	rgba getColorA() const;
	unsigned int setColorB(const rgba& color888B);
	rgba getColorB() const;
	void setColorLookUpValue(unsigned char index, unsigned char value);
};

struct DXT5block: public S3Base{
	unsigned char alpha[8];
	unsigned char color[8];
	/*!
	 * @brief Returns a block of 4x4 RGBA pixels.
	 */
	void decompress(std::vector<PG::UTIL::rgba>& outRGBAData) const;
	void compress(const PG::UTIL::RGBAImage& window, S3CompressionMethod method);

	unsigned int setColorA(const rgba& color888A);
	rgba getColorA() const;
	unsigned int setColorB(const rgba& color888B);
	rgba getColorB() const;
	void setColorLookUpValue(unsigned char index, unsigned char value);
	void setAlphaLookUpValue(unsigned char index, unsigned char value);
private:
	void buildAlpha(const PG::UTIL::RGBAImage& window);
};

template<class blockType>
static bool decompressS3(unsigned int width, unsigned int height,const char* inData, unsigned int inDataSize, char* outRGBAData){
	static_assert(std::is_base_of<PG::FILE::S3Base, blockType>::value, "blockType must inherit from S3Base");

	if(width == 0 || height == 0){
		PG_ERROR_STREAM("Image width or height is zero!");
		return true;
	}

	const unsigned int block_width = width/4.0;
	const unsigned int block_height = height/4.0;

	if(inDataSize < block_width*block_height){
		PG_ERROR_STREAM("Not enough blocks given! Expected "<<block_width*block_height<<" given "<<inDataSize<<" .");
		return true;
	}

	for(unsigned int y = 0; y < block_height; ++y)
		for(unsigned int x = 0; x < block_width; ++x){
			const unsigned int current_block = y*block_width+x;
			assert_Test("current_block is out if bound!", current_block > block_width*block_height);
			const unsigned int current_start_index = (y*width*4+x*4)*4;
			assert_Test("current_start_index is out if bound!", current_start_index > width*height*4);
			std::vector<PG::UTIL::rgba> RGBAData;
			const blockType* block = (blockType*) &inData[current_block*sizeof(blockType)];
			//const blockType& block = reinterpret_cast<blockType>( &inData[current_block*sizeof(blockType)] );
			block->decompress(RGBAData);

			for(unsigned int by = 0; by < 4; ++by)
			{
				const unsigned int current_index = current_start_index+by*width*4;
				memcpy(&outRGBAData[current_index], &RGBAData[by*4].r, 16);
			}

		}

	return false;
}

template<class blockType>
static bool decompressS3(unsigned int width, unsigned int height, const std::vector<char>& inData, char* outRGBAData){
	return decompressS3<blockType>(width, height, (char*) &inData[0], inData.size()/sizeof(blockType), outRGBAData);
}

template<class blockType>
static bool decompressS3(unsigned int width, unsigned int height, const std::vector<blockType>& inData, char* outRGBAData){
	return decompressS3<blockType>(width, height, (char*) &inData[0], inData.size(), outRGBAData);
}

template<class blockType>
static bool decompressS3(unsigned int width, unsigned int height, const std::vector<blockType>& inData, std::vector<PG::UTIL::rgba>& outRGBAData){
	outRGBAData.resize(width*height);
	return decompressS3<blockType>(width, height, inData, (char*) &outRGBAData[0].r);
}

template<class blockType>
static bool decompressS3(unsigned int width, unsigned int height, const std::vector<blockType>& inData, PG::UTIL::RGBAImage& outRGBAData){
	outRGBAData.resize(width, height);
	return decompressS3<blockType>(width, height, inData, (char*)&outRGBAData[0].r);
}


/*
 * @brief This function compresses a given DXT block. <br>
 *  The main goal is to find the best two points that minimize the error of a DXT.
 */
template<class blockType>
static bool compressS3(const PG::UTIL::RGBAImage& imageIn, std::vector<blockType>& blocksOut, S3CompressionMethod method = FIND_BEST){
	static_assert(std::is_base_of<PG::FILE::S3Base, blockType>::value, "blockType must inherit from S3Base");

	//PG_INFO_STREAM(imageIn.getWidth() << " "<<imageIn.getHeight());


	if( imageIn.size() == 0 ||  imageIn.getWidth() % 4 != 0 || imageIn.getHeight() % 4 != 0){
		PG_ERROR_STREAM("Image dimensions need to mod 4!");
		return true;
	}

	const unsigned int block_width = imageIn.getWidth()/4.0;
	const unsigned int block_height = imageIn.getHeight()/4.0;

	//read this http://stackoverflow.com/questions/470690/how-to-automatically-generate-n-distinct-colors
	blocksOut.resize(block_width*block_height);

	//extract 4x4 blocks and compress them
	for(unsigned int y = 0; y < block_height; ++y){
			for(unsigned int x = 0; x < block_width; ++x){
				PG::UTIL::RGBAImage window; //4x4 window
				imageIn.getWindow(uvec2(x*4,y*4), uvec2(4,4),window); //extract 4x4 window from image

				blockType& block = blocksOut[y*block_width+x];
				block.compress(window, method);
			}
	}


	return false;
}

} /* namespace FILE */
} /* namespace PG */


#endif /* INCLUDE_PG_FILES_PG_S3COMPRESSION_H_ */
