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
#ifndef INCLUDE_PG_FILES_PG_FILETESTS_H_
#define INCLUDE_PG_FILES_PG_FILETESTS_H_

#include <Util/PG_File.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Util/PG_ApiUtil.h>
#include <Files/PG_TX2.h>

namespace PG {
namespace FILE {

EXPORT bool isIMY(const PG::UTIL::File& file);
EXPORT bool isIMY(PG::STREAM::InByteFile& reader);

EXPORT unsigned int isIMYPackage(const PG::UTIL::File& file);
EXPORT unsigned int isIMYPackage(PG::STREAM::InByteFile& reader);

EXPORT bool isSpriteSheetPackage(const PG::UTIL::File& file);
EXPORT bool isSpriteSheetPackage(PG::STREAM::InByteFile& reader);

EXPORT bool isTX2(const PG::UTIL::File& file);
EXPORT bool isTX2(PG::STREAM::InByteFile& reader);

EXPORT bool isNISPACK(const PG::UTIL::File& file);
EXPORT bool isNISPACK(PG::STREAM::InByteFile& reader);

EXPORT bool isDSARC_FL(const PG::UTIL::File& file);
EXPORT bool isDSARC_FL(PG::STREAM::InByteFile& reader);

EXPORT tx2Type getTX2CompressionType(PG::STREAM::InByteFile& reader);
EXPORT std::string asString(tx2Type type);

//TODO
EXPORT bool isSpriteSheet(const PG::UTIL::File& file);
EXPORT bool isSpriteSheet(PG::STREAM::InByteFile& reader);

EXPORT bool isPSPFS(PG::STREAM::InByteFile& reader);
EXPORT bool isPSPFS(const PG::UTIL::File& file);

EXPORT bool isPSFS(PG::STREAM::InByteFile& reader);
EXPORT bool isPSFS(const PG::UTIL::File& file);


EXPORT bool isANMD2(PG::STREAM::InByteFile& reader);
EXPORT bool isANMD2(const PG::UTIL::File& file);

EXPORT bool isScript(PG::STREAM::InByteFile& reader);
EXPORT bool isScript(const PG::UTIL::File& file);

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_FILETESTS_H_ */
