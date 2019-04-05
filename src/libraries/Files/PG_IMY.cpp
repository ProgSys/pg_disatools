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
#include <Files/PG_IMY.h>

#include <iomanip>

#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamInOutByteFile.h>
#include <Stream/PG_StreamInByteArray.h>
#include <Util/PG_Exception.h>
#include <Util/PG_Array.h>

namespace PG {
namespace FILE {

PACK(struct imyHeader{
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
	unsigned short number_of_info_bytes;// 2 -> 34
} );

/*!
 * @param instream, is a wrapper class witch just can read
 * @param outstream, is a wrapper class witch can read and write
 */
bool decompressIMYSimple(PG::STREAM::In* instream, PG::STREAM::InOut* outstream){
	const unsigned int startOffset = outstream->pos();

	//read the header
	imyHeader header;
	instream->read((char*) &header, sizeof(imyHeader));

	if(header.magic_number != 0x00594D49){
		std::cerr << "IMY file magic number is wrong!" << std::endl;
		return FAILURE;
	}

	if( (header.compressionType >> 4) == 1){

		const unsigned int decompressedFileSize = header.streamOffset * header.unknown07; //not sure

		unsigned int infoBytesOffset = instream->pos();
		const unsigned int infoBytesOffsetEnd = infoBytesOffset + header.number_of_info_bytes;
		unsigned int dataOffset = infoBytesOffsetEnd;

		//create look up table
		PG::UTIL::Array<unsigned int, 4> lookUpTable;
		lookUpTable[0] = 2; // go back one short
		lookUpTable[1] = header.streamOffset;
		lookUpTable[2] = lookUpTable[1] + 2;
		lookUpTable[3] = lookUpTable[1] - 2;

		while( (outstream->pos()-startOffset) < decompressedFileSize && infoBytesOffset < infoBytesOffsetEnd){

			// read every info byte
			instream->seek(infoBytesOffset);
			unsigned char infoByte = instream->readChar();
			infoBytesOffset++;

			if( infoByte & 0xF0 ){
				if( (infoByte & 0x80) && (infoByte & 0x40)){
					//copy shorts from the already uncompressed stream by looking back
					const int index = (infoByte & 0x30) >> 4; // the value can only be 0-3
					const int shorts_to_copy = (infoByte & 0x0F) + 1;

					for (int i = 0; i < shorts_to_copy; i++){
						//read
						const unsigned int currentEnd = outstream->pos();
						outstream->seek(currentEnd - lookUpTable[index]);
						const short s = outstream->readShort();
						outstream->seek(currentEnd);
						outstream->writeShort(s);
					}

				}else{
					//copy a short from the compressed stream by looking back to a short
					const unsigned int lookback_bytes = (infoByte - 16)*2 + 2;
					instream->seek(dataOffset-lookback_bytes);
					outstream->writeShort(instream->readShort());
				}
			}else{
				// just copy shorts (2 byte)
				//you always copy at least one short
				const unsigned int copy_bytes = (infoByte+1)*2;
				char * const c = new char[copy_bytes];
				instream->seek(dataOffset);
				instream->read(c, copy_bytes);
				outstream->write(c, copy_bytes);
				dataOffset += copy_bytes;
				delete[] c;
			}
		}
	}else{
		std::cerr << "IMY compression type '"<<header.compressionType<<"' is not supported!" << std::endl;
		return FAILURE;
	}

	return SUCCESS;
}

bool decompressIMY(PG::STREAM::In* instream, PG::STREAM::InOut* outstream, PercentIndicator* percent ){

	const unsigned int startOffset = outstream->pos();
	imyHeader header;
	instream->read((char*) &header, 34);

	if(header.magic_number != 0x00594D49){
		PG_ERROR_STREAM("IMY file magic number is wrong!");
		return FAILURE;
	}

	if( (header.compressionType >> 4) == 1){
		const unsigned int decompressedFileSize = header.streamOffset * header.unknown07;
		unsigned int flagsOffset = instream->pos();
		const unsigned int compressedDataOffset = flagsOffset + header.number_of_info_bytes;
		unsigned int dataOffset = compressedDataOffset;

		PG::UTIL::Array<unsigned int, 4> lookUpTable;
		lookUpTable[0] = 2; // go back one short
		lookUpTable[1] = header.streamOffset;
		lookUpTable[2] = lookUpTable[1] + 2;
		lookUpTable[3] = lookUpTable[1] - 2;

		//unsigned long long compressionFlags = 0;
		unsigned char compressionFlags[128];
		unsigned int currentFlag = 0;


		while( (outstream->pos()-startOffset) < decompressedFileSize && flagsOffset < compressedDataOffset){
			if(percent) percent->percent = flagsOffset/float(compressedDataOffset)*100;
			//too speed up the reading, 128 flags are readed at once
			//slow way
			/*
			 * instream->seek(flagsOffset);
			 * compressionFlag = instream->readChar();
			 */
			if(currentFlag >= 128)
				currentFlag = 0;
			if(currentFlag == 0){
				instream->seek(flagsOffset);
				instream->read((char*)&compressionFlags[0],128);
			}
			unsigned char compressionFlag = compressionFlags[currentFlag];
			currentFlag++;
			flagsOffset++;

			if( compressionFlag & 0xF0 ){
				if( (compressionFlag & 0x80) && (compressionFlag & 0x40)){
					//copy shorts from the already uncompressed stream
					const int index = (compressionFlag & 0x30) >> 4; // the value can only be 0-3
					const int shorts_to_copy = (compressionFlag & 0x0F) + 1;
					const unsigned int bytes_to_write = ((compressionFlag & 0x0F) + 1)*2;
					const unsigned int currentEnd = outstream->pos();
					const unsigned int backOffset = currentEnd - lookUpTable[index];
					const unsigned int bytes_to_copy = currentEnd - backOffset;

					char * const c = new char[bytes_to_copy];
					outstream->seek(backOffset);
					outstream->read(c, bytes_to_copy);

					outstream->seek(currentEnd);
					unsigned int bytesWritten = 0;
					unsigned int currentByte = 0;
					while(bytesWritten < bytes_to_write){
						outstream->writeChar(c[currentByte]);
						bytesWritten++;
						currentByte++;
						if(currentByte >= bytes_to_copy)
							currentByte = 0;
					}

					delete[] c;
					/*
					for (int i = 0; i < shorts_to_copy; i++){
						//read
						const unsigned int currentEnd = outstream->pos();
						outstream->seek(currentEnd - lookUpTable[index]);
						const short s = outstream->readShort();
						outstream->seek(currentEnd);
						outstream->writeShort(s);
					}*/
				}else{
					//copy a short from the compressed stream by going back to a short
					const unsigned int stepsBack = (compressionFlag - 16)*2 + 2;
					instream->seek(dataOffset-stepsBack);
					outstream->writeShort(instream->readShort());
				}
			}else{
				// just copy shorts (2 byte)
				//you always copy at least one short
				const unsigned int readSize = (compressionFlag+1)*2;
				char * const c = new char[readSize];
				instream->seek(dataOffset);
				instream->read(c, readSize);
				outstream->write(c, readSize);
				dataOffset += readSize;
				delete[] c;
				/*
				for(unsigned char i = 0; i < compressionFlag; ++i){
					instream->seek(dataOffset);
					outstream->writeShort(instream->readShort());
					dataOffset += 2;
				}*/
			}

		}

	}else{
		PG_ERROR_STREAM("IMY compression type '"<<header.compressionType<<"' is not supported!");
		return FAILURE;
	}

	return SUCCESS;
}

bool decompressIMY(const char* bytesIn, unsigned int lenghtIn, const PG::UTIL::File& fileOut, PercentIndicator* percent ){
	try {
		PG::STREAM::InByteArray reader(bytesIn, lenghtIn);

		if(fileOut.exists())
			fileOut.remove();
		fileOut.create();
		PG::STREAM::InOutByteFile writer(fileOut);

		const bool s = decompressIMY((PG::STREAM::In*) &reader, (PG::STREAM::InOut*) &writer, percent);
		reader.close();
		writer.close();

		return s;
	} catch (PG::UTIL::Exception& e) {
		PG_ERROR_STREAM(e.what());
		PG_ERROR_STREAM("Couldn't decompress IMY!")
		return true;
	} catch (...) {
		PG_ERROR_STREAM("Couldn't decompress IMY!")
		return true;
	}

	return false;
}

bool decompressIMY(const PG::UTIL::File& fileIn, const PG::UTIL::File& fileOut, PercentIndicator* percent){

	try {
		PG::STREAM::InByteFile reader(fileIn);
		if(!reader.isopen()) return FAILURE;

		if(fileOut.exists())
			fileOut.remove();
		fileOut.create();
		PG::STREAM::InOutByteFile writer(fileOut);

		const bool s = decompressIMY((PG::STREAM::In*) &reader, (PG::STREAM::InOut*) &writer, percent);
		reader.close();
		writer.close();
		return s;
	} catch (PG::UTIL::Exception& e) {
		PG_ERROR_STREAM(e.what());
		PG_ERROR_STREAM("Couldn't decompress IMY '"<<fileIn<<"'!")
		return FAILURE;
	} catch (...) {
		PG_ERROR_STREAM("Couldn't decompress IMY '"<<fileIn<<"'!")
		return FAILURE;
	}

	return SUCCESS;
}

bool decompressIMY(const std::string& fileIn, const std::string& fileOut ){
	return decompressIMY(PG::UTIL::File(fileIn), PG::UTIL::File(fileOut));
}

/*!
 * @brief Uncompress a IMY package. One big file is usually compressed into several IMY and saved as a package.
 * @return true on error
 */
bool decompressIMYPackage(PG::STREAM::In* instream, PG::STREAM::InOut* outstream, PercentIndicator* percent ){

	//PG::STREAM::InByteFile reader(fileIn);
	//if(!reader.isopen()) return false;

	const unsigned int packages = instream->readUnsignedInt();
	const unsigned int decopressedSize = instream->readUnsignedInt();

	std::vector<unsigned int> offsets(packages);
	instream->read((char*) &offsets[0],packages*sizeof(unsigned int) );
	//offsets[packages+1] = reader.size();

	//if(fileOut.exists())
		//fileOut.remove();
	//fileOut.create();
	//PG::STREAM::InOutByteFile writer(fileOut);
	unsigned int i = 0;
	for(unsigned int offset: offsets){
	//for(unsigned int i = 0; i < packages; ++i){
		//const unsigned int offset = offsets[i];
		//const unsigned int size = offsets[i+1] -  offset;

		if(percent) percent->percent = (i/(float)packages) *100;
		/*
		if(i%10 == 0)
			std::cout <<"Progress: "<<std::setw(6)<<std::setprecision(3)<<  (i/(float)packages) *100<<"%" <<std::endl;
			*/
		i++;

		instream->seek(offset);
		if( decompressIMY(instream, outstream) ){
			PG_ERROR_STREAM("Couldn't decompress IMY file at "<<offset<<"!")
			return FAILURE;
		}
	}

	return false;
}

/*!
 * @return true on error
 */
bool decompressIMYPackage(const PG::UTIL::File& fileIn, const PG::UTIL::File& fileOut, PercentIndicator* percent ){
	try {
		PG::STREAM::InByteFile reader(fileIn);
		if(!reader.isopen()) return FAILURE;

		if(fileOut.exists())
			fileOut.remove();
		fileOut.create();
		PG::STREAM::InOutByteFile writer(fileOut);

		const bool s = decompressIMYPackage((PG::STREAM::In*) &reader, (PG::STREAM::InOut*) &writer, percent);

		reader.close();
		writer.close();

		if(s == FAILURE){
			PG_ERROR_STREAM("Couldn't decompress IMY package'"<<fileIn<<"'!");
			return FAILURE;
		}

	} catch (PG::UTIL::Exception& e) {
		PG_ERROR_STREAM(e.what());
		PG_ERROR_STREAM("Couldn't decompress IMY package'"<<fileIn<<"'!")
		return FAILURE;
	} catch (...) {
		PG_ERROR_STREAM("Couldn't decompress IMY package'"<<fileIn<<"'!")
		return FAILURE;
	}

	return SUCCESS;
}

bool decompressIMYPackage(const std::string& fileIn, const std::string& fileOut, PercentIndicator* percent ){
	return decompressIMYPackage(PG::UTIL::File(fileIn), PG::UTIL::File(fileOut), percent);
}

bool decompressIMYPackage(const char* bytesIn, unsigned int lenghtIn, const PG::UTIL::File& fileOut, PercentIndicator* percent ){
	try {
		PG::STREAM::InByteArray reader(bytesIn, lenghtIn);

		if(fileOut.exists())
			fileOut.remove();
		fileOut.create();
		PG::STREAM::InOutByteFile writer(fileOut);

		const bool s = decompressIMYPackage((PG::STREAM::In*) &reader, (PG::STREAM::InOut*) &writer, percent);

		writer.close();
		if(s == FAILURE){
			PG_ERROR_STREAM("Couldn't decompress IMY package!");
			return FAILURE;
		}

	} catch (PG::UTIL::Exception& e) {
		PG_ERROR_STREAM(e.what());
		PG_ERROR_STREAM("Couldn't decompress IMY package!")
		return FAILURE;
	} catch (...) {
		PG_ERROR_STREAM("Couldn't decompress IMY package!")
		return FAILURE;
	}

	return false;
}

} /* namespace FILE */
} /* namespace PG */
