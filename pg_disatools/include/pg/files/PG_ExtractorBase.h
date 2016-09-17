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
#include <stack>

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

/*!
 * @brief Basic info about a file, like its name, type, size and offset in the archive.
 * It also can be a external file, means it references a outside file witch needs to be added onto the archive.
 */
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
	EXPORT void setAsDummy(unsigned int offset = 0); // only used by PSPVS_V1 for deleting files

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

/*!
 * @brief This base class provides archive file managing functionality, like insert, delete or replace files.
 * Your  class witch inherits this base needs mainly to implement open and save.
 */
class ExtractorBase {
public:
	EXPORT ExtractorBase();

	/*!
	 * @brief Opens the given file and creates a file info for every file inside the archive.
	 *
	 * @param file target file to open
	 * @param percent used for multithreading so the progress can be tracked
	 * @return false on successes
	 * @see fileInfo
	 */
	EXPORT virtual bool open(const PG::UTIL::File& file, PercentIndicator* percent = nullptr) = 0;
	/*!
	 * @brief Saves the changes done to the archive, overwriting the original file.
	 * isChanged() should return false after this operation.
	 *
	 * @param percent used for multithreading so the progress can be tracked
	 * @return false on successes
	 */
	EXPORT bool save(PercentIndicator* percent = nullptr);
	/*!
	 * @brief Saves the changes done to the archive into a different file keeping the original file as it is.
	 * isChanged() should still return true as no changes should be done to the original file.
	 * If the param 'targetfile' is the original file then it should function as if the save function was called, overwriting the original file.
	 *
	 * @param targetfile target save file
	 * @param percent used for multithreading so the progress can be tracked
	 * @return false on successes
	 * @see save
	 */
	EXPORT virtual bool save(const PG::UTIL::File& targetfile, PercentIndicator* percent = nullptr) = 0;
	/*!
	 * @brief Is the given file inside the archive?
	 * @return true if file is inside the archive.
	 */
	EXPORT virtual bool exists(const PG::UTIL::File& file) const;
	/*!
	 * @brief Find a file info with the given name.
	 * @param infoOut returns reference file info
	 * @returns true if file info found
	 */
	EXPORT virtual bool find(const PG::UTIL::File& file, fileInfo& infoOut) const;
	/*!
	 * @brief Extract a file from the active into the given char array pointer.
	 * A new char pointer will the created.
	 * @param target vector address of the file to be extracted
	 * @param data a reference to a char pointer should be nullptr as it will be overwritten with a new one and the old one will not be freed
	 * @return the size of the data in byte
	 */
	EXPORT virtual unsigned int extract(const fileInfo& target, char* (&data) ) const;
	/*!
	 * @brief Extract a file from the active into the given char array pointer.
	 * A new char pointer will the created.
	 * @param target file to be extracted
	 * @param data a reference to a char pointer should be nullptr as it will be overwritten with a new one and the old one will not be freed
	 * @return the size of the data in byte
	 */
	EXPORT virtual unsigned int extract(const PG::UTIL::File& file, char* (&data) ) const;
	/*!
	 * @brief Extract a file from the active into a new file.
	 *
	 * @param target vector address of the file to be extracted
	 * @param targetFile target extraction file
	 * @return false on successes
	 */
	EXPORT virtual bool extract(const fileInfo& target, const PG::UTIL::File& targetFile ) const;
	/*!
	 * @brief Extract a file from the active into a new file.
	 *
	 * @param file to be extracted
	 * @param targetFile target extraction file
	 * @return false on successes
	 */
	EXPORT virtual bool extract(const PG::UTIL::File& file, const PG::UTIL::File& targetFile) const;
	/*!
	 * @brief Replace a file with another one, keeping the original file order.
	 * Functions similar as insert but will not push back a file if target not found.
	 * @param keepName should the old name be kept
	 * @return false on successes
	 * @see insert
	 */
	EXPORT virtual bool replace(fileInfo& target,const PG::UTIL::File& file, bool keepName = false);
	/*!
	 * @brief Replace a file with another one, keeping the original file order.
	 * Functions similar as insert but will not push back a file if target not found.
	 * @param keepName should the old name be kept
	 * @return false on successes
	 * @see insert
	 */
	EXPORT bool replace(const PG::UTIL::File& targetfile, const PG::UTIL::File& file, bool keepName = false);
	/*!
	 * @brief Insert a file into the archive, if a file with the same name is already inside then replace it.
	 *
	 * @param file target to insert into the archive
	 * @return false on successes
	 */
	EXPORT virtual bool insert(const PG::UTIL::File& file);
	/*!
	 * @brief Remove a file from the archive.
	 *
	 * @param file target to be removed, will be mainly removed by name
	 * @return false on successes, if file has bean removed
	 */
	EXPORT bool remove(const PG::UTIL::File& file);
	/*!
	 * @brief Remove a file from the archive.
	 *
	 * @param target memory address of the target
	 * @return false on successes, if file has bean removed
	 */
	EXPORT virtual bool remove(fileInfo& target);
	/*!
	 * @brief clear everything as if no archive is open
	 */
	EXPORT virtual void clear();
	/*!
	 * @brief Are there any files inside the archive?
	 * @returns true if no files are inside the archive
	 */
	EXPORT bool isEmpty() const;
	/*!
	 * @brief Has bean the opened archive changed in any way.
	 * If true then the file can be saved.
	 * @returns true if archive has bean changed
	 * @see save()
	 */
	EXPORT virtual bool isChanged() const;
	/*!
	 * @returns The number of files inside the archive
	 */
	EXPORT unsigned int size() const;
	/*!
	 * @brief If a error happens in any other operation then it will stack a error message.
	 * You can get the error messages one by one using this function.
	 * It will also return false if the archive is invalid.
	 *
	 * @returns string is empty, if archive is valid and no error messages are in the stack
	 */
	EXPORT virtual char const* getError();
	/*!
	 * @brief Get the current opned archive.
	 * @returns File of the current opned archive.
	 */
	EXPORT const PG::UTIL::File& getOpendFile() const;


	EXPORT virtual char const* getType() const = 0;

	EXPORT fileInfo& get(unsigned int index);
	EXPORT const fileInfo& get(unsigned int index) const;
	EXPORT const fileInfo& operator[](unsigned int index) const;
	EXPORT fileInfo* getDataPointer(unsigned int index) const;
	/*!
	 * @brief Gathers informations about the target file.
	 */
	EXPORT void getFileProperties(fileProperties& target) const;

	EXPORT virtual ~ExtractorBase();
protected:
	PG::UTIL::File m_file;
	std::vector<fileInfo> m_fileInfos;

	bool m_changed = false;
	unsigned int m_originalFileSize = 0;

	std::stack<std::string> m_errors;

	void pushError(const std::string& error);
};

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_EXTRACTORBASE_H_ */
