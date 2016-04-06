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

#include <pg/util/PG_PercentIndicator.h>
#include <pg/util/PG_ApiUtil.h>

namespace PG {
namespace FILE {

class FileInfoExtra{
public:
	EXPORT FileInfoExtra();

	EXPORT virtual void clear() = 0;

	EXPORT virtual ~FileInfoExtra();
};

struct fileInfo{
	enum type{
		PSPFS_V1,
		OLA,  /// Offset list archive
		COLA, /// Compressed offset list archive
		SOLA, /// Sprite sheets offset list archive (it also has a list of IDs)
		TX2,
		IMY,
		MPP,
		SH,   /// Sprite sheet
		GEO,
		UNKNOWN
	} fileType = UNKNOWN;

	PG::UTIL::File name;
	unsigned int size = 0; //size in byte
	unsigned int offset = 0; //offset from file beginning in byte
	unsigned int decompressedFileSize = 0;

	PG::UTIL::File externalFile;

	EXPORT fileInfo();
	EXPORT fileInfo(const std::string& name,unsigned int size,unsigned int offset);
	EXPORT fileInfo(const PG::UTIL::File& name,unsigned int size,unsigned int offset);
	EXPORT fileInfo(const fileInfo& info);

	EXPORT void operator=(const fileInfo& info);

	EXPORT const PG::UTIL::File& getName() const;
	EXPORT unsigned int getSize() const;
	EXPORT unsigned int getOffset() const;
	EXPORT const PG::UTIL::File& getExternalName() const;
	EXPORT std::string getFileExtension() const;

	EXPORT void setName(const PG::UTIL::File& name);
	EXPORT void setSize(unsigned int size);
	EXPORT void setOffset(unsigned int offset);
	EXPORT void setExternalName(const PG::UTIL::File& externalFile);
	EXPORT void setAsDummy(unsigned int offset = 0);

	EXPORT bool isExternalFile() const;
	EXPORT bool isCompressed() const;
	EXPORT bool isPackage() const;
	EXPORT bool isTexture() const;
	EXPORT bool isValid() const;

	EXPORT void clearExternalFile();
	EXPORT void clear();
};

struct fileProperties{
	fileProperties(fileInfo& fInfo): info(fInfo){}
	fileInfo& info;
	std::string textureCompression;
	unsigned short characterID = 0;
};

class ExtractorBase {
public:
	EXPORT ExtractorBase();

	EXPORT virtual bool open(const PG::UTIL::File& file, PercentIndicator* percent = nullptr) = 0;
	EXPORT bool save(PercentIndicator* percent = nullptr);
	EXPORT virtual bool save(const PG::UTIL::File& targetfile, PercentIndicator* percent = nullptr) = 0;


	EXPORT virtual bool insert(const PG::UTIL::File& file);
	EXPORT bool remove(const PG::UTIL::File& file);
	EXPORT virtual bool remove(fileInfo& target);


	EXPORT bool isEmpty() const;
	EXPORT unsigned int size() const;
	/*!
	 * @returns true if archive is correct, otherwise will return false and a error message.
	 */
	EXPORT virtual bool checkValid(std::string& errorMessageOut);

	EXPORT const PG::UTIL::File& getOpendFile() const;

	/*!
	 * @brief Is the given file inside the archive.
	 * @return true if file is inside the archive.
	 */
	EXPORT virtual bool exists(const PG::UTIL::File& file) const;
	EXPORT virtual bool isChanged() const;

	EXPORT virtual unsigned int extract(const fileInfo& target, char* (&data) ) const;
	EXPORT virtual unsigned int extract(const PG::UTIL::File& file, char* (&data) ) const;

	EXPORT virtual bool extract(const fileInfo& target, const PG::UTIL::File& targetFile ) const;
	/*!
	 * @brief Extract a file into the given target file.
	 * @return true on error
	 */
	EXPORT virtual bool extract(const PG::UTIL::File& file, const PG::UTIL::File& targetFile) const;

	EXPORT virtual bool replace(fileInfo& target,const PG::UTIL::File& file, bool keepName = false);
	EXPORT bool replace(const PG::UTIL::File& targetfile, const PG::UTIL::File& file, bool keepName = false);


	EXPORT virtual void clear();

	/*!
	 * @brief Find a file info with the given name.
	 * @param infoOut returns reference file info
	 * @returns true if file info found
	 */
	EXPORT virtual bool find(const PG::UTIL::File& file, fileInfo& infoOut) const;


	EXPORT fileInfo& get(unsigned int index);
	EXPORT const fileInfo& get(unsigned int index) const;
	EXPORT const fileInfo& operator[](unsigned int index) const;
	EXPORT fileInfo* getDataPointer(unsigned int index) const;

	EXPORT void getFileProperties(fileProperties& target) const;

	EXPORT virtual ~ExtractorBase();
protected:
	PG::UTIL::File m_file;
	std::vector<fileInfo> m_fileInfos;

	bool m_changed = false;
	unsigned int m_originalFileSize = 0;

	std::string m_lastError;
};

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_EXTRACTORBASE_H_ */
