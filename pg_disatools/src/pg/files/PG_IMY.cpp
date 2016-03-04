/*
 * PG_IMY.cpp
 *
 *  Created on: 03.03.2016
 *      Author: ProgSys
 */

#include <pg/files/PG_IMY.h>
#include <pg/util/PG_ByteInStream.h>
#include <pg/util/PG_ByteInFileStream.h>
#include <pg/util/PG_Exception.h>

namespace PG {
namespace FILE {

bool uncompressIMY(PG::UTIL::InStream* instream, PG::UTIL::RGBAImage& imageOut ){


	if(instream->readString(3) != "IMY"){
		PG_ERROR_STREAM("Magic numbers is wrong!")
		return true;
	}

	throw_Exception("uncompressIMY is currently not supported.");

	/*
	instream->skip(0); //skip one byte
	const unsigned width = instream->readShort();
	const unsigned height = instream->readShort();

	PG_INFO_STREAM(width<<" "<<height);

	instream->skip(24);
	instream->seek(148);

	imageOut.resize(128, 512);

	for(unsigned int i = 0; i < 7578; ++i){
		unsigned short s = instream->readShort();
		imageOut[i].r = s;
		imageOut[i].g = s >> 8;
	}
	//instream->read((char*) &imageOut[0].r, 110000);


	return false;
	*/
}

bool uncompressIMY(const PG::UTIL::File& fileIn, PG::UTIL::RGBAImage& imageOut ){
	try {
		PG::UTIL::ByteInFileStream reader(fileIn);
		const bool s = uncompressIMY((PG::UTIL::InStream*) &reader, imageOut);
		return s;
	} catch (PG::UTIL::Exception& e) {
		PG_ERROR_STREAM(e.what());
		PG_ERROR_STREAM("Couldn't read IMY image.")
		return true;
	} catch (...) {
		PG_ERROR_STREAM("Couldn't read IMY image.")
		return true;
	}

	return false;
}

bool uncompressIMY(const std::string& fileIn, PG::UTIL::RGBAImage& imageOut ){
	return uncompressIMY(PG::UTIL::File(fileIn),imageOut );
}

} /* namespace FILE */
} /* namespace PG */
