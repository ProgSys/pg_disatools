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
#include <Util/PG_File.h>
#include <Util/PG_ApiUtil.h>
#include <Util/PG_Exception.h>
#include <Files/PG_ExtractorBase.h>
#include <Files/PG_TX2.h>

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
	EXPORT bool open(const PG::UTIL::File& file, PercentIndicator* percent = nullptr) override;
	EXPORT bool save(const PG::UTIL::File& targetfile, PercentIndicator* percent = nullptr) override;

	EXPORT bool replace(fileInfo& target,const PG::UTIL::File& file, bool keepName = false) override;
	EXPORT bool insert(const PG::UTIL::File& file) override;
	EXPORT bool remove(std::vector<fileInfo>::iterator it) override;
	EXPORT void clear() override;

	EXPORT bool hasModels() const;
	EXPORT bool hasTextures() const;
	EXPORT bool hasNormalMaps() const;
	EXPORT int getNumberOfModels() const;
	EXPORT int getNumberOfTextures() const;
	EXPORT int getNumberOfNormalMaps() const;
	EXPORT bool getTexture(int index, tx2Image& imgOut) const;


	EXPORT char const* getError();

	EXPORT char const* getType() const;

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
