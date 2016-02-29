/*
 * Functions to save rgba data as different files.
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
#include <pg/files/PG_ImageFiles.h>
#include <sstream>
#include <pg/util/PG_BinaryFileWriter.h>

namespace PG {
namespace FILE {

void saveTGA(const std::string& filepath, unsigned int width, unsigned int height, const std::vector<PG::UTIL::rgba>& rgba){
	PG::UTIL::BinaryFileWriter writer(filepath);
	writer.writeInt(131072);
	writer.writeInt(0);
	writer.writeInt(0);
	writer.writeShort(width);
	writer.writeShort(height);
	writer.writeShort(2080);


	for(unsigned int y = 0; y < height; ++y ){
		for(unsigned int x = 0; x < width; ++x ){
			const unsigned int index = ((height-y-1)*width+x); //y-flip
			const PG::UTIL::rgba& pix = rgba[index];

			writer.writeChar(pix.b);
			writer.writeChar(pix.g);
			writer.writeChar(pix.r);
			writer.writeChar(pix.a);
		}
	}

	writer.writeLongLong(0);
	writer.writeLongLong(0);
	writer.writeString("TRUEVISION-XFILE.");
}

void savePGM(const std::string& filepath, unsigned int width, unsigned int height, const std::vector<PG::UTIL::rgba>& rgba){
	//return savePPM(outfilename);
	PG::UTIL::BinaryFileWriter writer(filepath);
	std::stringstream o;
	o<<"P6\n"<<width<<"\n"<<height<<"\n255\n";
	writer.writeString(o.str());

	for(const PG::UTIL::rgba& pix: rgba){
		writer.writeChar(pix.r);
		writer.writeChar(pix.g);
		writer.writeChar(pix.b);
	}
}

} /* namespace FILE */
} /* namespace PG */
