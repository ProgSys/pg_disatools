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
#ifndef INCLUDE_PG_FILES_PG_EXTRACTORBASE_H_
#define INCLUDE_PG_FILES_PG_EXTRACTORBASE_H_

#include <string>
#include <vector>
#include <pg/util/PG_File.h>


namespace PG {
namespace FILE {

struct fileInfo{
	PG::UTIL::File name;
	unsigned int size = 0; //size in byte
	unsigned int offset = 0; //offset from file beginning in byte

	fileInfo();
	fileInfo(const std::string& name,unsigned int size,unsigned int offset);
	fileInfo(const PG::UTIL::File& name,unsigned int size,unsigned int offset);
};

class ExtractorBase {
public:
	ExtractorBase();

	virtual bool open(const PG::UTIL::File& file) = 0;
	virtual bool exists(const PG::UTIL::File& file) const = 0;
	virtual bool extract(const PG::UTIL::File& file, const PG::UTIL::File& targetFile) const = 0;
	virtual unsigned int extract(const PG::UTIL::File& file, char* (&data) ) const = 0;
	virtual bool insert(const PG::UTIL::File& file) = 0;
	virtual bool remove(const PG::UTIL::File& file) = 0;
	virtual bool save() = 0;
	virtual bool save(const PG::UTIL::File& targetfile) = 0;
	virtual void clear() = 0;
	virtual bool isEmpty() const = 0;
	virtual bool isChanged() const;
	virtual unsigned int size() const = 0;

	virtual const fileInfo& get(unsigned int index) const = 0;
	const fileInfo& operator[](unsigned int index) const;
	fileInfo* getDataPointer(unsigned int index) const;

	virtual ~ExtractorBase();
protected:

	bool m_changed = false;
};

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_EXTRACTORBASE_H_ */
