/*
 * Functions to save rgba data as different files.
 *
 *  Copyright (C) 2016  ProgSys
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef INCLUDE_PG_FILES_PG_IMAGEFILES_H_
#define INCLUDE_PG_FILES_PG_IMAGEFILES_H_

#include <string>
#include <vector>
#include <pg/util/PG_Vector.h>

namespace PG {
namespace FILE {

/*!
 * @brief Will create a very simple TGA8888 image.
 */
void saveTGA(const std::string& filepath, unsigned int width, unsigned int height, const std::vector<PG::UTIL::rgba>& rgba);


/*!
 * @brief Will create a very simple PGM P6 (RGB888 no alpha) image.
 */
void savePGM(const std::string& filepath, unsigned int width, unsigned int height, const std::vector<PG::UTIL::rgba>& rgba);

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_IMAGEFILES_H_ */
