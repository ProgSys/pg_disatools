/*
 * PG_Script.h
 *
 *  Created on: 01.06.2025
 *      Author: ProgSys
 */

#pragma once

#include <string>
#include <vector>
#include <Util/PG_File.h>
#include <Util/PG_ApiUtil.h>
#include <Files/PG_ExtractorBase.h>
#include <iostream>

namespace PG {
namespace FILE {

class Script : public ExtractorBase {
public:
	Script();
	Script(const PG::UTIL::File& file);
	Script(const std::string& file);

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

	virtual ~Script();
private:
	PG::UTIL::File m_file_buffer;
};

} /* namespace FILE */
} /* namespace PG */
