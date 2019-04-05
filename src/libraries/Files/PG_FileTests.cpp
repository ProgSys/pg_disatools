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
#include <Files/PG_FileTests.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamOutByteFile.h>
#include <Files/PG_SH.h>

namespace PG {
namespace FILE {

inline bool openFile(const PG::UTIL::File& file, bool (*func)(PG::STREAM::InByteFile& reader)){
	PG::STREAM::InByteFile reader(file);

	if(reader.isopen()){
		const bool b = func(reader);
		reader.close();
		return b;
	}

	return false;
}

bool isIMY(const PG::UTIL::File& file){
	return openFile(file, isIMY);
}

bool isIMY(PG::STREAM::InByteFile& reader){
	return reader.readString(3) == "IMY";
}



unsigned int isIMYPackage(const PG::UTIL::File& file){
	PG::STREAM::InByteFile reader(file);

	if(reader.isopen()){
		const unsigned int b = isIMYPackage(reader);
		reader.close();
		return b;
	}

	return false;
}

unsigned int isIMYPackage(PG::STREAM::InByteFile& reader){
	const unsigned int start = reader.pos();
	const unsigned int number_of_files = reader.readUnsignedInt();
	if(number_of_files > 9000) return 0;

	const unsigned int commpress_size = reader.readUnsignedInt();
	const unsigned int first_offset = reader.readUnsignedInt();
	if(first_offset > commpress_size) return 0;
	reader.seek(start+first_offset);

	if(isIMY(reader)){
		return number_of_files*commpress_size;
	}
	return 0;
}

bool isSpriteSheetPackage(const PG::UTIL::File& file){
	return openFile(file, isSpriteSheetPackage);
}

bool isSpriteSheetPackage(PG::STREAM::InByteFile& reader){
	const unsigned int start = reader.pos();
	const unsigned int number_of_files = reader.readUnsignedInt();
	if(number_of_files > 9000) return false;

	const unsigned int firstFileOffset = reader.readUnsignedInt();

	const unsigned int chracterIdsOffset = number_of_files*4+4;
	if(firstFileOffset < chracterIdsOffset+number_of_files*2) return false;

	return true;
}


bool isTX2(const PG::UTIL::File& file){
	return openFile(file, isTX2);
}

bool isTX2(PG::STREAM::InByteFile& reader){
	const unsigned short width = reader.readUnsignedShort();
	const unsigned short height = reader.readUnsignedShort();

	const unsigned short type = reader.readUnsignedShort();
	const unsigned short same = reader.readUnsignedShort();

	const unsigned short colorTableSize = reader.readUnsignedShort();
	reader.skip(2);

	if(width == 0 || height == 0)
		return false;
	//if(reader.readUnsignedInt() != 65536) //width*height)
		//return false;

	switch (type) {
		case tx2Type::DXT1:
		{
			if(width%4 != 0 || height%4 != 0 || colorTableSize != 0 )
				return false;
		}
			break;
		case tx2Type::DXT5:
		{
			if(width%4 != 0 || height%4 != 0 || colorTableSize != 0)
				return false;
		}
			break;
		case tx2Type::BGRA:
		{
			if(( colorTableSize != 0 && colorTableSize != 16) )
				return false;
		}
			break;
		case tx2Type::COLORTABLE_RGBA256:
		{
			if(colorTableSize == 0 || colorTableSize > 500)
				return false;
		}
			break;
		case tx2Type::COLORTABLE_BGRA256:
		{
			if(colorTableSize == 0 || colorTableSize > 500)
				return false;
		}
			break;
		case tx2Type::COLORTABLE_BGRA16:
		{
			if(colorTableSize != 16  && colorTableSize != 256)
				return false;
		}
			break;
		case tx2Type::COLORTABLE_RGBA16:
		{
			if(colorTableSize != 16 && colorTableSize != 256)
				return false;
		}
			break;
		default:
			return false;
			break;
	}

	return true;
}


bool isNISPACK(const PG::UTIL::File& file){
	return openFile(file, isNISPACK);
}
bool isNISPACK(PG::STREAM::InByteFile& reader){
	return reader.readString(7) == "NISPACK";
}

bool isDSARC_FL(const PG::UTIL::File& file){
	return openFile(file, isDSARC_FL);
}

bool isDSARC_FL(PG::STREAM::InByteFile& reader){
	return reader.readString(8) == "DSARC FL";
}


tx2Type getTX2CompressionType(PG::STREAM::InByteFile& reader){
	reader.skip(4);
	const unsigned short type = reader.readUnsignedShort();

	switch (type) {
		case tx2Type::DXT1:
		{
			return tx2Type::DXT1;
		}
			break;
		case tx2Type::DXT5:
		{
			return tx2Type::DXT5;
		}
			break;
		case tx2Type::BGRA:
		{
			return tx2Type::BGRA;
		}
			break;
		case tx2Type::COLORTABLE_RGBA256:
		{
			return tx2Type::COLORTABLE_RGBA256;
		}
			break;
		case tx2Type::COLORTABLE_BGRA256:
		{
			return tx2Type::COLORTABLE_BGRA256;
		}
			break;
		case tx2Type::COLORTABLE_BGRA16:
		{
			return tx2Type::COLORTABLE_BGRA16;
		}
			break;
		case tx2Type::COLORTABLE_RGBA16:
		{
			return  tx2Type::COLORTABLE_RGBA16;
		}
			break;
		default:
		{
			PG_INFO_STREAM("Unknown texture type: "<<type);
			return tx2Type::TX2ERROR;
		}
			break;
	}

	return tx2Type::TX2ERROR;
}

std::string asString(tx2Type type){
	switch (type) {
		case tx2Type::DXT1:
		{
			return "DXT1";
		}
			break;
		case tx2Type::DXT5:
		{
			return "DXT5";
		}
			break;
		case tx2Type::BGRA:
		{
			return "BGRA";
		}
			break;
		case tx2Type::COLORTABLE_RGBA256:
		{
			return "COLORTABLE RGBA256";
		}
			break;
		case tx2Type::COLORTABLE_BGRA256:
		{
			return "COLORTABLE BGRA256";
		}
			break;
		case tx2Type::COLORTABLE_BGRA16:
		{
			return "COLORTABLE BGRA16";
		}
			break;
		case tx2Type::COLORTABLE_RGBA16:
		{
			return "COLORTABLE RGBA16";
		}
			break;
		default:
			return "";
			break;
	}
}


bool isSpriteSheet(const PG::UTIL::File& file){
	return openFile(file, isSpriteSheet);
}

bool isSpriteSheet(PG::STREAM::InByteFile& reader){
	const unsigned int startPos = reader.pos();
	shfileHeader header;
	reader.read((char*)&header, sizeof(shfileHeader));

	std::vector<unsigned int> pointers(4);
	reader.read((char*)&pointers[0], 4*sizeof(unsigned int));

	if(pointers[0] == 0 || pointers[1] == 0 || pointers[2] == 0 || pointers[3] == 0) return false;
	if(pointers[3]+startPos >= reader.size()) return false;

	return true;
}


bool isPSPFS(PG::STREAM::InByteFile& reader){
	if(reader.readString(8) == "PSPFS_V1"){
		return true;
	}
	return false;
}

bool isPSPFS(const PG::UTIL::File& file){
	return openFile(file, isPSPFS);
}

EXPORT bool isANMD2(PG::STREAM::InByteFile& reader){
	const unsigned int files = reader.readUnsignedInt();
	if( files > 0 && files < 9000){
		if(reader.readUnsignedInt() != 0) return false;
		if(reader.readUnsignedInt() != 0) return false;
		if(reader.readUnsignedInt() != 0) return false;
		return true;
	}
	return false;
}

EXPORT bool isANMD2(const PG::UTIL::File& file){
	return openFile(file, isANMD2);
}


} /* namespace FILE */
} /* namespace PG */
