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

#include "PG_SOLA.h"
#include <algorithm>

#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamOutByteFile.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_Exception.h>
#include <Util/PG_StringUtil.h>
#include <Disa/fileformats/PG_DisaFileTests.h>

#include <Stream/PG_StreamInByteArray.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamInOutByteVector.h>
#include <resources/PG_ResourceManager.h>

inline void skipSCVColumn(PG::STREAM::InByteFile& reader) {
	char c;
	while (!reader.eof() && (c = reader.readChar()) != ',' && c != '\n') {}
}

inline std::string readSCVColumn(PG::STREAM::InByteFile& reader) {
	std::string str;
	char c;
	while (!reader.eof() && (c = reader.readChar()) != ',' && c != '\n') {
		str.push_back(c);
	}
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
	return str;
}

inline void skipSCVLine(PG::STREAM::InByteFile& reader) {
	char c;
	while (!reader.eof() && (c = reader.readChar()) != '\n') {}
}



static inline std::map<unsigned short, std::string> readFileNames(std::string filename) {
	std::map<unsigned short, std::string> outNames;
	std::transform(filename.begin(), filename.end(), filename.begin(), ::toupper);

	PG::STREAM::InByteFile reader(PG_ResourceManager::get()->getResourceLocation() + "/script/disa/nameTables/" + filename + ".csv");
	if (reader.isOpen()) {
		skipSCVLine(reader);

		while (!reader.eof()) {
			unsigned short id = PG_StringUtil::toInt(readSCVColumn(reader));
			if (id > 0) {
				outNames[id] = readSCVColumn(reader);
			}
			skipSCVLine(reader);
		}

	}
	reader.close();

	return outNames;
}


bool PG_SOLA::open(std::shared_ptr<PG::STREAM::In> reader) {
	clear();
	if (!reader->isOpen()) {
		OUT_ERROR("Reader is not open!");
		return FAILURE;
	}

	std::map<unsigned short, std::string> nameTable;
	if (auto entry = getParentEntry()) {
		nameTable = readFileNames(entry.getInfo().getName());
	}

	unsigned int fileSize = reader->size();
	const unsigned int numberOfFiles = reader->readUnsignedInt();
	const unsigned int headerSize = sizeof(int) + numberOfFiles * sizeof(int);

	FILE_CONTAINER_ERROR_TEST(numberOfFiles > 9000, "*.DAT has too many files!");

	std::vector<PG_DisaFileEntry> contains;
	contains.reserve(numberOfFiles);
	unsigned int lastOffset = reader->readUnsignedInt();

	for (unsigned int i = 0; i < numberOfFiles; ++i) {
		PG_DisaFileEntry entry = PG_DisaFileEntry(this);
		PG_DisaFileInfo info;
		info.setOffset(lastOffset);
		if (i != numberOfFiles - 1) {
			lastOffset = reader->readUnsignedInt();
		} else
			lastOffset = fileSize;

		FILE_CONTAINER_ERROR_TEST(lastOffset < info.getOffset(), "File offset order is wrong! " << "(" << lastOffset << " < " << info.getOffset() << ") at: " << reader->pos());
		info.setSize(lastOffset - info.getOffset());
	
		FILE_CONTAINER_ERROR_TEST(info.getOffset() < headerSize, "File offset is smaller then the header! (" << info.getOffset() << " < " << headerSize << ")");
		FILE_CONTAINER_ERROR_TEST(info.getOffset() + info.getSize() > fileSize, "File is outside the range (" << (info.getOffset() + info.getSize()) << " > " << fileSize << ")!");

		//try dertermin file type
		const unsigned int currentPos = reader->pos();
		info.setType(PG_DisaFileInfo::SH);

		reader->seek(currentPos);
		entry.setInfo(std::move(info));
		contains.push_back(entry);
	}

	std::vector<unsigned short> chractersIDs;
	chractersIDs.resize(numberOfFiles);
	reader->read((char*)&chractersIDs[0], chractersIDs.size() * sizeof(short));

	auto it = chractersIDs.begin();
	for (auto& entry: contains) {
		std::stringstream o;
		o.fill('0');
		o.width(4);
		o << *it;
		o << "_";

		if (auto findIt = nameTable.find(*it); findIt != nameTable.end() && !findIt->second.empty()) {
			o << findIt->second;
		}
		else {
			o << "FILE.SH";
		}
		entry.setExtraData(*it);
		entry.getInfoOriginal().setName(o.str());
		reader->seek(entry.getInfoOriginal().getOffset());
		++it;
	}


	set(std::move(contains));

	return SUCCESS;
}


bool PG_SOLA::save(std::shared_ptr<PG::STREAM::Out> writer) const{

	m_originalReader = createReaderOriginal();

	writer->writeInt(size());

	
	const unsigned int startOffset = writer->pos();
	std::vector<unsigned int> fileTable;
	fileTable.resize(size());
	writer->writeData(fileTable);
	std::vector<unsigned short> idTable;
	idTable.resize(size());
	writer->writeData(idTable);
	const unsigned int fileTableEndOffset = writer->pos();

	std::vector<char> data;
	auto tableIt = fileTable.begin();
	auto idIt = fileTable.begin();
	for (const auto& entry : *this) {
		*tableIt = writer->pos();
		*tableIt = entry.getExtraData().get<unsigned short>();

		auto reader = entry.createReader();
		if (!reader || !reader->isOpen()) {
			doneSaving();
			return FAILURE;
		}
		reader->read(data, reader->size());
		++tableIt;
		++idIt;
	}

	const unsigned int endOffset = writer->pos();
	writer->seek(startOffset);
	writer->writeData(fileTable);
	writer->writeData(idTable);
	writer->seek(endOffset);

	doneSaving();
	return SUCCESS;
}
