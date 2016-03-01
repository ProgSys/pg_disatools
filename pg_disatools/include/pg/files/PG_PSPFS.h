/*
 * PG_PSPFS.h
 *
 *  Created on: 01.03.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_PG_FILES_PG_PSPFS_H_
#define INCLUDE_PG_FILES_PG_PSPFS_H_

#include <string>
#include <vector>
#include <pg/util/PG_File.h>
#include <pg/util/PG_ApiUtil.h>

namespace PG {
namespace FILE {

struct filePSPFSInfo{
	std::string name;

	unsigned int unknown = 0; //TODO used for dat files witch are inside SUBDATA.DAT

	unsigned int size = 0; //size in byte
	unsigned int offset = 0; //offset from file beginning in byte

	//extra info
	PG::UTIL::File externalFile;

	bool isExternalFile() const{
		return !externalFile.isEmpty();
	}
};

class PSPFS {
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
	PG_UTIL_API bool extract(const PG::UTIL::File& file, const PG::UTIL::File& targetFile);

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

	std::vector<filePSPFSInfo> const& getFileInfos() const{
		return m_filePSPFSInfos;
	}

	PG_UTIL_API virtual ~PSPFS();

private:
	PG::UTIL::File m_file;
	PG::UTIL::File m_file_buffer;

	std::vector<filePSPFSInfo> m_filePSPFSInfos;

	bool findfilePSPFSInfo(const PG::UTIL::File& file, filePSPFSInfo& infoOut) const;

};

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_PSPFS_H_ */
