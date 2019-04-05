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
#ifndef INCLUDE_PG_FILES_PG_SOLA_H_
#define INCLUDE_PG_FILES_PG_SOLA_H_

#include <string>
#include <vector>
#include <Util/PG_File.h>
#include <Files/PG_ExtractorBase.h>
#include <Util/PG_ApiUtil.h>

namespace PG {
namespace FILE {

/*!
 * @brief This archive only holds sprite sheets and is very simmilar to a Offset List Archive,
 * but after the sprte pointers, there is also for each sprite a ID (unsigned short - 2 byte),
 * which acts like a filename.
 * Means you can add new character or effect sprite sheets and reference them in the game scripts.
 */
class SOLA: public ExtractorBase {
public:
	EXPORT SOLA();
	EXPORT SOLA(const PG::UTIL::File& file);

	/*!
	 * @brief Opens the given START.DAT file.
	 * @return true on error
	 */
	EXPORT bool open(const PG::UTIL::File& file, PercentIndicator* percent = nullptr) final;
	EXPORT bool save(const PG::UTIL::File& targetfile, PercentIndicator* percent = nullptr) final;

	EXPORT bool insert(const PG::UTIL::File& file);
	EXPORT bool insert(const PG::UTIL::File& file, unsigned short id);

	EXPORT bool remove(fileInfo_iterator target);

	EXPORT void clear();

	EXPORT char const* getType() const;

	EXPORT virtual ~SOLA();
private:
	std::vector<unsigned short> m_chractersIDs;
};

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_SOLA_H_ */
