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

struct S3Base{};

struct DXT1block: public S3Base{
	unsigned char color[8];
	/*!
	 * @brief Returns a block of 4x4 RGBA pixels.
	 */
	void decompress(std::vector<PG::UTIL::rgba>& outRGBAData) const;

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
}

template<class blockType>
static bool decompressS3(unsigned int width, unsigned int height, const std::vector<blockType>& inData, PG::UTIL::RGBAImage& outRGBAData){
	outRGBAData.resize(width, height);
	return decompressS3(width, height, inData, (char*)&outRGBAData[0].r);
}

//template<class blockType>
static bool compressS3(const PG::UTIL::RGBAImage& imageIn, std::vector<DXT1block>& blocksOut){
	PG_INFO_STREAM(imageIn.getWidth() << " "<<imageIn.getHeight());


	if( imageIn.size() == 0 ||  imageIn.getWidth() % 4 != 0 || imageIn.getHeight() % 4 != 0){
		PG_ERROR_STREAM("Image dimensions need to mod 4!");
		return true;
	}

	const unsigned int block_width = imageIn.getWidth()/4.0;
	const unsigned int block_height = imageIn.getHeight()/4.0;
	//read this http://stackoverflow.com/questions/470690/how-to-automatically-generate-n-distinct-colors
	blocksOut.resize(block_width*block_height);
	//extract 4x4 blocks and compress them
	for(unsigned int y = 0; y < block_height; ++y)
			for(unsigned int x = 0; x < block_width; ++x){
				PG::UTIL::RGBAImage window;
				//PG_INFO_STREAM("window start: "<<uvec2(x*4,y*4)<<" size: "<<uvec2(4,4));
				imageIn.getWindow(uvec2(x*4,y*4), uvec2(4,4),window);

				//find the max and min color
				Array<rgba,4> points; // min max interpolateA interpolateB or alpha
				points[0] = rgba(255,255,255,255); // min
				unsigned int distanceC0 = 255*255*3;
				points[1] = rgba(0,0,0,0); //max
				unsigned int distanceC1 = 0;

				bool isalpha = false;

				for(const rgba& pixel: window){
					const unsigned int distancePix = pixel.r*pixel.r + pixel.g*pixel.g + pixel.b*pixel.b;
					//min test
					if(distanceC0 > distancePix){
						points[0] = pixel;
						distanceC0  = distancePix;
					}
					//test max
					if(distanceC1 < distancePix){
						points[1] = pixel;
						distanceC1  = distancePix;
					}
					//test alpha
					if(pixel.a < 128)
						isalpha = true;
				}

				//PG_INFO_STREAM("Found: "<<points[0] << points[1]);

				DXT1block& block = blocksOut[y*block_width+x];
				const unsigned int a = block.setColorA(points[0]);
				const unsigned int b = block.setColorB(points[1]);

				//set correct order
				if((isalpha && a > b)){
						rgba b = points[0];
						points[0] = points[1];
						points[1] = b;

						block.setColorA(points[0]);
						block.setColorB(points[1]);
				}else if(a < b){
						rgba b = points[0];
						points[0] = points[1];
						points[1] = b;

						block.setColorA(points[0]);
						block.setColorB(points[1]);
				}else if( a == b ){
					block.setColorA(points[0]);
					block.setColorB(points[1]);
				}


					/*
				PG_INFO_STREAM("INT:      rgba1: "<<a<< " rgba2: "<<b);
				PG_INFO_STREAM("DIS:      rgba1: "<<distanceC0<< " rgba2: "<<distanceC1);
				PG_INFO_STREAM("ORIGINAL: rgba1: "<<points[0]<< " rgba2: "<<points[1]);
				PG_INFO_STREAM("DATA:     rgba1: "<<block.getColorA()<< " rgba2: "<<block.getColorB());
				const rgba& co = window[4*4-1];
				PG_INFO_STREAM("WINDOW:   rgba1: "<< co << " d: "<<co.r*co.r+co.g*co.g+co.b*co.b);
				*/

				//build look up table
				std::vector<char> lookup;
				lookup.reserve(16);

				/*
				if(isalpha){
					//PG_INFO_STREAM("ALPHA");
					points[2] = interpolate(points[0],points[1], 0.5 );
					points[3] = rgba(0,0,0,0);
					//PG_INFO_STREAM("rgba3: "<<points[2]<< " rgba4: "<<points[3]);

					DXT1block& block = blocksOut[y*block_width+x];
					block.setColorA(points[0]);
					block.setColorB(points[1]);

					for(unsigned char i = 0; i < 16; ++i){
						const rgba& pixel = window[i];
						if(pixel.a < 128){
							block.setColorLookUpValue(i,3);
							continue;
						}

						struct s{
							float pixelDistance = 900; //big number
							char index = 0;
						} nearest;

						for(unsigned char i = 0; i < 3; i++){
							float dista = PG::UTIL::distance(pixel,points[i]);
							if(nearest.pixelDistance >  dista){
								nearest.pixelDistance = dista;
								nearest.index = i;
							}
						}
						block.setColorLookUpValue(i,nearest.index);
					}

				}else{
				*/

					//get the other colors
					points[2] = interpolate(points[0],points[1], 1.0/3.0 );
					points[3] = interpolate(points[0],points[1], 2.0/3.0 );

					//PG_INFO_STREAM(points[0] << points[1]<<points[2] << points[3]);

					for(unsigned char i = 0; i < 16; ++i){
						const rgba& pixel = window[i];
						struct s{
							float pixelDistance = 900; //big number
							char index = 0;
						} nearest;

						for(unsigned char i = 0; i < 4; i++){
							float dista = PG::UTIL::distance(pixel,points[i]);
							//PG_INFO_STREAM(nearest.pixelDistance << " dista: "<<dista<<" i: "<<(int)nearest.index)
							if(nearest.pixelDistance >  dista){
								nearest.pixelDistance = dista;
								nearest.index = i;
								if(dista == 0) break;
							}
						}
						block.setColorLookUpValue(i,nearest.index);
					}

				//}


			}

	return false;
}

} /* namespace FILE */
} /* namespace PG */


#endif /* INCLUDE_PG_FILES_PG_S3COMPRESSION_H_ */
