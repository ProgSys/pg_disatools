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
#include <limits>
#include <sstream>

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


const PG_DisaExtraFields& PG_SOLA::describeExtraData() const {
	static const PG_DisaExtraFields fields =
		makeIdDescription<unsigned short>("Sprite ID", "What the game references this sprite sheet by");
	return fields;
}

std::string PG_SOLA::buildEntryName(const PG_DisaFileEntry& entry) const {
	return buildName(getSpriteId(entry));
}

std::string PG_SOLA::buildName(unsigned short spriteId) const {
	std::stringstream o;
	o.fill('0');
	o.width(4);
	o << spriteId << "_";

	if (auto findIt = m_nameTable.find(spriteId); findIt != m_nameTable.end() && !findIt->second.empty()) {
		o << findIt->second;
	}
	else {
		o << "FILE." << m_fileExtension;
	}

	return o.str();
}

unsigned short PG_SOLA::buildSpriteId(const std::string& filename) {
	//a file name is whatever the user typed, so a name that is no number carries no id
	try {
		const long long value = std::stoll(PG_File(filename).getName());
		if (value < 0 || value > std::numeric_limits<unsigned short>::max()) return 0;
		return (unsigned short)value;
	} catch (...) {
		return 0;
	}
}

unsigned short PG_SOLA::getSpriteId(const PG_DisaFileEntry& entry) {
	//a entry read from a SOLA keeps its id, one the user added only has the name it was given
	if (const PG_Variety& extraData = entry.getExtraData(); extraData.is<unsigned short>()) {
		return extraData.get<unsigned short>();
	}
	return buildSpriteId(entry.getInfo().getName().getFile());
}

bool PG_SOLA::open(std::shared_ptr<PG::STREAM::In> reader, const std::function<void(PG_DisaFileEntry&)>& visitor) {
	FILE_CONTAINER_ERROR_TEST(!reader->isOpen(), "Reader is not open!");

	m_nameTable.clear();
	if (auto entry = getParentEntry()) {
		m_nameTable = readFileNames(entry.getInfo().getName());
	}

	unsigned int fileSize = reader->size();
	const unsigned int numberOfFiles = reader->readUnsignedInt();
	const unsigned int headerSize = sizeof(int) + numberOfFiles * sizeof(int);

	FILE_CONTAINER_ERROR_TEST(numberOfFiles > 9000, "*.DAT has too many files!");

	std::vector<PG_DisaFileEntry> contains;
	contains.reserve(numberOfFiles);
	unsigned int lastOffset = reader->readUnsignedInt();

	//determin file type
	auto lastLoc = reader->pos();
	reader->seek(lastOffset);
	const auto fileType = PG_DisaFileInfo::typeFromFileFormat(reader.get());
	reader->seek(lastLoc);
	m_fileExtension = PG_DisaFileInfo::toFileExtension(fileType);

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
		info.setType(fileType);

		reader->seek(currentPos);
		entry.setInfo(std::move(info));
		if (visitor) visitor(entry);
		contains.push_back(std::move(entry));
	}

	std::vector<unsigned short> chractersIDs;
	chractersIDs.resize(numberOfFiles);
	reader->read((char*)&chractersIDs[0], chractersIDs.size() * sizeof(short));

	auto it = chractersIDs.begin();
	for (auto& entry: contains) {
		entry.setExtraData(*it);
		entry.getInfo().setName(buildName(*it));
		reader->seek(entry.getInfo().getOffset());
		++it;
	}


	set(std::move(contains));

	return SUCCESS;
}


bool PG_SOLA::save(std::shared_ptr<PG::STREAM::Out> writer) const{
	const unsigned int startPos = writer->pos();

	m_originalReader = createReaderOriginal();

	writer->writeInt(size());

	//write file table
	const unsigned int startFileTable = writer->pos();
	std::vector<unsigned int> fileTable(size());
	std::vector<unsigned short> idTable(size());

	writer->writeData(fileTable);
	writer->writeData(idTable);

	//write content
	std::vector<char> data;
	auto tableIt = fileTable.begin();
	auto idIt = idTable.begin();
	for (const auto& entry : *this) {
		*tableIt = writer->pos() - startPos;
		*idIt = getSpriteId(entry);

		auto reader = entry.createReader();
		if (!reader || !reader->isOpen()) {
			doneSaving();
			return FAILURE;
		}
		reader->read(data, reader->size());
		++tableIt;
		++idIt;
		writer->writeData(data);
	}

	const unsigned int endOffset = writer->pos();
	writer->seek(startFileTable);
	writer->writeData(fileTable);
	writer->writeData(idTable);
	writer->seek(endOffset);

	doneSaving();
	return SUCCESS;
}
