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
#pragma once

#include <string>
#include <Util/PG_File.h>
#include <Stream/PG_StreamIn.h>
#include <Stream/PG_StreamInOut.h>
#include <Util/PG_File.h>
#include <functional>

namespace PG_Disa {

	/*!
	 * @brief Uncompress a single IMY file.
	 * <br>
	 * Krisan Thyme provided the original source code!
	 * <b>Thank you!</b>
	 * @return true on error
	 */
	bool decompressIMY(PG::STREAM::In* instream, PG::STREAM::InOut* outstream);

	/*!
	 * @brief Uncompress a single IMY file.
	 * @return true on error
	 */
	bool decompressIMY(const char* bytesIn, unsigned int lenghtIn, const PG_File& fileOut);

	/*!
	 * @brief Uncompress a single IMY file.
	 * @return true on error
	 */
	bool decompressIMY(const PG_File& fileIn, const PG_File& fileOut);

	/*!
	 * @brief Uncompress a single IMY file.
	 * @return true on error
	 */
	bool decompressIMY(const std::string& fileIn, const std::string& fileOut);


	/*!
	 * @brief Uncompress a IMY package. One big file is usually compressed into several IMY and saved as a package.
	 * @return true on error
	 */
	bool decompressIMYPackage(PG::STREAM::In* instream, PG::STREAM::InOut* outstream, const std::function<void(float)>& progress = {});

	/*!
	 * @brief Uncompress a IMY package. One big file is usually compressed into several IMY and saved as a package.
	 * @return true on error
	 */
	bool decompressIMYPackage(const PG_File& fileIn, const PG_File& fileOut);

	/*!
	 * @brief Uncompress a IMY package. One big file is usually compressed into several IMY and saved as a package.
	 * @return true on error
	 */
	bool decompressIMYPackage(const std::string& fileIn, const std::string& fileOut);

	/*!
	 * @brief Uncompress a IMY package. One big file is usually compressed into several IMY and saved as a package.
	 * @return true on error
	 */
	bool decompressIMYPackage(const char* bytesIn, unsigned int lenghtIn, const PG_File& fileOut);

} /* namespace PG_Disa */


