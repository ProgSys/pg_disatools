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

#include <pg/files/PG_Convert.h>

#include <algorithm>

#include <pg/util/PG_Base.h>
#include <pg/util/PG_Image.h>
#include <pg/files/PG_TX2.h>
#include <pg/files/PG_VTF.h>
#include <pg/files/PG_ImageFiles.h>
#include <pg/util/PG_BinaryFileWriter.h>

namespace PG {
namespace FILE {

bool convertTX2ToImage(const PG::UTIL::File& fileIn, const PG::UTIL::File& fileOut, outFileFormat formatOut){
	std::string extension = fileIn.getFileExtension();
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

	inFileFormat formatIn;

	if(extension == "vtf"){
		formatIn = VTF;
	}else if(extension == "tx2"){
		formatIn = TX2;
	}else if(extension == "mpp"){
		PG_ERROR_STREAM("*.mpp is a achieve containing multiple files.");
		return true;
	}else{
		PG_ERROR_STREAM("Unknown file extension: '"<<extension<<"'!");
		return true;
	}

	PG::UTIL::RGBAImage image;
	switch (formatIn) {
		case VTF:
		{
			if(decompressVTF(fileIn,image) ){
				PG_ERROR_STREAM("Couldn't decompress VTF Image!");
				return true;
			}
		}
			break;
		case TX2:
		{
			if(decompressTX2(fileIn,image )){
				PG_ERROR_STREAM("Couldn't decompress TX2 Image!");
				return true;
			}
		}
		break;
		default:
			return true;
		break;
	}


	switch (formatOut) {
		case PGM:
		{
			PG_INFO_STREAM("Writing PGM. (RGB)");
			savePGM(fileOut.getPath(), image);
		}
			break;
		case TGA:
		{
			PG_INFO_STREAM("Writing TGA. (RGBA)");
			saveTGA(fileOut.getPath(), image);
		}
		break;
		default:
			return true;
		break;
	}

	return false;
}


bool convertTX2ToImage(const std::string& fileIn, const std::string& fileOut, outFileFormat formatOut){
	return convertTX2ToImage(PG::UTIL::File(fileIn),PG::UTIL::File(fileOut),formatOut);
}

bool convertImageToTX2(const std::string& fileIn, const std::string& fileOut){
	std::vector<char> bytes;
	PG::UTIL::RGBAImage image;
	PG::UTIL::File file(fileIn);
	std::string extension = file.getFileExtension();
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);



	if(extension == "tga"){
		loadTGA(fileIn, image);
	}else{
		PG_INFO_STREAM("File format "<<extension<<" not supported.");
		return true;
	}

	if(compressTX2(image, DXT1, bytes)){
		PG_INFO_STREAM("Couldn't compress image to DXT1.");
		return true;
	}

	PG::UTIL::BinaryFileWriter writer(fileOut);
	writer.write((char*)&bytes[0], bytes.size());

	return false;
}

} /* namespace FILE */
} /* namespace PG */
