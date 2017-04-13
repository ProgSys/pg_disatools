/*
 * PG_PS3FS.h
 *
 *  Created on: 13.01.2017
 *      Author: ProgSys
 */

#ifndef INCLUDE_PG_FILES_PG_PS3FS_H_
#define INCLUDE_PG_FILES_PG_PS3FS_H_

#include <vector>
#include <pg/util/PG_File.h>
#include <pg/util/PG_ApiUtil.h>
#include <pg/files/PG_ExtractorBase.h>
#include <iostream>

namespace PG {
namespace FILE {

/*!
 * @brief PlayStation 3 file system, use for example in 'Criminal Girls: Invite Only'
 */
class PS3FS: public ExtractorBase  {
public:
	EXPORT PS3FS();
	EXPORT PS3FS(const PG::UTIL::File& file);
	EXPORT PS3FS(const std::string& file);

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

	EXPORT char const* getType() const;

	EXPORT virtual ~PS3FS();

};

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_PS3FS_H_ */
