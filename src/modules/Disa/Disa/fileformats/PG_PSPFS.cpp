/*
 * The MIT License (MIT)
 *
 *	Copyright (c) 2020 ProgSys
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

#include "PG_PSPFS.h"
#include <Util/Debug/PG_DebugOut.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamOutByteFile.h>
#include <Stream/PG_StreamInOutByteFile.h>
#include <Disa/resources/PG_DisaGameResourceSource.h>


bool PG_PSPFS::open(std::shared_ptr<PG::STREAM::In> reader, const std::function<void(PG_DisaFileEntry&)>& visitor) {
    if (reader->size() <= 0) return FAILURE;
   
    if (reader->readString(8) != "PSPFS_V1") {
        FILE_CONTAINER_ON_ERROR("Unexpected magic number, expected 'PSPFS_V1'!");
    }

    const unsigned int number_of_files = reader->readUnsignedInt();
    FILE_CONTAINER_ERROR_TEST(number_of_files > 50000, "Too many files!" );

    reader->readUnsignedInt(); //Unknown
    std::vector<PG_DisaFileEntry> contains;
    contains.reserve(number_of_files);

    struct FileEntryHeaderInfo {
        std::array<char, 40> name;
        unsigned int decompressedSize;
        unsigned int size;
        unsigned int offset;
    };
    std::vector<FileEntryHeaderInfo> fileEntries;
    reader->read(fileEntries, number_of_files);

    //read the file infos
    for (const FileEntryHeaderInfo& fileEntryData: fileEntries) {
        PG_DisaFileEntry entry = PG_DisaFileEntry(this);
        PG_DisaFileInfo info;
        info.setName(std::string(fileEntryData.name.begin(), std::ranges::find(fileEntryData.name, char(0x00))));
        info.setDecompressedSize(fileEntryData.decompressedSize);
        info.setSize(fileEntryData.size);
        info.setOffset(fileEntryData.offset);
        info.determinTypeFromFileExtention();
        entry.setInfo(std::move(info));
        if(visitor) visitor(entry);

        contains.emplace_back(std::move(entry));
    }

    m_originalReader = reader;
    const unsigned int currentPos = reader->pos();
    for (PG_DisaFileEntry& entry : contains) {
        if (entry.getContainer()) continue;
        if (entry.getInfo().getType() != PG_DisaFileInfo::UNKNOWN) continue;
        auto entryReader = entry.createReader();
        entryReader->rewind();
        entry.getInfo().determinTypeFromFileFormat(entryReader.get());
    }
    reader->seek(currentPos);
    m_originalReader.reset();

    set(std::move(contains));

    return SUCCESS;
}

static inline void convertToDUMMY(PG_DisaFileInfo& item, std::vector<char>& data) {

    //set header data
    item.setName(PG_File("DUMMY.DAT"));
    item.setDecompressedSize(0);
    item.setSize(5); //the file is small, the buffer is big

    //write dummy data
    data.resize(512, 0);
    data[0] = 'D';
    data[1] = 'U';
    data[2] = 'M';
    data[3] = 'M';
    data[4] = 'Y';
}

static inline void fillDUMMY(std::vector<char>& data) {
    //write dummy data
    data.resize(512, 0);
    data[0] = 'D';
    data[1] = 'U';
    data[2] = 'M';
    data[3] = 'M';
    data[4] = 'Y';
}

bool PG_PSPFS::save(std::shared_ptr<PG::STREAM::Out> writer) const {
    if (!writer || empty()) return FAILURE;

    writer->writeString("PSPFS_V1");
    writer->writeInt(size());
    writer->writeInt(0);// mystery int

    m_originalReader.reset();
    m_originalReader = createReaderOriginal();


    std::vector<std::pair<unsigned int, unsigned int>> saveOrder;//try to mentain previous order, as game uses hard codded stuff
    saveOrder.reserve(size());
    int i = 0;
    for (const PG_DisaFileEntry& entry : *this) {
        saveOrder.emplace_back(i++, entry.getInfo().getOffset());
    }

    std::ranges::sort(saveOrder, {}, &std::pair<unsigned int, unsigned int>::second);

    const unsigned int startFileTable = writer->pos(); //get the first header position
    std::vector<FileTableEntry> fileTable(size());
    //fill empty header (will be overwritten later)
    writer->writeData(fileTable);


    std::vector<char> data;
    PG_DisaFileInfo item;

    for (auto [index, oldMem]: saveOrder){
        data.clear();
        const auto& entry = get(index);
        item = get(index).getInfo();
        auto reader = entry.createReader();
        if (!reader) OUT_WARNING("Convert failed! Writing dummy file instead!");
        else if (item.getName() != "DUMMY.DAT") {
            item.setSize(reader->size());
            reader->read(data, item.getSize());
            reader.reset();
        }

        if (data.empty()) {
            convertToDUMMY(item, data);
        }

        const std::string fileName = item.getName().getFile();
        auto& fileTableEntry = fileTable[index];

        memcpy(fileTableEntry.name, fileName.c_str(), std::min(fileName.size(), (std::size_t)40));
        fileTableEntry.decompressedFileSize = item.getDecompressedSize();
        fileTableEntry.fileSize = item.getSize();
        fileTableEntry.offset = writer->pos();

        writer->writeData(data);
        //emit saveProgress(std::distance(fileTable.begin(), fileTableIt), fileTable.size());
    }

    //write file table
    const unsigned int endOffset = writer->pos();
    writer->seek(startFileTable);
    writer->writeData(fileTable);
    writer->seek(endOffset);
    doneSaving();

    return SUCCESS;
}

bool PG_PSPFS::save() {
    if (getFile().isEmpty()) return FAILURE;

    //optimisation: if only one file changed then we check if its the same or smaller size, if yes then we just override it
    auto writer = std::make_shared<PG::STREAM::InOutByteFile>(getFile());
    if (!writer->isOpen()) return FAILURE;
    writer->seek(0);

    if (writer->readString(8) != "PSPFS_V1") {
        writer.reset();
        return PG_DisaFileContainer::save();
    }

    const unsigned int numberOfFiles = writer->readUnsignedInt();
    if (numberOfFiles != size()) {
        writer.reset();
        return PG_DisaFileContainer::save();
    }

    writer->skip(sizeof(unsigned int));

    std::vector<FileTableEntry> fileTable(size());

    struct ChangedEntry {
        PG_DisaFileEntry* entry = nullptr;
        std::vector<FileTableEntry>::iterator it;
        unsigned int newSize = 0;
        std::shared_ptr<PG::STREAM::In> reader;
    };

    std::vector<ChangedEntry> changes;

    auto fileTableIt = fileTable.begin();
    PG_DisaFileInfo item;
    for (PG_DisaFileEntry& entry : *this) {
        item = entry.getInfo();

        const std::string fileName = item.getName().getFile();
        memcpy(fileTableIt->name, fileName.c_str(), std::min(fileName.size(), (std::size_t)40));
        fileTableIt->decompressedFileSize = item.getDecompressedSize();
        fileTableIt->offset = item.getOffset();

        if (entry.getContainer() && entry.getContainer()->isModified()) {
            if (changes.size() > 4) {
                writer.reset();
                return PG_DisaFileContainer::save(); //more then one file changed
            }
            changes.emplace_back(&entry, fileTableIt);
        }
        else {
            fileTableIt->fileSize = item.getSize();
        }
        ++fileTableIt;
    }

    if (changes.empty()) return SUCCESS; //there is nothing to save

    //check if changes size is equal or smaller
    for (auto& change : changes) {
        change.reader = change.entry->createReader();
        change.newSize = change.reader->size();
        if (change.newSize > 999999991) return FAILURE;
        if (change.newSize > change.entry->getInfo().getSize()) {
            changes.clear();
            writer.reset();
            return PG_DisaFileContainer::save(); //other files must be shifted 
        }
    }

    //a game source reading this file waits until it is written
    auto guard = PG_DisaGameResourceSource::guardFile(getFile());
    //modefy
    std::vector<char> data;
    for (auto& change : changes) {
        change.it->fileSize = change.newSize;
        writer->seek(change.it->offset);
        change.reader->read(data, change.newSize);
        writer->writeData(data);
        change.reader.reset();
    }

    //write header
    writer->seek(8 + sizeof(int) * 2);
    writer->writeData(fileTable);

    return SUCCESS;
}
