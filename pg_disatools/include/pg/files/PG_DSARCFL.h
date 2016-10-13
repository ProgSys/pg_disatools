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
#ifndef SRC_PG_FILES_PG_DSARCFL_H_
#define SRC_PG_FILES_PG_DSARCFL_H_

#include <string>
#include <vector>
#include <pg/util/PG_File.h>
#include <pg/util/PG_ApiUtil.h>
#include <pg/files/PG_ExtractorBase.h>
#include <iostream>

namespace PG {
namespace FILE {

class DSARC_FL: public ExtractorBase {
public:
	DSARC_FL();
	DSARC_FL(const PG::UTIL::File& file);
	DSARC_FL(const std::string& file);
	virtual ~DSARC_FL();

	/*!
	 * @brief Opens the given PSPFS file.
	 * @return true on error
	 */
	EXPORT bool open(const PG::UTIL::File& file, PercentIndicator* percent = nullptr);
	/*!
	 * @brief Saves the changes done with insert or remove.
	 * @return true on error
	 */
	EXPORT bool save(const PG::UTIL::File& targetfile, PercentIndicator* percent = nullptr);
	/*!
	 * @brief Add a file into the archive. Changes will only be applied when you save.
	 * @return true on error
	 * @see save()
	 */
	//EXPORT bool insert(const PG::UTIL::File& file);
	/*!
	 * @brief Remove a file from the archive. Changes will only be applied when you save.
	 * @return true on error
	 * @see save()
	 */
	//EXPORT bool remove(fileInfo& target);


	//EXPORT void clear();

	EXPORT char const* getType() const;
private:
	PG::UTIL::File m_file_buffer;
};

} /* namespace FILE */
} /* namespace PG */

#endif /* SRC_PG_FILES_PG_DSARCFL_H_ */
