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


	PG::UTIL::ByteInFileStream reader("C:/Users/ProgSys/Desktop/Disgaea/PC/MPP/geo/GEOMETRY0.GEO");
	//PG::UTIL::BinaryFileWriter writer("C:/Users/ProgSys/Desktop/Disgaea/PC/MPP/geo/GEOMETRY0.obj");

	 ofstream writer;
	 writer.open ("C:/Users/ProgSys/Desktop/Disgaea/PC/MPP/geo/GEOMETRY0.obj");

	reader.skip(304);

	std::vector<unsigned int> index;

	for(unsigned int i = 0; i < (103*3*3); i+=3){
		writer << "v " << reader.readFloat();
		writer << " ";
		writer << reader.readFloat();
		writer << " ";
		writer << reader.readFloat();
		writer << "\n";

		index.push_back(i/3 +1);
	}

	//OUTSTR("# "<<i);
	OUTSTR("");
	for(unsigned int i = 0; i < index.size(); i+=3){
		writer << "f ";
		writer << index[i];
		writer << " ";
		writer << index[i+1];
		writer << " ";
		writer << index[i+2];
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
