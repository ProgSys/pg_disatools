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
#include <pg/util/PG_VectorUtil.h>
#include <pg/stream/PG_StreamInByteFile.h>

#include <pg/util/PG_Matrix.h>

#include <cstdlib>
#include <cmath>
#include <iomanip>

#include <pg/util/PG_Image.h>
#include <pg/files/PG_MPD.h>


#define OUTSTR(x) std::cout << x << std::endl

typedef         char   i8 ;
typedef         unsigned char   ui8 ;

typedef         short   i16 ;
typedef         unsigned short   ui16 ;

typedef         int   i32 ;
typedef         unsigned int   ui32 ;

typedef struct
{
	i16 unknown0;
	i16 size_data1;
	i16 size_data2;
	i16 size_data3;
	i16 number_of_colorTabels;

	i16 number_of_spriteImagesInfos;
	i16 size_data6;
	i16 size_data7;
	i16 size_data8;
	i16 size_data9;
} __attribute__((packed, aligned(1))) sh2fileHeader;

typedef struct
{
	ui32 address_data1;
	ui32 address_data2;
	ui32 address_data3;
	ui32 address_colorTabels;
	ui32 address_spriteImagesInfos;
	ui32 address_data6;
	ui32 address_data7;
	ui32 address_data8;
	ui32 address_data9;
	ui32 zero;
} __attribute__((packed, aligned(1))) sh2fileAddresses;

typedef struct
{
	ui32 size;
	ui32 address;
} __attribute__((packed, aligned(1))) sh2fileColorTable;

typedef struct
{
	ui16 width;
	ui16 height;
	ui8 unknowna;
	ui8 unknownb;
	i16 unknown2;
	ui32 size;
} __attribute__((packed, aligned(1))) sh2fileSpriteImages;

int main(int argc, char* argv[]){
	OUTSTR("Start");



	PG::STREAM::InByteFile reader("D:/Users/ProgSys/ownCloudHome/Public/Work/Modding/Disgaea 2 PC/work/sprites/sprite.sh2");
	if(!reader.isopen()) return 0;

	sh2fileHeader header;
	sh2fileAddresses addresses;

	std::vector<sh2fileColorTable> colorTablesInfos;
	std::vector<sh2fileSpriteImages> spriteImagesInfos;

	reader.read((char*)&header, sizeof(sh2fileHeader));
	reader.read((char*)&addresses, sizeof(sh2fileAddresses));

	colorTablesInfos.resize(header.number_of_colorTabels);
	spriteImagesInfos.resize(header.number_of_spriteImagesInfos);


	reader.seek(addresses.address_colorTabels);
	reader.read((char*)&colorTablesInfos[0], colorTablesInfos.size()*sizeof(sh2fileColorTable));
	reader.seek(addresses.address_spriteImagesInfos);
	reader.read((char*)&spriteImagesInfos[0], spriteImagesInfos.size()*sizeof(sh2fileSpriteImages));

	reader.seek(addresses.address_data9+header.size_data9*4);

	std::vector<PG::FILE::ColorTable> colortables(colorTablesInfos.size());
	auto itInfo = colorTablesInfos.begin();
	auto itTable = colortables.begin();
	while(itInfo != colorTablesInfos.end()){
		itTable->resize(itInfo->size);
		OUTSTR("Color table at: "<<reader.pos()<<" s: "<<itInfo->size);
		reader.read((char*)&(*itTable)[0].r, itTable->size()*sizeof(PG::UTIL::rgba));

		for(PG::UTIL::rgba& bgra: (*itTable)){
			const char r = bgra.b;
			bgra.b = bgra.r;
			bgra.r = r;
		}

		itInfo++;
		itTable++;
	}

	if(colortables.empty()){
		OUTSTR("No colortables!");
		return 1;
	}

	if(reader.pos() >= reader.size() ) return 0;
	unsigned int i = 0;
	for(const sh2fileSpriteImages& info: spriteImagesInfos){
		PG::UTIL::RGBAImage img(info.width, info.height);
		OUTSTR("Color image "<<i << " at: "<<reader.pos());


		for(unsigned int i = 0; i < img.size(); i+=2){
			if(reader.eof()) break;
			const unsigned char c = reader.readUnsignedChar();
			img[i] = colortables[0][c & 0x0F];
			img[i+1] = colortables[0][ (c >> 4) & 0x0F];

		}

		PG::FILE::saveTGA(SSTR("D:/Users/ProgSys/ownCloudHome/Public/Work/Modding/Disgaea 2 PC/work/sprites/sprite_"<<i<<".tga"), img);
		i++;
	}



	OUTSTR("END");
}
