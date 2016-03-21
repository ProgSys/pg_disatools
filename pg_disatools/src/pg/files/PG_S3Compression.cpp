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

#include <pg/files/PG_S3Compression.h>

#include <sstream>
#include <algorithm>
#include <type_traits>
#include <limits>

#include <pg/util/PG_Array.h>
#include <pg/util/PG_Base.h>

#include <pg/util/PG_VectorUtil.h>
#include <cmath>

namespace PG {
namespace FILE {

using namespace UTIL;

void findSameColorBestMatch(PG::UTIL::rgba target, PG::UTIL::rgba& c0, PG::UTIL::rgba& c1){

	int cy = std::floor((3*int(target.r))/16.0) * 8;
	if(cy > 248) cy = 248;
	int cx = 0;
	int cxpart;
	if( (cxpart = int(target.r)*3 - cy*2) > 0){
		cx = std::round(cxpart/8.0) * 8;
		if((int(target.r)-1) % 8 == 0) cx +=8;
	}
	if(cx > 248) cx = 248;

	c0.r = cx;
	c1.r = cy;


	cy = std::floor((3*int(target.g))/8.0) * 4;
	if(cy > 252) cy = 252;
	cx = 0;
	if( (cxpart = int(target.g)*3 - cy*2) > 0){
		cx = std::round(cxpart/4.0) * 4;
		if((target.g+1) % 8 == 0){ cx -=4; cy +=4;}
		else if((target.g+1) % 4 == 0) cx +=4;
	}
	if(cx > 252) cx = 252;

	c0.g = cx;
	c1.g = cy;


	cy = std::floor((3*int(target.b))/16.0) * 8;
	if(cy > 248) cy = 248;
	cx = 0;
	if( (cxpart = int(target.b)*3 - cy*2) > 0){
		cx = std::round(cxpart/8.0) * 8;
		if((int(target.b)-1) % 8 == 0) cx +=8;
	}
	if(cx > 248) cx = 248;

	c0.b = cx;
	c1.b = cy;
}

short getShort(const PG::UTIL::rgba& color888){
	const char g = color888.g & 0xFC;
	const char a = ((color888.b & 0xF8) >> 3) | (g << 3);
	const char b = (color888.r & 0xF8) | ((g >> 5) & 0x07);
	return (b << 8) + a;
}

int roundDXTColor(double x)
{
    if (x < 0.0)
        return (int)(x - 0.51);
    else
        return (int)(x + 0.49);
}

inline PG::UTIL::rgba toRGB565( const PG::UTIL::rgba& rgba888){
	return PG::UTIL::rgba( std::round(rgba888.r/8.0)*8  , std::round(rgba888.g/4.0)*4 , std::round(rgba888.b/8.0)*8 , rgba888.a);
}

inline PG::UTIL::rgba clampToRGBA(const PG::UTIL::ivec4& vec){
	return PG::UTIL::rgba( std::max(std::min(vec.r, 255), 0), std::max(std::min(vec.g, 255), 0), std::max(std::min(vec.b, 255), 0), std::max(std::min(vec.a, 255), 0)  );
}

inline void getMinMaxRGBA(const PG::UTIL::rgba& rgba0In, const PG::UTIL::rgba& rgba1In, PG::UTIL::rgba& minOut, PG::UTIL::rgba& maxOut){
	if(rgba0In.r > rgba1In.r){
		minOut.r = rgba1In.r;
		maxOut.r = rgba0In.r;
	}else{
		minOut.r = rgba0In.r;
		maxOut.r = rgba1In.r;
	}

	if(rgba0In.g > rgba1In.g){
		minOut.g = rgba1In.g;
		maxOut.g = rgba0In.g;
	}else{
		minOut.g = rgba0In.g;
		maxOut.g = rgba1In.g;
	}

	if(rgba0In.b > rgba1In.b){
		minOut.b = rgba1In.b;
		maxOut.b = rgba0In.b;
	}else{
		minOut.b = rgba0In.b;
		maxOut.b = rgba1In.b;
	}

}

PG::UTIL::rgba operator-(const PG::UTIL::rgba& rgba, const PG::UTIL::ivec4& vec){
	const PG::UTIL::ivec4 buffvec( int(rgba.r)-vec.r,int(rgba.g)-vec.g,int(rgba.b)-vec.b,int(rgba.a)-vec.a );
	return clampToRGBA(buffvec);
}

PG::UTIL::rgba operator+(const PG::UTIL::rgba& rgba, const PG::UTIL::ivec4& vec){
	const PG::UTIL::ivec4 buffvec( int(rgba.r)+vec.r,int(rgba.g)+vec.g,int(rgba.b)+vec.b,int(rgba.a)+vec.a );
	return clampToRGBA(buffvec);
}


inline void findMinError(const PG::UTIL::RGBAImage& window, const PG::UTIL::Array<PG::UTIL::rgba, 4>& colors, float& error, PG::UTIL::rgba& c0out, PG::UTIL::rgba& c1out  ){
	float found_error = 0;
	for(const PG::UTIL::rgba& color: window){
		float dis = 0;
		float min_error = 999999;
		for(const PG::UTIL::rgba& compare: colors){
			if( min_error > (dis = distanceRGB(color,compare)) )
					min_error = dis;
		}
		found_error += min_error;
	}

	if( (found_error - error) < -2e-005){
		error = found_error;
		c0out = colors[0];
		c1out = colors[1];
	}
}

bool compressDXT1(const PG::UTIL::RGBAImage& window, PG::UTIL::rgba& c0out, PG::UTIL::rgba& c1out, bool testalpha = true){
	// = the slowest and stupidest way doing it =

			//does have the image pixels with a alpha below 128?
			bool alpha = false;
			if(testalpha)
				for(const PG::UTIL::rgba& color: window){
					if(color.a < 128){
						alpha = true;
						break;
					}
				}

			float error = 99999999.0f;

			if(alpha){
				//image has has alpha
				for(const PG::UTIL::rgba& color1: window)
					for(const PG::UTIL::rgba& color2: window){
						if(color1.a == 0 && color2.a == 0) continue;
						PG::UTIL::rgba c2 = interpolate(color1,color2, 0.5);

						float found_error = 0;
						for(const PG::UTIL::rgba& color: window){
							float dis = distanceRGB(color,color1);
							float min_error = dis;
							char index = 0;
							if( min_error > (dis = distanceRGB(color,color2)) )
								min_error = dis;

							if( min_error > (dis = distanceRGB(color,c2)) )
								min_error = dis;

							found_error += min_error;
						}
						if( (found_error - error) < -2e-005){
							error = found_error;
							c0out = color1;
							c1out = color2;
						}
					}

				//is there only one color?
				if(c0out == c1out)
					c0out.clear();

				if( getShort(c0out) > getShort(c1out)){
					PG::UTIL::rgba cb = c0out;
					c0out = c1out;
					c1out = cb;
				}

				return true;


			}else{
				//image has no alpha
				unsigned int startIndex = 1;
				std::vector<PG::UTIL::rgba> uniqueColors;
				for(const PG::UTIL::rgba& color1: window){ //x
					bool found = false;
					for(const PG::UTIL::rgba& tColor: uniqueColors){
						if(tColor == color1){
							found = true;
							break;
						}
					}
					if(!found) uniqueColors.push_back(color1);

					for(unsigned int i = startIndex; i < 16; ++i){ //y
						const PG::UTIL::rgba& color2 = window[i];
						if(color1.a == 0 && color2.a == 0) continue;
						if(color1 == color2) continue;

						//the four color combinations

						// x - t0 - t1 - y
						PG::UTIL::Array<PG::UTIL::rgba, 4> c;
						c[0] = toRGB565(color1);
						c[1] = toRGB565(color2);
						c[2] = interpolate(c[0],c[1],  1.0/3.0);
						c[3] = interpolate(c[0],c[1],  2.0/3.0);

						findMinError(window, c, error, c0out, c1out);

						// x - t0 - y - t1
						PG::UTIL::Array<PG::UTIL::rgba, 4> c1;
						c1[0] = c[0];
						c1[1] = toRGB565(clampToRGBA(  PG::UTIL::ivec4( ( int(color2.r)*3-int(color1.r))*0.5, (int(color2.g)*3-int(color1.g))*0.5, (int(color2.b)*3-int(color1.b))*0.5 )   ));
						c1[2] = interpolate(c1[0],c1[1],  1.0/3.0);
						c1[3] = interpolate(c1[0],c1[1],  2.0/3.0);

						findMinError(window, c1, error, c0out, c1out);

						// t0 - x - t1 - y
						PG::UTIL::Array<PG::UTIL::rgba, 4> c2;
						c2[0] = toRGB565(clampToRGBA(  PG::UTIL::ivec4( ( int(color1.r)*3-int(color2.r))*0.5, (int(color1.g)*3-int(color2.g))*0.5, (int(color1.b)*3-int(color2.b))*0.5 )   ));
						c2[1] = c[1];
						c2[2] = interpolate(c2[0],c2[1],  1.0/3.0);
						c2[3] = interpolate(c2[0],c2[1],  2.0/3.0);

						findMinError(window, c2, error, c0out, c1out);


						// x - y - t0 - t1
						PG::UTIL::Array<PG::UTIL::rgba, 4> c3;
						c3[0] = c[0];
						c3[1] = toRGB565(clampToRGBA(  PG::UTIL::ivec4(  int(color2.r)*3-int(color1.r)*2 , int(color2.g)*3-int(color1.g)*2, int(color2.b)*3-int(color1.b)*2 )   ));
						c3[2] = interpolate(c3[0],c3[1],  1.0/3.0);
						c3[3] = interpolate(c3[0],c3[1],  2.0/3.0);

						//PG_INFO_STREAM( "c0: "<<color1<<" c1: "<< color2<<" r: "<<PG::UTIL::ivec4(  int(color2.r)*3-int(color1.r)*2 , int(color2.g)*3-int(color1.g)*2, int(color2.b)*3-int(color1.b)*2 ) << " o: "<< c3[1])
						findMinError(window, c3, error, c0out, c1out);

						// t0 - t1 - x - y
						PG::UTIL::Array<PG::UTIL::rgba, 4> c4;
						c4[0] = toRGB565(clampToRGBA(  PG::UTIL::ivec4(  int(color1.r)*3-int(color2.r)*2 , int(color1.g)*3-int(color2.g)*2, int(color1.b)*3-int(color2.b)*2 )   ));
						c4[1] = c[1];
						c4[2] = interpolate(c4[0],c4[1],  1.0/3.0);
						c4[3] = interpolate(c4[0],c4[1],  2.0/3.0);

						//PG_INFO_STREAM( "c0: "<<color2<<" c1: "<< color1<<" r: "<<PG::UTIL::ivec4(  int(color1.r)*3-int(color2.r)*2 , int(color1.g)*3-int(color2.g)*2, int(color1.b)*3-int(color2.b)*2 )  << " o: "<< c4[0])
						findMinError(window, c4, error, c0out, c1out);

						// t0 - x - y - t1
						PG::UTIL::Array<PG::UTIL::rgba, 4> c5;
						const PG::UTIL::ivec4 diff( std::abs(int(color1.r)-int(color2.r)), std::abs(int(color1.g)-int(color2.g)), std::abs(int(color1.b)-int(color2.b)) );
						//c3[0] = toRGB565(color1-diff);
						//c3[1] = toRGB565(color2+diff);


						if(color1.r > color2.r){
							c5[0].r = color1.r + diff.r;
							c5[1].r = color2.r - diff.r;
						}else{
							c5[0].r = color1.r - diff.r;
							c5[1].r = color2.r + diff.r;
						}
						if(color1.g > color2.g){
							c5[0].g = color1.g + diff.g;
							c5[1].g = color2.g - diff.g;
						}else{
							c5[0].g = color1.g - diff.g;
							c5[1].g = color2.g + diff.g;
						}
						if(color1.b > color2.b){
							c5[0].b = color1.b + diff.b;
							c5[1].b = color2.b - diff.b;
						}else{
							c5[0].b = color1.b - diff.b;
							c5[1].b = color2.b + diff.b;
						}
						c5[0] = toRGB565(c5[0]);
						c5[1] = toRGB565(c5[1]);
						c5[2] = interpolate(c5[0],c5[1],  1.0/3.0);
						c5[3] = interpolate(c5[0],c5[1],  2.0/3.0);

						//PG_INFO_STREAM("A: "<<color1<<" B: "<<color2<< " sub: "<< (color1-diff) <<" add: "<< (color2+diff)<<" c5[0]: "<<c5[0]<<" c5[1]: "<<c5[1]);

						findMinError(window, c5, error, c0out, c1out);
					}

					startIndex++;
				}//for loop end

				//is there only one color?
				if(uniqueColors.size() == 1)
					findSameColorBestMatch(uniqueColors[0], c0out,c1out);

				//sort the two key collors
				unsigned int i1,i2;
				if( (i1 = getShort(c0out)) < ( i2 = getShort(c1out))){
					PG::UTIL::rgba cb = c0out;
					c0out = c1out;
					c1out = cb;
				}else if(testalpha && uniqueColors.size() != 2 && i1 == i2){
					if(c0out.g >= 4)
						c0out.g -= 4;
					else
						c0out.g += 4;
				}

				return false;

			}//alpha else end
}

inline int getColor(unsigned char a, unsigned char b, rgba& outRGBA){
	outRGBA.r = b & 0xF8;
	outRGBA.g = (b << 5) + ((a & 0xE0) >> 3);
	outRGBA.b = a << 3;
	outRGBA.a = 255;
	//scaleRGB565to888(outRGBA);
	return (b << 8) + a;
}

void DXT1block::decompress(std::vector<PG::UTIL::rgba>& outRGBAData) const{
	Array<rgba,4> c; // the 4 interpolated colors
	const int c0Val = getColor(color[0], color[1], c[0]);
	const int c1Val = getColor(color[2], color[3], c[1]);

	if(c0Val > c1Val){
		c[2] = interpolate(c[0],c[1], 1.0/3.0 );
		c[3] = interpolate(c[0],c[1], 2.0/3.0 );
	}else{
		c[2] = interpolate(c[0],c[1], 0.5 );
		c[3].a = 0;
	}

	//PG_INFO_STREAM(c[0]<<c[1]<<c[2]<<c[3]);

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

void DXT1block::compress(const PG::UTIL::RGBAImage& window, S3CompressionMethod method){

	PG::UTIL::rgba c0;
	PG::UTIL::rgba c1;

	switch (method) {
		case MIN_MAX:
		{
			c0 = rgba(255,255,255,255); // min
			unsigned int distanceC0 = 255*255*3;
			c1 = rgba(0,0,0,0); // min
			unsigned int distanceC1 = 0;

			bool alpha = false;

			for(const rgba& pixel: window){
				if(pixel.a == 0) continue;
				const unsigned int distancePix = pixel.r*pixel.r + pixel.g*pixel.g + pixel.b*pixel.b;
				//min test
				if(distanceC0 > distancePix){
					c0 = pixel;
					distanceC0  = distancePix;
				}
				//test max
				if(distanceC1 < distancePix){
					c1 = pixel;
					distanceC1  = distancePix;
				}
				//test alpha
				if(pixel.a < 128)
					alpha = true;
			}

			//is there only one color?
			if(c0 == c1)
				findSameColorBestMatch(c0, c0,c1);

			if(alpha){
				if( setColorA(c0) > setColorB(c1)){
					PG::UTIL::rgba cb = c0;
					c0 = c1;
					c1 = cb;
					setColorA(c0);
					setColorB(c1);
				}

				//build look up table
				PG::UTIL::rgba c2 = interpolate(c0,c1, 0.5);

				for(unsigned int i = 0; i < 16; i++){
					const PG::UTIL::rgba& color = window[i];
					if(color.a < 128){
						setColorLookUpValue(i,3);
						continue;
					}

					float dis = distanceRGB(color,c0);
					float min_error = dis;
					char index = 0;
					if( min_error > (dis = distanceRGB(color,c1)) ){
						min_error = dis;
						index = 1;
					}
					if( min_error > (dis = distanceRGB(color,c2)) ){
						min_error = dis;
						index = 2;
					}
					setColorLookUpValue(i,index);
				}

			}else{
				unsigned int i1,i2;
				if( (i1 = setColorA(c0)) < ( i2 = setColorB(c1))){
					PG::UTIL::rgba cb = c0;
					c0 = c1;
					c1 = cb;
					setColorA(c0);
					setColorB(c1);
				}else if(i1 == i2){
					color[0]++;
				}

				//build look up table
				PG::UTIL::rgba c2 = interpolate(c0,c1,  1.0/3.0);
				PG::UTIL::rgba c3 = interpolate(c0,c1,  2.0/3.0);

				for(unsigned int i = 0; i < 16; i++){
					const PG::UTIL::rgba& color = window[i];
					float dis = distanceRGB(color,c0);
					float min_error = dis;
					char index = 0;
					if( min_error > (dis = distanceRGB(color,c1)) ){
						min_error = dis;
						index = 1;
					}
					if( min_error > (dis = distanceRGB(color,c2)) ){
						min_error = dis;
						index = 2;
					}
					if( min_error > (dis = distanceRGB(color,c3)) ){
						index = 3;
					}
					setColorLookUpValue(i,index);
				}

			}

		}
			break;
		case FIND_BEST:
		{
			// = the slowest and stupidest way doing it =

			if(compressDXT1(window, c0,c1)){
				//alpha
				//build look up table
				PG::UTIL::rgba c2 = interpolate(c0,c1, 0.5);

				for(unsigned int i = 0; i < 16; i++){
					const PG::UTIL::rgba& color = window[i];
					if(color.a < 128){
						setColorLookUpValue(i,3);
						continue;
					}

					float dis = distanceRGB(color,c0);
					float min_error = dis;
					char index = 0;
					if( min_error > (dis = distanceRGB(color,c1)) ){
						min_error = dis;
						index = 1;
					}
					if( min_error > (dis = distanceRGB(color,c2)) ){
						min_error = dis;
						index = 2;
					}
					setColorLookUpValue(i,index);
				}
			}else{
				//no alpha
				//build look up table
				PG::UTIL::rgba c2 = interpolate(c0,c1,  1.0/3.0);
				PG::UTIL::rgba c3 = interpolate(c0,c1,  2.0/3.0);

				for(unsigned int i = 0; i < 16; i++){
					const PG::UTIL::rgba& color = window[i];
					float dis = distanceRGB(color,c0);
					float min_error = dis;
					char index = 0;
					if( min_error > (dis = distanceRGB(color,c1)) ){
						min_error = dis;
						index = 1;
					}
					if( min_error > (dis = distanceRGB(color,c2)) ){
						min_error = dis;
						index = 2;
					}
					if( min_error > (dis = distanceRGB(color,c3)) ){
						index = 3;
					}
					setColorLookUpValue(i,index);
				}
			}//alpha else end

			/*
			//does have the image pixels with a alpha below 128?
			bool alpha = false;
			for(const PG::UTIL::rgba& color: window){
				if(color.a < 128){
					alpha = true;
					break;
				}
			}

			float error = 99999999.0f;


			if(alpha){
				//image has has alpha
				for(const PG::UTIL::rgba& color1: window)
					for(const PG::UTIL::rgba& color2: window){
						if(color1.a == 0 || color2.a == 0) continue;
						PG::UTIL::rgba c2 = interpolate(color1,color2, 0.5);

						float found_error = 0;
						for(const PG::UTIL::rgba& color: window){
							if(color.a < 128)
								continue;

							float dis = distanceRGB(color,color1);
							float min_error = dis;
							char index = 0;
							if( min_error > (dis = distanceRGB(color,color2)) )
								min_error = dis;

							if( min_error > (dis = distanceRGB(color,c2)) )
								min_error = dis;

							found_error += min_error;
						}

						if( found_error < error){
							error += found_error;
							c0 = color1;
							c1 = color2;
						}
					}

				//is there only one color?
				if(c0 == c1)
					c1.clear();

				if( setColorA(c0) > setColorB(c1)){
					PG::UTIL::rgba cb = c0;
					c0 = c1;
					c1 = cb;
					setColorA(c0);
					setColorB(c1);
				}

				//build look up table
				PG::UTIL::rgba c2 = interpolate(c0,c1, 0.5);

				for(unsigned int i = 0; i < 16; i++){
					const PG::UTIL::rgba& color = window[i];
					if(color.a < 128){
						setColorLookUpValue(i,3);
						continue;
					}

					float dis = distanceRGB(color,c0);
					float min_error = dis;
					char index = 0;
					if( min_error > (dis = distanceRGB(color,c1)) ){
						min_error = dis;
						index = 1;
					}
					if( min_error > (dis = distanceRGB(color,c2)) ){
						min_error = dis;
						index = 2;
					}
					setColorLookUpValue(i,index);
				}


			}else{
				//image has no alpha
				for(const PG::UTIL::rgba& color1: window)
					for(const PG::UTIL::rgba& color2: window){
						PG::UTIL::rgba c2 = interpolate(color1,color2,  1.0/3.0);
						PG::UTIL::rgba c3 = interpolate(color1,color2,  2.0/3.0);
						float found_error = 0;
						for(const PG::UTIL::rgba& color: window){
							float dis = distanceRGB(color,color1);
							float min_error = dis;
							char index = 0;
							if( min_error > (dis = distanceRGB(color,color2)) )
								min_error = dis;

							if( min_error > (dis = distanceRGB(color,c2)) )
								min_error = dis;

							if( min_error > (dis = distanceRGB(color,c3)) )
								min_error = dis;

							found_error += min_error;
						}

						if( found_error < error){
							error = found_error;
							c0 = color1;
							c1 = color2;
						}
					}

				//is there only one color?
				if(c0 == c1)
					c1.clear();

				unsigned int i1,i2;
				if( (i1 = setColorA(c0)) < ( i2 = setColorB(c1))){
					PG::UTIL::rgba cb = c0;
					c0 = c1;
					c1 = cb;
					setColorA(c0);
					setColorB(c1);
				}else if(i1 == i2){
					color[0]++;
				}

				//build look up table
				PG::UTIL::rgba c2 = interpolate(c0,c1,  1.0/3.0);
				PG::UTIL::rgba c3 = interpolate(c0,c1,  2.0/3.0);

				for(unsigned int i = 0; i < 16; i++){
					const PG::UTIL::rgba& color = window[i];
					float dis = distanceRGB(color,c0);
					float min_error = dis;
					char index = 0;
					if( min_error > (dis = distanceRGB(color,c1)) ){
						min_error = dis;
						index = 1;
					}
					if( min_error > (dis = distanceRGB(color,c2)) ){
						min_error = dis;
						index = 2;
					}
					if( min_error > (dis = distanceRGB(color,c3)) ){
						index = 3;
					}
					setColorLookUpValue(i,index);
				}


			}//alpha else end
			*/
		}
			break;
		default:
			break;
	}
}


unsigned int DXT1block::setColorA(const rgba& color888A){
    // RGB565=(((r&0xF8)<<8)|((g&0xFC)<<3)|((b&0xF8)>>3));
	char g = color888A.g & 0xFC;
	color[0] = ((color888A.b & 0xF8) >> 3) | (g << 3);
	color[1] = (color888A.r & 0xF8) | ((g >> 5) & 0x07);
	return (color[1] << 8) + color[0];
}

rgba DXT1block::getColorA() const{
	rgba c;
	getColor(color[0], color[1], c);
	return c;
}

unsigned int DXT1block::setColorB(const rgba& color888B){
    // RGB565=(((r&0xF8)<<8)|((g&0xFC)<<3)|((b&0xF8)>>3));
	char g = color888B.g & 0xFC;
	color[2] = ((color888B.b & 0xF8) >> 3) | (g << 3);
	color[3] = (color888B.r & 0xF8) | ((g >> 5) & 0x07);
	return (color[3] << 8) + color[2];
}

rgba DXT1block::getColorB() const{
	rgba c;
	getColor(color[2], color[3], c);
	return c;
}

void DXT1block::setColorLookUpValue(unsigned char index, unsigned char value){
	const unsigned char i = index/4.0f ;
	const unsigned char move = (index-i*4);

	color[i+ 4] = color[i+ 4] | ((value & 0x03) << move*2);
	//PG_INFO_STREAM("Setting value "<<(int)(value & 0x03)<<" to index "<<(int)index<< " at color "<< (i+ 4)<<" offset "<< (move*2)<< " fin: "<<(int)color[i+ 4]);
}


void DXT5block::decompress(std::vector<PG::UTIL::rgba>& outRGBAData) const{
	Array<rgba,4> c; // the 4 interpolated colors
	const int c0Val = getColor(color[0], color[1], c[0]);
	const int c1Val = getColor(color[2], color[3], c[1]);

	c[2] = interpolate(c[0],c[1], 1.0/3.0 );
	c[3] = interpolate(c[0],c[1], 2.0/3.0 );

	/*
	PG_INFO_STREAM(c[0])
	PG_INFO_STREAM(c[1])
	PG_INFO_STREAM(c[2])
	PG_INFO_STREAM(c[3])
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
				const unsigned int u = (alpha[t1]>>6) | ( (alpha[t2]<<2) & 0x04);
				outRGBAData[i].a = a[u];
			}else{
				const unsigned int u = (alpha[t1]>>7) | (alpha[t2]<<1 & 0x06);
				outRGBAData[i].a = a[u];
			}

		}
		start_bit+=3;
	}
}

void DXT5block::compress(const PG::UTIL::RGBAImage& window, S3CompressionMethod method){
	PG::UTIL::rgba c0;
	PG::UTIL::rgba c1;

	switch (method) {
		case MIN_MAX:
		{
			c0 = rgba(255,255,255,255); // min
			unsigned int distanceC0 = 255*255*3;
			c1 = rgba(0,0,0,0); // min
			unsigned int distanceC1 = 0;

			bool alpha = false;

			for(const rgba& pixel: window){
				if(pixel.a == 0) continue;
				const unsigned int distancePix = pixel.r*pixel.r + pixel.g*pixel.g + pixel.b*pixel.b;
				//min test
				if(distanceC0 > distancePix){
					c0 = pixel;
					distanceC0  = distancePix;
				}
				//test max
				if(distanceC1 < distancePix){
					c1 = pixel;
					distanceC1  = distancePix;
				}
				//test alpha
				if(pixel.a < 128)
					alpha = true;
			}

			//is there only one color?
			if(c0 == c1)
				findSameColorBestMatch(c0, c0, c1);


			unsigned int i1,i2;
			if( (i1 = setColorA(c0)) < ( i2 = setColorB(c1))){
				PG::UTIL::rgba cb = c0;
				c0 = c1;
				c1 = cb;
				setColorA(c0);
				setColorB(c1);
			}else if(i1 == i2){
				color[0]++;
			}

			//build look up table
			PG::UTIL::rgba c2 = interpolate(c0,c1,  1.0/3.0);
			PG::UTIL::rgba c3 = interpolate(c0,c1,  2.0/3.0);

			for(unsigned int i = 0; i < 16; i++){
				const PG::UTIL::rgba& color = window[i];
				float dis = distance(color,c0);
				float min_error = dis;
				char index = 0;
				if( min_error > (dis = distanceRGB(color,c1)) ){
					min_error = dis;
					index = 1;
				}
				if( min_error > (dis = distanceRGB(color,c2)) ){
					min_error = dis;
					index = 2;
				}
				if( min_error > (dis = distanceRGB(color,c3)) ){
					index = 3;
				}
				setColorLookUpValue(i,index);
			}



		}
			break;
		case FIND_BEST:
		{
			compressDXT1(window, c0,c1, false);

			setColorA(c0);
			setColorB(c1);

			//PG_INFO_STREAM("FIN FOUND KEY COLORS: "<<c0<<" "<<c1);

			//build color look up table
			PG::UTIL::rgba c2 = interpolate(c0,c1,  1.0/3.0);
			PG::UTIL::rgba c3 = interpolate(c0,c1,  2.0/3.0);

			for(unsigned int i = 0; i < 16; i++){
				const PG::UTIL::rgba& color = window[i];
				float dis = distanceRGB(color,c0);
				float min_error = dis;
				char index = 0;
				if( min_error > (dis = distanceRGB(color,c1)) ){
					min_error = dis;
					index = 1;
				}
				if( min_error > (dis = distanceRGB(color,c2)) ){
					min_error = dis;
					index = 2;
				}
				if( min_error > (dis = distanceRGB(color,c3)) ){
					index = 3;
				}
				setColorLookUpValue(i,index);

			}

		}
			break;
		default:
			break;
	}

	buildAlpha(window);
}

unsigned int DXT5block::setColorA(const rgba& color888A){
	// gggbbbbb rrrrrggg
	color[0] = (color888A.b >> 3) | ((color888A.g & 0x1C) << 3) ;
	color[1] = (color888A.r & 0xF8) | (color888A.g >> 5);
	return (color[1] << 8) + color[0];
}

rgba DXT5block::getColorA() const{
	rgba c;
	getColor(color[0], color[1], c);
	return c;
}

unsigned int DXT5block::setColorB(const rgba& color888B){
	// gggbbbbb rrrrrggg
	color[2] = (color888B.b >> 3) | ((color888B.g & 0x1C) << 3) ;
	color[3] = (color888B.r & 0xF8) | (color888B.g >> 5);
	return (color[3] << 8) + color[2];
}

rgba DXT5block::getColorB() const{
	rgba c;
	getColor(color[2], color[3], c);
	return c;
}

void DXT5block::setColorLookUpValue(unsigned char index, unsigned char value){
	const unsigned char i = index/4.0f ;
	const unsigned char move = (index-i*4);

	color[i+ 4] = color[i+ 4] | ((value & 0x03) << move*2);
	//PG_INFO_STREAM("Setting value "<<(int)(value & 0x03)<<" to index "<<(int)index<< " at color "<< (i+ 4)<<" offset "<< (move*2)<< " fin: "<<(int)color[i+ 4]);
}

void DXT5block::setAlphaLookUpValue(unsigned char index, unsigned char value){
	const unsigned int startBit = 16+(index*3);
	const unsigned int endBit = 16+3+(index*3);
	const unsigned int startIndex = (startBit)/8.0f;
	const unsigned int endIndex = (endBit)/8.0f;

	if(startIndex == endIndex){
		const unsigned int offset = startBit - 8*startIndex;
		alpha[startIndex] = alpha[startIndex] | ((value & 0x07) << offset);
	}else{ //value is between two bytes
		const unsigned int startOffset = startBit - 8*startIndex;
		const unsigned int endOffset = 3 - (endBit - 8*endIndex);
		const unsigned char v = value & 0x07;
		alpha[startIndex] = alpha[startIndex] | ((v & 0x07) << startOffset);
		alpha[endIndex] = alpha[endIndex] | ((v & 0x07) >> endOffset);
	}

}


void DXT5block::buildAlpha(const PG::UTIL::RGBAImage& window){
	//min max alpha
	unsigned char alphaMin = 255;
	unsigned char alphaMax = 0;

	//find min max alpha
	for(const PG::UTIL::rgba& color: window){
		if(color.a < alphaMin)
			alphaMin = color.a;

		if(color.a > alphaMax)
			alphaMax = color.a;
	}

	//build the 8 alpha values
	PG::UTIL::Array<unsigned char,8> a;
	if(alphaMin == 0 && alphaMax == 255){
		//find the second smallest and biggest
		alphaMin = 255;
		alphaMax = 0;
		for(const PG::UTIL::rgba& color: window){
			if(color.a == 0 || color.a == 255) continue;

			if(color.a < alphaMin)
				alphaMin = color.a;
			if(color.a > alphaMax)
				alphaMax = color.a;
		}

		if(alphaMin == 0 && alphaMax == 255){
			alphaMin = 128;
			alphaMax = 128;
		}else if(alphaMin == alphaMax){
			if(alphaMin != 0)
				alphaMin--;
			else
				alphaMax++;
		}

		a[0] = alphaMin;
		a[1] = alphaMax;
		a[2] =  interpolate(a[0], a[1], 1.0/5.0);
		a[3] =  interpolate(a[0], a[1], 2.0/5.0);
		a[4] =  interpolate(a[0], a[1], 3.0/5.0);
		a[5] =  interpolate(a[0], a[1], 4.0/5.0);
		a[6] = 0;
		a[7] = 255;

	}else{
		if(alphaMin == alphaMax)
			if(alphaMax != 255)
				alphaMax++;
			else
				alphaMin--;

		//PG_INFO_STREAM((int)alphaMax<< " "<<(int)alphaMin);
		a[0] = alphaMax;
		a[1] = alphaMin;
		a[2] =  interpolate(a[0], a[1], 1.0/7.0);
		a[3] =  interpolate(a[0], a[1], 2.0/7.0);
		a[4] =  interpolate(a[0], a[1], 3.0/7.0);
		a[5] =  interpolate(a[0], a[1], 4.0/7.0);
		a[6] =  interpolate(a[0], a[1], 5.0/7.0);
		a[7] =  interpolate(a[0], a[1], 6.0/7.0);
	}
	alpha[0] = a[0];
	alpha[1] = a[1];
	//build the alpha color table
	for(unsigned int i = 0; i < 16; i++){
		const PG::UTIL::rgba& color = window[i];
		unsigned char index = 0;
		float distance = 99999999.0f;

		for(unsigned char l = 0; l < 8; l++){
			float dis;
			if(distance > (dis = PG::UTIL::distance(color.a,a[l]))){
				distance = dis;
				index = l;
			}
		}
		//PG_INFO_STREAM("index"<<(int)index);
		setAlphaLookUpValue(i,index);
	}
}

} /* namespace FILE */
} /* namespace PG */
