/*
 * The MIT License (MIT)
 *
 *	Copyright (c) 2025 ProgSys
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
#pragma once

#include <array>
#include <cstdint>
#include <string>

#include <Util/Typedefs.h>
#include <Disa/dataStructs/PG_DisaTalkDatStructs.h>
#include <Disa/fileformats/PG_DisaFileContainer.h>

/*!
 * @brief Archive that holds all the talk scripts, the linear scripting language the game uses
 * for conversations and menus. Found at SUBDATA.DAT->START.DAT->TALK.DAT.
 *
 * The header is the number of scripts twice, followed by a PG_DisaTalkDatScriptEntry per script.
 * The start addresses are relative to the end of the header, where the script data starts.
 * A script stores no length, the game simply runs it until a return op code, so the length
 * used here is the distance to the script that follows it in the file.
 */
class PG_TalkScript : public PG_DisaFileContainer {
public:
	using PG_DisaFileContainer::PG_DisaFileContainer;

	using PG_DisaFileContainer::open;
	bool open(std::shared_ptr<PG::STREAM::In> reader, const std::function<void(PG_DisaFileEntry&)>& visitor) final;
	bool save(std::shared_ptr<PG::STREAM::Out> writer) const final;

	PG_Variety createDefaultExtraData(const std::string& filename = {}) final { return PG_Variety(buildScriptInfo(filename)); }

	/*!
	 * @brief The file table entry a entry is stored under.
	 * @details Taken from the file table when the entry was read, otherwise rebuilt from the
	 * file name, which is how a file the user added gets its search values.
	 */
	static PG_DisaTalkDatScriptEntry getScriptInfo(const PG_DisaFileEntry& entry);
	/*!
	 * @brief The file name a entry with the given file table entry gets.
	 * @details A script with a ID is named after it, as the ID is unique. One without is only
	 * identified by what the game searches it with, so the name lists those instead, like
	 * '0012_102+2200,2201-2300.TSCRIPT'. Only filters that are actually present are listed.
	 */
	static std::string buildScriptName(const PG_DisaTalkDatScriptEntry& info);
	/*!
	 * @brief The file table entry a file name describes, the reverse of buildScriptName.
	 * @details Everything the name doesn't mention stays zero, a name that is no talk script
	 * name at all gives a all zero header.
	 */
	static PG_DisaTalkDatScriptEntry buildScriptInfo(const std::string& filename);
};
