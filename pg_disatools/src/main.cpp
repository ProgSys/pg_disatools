/*
 * Main just for testing.
 *
 *  Copyright (C) 2016  ProgSys
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <iostream>
#include <string>


#include <pg/files/PG_S3File.h>
#include <pg/util/PG_BinaryFileTokenizer.h>
#include <pg/util/PG_Vector.h>
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
