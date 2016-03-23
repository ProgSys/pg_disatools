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
#include <pg/files/PG_IMY.h>
#include <pg/stream/PG_StreamInByteFile.h>
#include <pg/stream/PG_StreamInOutByteFile.h>
#include <pg/util/PG_Exception.h>
#include <pg/util/PG_Array.h>

namespace PG {
namespace FILE {

struct imyHeader{
	unsigned int magic_number; // 4

	unsigned short unknown02; // 2
	unsigned short unknown03; // 2 -> 8
	unsigned short streamOffset; //was width // 2
	unsigned char compressionType; // 1
	unsigned char unknown06; // 1
	unsigned short unknown07; //was height // 2
	unsigned short unknown08; //was paletteSize // 2 -> 16
	unsigned int zero0; //padding // 4
	unsigned int zero1; //padding // 4 -> 24
	unsigned int zero2; //padding // 4
	unsigned int zero3; //padding // 4 -> 32
	unsigned short compressionInfoBits;// 2 -> 34
};

bool decompressIMY(PG::STREAM::In* instream, PG::STREAM::InOut* outstream ){

	const unsigned int startOffset = outstream->pos();
	imyHeader header;
	instream->read((char*) &header, 34);

	if(header.magic_number != 0x00594D49){
		PG_ERROR_STREAM("IMY magic number is wrong!");
		return true;
	}

	if( (header.compressionType >> 4) == 1){
		const unsigned int decompressedFileSize = header.streamOffset * header.unknown07;
		unsigned int flagsOffset = instream->pos();
		const unsigned int compressedDataOffset = flagsOffset + header.compressionInfoBits;
		unsigned int dataOffset = compressedDataOffset;

		PG::UTIL::Array<unsigned int, 4> lookUpTable;
		lookUpTable[0] = 2; // go back one short
		lookUpTable[1] = header.streamOffset;
		lookUpTable[2] = lookUpTable[1] + 2;
		lookUpTable[3] = lookUpTable[1] - 2;

		while( (outstream->pos()-startOffset) < decompressedFileSize && flagsOffset < compressedDataOffset){
			instream->seek(flagsOffset);
			unsigned char compressionFlag = instream->readChar();
			flagsOffset++;

			if( compressionFlag & 0xF0 ){
				if( (compressionFlag & 0x80) && (compressionFlag & 0x40)){
					//copy shorts from the already uncompressed stream
					const int index = (compressionFlag & 0x30) >> 4; // the value can only be 0-3
					const int shorts_to_copy = (compressionFlag & 0x0F) + 1;

					for (int i = 0; i < shorts_to_copy; i++){
						//read
						const unsigned int currentEnd = outstream->pos();
						outstream->seek(currentEnd - lookUpTable[index]);
						const short s = outstream->readShort();
						outstream->seek(currentEnd);
						outstream->writeShort(s);
					}
				}else{
					//copy a short from the compressed stream by going back to a short
					const unsigned int stepsBack = (compressionFlag - 16)*2 + 2;
					instream->seek(dataOffset-stepsBack);
					outstream->writeShort(instream->readShort());
				}
			}else{
				// just copy shorts (2 byte)
				compressionFlag++; //you always copy at least one short
				for(unsigned char i = 0; i < compressionFlag; ++i){
					instream->seek(dataOffset);
					outstream->writeShort(instream->readShort());
					dataOffset += 2;
				}
			}

		}

	}else{
		PG_ERROR_STREAM("IMY compression type '"<<header.compressionType<<"' is not supported!");
		return true;
	}

	return false;
}

bool decompressIMY(const PG::UTIL::File& fileIn, const PG::UTIL::File& fileOut){

	try {
		PG::STREAM::InByteFile reader(fileIn);

		if(fileOut.exists())
			fileOut.remove();
		fileOut.create();
		PG::STREAM::InOutByteFile writer(fileOut);

		const bool s = decompressIMY((PG::STREAM::In*) &reader, (PG::STREAM::InOut*) &writer);
		return s;
	} catch (PG::UTIL::Exception& e) {
		PG_ERROR_STREAM(e.what());
		PG_ERROR_STREAM("Couldn't read IMY '"<<fileIn<<"'!")
		return true;
	} catch (...) {
		PG_ERROR_STREAM("Couldn't read IMY '"<<fileIn<<"'!")
		return true;
	}

	return false;
}

bool decompressIMY(const std::string& fileIn, const std::string& fileOut ){
	return decompressIMY(PG::UTIL::File(fileIn), PG::UTIL::File(fileOut));
}



} /* namespace FILE */
} /* namespace PG */
