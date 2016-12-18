/*
 * PG_ANMD2.h
 *
 *  Created on: 01.12.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_PG_FILES_PG_ANMD2_H_
#define INCLUDE_PG_FILES_PG_ANMD2_H_

#include <string>
#include <vector>
#include <pg/util/PG_File.h>
#include <pg/util/PG_ApiUtil.h>
#include <pg/files/PG_ExtractorBase.h>
#include <iostream>

namespace PG {
namespace FILE {

class ANMD2: public ExtractorBase {
public:
	ANMD2();
	ANMD2(const PG::UTIL::File& file);
	ANMD2(const std::string& file);

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

	char const* getType() const;

	virtual ~ANMD2();
private:
	PG::UTIL::File m_file_buffer;
};

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_ANMD2_H_ */
