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
#include <pg/stream/PG_StreamIn.h>
#include <pg/stream/PG_StreamInOut.h>
#include <pg/util/PG_PercentIndicator.h>

namespace PG {
namespace FILE {

/*!
 * @brief Uncompress a single IMY file.
 * <br>
 * Krisan Thyme provided the original source code!
 * <b>Thank you!</b>
 * @return true on error
 */
bool decompressIMY(PG::STREAM::In* instream, PG::STREAM::InOut* outstream, PercentIndicator* percent = nullptr );

/*!
 * @brief Uncompress a single IMY file.
 * @return true on error
 */
bool decompressIMY(const char* bytesIn, unsigned int lenghtIn, const PG::UTIL::File& fileOut, PercentIndicator* percent = nullptr );

/*!
 * @brief Uncompress a single IMY file.
 * @return true on error
 */
bool decompressIMY(const PG::UTIL::File& fileIn, const PG::UTIL::File& fileOut, PercentIndicator* percent = nullptr );

/*!
 * @brief Uncompress a single IMY file.
 * @return true on error
 */
bool decompressIMY(const std::string& fileIn, const std::string& fileOut, PercentIndicator* percent = nullptr );


/*!
 * @brief Uncompress a IMY package. One big file is usually compressed into several IMY and saved as a package.
 * @return true on error
 */
bool decompressIMYPackage(PG::STREAM::In* instream, PG::STREAM::InOut* outstream, PercentIndicator* percent = nullptr );

/*!
 * @brief Uncompress a IMY package. One big file is usually compressed into several IMY and saved as a package.
 * @return true on error
 */
bool decompressIMYPackage(const PG::UTIL::File& fileIn, const PG::UTIL::File& fileOut, PercentIndicator* percent = nullptr );

/*!
 * @brief Uncompress a IMY package. One big file is usually compressed into several IMY and saved as a package.
 * @return true on error
 */
bool decompressIMYPackage(const std::string& fileIn, const std::string& fileOut, PercentIndicator* percent = nullptr );

/*!
 * @brief Uncompress a IMY package. One big file is usually compressed into several IMY and saved as a package.
 * @return true on error
 */
bool decompressIMYPackage(const char* bytesIn, unsigned int lenghtIn, const PG::UTIL::File& fileOut, PercentIndicator* percent = nullptr );

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_IMY_H_ */
