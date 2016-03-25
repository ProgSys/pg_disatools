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
#include <memory>

namespace PG {
namespace FILE {

class FileInfoExtra{
public:
	FileInfoExtra();

	virtual void clear() = 0;

	virtual ~FileInfoExtra();
};

struct PercentIndicator{
	float percent;
};

struct fileInfo{
	PG::UTIL::File name;
	unsigned int size = 0; //size in byte
	unsigned int offset = 0; //offset from file beginning in byte
	unsigned int decompressedFileSize = 0;

	PG::UTIL::File externalFile;

	bool compressed = false;
	bool package = false;
	bool texture = false;

	fileInfo();
	fileInfo(const std::string& name,unsigned int size,unsigned int offset);
	fileInfo(const PG::UTIL::File& name,unsigned int size,unsigned int offset);
	fileInfo(const fileInfo& info);

	void operator=(const fileInfo& info);

	const PG::UTIL::File& getName() const;
	unsigned int getSize() const;
	unsigned int getOffset() const;
	const PG::UTIL::File& getExternalName() const;
	std::string getFileExtension() const;

	void setName(const PG::UTIL::File& name);
	void setSize(unsigned int size);
	void setOffset(unsigned int offset);
	void setExternalName(const PG::UTIL::File& externalFile);
	void setAsDummy(unsigned int offset = 0);

	bool isExternalFile() const;
	bool isCompressed() const;
	bool isPackage() const;
	bool isTexture() const;
	bool isValid() const;

	void clearExternalFile();
	void clear();
};

class ExtractorBase {
public:
	ExtractorBase();

	virtual bool open(const PG::UTIL::File& file, PercentIndicator* percent = nullptr) = 0;
	bool save(PercentIndicator* percent = nullptr);
	virtual bool save(const PG::UTIL::File& targetfile, PercentIndicator* percent = nullptr) = 0;


	virtual bool insert(const PG::UTIL::File& file);
	bool remove(const PG::UTIL::File& file);
	virtual bool remove(fileInfo& target);


	bool isEmpty() const;
	unsigned int size() const;
	/*!
	 * @returns true if archive is correct, otherwise will return false and a error message.
	 */
	virtual bool checkValid(std::string& errorMessageOut);

	const PG::UTIL::File& getOpendFile() const;

	/*!
	 * @brief Is the given file inside the archive.
	 * @return true if file is inside the archive.
	 */
	virtual bool exists(const PG::UTIL::File& file) const;
	virtual bool isChanged() const;

	virtual unsigned int extract(const fileInfo& target, char* (&data) ) const;
	virtual unsigned int extract(const PG::UTIL::File& file, char* (&data) ) const;

	virtual bool extract(const fileInfo& target, const PG::UTIL::File& targetFile ) const;
	/*!
	 * @brief Extract a file into the given target file.
	 * @return true on error
	 */
	virtual bool extract(const PG::UTIL::File& file, const PG::UTIL::File& targetFile) const;

	virtual bool replace(fileInfo& target,const PG::UTIL::File& file, bool keepName = false);
	bool replace(const PG::UTIL::File& targetfile, const PG::UTIL::File& file, bool keepName = false);


	virtual void clear();

	/*!
	 * @brief Find a file info with the given name.
	 * @param infoOut returns reference file info
	 * @returns true if file info found
	 */
	virtual bool find(const PG::UTIL::File& file, fileInfo& infoOut) const;


	fileInfo& get(unsigned int index);
	const fileInfo& get(unsigned int index) const;
	const fileInfo& operator[](unsigned int index) const;
	fileInfo* getDataPointer(unsigned int index) const;

	void getFileProperties(fileInfo& target) const;

	virtual ~ExtractorBase();
protected:
	PG::UTIL::File m_file;
	std::vector<fileInfo> m_fileInfos;

	bool m_changed = false;
	unsigned int m_originalFileSize = 0;

};

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_EXTRACTORBASE_H_ */
