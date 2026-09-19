/*
 * The MIT License (MIT)
 *
 *	Copyright (c) 2024 ProgSys
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

#include <map>
#include <string>

#include <Disa/fileformats/PG_DisaFileContainer.h>

 /*!
  * @brief This archive only holds sprite sheets and is very simmilar to a Offset List Archive,
  * but after the sprte pointers, there is also for each sprite a ID (unsigned short - 2 byte),
  * which acts like a filename.
  * Means you can add new character or effect sprite sheets and reference them in the game scripts.
  */
class PG_SOLA : public PG_DisaFileContainer {
public:
    using PG_DisaFileContainer::PG_DisaFileContainer;

    using PG_DisaFileContainer::open;
    bool open(std::shared_ptr<PG::STREAM::In> reader, const std::function<void(PG_DisaFileEntry&)>& visitor) final;
    bool save(std::shared_ptr<PG::STREAM::Out> writer) const final;

    PG_Variety createDefaultExtraData(const std::string& filename = {}) final { return PG_Variety(buildSpriteId(filename)); }
    const PG_DisaExtraFields& describeExtraData() const final;
    std::string buildEntryName(const PG_DisaFileEntry& entry) const final;

    /*!
     * @brief The id an entry is stored under, it is what the game references a sprite sheet by.
     * @details Taken from the id list when the entry was read, otherwise rebuilt from the file
     * name, which is how a file the user added gets its id.
     */
    static unsigned short getSpriteId(const PG_DisaFileEntry& entry);
    /*!
     * @brief The id a file name describes, the number the name starts with.
     * @details A name that starts with no number at all gives 0.
     */
    static unsigned short buildSpriteId(const std::string& filename);
private:
    /*!
     * @brief The file name an entry with the given id gets, like '0012_ETNA.SH'.
     * @details The readable part comes from this archive's name table, and falls back to 'FILE'
     * plus the extension every entry of this archive shares.
     */
    std::string buildName(unsigned short spriteId) const;

    std::map<unsigned short, std::string> m_nameTable; //what the CSV beside the archive calls each id
    std::string m_fileExtension; //every entry of a SOLA holds the same file type
};

