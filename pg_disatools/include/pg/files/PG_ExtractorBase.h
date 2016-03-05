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

	PG::UTIL::File externalFile;

	fileInfo();
	fileInfo(const std::string& name,unsigned int size,unsigned int offset);
	fileInfo(const PG::UTIL::File& name,unsigned int size,unsigned int offset);

	const PG::UTIL::File& getName() const;
	unsigned int getSize() const;
	unsigned int getOffset() const;
	const PG::UTIL::File& getExternalName() const;
	std::string getFileExtension() const;

	void setName(const PG::UTIL::File& name);
	void setSize(unsigned int size);
	void setOffset(unsigned int offset);
	void setExternalName(const PG::UTIL::File& externalFile);

	bool isExternalFile() const;
	void clearExternalFile();
	void clear();
};

class ExtractorBase {
public:
	ExtractorBase();

	virtual bool open(const PG::UTIL::File& file) = 0;
	/*!
	 * @brief Is the given file inside the archive.
	 * @return true if file is inside the archive.
	 */
	virtual bool exists(const PG::UTIL::File& file) const;
	/*!
	 * @brief Extract a file into the given target file.
	 * @return true on error
	 */
	virtual bool extract(const PG::UTIL::File& file, const PG::UTIL::File& targetFile) const;
	virtual unsigned int extract(const PG::UTIL::File& file, char* (&data) ) const;
	virtual bool insert(const PG::UTIL::File& file) = 0;
	virtual bool remove(const PG::UTIL::File& file) = 0;
	virtual bool save() = 0;
	virtual bool save(const PG::UTIL::File& targetfile) = 0;
	virtual void clear() = 0;
	virtual bool isEmpty() const = 0;
	virtual bool isChanged() const;
	virtual const PG::UTIL::File& getOpendFile() const = 0;
	virtual unsigned int size() const = 0;

	virtual const fileInfo& get(unsigned int index) const = 0;
	/*!
	 * @brief Find a file info with the given name.
	 * @returns true if file info found
	 */
	virtual bool find(const PG::UTIL::File& file, fileInfo& infoOut) const = 0;
	const fileInfo& operator[](unsigned int index) const;
	fileInfo* getDataPointer(unsigned int index) const;

	virtual ~ExtractorBase();
protected:

	bool m_changed = false;


};

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_EXTRACTORBASE_H_ */
