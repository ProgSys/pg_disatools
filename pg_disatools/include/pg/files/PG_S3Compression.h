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
#include <pg/util/PG_Array.h>
#include <pg/util/PG_Image.h>

namespace PG {
namespace FILE {

struct S3Base{};

struct DXT1block: public S3Base{
	unsigned char color[8];
	/*!
	 * @brief Returns a block of 4x4 RGBA pixels.
	 */
	void decompress(std::vector<PG::UTIL::rgba>& outRGBAData) const;
};

struct DXT5block: public S3Base{
	unsigned char alpha[8];
	unsigned char color[8];
	/*!
	 * @brief Returns a block of 4x4 RGBA pixels.
	 */
	void decompress(std::vector<PG::UTIL::rgba>& outRGBAData) const;
};

template<class blockType>
static bool decompressS3(unsigned int width, unsigned int height, const std::vector<blockType>& inData, char* outRGBAData){
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

	for(unsigned int y = 0; y < block_height; ++y)
		for(unsigned int x = 0; x < block_width; ++x){
			const unsigned int current_block = y*block_width+x;
			assert_Test("current_block is out if bound!", current_block > block_width*block_height);
			const unsigned int current_start_index = (y*width*4+x*4)*4;
			assert_Test("current_start_index is out if bound!", current_start_index > width*height*4);
			std::vector<PG::UTIL::rgba> RGBAData;
			const blockType& block = inData[current_block];
			block.decompress(RGBAData);

			for(unsigned int by = 0; by < 4; ++by)
			{
				const unsigned int current_index = current_start_index+by*width*4;
				memcpy(&outRGBAData[current_index], &RGBAData[by*4].r, 16);
			}

		}

	return false;
}

template<class blockType>
static bool decompressS3(unsigned int width, unsigned int height, const std::vector<blockType>& inData, std::vector<PG::UTIL::rgba>& outRGBAData){
	outRGBAData.resize(width*height);
	return uncompressS3(width, height, inData, (char*) &outRGBAData[0].r);
	/*
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
			std::vector<PG::UTIL::rgba> RGBAData;
			const blockType& block = inData[current_block];
			block.uncompress(RGBAData);

			for(unsigned int by = 0; by < 4; ++by)
			{
				const unsigned int current_index = current_start_index+by*width;
				memcpy(&outRGBAData[current_index].r, &RGBAData[by*4].r, 16);
			}

		}

	return false;
	*/
}

template<class blockType>
static bool decompressS3(unsigned int width, unsigned int height, const std::vector<blockType>& inData, PG::UTIL::RGBAImage& outRGBAData){
	outRGBAData.resize(width, height);
	return decompressS3(width, height, inData, (char*)&outRGBAData[0].r);
}

} /* namespace FILE */
} /* namespace PG */


#endif /* INCLUDE_PG_FILES_PG_S3COMPRESSION_H_ */
