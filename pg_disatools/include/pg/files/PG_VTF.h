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
#ifndef INCLUDE_PG_FILES_PG_VTF_H_
#define INCLUDE_PG_FILES_PG_VTF_H_

#include <string>
#include <vector>
#include <pg/util/PG_Vector.h>
#include <pg/util/PG_Image.h>
#include <pg/util/PG_File.h>

#include <pg/stream/PG_StreamIn.h>
#include <pg/util/PG_ApiUtil.h>

namespace PG {
namespace FILE {

/*!
 * @brief Decompresses a Valve Texture Format image.
 * @param imageOut output rgba image
 * @return true if decompression was unsuccessful.
 */
EXPORT bool decompressVTF(PG::STREAM::In* instream, PG::UTIL::RGBAImage& imageOut );

/*!
 * @brief Decompresses a Valve Texture Format image file.
 * @param file Path to file.
 * @param imageOut output rgba image
 * @return true if decompression was unsuccessful.
 */
EXPORT bool decompressVTF(const PG::UTIL::File& file, PG::UTIL::RGBAImage& imageOut );

/*!
 * @brief Decompresses a Valve Texture Format image file.
 * @param file Path to file.
 * @param imageOut output rgba image
 * @return true if decompression was unsuccessful.
 */
EXPORT bool decompressVTF(const std::string& file, PG::UTIL::RGBAImage& imageOut );

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_VTF_H_ */
