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

#include "PG_OLA.h"
#include <algorithm>

#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamOutByteFile.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_Exception.h>
#include <Disa/fileformats/PG_IMY.h>
#include <Stream/PG_StreamInByteArray.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamInOutByteVector.h>
#include <resources/PG_ResourceManager.h>



void PG_OLA::close() {
	PG_DisaFileContainer::clear();
	m_isCompressed = false;
	m_decompressedSize = 0;
}

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



static inline std::vector<std::string> readFileNames(std::string filename) {
	std::vector<std::string> outNames;
	std::transform(filename.begin(), filename.end(), filename.begin(), ::toupper);
	PG_File nameTable(PG_ResourceManager::get()->getResourceLocation() + "/script/disa/nameTables/" + filename + ".csv");
	if (!nameTable.exists()) {
		OUT_WARNING("There is no nametable provided for \"" << filename << "\".");
		return outNames;
	}

	PG::STREAM::InByteFile reader(nameTable);
	if (reader.isOpen()) {
		skipSCVLine(reader);

		while (!reader.eof()) {
			skipSCVColumn(reader);
			outNames.push_back(readSCVColumn(reader));
			skipSCVLine(reader);
		}

	}
	reader.close();

	return outNames;
}


bool PG_OLA::open(std::shared_ptr<PG::STREAM::In> reader) {
	close();
	if (!reader || !reader->isOpen()) {
		OUT_ERROR("Reader is not open!");
		return FAILURE;
	}

	std::vector<std::string> nameTable;
	if (auto entry = getParentEntry()) {
		nameTable = readFileNames(entry.getInfo().getName());
	}

	unsigned int fileSize = reader->size();
	const unsigned int numberOfFiles = reader->readUnsignedInt();

	//test if this is a IMY file
	{
		m_decompressedSize = reader->readUnsignedInt();
		const unsigned int compressedSize = reader->readUnsignedInt();
		m_isCompressed = m_decompressedSize > compressedSize;
		reader->seek(4);
	}

	const unsigned int headerSize = (m_isCompressed) ? 8 + numberOfFiles * sizeof(int) : 4 + numberOfFiles * sizeof(int);
	if (m_isCompressed) reader->skip(4);

	FILE_CONTAINER_ERROR_TEST(numberOfFiles > 9000, "START.DAT has too many files!");

	std::vector<PG_DisaFileEntry> contains;
	contains.reserve(numberOfFiles);
	unsigned int lastOffset = reader->readUnsignedInt();
	m_isHeaderSizeIncluded = lastOffset != 0;
	if (!m_isHeaderSizeIncluded) {
		lastOffset += headerSize;
	}

	for (unsigned int i = 0; i < numberOfFiles; ++i) {
		PG_DisaFileEntry entry = PG_DisaFileEntry(this);
		PG_DisaFileInfo info;
		info.setOffset(lastOffset);
		if (i != numberOfFiles - 1) {
			lastOffset = reader->readUnsignedInt();
			if (!m_isHeaderSizeIncluded) {
				lastOffset += headerSize;
			}
		} else
			lastOffset = fileSize;

		FILE_CONTAINER_ERROR_TEST(lastOffset < info.getOffset(), "File offset order is wrong! " << "(" << lastOffset << " < " << info.getOffset() << ") at: " << reader->pos());
		info.setSize(lastOffset - info.getOffset());
	
		FILE_CONTAINER_ERROR_TEST(info.getOffset() < headerSize, "File offset is smaller then the header! (" << info.getOffset() << " < " << headerSize << ")");
		FILE_CONTAINER_ERROR_TEST(info.getOffset() + info.getSize() > fileSize, "File is outside the range (" << (info.getOffset() + info.getSize()) << " > " << fileSize << ")!");

		//try dertermin file type
		{
			std::stringstream o;
			if (i < nameTable.size()) {
				o << nameTable[i];
			}
			else {
				o << "FILE";
				o.fill('0');
				o.width(4);
				o << i << ".DAT";
			}
			info.setName(o.str());
			info.determinTypeFromFileExtention();
		}

		entry.setInfo(std::move(info));
		contains.push_back(entry);
	}

	m_originalReader = reader;
	const unsigned int currentPos = reader->pos();
	for (PG_DisaFileEntry& entry : contains) {
		if (entry.getContainer()) continue;
		auto entryReader = entry.createReader();
		entryReader->rewind();
		entry.getInfo().determinTypeFromFileFormat(entryReader.get());
		//we don't trust the deffinition file
		if(entry.getInfo().getType() == PG_DisaFileInfo::UNKNOWN) entry.getInfo().getName().replaceFileExtension("DAT");
	}
	reader->seek(currentPos);
	m_originalReader.reset();

	set(std::move(contains));

	return SUCCESS;
}


bool PG_OLA::save(std::shared_ptr<PG::STREAM::Out> writer) const {
	const auto startPos = writer->pos();
	m_originalReader = createReaderOriginal();

	writer->writeInt(size());
	if (isCompressed()) {
		writer->writeInt(m_decompressedSize);
	}
	
	const unsigned int startOffset = writer->pos();
	std::vector<unsigned int> fileTable;
	fileTable.resize(size());
	writer->writeData(fileTable);
	const unsigned int fileTableEndOffset = m_isHeaderSizeIncluded? startPos: writer->pos();

	std::vector<char> data;
	auto it = fileTable.begin();
	for (auto& entry : *this) {
		*it = writer->pos() - fileTableEndOffset;
		auto reader = entry.createReader();
		if (!reader || !reader->isOpen()) {
			doneSaving();
			return FAILURE;
		}
		reader->read(data, reader->size());
		writer->writeData(data);
		++it;
	}

	const unsigned int endOffset = writer->pos();
	writer->seek(startOffset);
	writer->writeData(fileTable);
	writer->seek(endOffset);

	doneSaving();
	return SUCCESS;
}

bool PG_OLA::uncompress(PG::STREAM::Out* writer) {
	if (!m_isCompressed && m_decompressedSize > 0 && !m_file.isEmpty()) return false;

	auto reader = createReader();

	if (!reader || !reader->isOpen()) {
		OUT_ERROR("Failed to open file to decompress!");
		return FAILURE;
	}

	PG::STREAM::InOutByteVector out(m_decompressedSize);
	auto progressInfo = [this](float progress) {
		emit decompresProgress(progress);
		};

	if (!PG_Disa::decompressIMYPackage(reader.get(), &out, progressInfo)) {
		OUT_ERROR("Failed to decompress!");
		return FAILURE;
	}
	if (auto* w = dynamic_cast<PG::STREAM::InOutByteVector*>(writer)) {
		w->getData() = std::move(out.getData());
		w->seek(w->getData().size());
	}
	else {
		writer->writeData(out.getData());
	}

	return SUCCESS;
}