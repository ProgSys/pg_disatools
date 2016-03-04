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
#ifndef INCLUDE_PG_FILES_PG_IMY_H_
#define INCLUDE_PG_FILES_PG_IMY_H_

#include <string>
#include <pg/util/PG_File.h>
#include <pg/util/PG_Image.h>
#include <pg/util/PG_InStream.h>

namespace PG {
namespace FILE {

/*!
 * @return true on error
 */
bool uncompressIMY(PG::UTIL::InStream* instream, PG::UTIL::RGBAImage& imageOut );

/*!
 * @return true on error
 */
bool uncompressIMY(const PG::UTIL::File& fileIn, PG::UTIL::RGBAImage& imageOut );

/*!
 * @return true on error
 */
bool uncompressIMY(const std::string& fileIn, PG::UTIL::RGBAImage& imageOut );

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_IMY_H_ */
