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
#include <pg/util/PG_ByteInFileStream.h>
#include <algorithm>
#include <pg/util/PG_BinaryFileWriter.h>
#include <fstream>
#include <pg/util/PG_Vector.h>
#include <cstdlib>
#include <cmath>

#define OUTSTR(x) std::cout << x << std::endl

struct testStr{
	testStr(const std::string& str): name(str){}
	std::string name;
};

void findTwoColorBestMatch(PG::UTIL::rgba& target, PG::UTIL::rgba& c0, PG::UTIL::rgba& c1){

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
	if(cy > 251) cy = 251;
	cx = 0;
	if( (cxpart = int(target.g)*3 - cy*2) > 0){
		cx = std::round(cxpart/4.0) * 4;
		if((target.g+1) % 8 == 0){ cx -=4; cy +=4;}
		else if((target.g+1) % 4 == 0)cx +=4;
	}
	if(cx > 251) cx = 251;

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

void findBestMatch(int target, ofstream& myfile){

	int y = std::floor((3*(target))/8.0) * 4;
	if(y > 252) y = 252;
	int x = 0;
	int xpart;
	if( (xpart = target*3 - y*2) > 0){
		x = std::round(xpart/4.0) * 4;
		if((target+1) % 8 == 0){ x -=4; y +=4;}
		else if((target+1) % 4 == 0) x +=4;
	}
	if(x > 252) x = 252;


	unsigned int control = x*(1.0/3.0)+y*(2.0/3.0);

	OUTSTR(target<< " x: "<<x<<" y: "<<y<<" c: "<<control<<" err: " << std::abs(int(target)-int(control)) );

	myfile<<(int)target<<";"<<x<<";"<<y<<";"<<control<<";"<<std::abs(int(target)-int(control));

	control = 0;
	//find best
	unsigned int error = 255*255;
	for(unsigned int x2 = 0; x2 <= 255; x2+=4){
		for(unsigned int y2 = 0; y2 <= 255; y2+=4){
			unsigned int c = x2*(1.0/3.0)+y2*(2.0/3.0);
			unsigned int diff = std::abs(int(target)-int(c));
			if(error > diff){
				error = diff;
				x = x2;
				y = y2;
				control = c;
				//OUTSTR(" Pre: x: "<<x<<" y: "<<y<<" c: "<<control<<" err: " << error<<" c: "<<c<<" - "<< std::abs(int(target)-int(c)));
			}
		}
	}
	OUTSTR(" Best: x: "<<x<<" y: "<<y<<" c: "<<control<<" err: " << error );
	myfile<<";"<<xpart/8.0<<";"<<x<<";"<<y<<";"<<control<<";"<<error;
	myfile<<"\n";
}


int roundDXTColor(double x)
{
    if (x < 0.0)
        return (int)(x - 0.51);
    else
        return (int)(x + 0.49);
}

struct imyHeader{
	unsigned int magic_number;

	unsigned short unknown02;
	unsigned short unknown03;
	unsigned short unknown04; //was width
	unsigned char compressionFlag;
	unsigned char unknown06;
	unsigned short unknown07; //was height
	unsigned short unknown08; //was paletteSize
	unsigned int unknown09; //padding
	unsigned int unknown10; //padding
	unsigned int unknown11; //padding
	unsigned int unknown12; //padding
	unsigned short compressedDataPointer;
};

/*
bool uncompressIMY(const std::string& imyfile){
	PG::UTIL::ByteInFileStream reader(imyfile);

	if(!reader.isopen()){
		OUTSTR("Coundn't open "<<imyfile);
		return true;
	}

	imyHeader header;

	reader.read((char*) &header, sizeof(imyHeader));

	if(header.magic_number != 0x00594D49){
		OUTSTR("IMY magic number is wrong "<<imyfile);
		return true;
	}

	//Int32 compressedDataInfoPointer = (Int32)fs.Position;
	//Int32 compressedDataPointer = (Int32)(fs.Position + imyHeader.compressedDataPointer);
	unsigned int decompressedFileSize = header.unknown04 * header.unknown07;


	PG::UTIL::Array<unsigned int, 4> lookUpTable;
	lookUpTable[0] = 0xFFFFFFFE;
	lookUpTable[1] = ~header.unknown04;
	lookUpTable[2] = lookUpTable[1] + 2;
	lookUpTable[3] = lookUpTable[1] - 2;


	unsigned int bytesProcessed = 0;
    while (bytesProcessed < decompressedFileSize)
    {
    	unsigned char info = reader.readChar();
    }

	return false;
}*/

/*!
 * @brief Testing main method, just for testing.
 */
int main(int argc, char* argv[]){
	OUTSTR("Start");


	return 0;

	ofstream myfile;
	myfile.open ("results2.csv");
	myfile<<"Target"<<";"<<"X"<<";"<<"Y"<<";"<<"Ergebnis"<<";"<<"Unterschied";
	myfile<<";;"<<"X"<<";"<<"Y"<<";"<<"Ergebnis"<<";"<<"Unterschied";
	myfile<<"\n";
	for(unsigned int i = 0; i <= 255; i++)
		findBestMatch(i,myfile);
	myfile.close();
	return 0;

	char a[2];
	a[0] = 0x0B;
	a[1] = 0x89;
	OUTSTR("Start: "<<(int)a[0]<<" "<<(int)a[1]);

	PG::UTIL::rgb pix;
	pix.r =  a[0] << 3;
	pix.g = ((a[1] & 0x07)<<5) | (a[0] >> 3);
	pix.b =  a[1] & 0xF8;
	//scaleRGB565to888(pix);
	OUTSTR("rgb: "<<pix);



	a[0] = (pix.r >> 3) | ((pix.g & 0x1C)<<3);
	a[1] = (pix.b & 0xF8) | (pix.g >> 5);

	OUTSTR("back: "<<(int)a[0]<<" "<<(int)a[1]);

	//return 0;

	PG::UTIL::ByteInFileStream reader("C:/Users/ProgSys/Desktop/Disgaea/PC/MPP/geo/GEOMETRY0.GEO");
	//PG::UTIL::BinaryFileWriter writer("C:/Users/ProgSys/Desktop/Disgaea/PC/MPP/geo/GEOMETRY0.obj");

	 ofstream writer;
	 writer.open ("C:/Users/ProgSys/Desktop/Disgaea/PC/MPP/geo/GEOMETRY0.obj");

	reader.skip(304);
	//reader.skip(10192);

	std::vector<unsigned int> index;

	//103;
	unsigned int tryangles = 103;

	for(unsigned int i = 0; i < (tryangles*3); i++){
		PG::UTIL::vec3 vertex;
		reader.read((char*)&vertex.x, sizeof(float)*3);

		writer << "v " << vertex.x<<" "<<vertex.z<<" "<<-vertex.y<<"\n"; // openGl swap z and y

		index.push_back(i +1);
	}

	for(unsigned int i = 0; i < (tryangles); i++){
		PG::UTIL::vec3 normal;
		reader.read((char*)&normal.x, sizeof(PG::UTIL::vec3));

		writer << "vn " << normal.x<<" "<<normal.y<<" "<<normal.z<<"\n";
		writer << "vn " << normal.x<<" "<<normal.y<<" "<<normal.z<<"\n";
		writer << "vn " << normal.x<<" "<<normal.y<<" "<<normal.z<<"\n";
		//index.push_back(i/3 +1);
	}

	OUTSTR(index.size());
	for(unsigned int i = 0; i < (tryangles*3); i++){
		PG::UTIL::vec2 uv;
		reader.read((char*)&uv.x, sizeof(float)*2);

		writer << "vt " << uv.x<<" "<<uv.y<<" 0.0\n"; // openGl swap z and y
		//writer << "vt 0.000000 0.000000 0.000000\n";

		//index.push_back(i/3 +1);
	}









	//OUTSTR("# "<<i);
	for(unsigned int i = 0; i < index.size(); i+=3){
		writer << "f ";
		writer << index[i]<<"/"<<index[i]<<"/"<<index[i];
		//writer << index[i]<<"/"<<index[i];
		//writer << index[i]<<"//"<<index[i];
		//writer << index[i];
		writer << " ";
		writer << index[i+1]<<"/"<<index[i+1]<<"/"<<index[i+1];
		//writer << index[i+1]<<"/"<<index[i+1];
		//writer << index[i+1]<<"//"<<index[i+1];
		//writer << index[i+1];
		writer << " ";
		writer << index[i+2]<<"/"<<index[i+2]<<"/"<<index[i+2];
		//writer << index[i+2]<<"/"<<index[i+2];
		//writer << index[i+2]<<"//"<<index[i+2];
		//writer << index[i+2];
		writer << "\n";

		//OUTSTR("f "<<index[i]<<" "<<index[i+1]<<" "<<index[i+2]);
	}


	//PG::UTIL::RGBAImage image;
	//PG::FILE::uncompressIMY("C:/Users/ProgSys/Desktop/Disgaea/PC/IMY/test.imy",image);
	//PG::FILE::saveTGA("C:/Users/ProgSys/Desktop/Disgaea/PC/IMY/test.tga", image);

	PG::FILE::PSPFS pspfile;

	OUTSTR("Done");

	return 0;
}
