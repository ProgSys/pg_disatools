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
#ifndef INCLUDE_PG_FILES_PG_IMAGEFILES_H_
#define INCLUDE_PG_FILES_PG_IMAGEFILES_H_

#include <string>
#include <vector>
#include <pg/util/PG_Colors.h>

namespace PG {
namespace FILE {

/*!
 * @brief Will create a very simple TGA8888 image.
 */
void saveTGA(const std::string& filepath, unsigned int width, unsigned int height, const std::vector<PG::UTIL::RGBA>& rgba);


/*!
 * @brief Will create a very simple PGM P6 (RGB888 no alpha) image.
 */
void savePGM(const std::string& filepath, unsigned int width, unsigned int height, const std::vector<PG::UTIL::RGBA>& rgba);

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_IMAGEFILES_H_ */
