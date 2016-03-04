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

#ifndef INCLUDE_PG_FILES_PG_PSPFS_H_
#define INCLUDE_PG_FILES_PG_PSPFS_H_

#include <string>
#include <vector>
#include <pg/util/PG_File.h>
#include <pg/util/PG_ApiUtil.h>
#include <pg/files/PG_ExtractorBase.h>
#include <iostream>

namespace PG {
namespace FILE {

struct filePSPFSInfo{

	fileInfo baseInfo;
	unsigned int unknown = 0; //TODO used for dat files witch are inside SUBDATA.DAT


	PG::UTIL::File externalFile; //extra info

	filePSPFSInfo();

	filePSPFSInfo(const filePSPFSInfo& info);

	const PG::UTIL::File& getName() const{
		return baseInfo.name;
	}
	unsigned int getSize() const{
		return baseInfo.size;
	}
	unsigned int getOffset() const{
		return baseInfo.offset;
	}
	const PG::UTIL::File& getExternalFile() const{
		return externalFile;
	}

	void setName(const PG::UTIL::File& file){
		baseInfo.name = file;
	}
	void setNameUnclean(std::string file);

	void setSize(unsigned int size){
		baseInfo.size = size;
	}
	void setOffset(unsigned int offset){
		baseInfo.offset = offset;
	}

	void setAsDummy(unsigned int offset = 0);


	bool isExternalFile() const;

	std::string getFileExtention() const;

    bool operator < (const filePSPFSInfo& str) const;
};

class PSPFS: public ExtractorBase {
public:
	PG_UTIL_API PSPFS();
	PG_UTIL_API PSPFS(const PG::UTIL::File& file);
	PG_UTIL_API PSPFS(const std::string& file);

	/*!
	 * @brief Opens the given PSPFS file.
	 * @return true on error
	 */
	PG_UTIL_API bool open(const PG::UTIL::File& file);

	/*!
	 * @brief Is the given file inside the archive.
	 * @return true if file is inside the archive.
	 */
	PG_UTIL_API bool exists(const PG::UTIL::File& file) const;

	/*!
	 * @brief Extract a file into the given target file.
	 * @return true on error
	 */
	PG_UTIL_API bool extract(const PG::UTIL::File& file, const PG::UTIL::File& targetFile) const;

	PG_UTIL_API unsigned int extract(const PG::UTIL::File& file, char* (&data) ) const;
	/*!
	 * @brief Add a file into the archive. Changes will only be applied when you save.
	 * @return true on error
	 * @see save()
	 */
	PG_UTIL_API bool insert(const PG::UTIL::File& file);
	/*!
	 * @brief Remove a file from the archive. Changes will only be applied when you save.
	 * @return true on error
	 * @see save()
	 */
	PG_UTIL_API bool remove(const PG::UTIL::File& file);

	/*!
	 * @brief Saves the changes done with insert or remove.
	 * @return true on error
	 */
	PG_UTIL_API bool save();
	PG_UTIL_API bool save(const PG::UTIL::File& targetfile);

	PG_UTIL_API void clear();


	PG_UTIL_API unsigned int size() const;

	PG_UTIL_API PG::UTIL::File getOpendFile() const;

	PG_UTIL_API bool isEmpty() const;

	PG_UTIL_API const fileInfo& get(unsigned int index) const;

	PG_UTIL_API virtual ~PSPFS();

private:
	PG::UTIL::File m_file;
	PG::UTIL::File m_file_buffer;

	std::vector<filePSPFSInfo> m_filePSPFSInfos;
	unsigned int m_originalFileSize = 0;

	bool findfilePSPFSInfo(const PG::UTIL::File& file, filePSPFSInfo& infoOut) const;

};

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_PSPFS_H_ */
