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


#include <iostream>
#include <string>
#include <pg/files/PG_IMY.h>
#include <pg/files/PG_ImageFiles.h>
#include <pg/files/PG_PSPFS.h>
#include <algorithm>
#include <pg/util/PG_Array.h>
#include <fstream>
#include <pg/util/PG_Vector.h>

#include <cstdlib>
#include <cmath>

#define OUTSTR(x) std::cout << x << std::endl

struct testStr{
	testStr(const std::string& str): name(str){}
	std::string name;
};

void findSameColorBestMatch(PG::UTIL::rgba target, PG::UTIL::rgba& c0, PG::UTIL::rgba& c1){
	//found my math and try and error
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

/*!
 * @brief Testing main method, just for testing.
 */
int main(int argc, char* argv[]){
	OUTSTR("Start");

	//PG::FILE::decompressIMY("C:/Users/ProgSys/Desktop/Disgaea/PC/IMY/first.IMY","C:/Users/ProgSys/Desktop/Disgaea/PC/IMY/first_un.IMY.bin");
	//PG::FILE::decompressIMYPackage("C:/Users/ProgSys/Desktop/Disgaea/PC/IMY/START.DAT","C:/Users/ProgSys/Desktop/Disgaea/PC/IMY/START_UN.DAT");

	unsigned int c0 = 0;
	unsigned int c1 = 0;
	int target = 199;
	int error = 99999999;
	for(unsigned int x = 0; x < 255; x += 4)
		for(unsigned int y = 0; y < 255; y += 4){
			int reconstruction = x*(1.0/3.0)+  y*(2.0/3.0);
			int buf_error = std::abs(target-reconstruction);
			if(buf_error < error){
				error = buf_error;
				c0 = x;
				c1 = y;
			}
		}

	OUTSTR("c0: "<<c0<<" c1: "<<c1<<" err: "<<error);

	int cy = std::floor((3*int(target))/8.0) * 4;
	if(cy > 252) cy = 252;
	int cx = 0;
	int cxpart = 0;
	if( (cxpart = int(target)*3 - cy*2) > 0){
		cx = std::round(cxpart/4.0) * 4;
		if((target+1) % 8 == 0){ cx -=4; cy +=4;}
		else if((target+1) % 4 == 0) cx +=4;
	}
	if(cx > 252) cx = 252;
	OUTSTR("c0: "<<cx<<" c1: "<<cy);

	OUTSTR("END");

	return 0;
}
