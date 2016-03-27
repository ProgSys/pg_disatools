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
#include <pg/stream/PG_StreamInByteFile.h>

#include <cstdlib>
#include <cmath>
#include <iomanip>

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



typedef struct
{
	unsigned short unknown0;
	unsigned short unknown1;
} __attribute__((packed, aligned(1))) part0;

std::ostream& operator<<(std::ostream& o,const part0& i){
	o <<"("<<i.unknown0<<", "<<i.unknown1<<", ["<<  (  (int)(i.unknown1<<2 | i.unknown0))<<"] )";
	return o;
}

typedef struct
{
	unsigned short unknown0;
	unsigned short unknown1;
} __attribute__((packed, aligned(1))) part1;

std::ostream& operator<<(std::ostream& o,const part1& i){
	o <<"("<<i.unknown0<<", "<<i.unknown1<<", ["<<  (  (int)(i.unknown1<<2 | i.unknown0)) <<"] )";
	return o;
}

typedef struct
{
	unsigned short external_sheet; // get a sheet from different file by it's ID
	unsigned char sheet; // there is not one big sprite sheet but multiple smaller one
	unsigned char colortable; // the 16 rgba colortable which the sheet should use
	short offsetx;
	short offsety;

	//sprite info
	unsigned short x;
	unsigned short y;
	unsigned short width;
	unsigned short height;
	unsigned short scalex; // 0-100
	unsigned short scaley; // 0-100

	// only rotationx and rotationz seam to have a effect on 2D sprites
	short rotationx; //not confirmed
	short rotationy; //not confirmed
	short rotationz; //not confirmed

	unsigned short mirror; //not confirmed

} __attribute__((packed, aligned(1))) part3;

std::ostream& operator<<(std::ostream& o,const part3& i){
	o <<"("<<std::setw(4)<<i.external_sheet<<", "<<std::setw(4)<<(int)i.sheet<<", "<<std::setw(4)<<(int)i.colortable<<", "<<std::setw(4)<<i.offsetx
			<<", "<<std::setw(4)<<i.offsety<<", "<<std::setw(4)<<i.x<<", "<<std::setw(4)<<i.y<<", "<<std::setw(4)<<i.width<<", "<<std::setw(4)<<i.height<<", "<<std::setw(4)<<i.scalex
			<<", "<<std::setw(4)<<i.scaley<<", "<<std::setw(4)<<i.rotationx<<", "<<std::setw(4)<<i.rotationy<<", "<<std::setw(4)<<i.rotationz<<", "<<std::setw(4)<<i.mirror<<")";
	return o;
}


template <typename T>
inline void printInt(const std::vector<T>& arr, std::ofstream& myfile){
	unsigned int count = 0;
	for(const T& i: arr){
		myfile<<i<<", ";
		count++;
		if(count == 4){
			myfile<<"\n";
			count = 0;
		}
	}

}
/*!
 * @brief Testing main method, just for testing.
 */
int main(int argc, char* argv[]){
	OUTSTR("Start");
	PG::STREAM::InByteFile reader("C:/Users/ProgSys/Desktop/Disgaea/PC/IMY/laharl.DAT");
	const unsigned int file_size = reader.size();
	std::vector<unsigned short> header(8);
	std::vector<unsigned int> addresses(4);


	std::vector<part0> something0(((436)/sizeof(part0)));
	OUTSTR("size: 4 == " <<sizeof(part0)<<" size: 4 == " <<sizeof(part1)<<" size: 28 == " <<sizeof(part3) );
	std::vector<part1> something1((2088-468)/sizeof(part1));
	std::vector<part0> something2((8816-2088)/sizeof(part0));
	std::vector<part3> something3((37776-8816)/sizeof(part3));

	reader.read((char*)&header[0],header.size()*sizeof(short) );
	reader.read((char*)&addresses[0],addresses.size()*sizeof(int) );

	reader.read((char*)&something0[0],something0.size()*sizeof(part0) );
	reader.read((char*)&something1[0],something1.size()*sizeof(part1) );

	reader.read((char*)&something2[0],something2.size()*sizeof(part0) );
	reader.read((char*)&something3[0],something3.size()*sizeof(part3) );
	OUTSTR(reader.pos()<<" "<<something0.size());

	std::ofstream myfile;
	myfile.open ("C:/Users/ProgSys/Desktop/Disgaea/PC/IMY/laharl_parse.txt");
	if(!myfile.is_open()){
		OUTSTR("Failed to open");
		return 1;
	}
	myfile<<"header\n";
	for(unsigned short i: header)
		myfile<<i<<", ";
	OUTSTR("addresses: "<<addresses.size());
	myfile<<"\n\naddresses: "<< addresses.size()<<"\n";
	printInt(addresses, myfile);

	OUTSTR("something0: "<<something0.size());
	myfile<<"\n\nsomething0: "<< something0.size()<<"\n";
	printInt(something0, myfile);

	OUTSTR("something1: "<<something1.size());
	myfile<<"\n\nsomething1: "<< something1.size()<<"\n";
	printInt(something1, myfile);

	OUTSTR("something2: "<<something2.size());
	myfile<<"\n\nsomething2: "<< something2.size()<<"\n";
	printInt(something2, myfile);

	OUTSTR("something3: "<<something3.size());
	myfile<<"\n\nsomething3: "<< something3.size()<<"\n";
	printInt(something3, myfile);
	myfile.close();

	OUTSTR("sprites ct: "<<37776<<" portrait ct: "<<(37776+4*16)<<" coulds ct: "<<37776+4*16*2<< " end: "<<37776+4*16*3);
	//reader.seek(37776); //sprites colortable
	reader.seek(37776+4*16); //portrait colortable
	//reader.seek(37776+4*16*2); //coulds colortable
	//reader.seek(37776+4*16*4); //coulds colortable

	OUTSTR("a");
	std::vector<PG::UTIL::rgba> colortable(16);
	reader.read((char*)&colortable[0], colortable.size()*sizeof(PG::UTIL::rgba));

	for(PG::UTIL::rgba& color: colortable){
		const char r = color.r;
		color.r = color.b;
		color.b = r;
	}

	PG::UTIL::RGBAImage imageOut(256,1920);

	reader.seek(38480);
	//unsigned int skip = 65537;
	//reader.skip(skip);

	PG_INFO_STREAM(38480-37776);
	const unsigned int read_bytes = imageOut.size();
	//const unsigned int read_bytes = file_size-skip-38480;
	PG_INFO_STREAM("file size: "<<file_size<<" read bytes: "<<read_bytes);
	for(unsigned int i = 0; i < read_bytes; i+=2){
		const char c = reader.readUnsignedChar();

		//const unsigned int pos = i*2;
		imageOut[i] = colortable[ c & 0x0F];
		imageOut[i+1] = colortable[ (c >> 4) & 0x0F ];
	}
	PG_INFO_STREAM(reader.pos() << " size: "<<imageOut.size()*4);
	reader.close();

	PG::FILE::saveTGA("C:/Users/ProgSys/Desktop/Disgaea/PC/IMY/laharl.tga",imageOut);


	OUTSTR("END");

	return 0;
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
