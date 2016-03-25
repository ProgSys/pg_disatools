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
#include <pg/files/PG_FileTests.h>
#include <pg/stream/PG_StreamInByteFile.h>
#include <pg/stream/PG_StreamOutByteFile.h>
#include <pg/files/PG_TX2.h>

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

	return false;;
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

bool isPSPFS(PG::STREAM::InByteFile& reader){
	if(reader.readString(8) == "PSPFS_V1"){
		return true;
	}
	return false;
}

bool isPSPFS(const PG::UTIL::File& file){
	return openFile(file, isPSPFS);
}


} /* namespace FILE */
} /* namespace PG */
