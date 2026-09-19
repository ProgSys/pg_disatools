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

#include <Disa/fileformats/PG_DisaFileContainer.h>

 /*!
  * @brief Archive that holds all the scripts run by the game, unlike TALK.DAT it handles
  * map logic and skills rather then dialog. Found at SUBDATA.DAT->START.DAT->SCRIPT.DAT.
  *
  * The header is the number of scripts and a zero, followed by a list of addresses
  * (4 byte each) and a equally long list of IDs (4 byte each), which act like a filename.
  * The addresses are relative to the end of the header, where the script data starts.
  * A script has no stored length, the game simply runs it until a return op code,
  * so the length used here is the distance to the script that follows it in the file.
  */
class PG_Script : public PG_DisaFileContainer {
public:
    using PG_DisaFileContainer::PG_DisaFileContainer;

    using PG_DisaFileContainer::open;
    bool open(std::shared_ptr<PG::STREAM::In> reader, const std::function<void(PG_DisaFileEntry&)>& visitor) final;
    bool save(std::shared_ptr<PG::STREAM::Out> writer) const final;

    PG_Variety createDefaultExtraData(const std::string& filename = {}) final { return PG_Variety(buildScriptId(filename)); }
    const PG_DisaExtraFields& describeExtraData() const final;
    std::string buildEntryName(const PG_DisaFileEntry& entry) const final;

    /*!
     * @brief The script ID an entry is stored under, it is what the game references a script by.
     * @details Taken from the ID list when the entry was read, otherwise rebuilt from the file
     * name, which is how a file the user added gets its ID.
     */
    static unsigned int getScriptId(const PG_DisaFileEntry& entry);
    /*!
     * @brief The file name an entry with the given script ID gets, like '1000001.SCRIPT'.
     */
    static std::string buildScriptName(unsigned int scriptId);
    /*!
     * @brief The script ID a file name describes, the reverse of buildScriptName.
     * @details A name that is no script name at all gives 0.
     */
    static unsigned int buildScriptId(const std::string& filename);
};
