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
#ifndef INCLUDE_PG_FILES_PG_TX2_H_
#define INCLUDE_PG_FILES_PG_TX2_H_

#include <string>
#include <vector>
#include <pg/util/PG_Vector.h>
#include <pg/util/PG_Image.h>
#include <pg/util/PG_File.h>

#include <pg/Stream/PG_StreamIn.h>
#include <pg/util/PG_ApiUtil.h>

namespace PG {
namespace FILE {

enum tx2Type: unsigned short {
	DXT1 = 0, DXT5 = 2, BGRA = 3,  COLORTABLE_BGRA16 = 16, COLORTABLE_RGBA16 = 17, COLORTABLE_BGRA256 = 256 , COLORTABLE_RGBA256 = 257
};

/*!
 * @brief Decompresses a TX2 Disgaea PC image.
 * @param imageOut output rgba image
 * @return true if decompression was unsuccessful.
 */
bool decompressTX2(PG::STREAM::In* instream, PG::UTIL::RGBAImage& imageOut );

/*!
 * @brief Decompresses a TX2 Disgaea PC image file.
 * @param file Path to file.
 * @param imageOut output rgba image
 * @return true if decompression was unsuccessful.
 */
bool decompressTX2(const PG::UTIL::File& file, PG::UTIL::RGBAImage& imageOut );

/*!
 * @brief Decompresses a TX2 Disgaea PC image file.
 * @param file Path to file.
 * @param imageOut output rgba image
 * @return true if decompression was unsuccessful.
 */
bool decompressTX2(const std::string& file, PG::UTIL::RGBAImage& imageOut );

/*!
 * @brief Decompresses a TX2 Disgaea PC image bytes.
 * @param imageOut output rgba image
 * @return true if decompression was unsuccessful.
 */
bool decompressTX2(const std::vector<char>& bytesIn, PG::UTIL::RGBAImage& imageOut );

/*!
 * @brief Decompresses a TX2 Disgaea PC image bytes.
 * @param imageOut output rgba image
 * @return true if decompression was unsuccessful.
 */
bool decompressTX2(const char* bytesIn, unsigned int lenghtIn, PG::UTIL::RGBAImage& imageOut );


bool compressTX2(const PG::UTIL::RGBAImage& imageIn, tx2Type compressionTypeIn, std::vector<char>& bytesOut);

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_TX2_H_ */
