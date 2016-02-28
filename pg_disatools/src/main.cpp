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


#include <pg/files/PG_S3File.h>
#include <pg/util/PG_BinaryFileTokenizer.h>
#include <pg/util/PG_Colors.h>
#include <pg/util/PG_ByteStream.h>

#define OUTSTR(x) std::cout << x << std::endl

/*!
 * @brief Testing main method, just for testing.
 */
int main(int argc, char* argv[]){
	OUTSTR("Start");

	char* data = new char[10];
	for(unsigned int i = 0; i < 10; i++){
		data[i] = 10;
	}
	data[0] = 0;
	data[1] = 1;

	data[2] = 0;
	data[3] = 0;

	data[4] = 80;
	data[5] = 1;

	PG::UTIL::ByteStream stream(data, 10);
	OUTSTR(stream.readInt());
	OUTSTR(stream.readShort());

	delete[] data;
	/*
	//std::string target = "C:/Users/ProgSys/Desktop/Disgaea/texture_analys/BU3202.TX2";
	//std::string out = "C:/Users/ProgSys/Desktop/Disgaea/texture_analys/BU3202.tga";

	std::string target = "C:/Users/ProgSys/Desktop/Disgaea/texture_analys/mppTex.tx2";
	std::string out = "C:/Users/ProgSys/Desktop/Disgaea/texture_analys/mppTex.tga";


	//std::string target = "C:/Users/ProgSys/Desktop/Disgaea/texture_analys/alphatest.TX2";
	//std::string out = "C:/Users/ProgSys/Desktop/Disgaea/texture_analys/alphatest.tga";


	//std::string target = "C:/Users/ProgSys/Desktop/Disgaea/texture_analys/32gradDXT1.vtf";
	//std::string out = "C:/Users/ProgSys/Desktop/Disgaea/texture_analys/32gradDXT1.ppm";

	//std::string target = "C:/Users/ProgSys/Desktop/Disgaea/texture_analys/2BlockDXT1.vtf";
	//std::string out = "C:/Users/ProgSys/Desktop/Disgaea/texture_analys/2BlockDXT1.ppm";

	PG::FILE::S3File file(target);
	file.save(out, PG::FILE::S3File::TGA);

	*/
	OUTSTR("Done");

	return 0;
}
