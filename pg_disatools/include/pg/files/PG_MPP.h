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
#ifndef INCLUDE_PG_FILES_PG_MPP_H_
#define INCLUDE_PG_FILES_PG_MPP_H_

#include <string>
#include <vector>
#include <pg/util/PG_File.h>
#include <pg/util/PG_ApiUtil.h>
#include <pg/util/PG_Exception.h>
#include <pg/files/PG_ExtractorBase.h>

namespace PG {
namespace FILE {

class MPP: public ExtractorBase  {
public:
	EXPORT MPP();
	EXPORT MPP(const PG::UTIL::File& file);

	/*!
	 * @brief Opens the given MPP file.
	 * @return true on error
	 */
	EXPORT bool open(const PG::UTIL::File& file, PercentIndicator* percent = nullptr);
	EXPORT bool save(const PG::UTIL::File& targetfile, PercentIndicator* percent = nullptr);

	EXPORT bool replace(fileInfo& target,const PG::UTIL::File& file, bool keepName = false);
	EXPORT bool insert(const PG::UTIL::File& file);
	EXPORT bool remove(fileInfo& target);
	EXPORT void clear();

	EXPORT std::string getError();

	EXPORT std::string getType() const;

	EXPORT virtual ~MPP();
private:
	bool m_hasNormals = false; // remastered maps with better textures
	std::vector<fileInfo>::iterator m_texturesEnd;
	std::vector<fileInfo>::iterator m_normalsEnd;

	bool findFileInfo(const PG::UTIL::File& file, fileInfo& infoOut) const;
};

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_MPP_H_ */
