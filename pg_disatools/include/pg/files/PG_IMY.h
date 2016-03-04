/*
 * PG_IMY.h
 *
 *  Created on: 03.03.2016
 *      Author: ProgSys
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
