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

#define OUTSTR(x) std::cout << x << std::endl

struct testStr{
	testStr(const std::string& str): name(str){}
	std::string name;
};

/*!
 * @brief Testing main method, just for testing.
 */
int main(int argc, char* argv[]){
	OUTSTR("Start");

	char a[2];
	a[0] = 0;
	a[1] = 0;


	char* array = &a[0];

	array[0] = (array[0]) | (0x01 << 5);

	OUTSTR((int)a[0] <<(int) a[1]);

	return 0;

	PG::UTIL::ByteInFileStream reader("C:/Users/ProgSys/Desktop/Disgaea/PC/MPP/geo/GEOMETRY0.GEO");
	//PG::UTIL::BinaryFileWriter writer("C:/Users/ProgSys/Desktop/Disgaea/PC/MPP/geo/GEOMETRY0.obj");

	 ofstream writer;
	 writer.open ("C:/Users/ProgSys/Desktop/Disgaea/PC/MPP/geo/GEOMETRY0.obj");

	//reader.skip(304);
	reader.skip(10192);

	std::vector<unsigned int> index;

	//103;
	unsigned int tryangles = 103;

	for(unsigned int i = 0; i < (tryangles*3); i++){
		PG::UTIL::vec3 vertex;
		reader.read((char*)&vertex.x, sizeof(float)*3);

		writer << "v " << vertex.x<<" "<<vertex.y<<" "<<vertex.z<<"\n"; // openGl swap z and y

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
