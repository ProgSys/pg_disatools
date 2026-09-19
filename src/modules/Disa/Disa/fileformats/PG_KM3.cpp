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
#include "PG_KM3.h"

#include <limits>
#include <sstream>

#include <Util/Debug/PG_DebugOut.h>
#include <Stream/PG_StreamIn.h>
#include <Stream/PG_StreamOut.h>

static constexpr unsigned int alignUp(unsigned int value) {
	constexpr unsigned int alignment = 16; //every entry starts on a 16 byte boundary, the game expects the model data aligned
	const unsigned int rest = value % alignment;
	return rest ? value + (alignment - rest) : value;
}

const PG_DisaExtraFields& PG_KM3::describeExtraData() const {
	static const PG_DisaExtraFields fields =
		makeIdDescription<unsigned short>("Effect ID", "What the game references this effect by");
	return fields;
}

std::string PG_KM3::buildEntryName(const PG_DisaFileEntry& entry) const {
	return buildEffectName(getEffectId(entry));
}

std::string PG_KM3::buildEffectName(unsigned short effectId) {
	std::stringstream o;
	o.fill('0');
	o.width(4);
	o << effectId << ".GEO";
	return o.str();
}

unsigned short PG_KM3::buildEffectId(const std::string& filename) {
	//a file name is whatever the user typed, so a name that is no number carries no id
	try {
		const long long value = std::stoll(PG_File(filename).getName());
		if (value < 0 || value > std::numeric_limits<unsigned short>::max()) return 0;
		return (unsigned short)value;
	} catch (...) {
		return 0;
	}
}

unsigned short PG_KM3::getEffectId(const PG_DisaFileEntry& entry) {
	//a entry read from a KM3 keeps its id, one the user added only has the name it was given
	if (const PG_Variety& extraData = entry.getExtraData(); extraData.is<unsigned short>()) {
		return extraData.get<unsigned short>();
	}
	return buildEffectId(entry.getInfo().getName().getFile());
}

unsigned int PG_KM3::computeHeaderSize(unsigned int numberOfFiles) {
	constexpr unsigned int fields = sizeof(unsigned int) * 2; //the entry count and the header size itself
	constexpr unsigned int perEntry = sizeof(unsigned int) + sizeof(unsigned short); //one size and one id
	return alignUp(fields + numberOfFiles * perEntry);
}

bool PG_KM3::open(std::shared_ptr<PG::STREAM::In> reader, const std::function<void(PG_DisaFileEntry&)>& visitor) {
	FILE_CONTAINER_ERROR_TEST(!reader || !reader->isOpen(), "Reader is not open!");

	const unsigned int fileSize = reader->size();
	const unsigned int numberOfFiles = reader->readUnsignedInt();
	FILE_CONTAINER_ERROR_TEST(numberOfFiles == 0 || numberOfFiles > 9000, "KM3 has an invalid file count (" << numberOfFiles << ")!");

	const unsigned int headerSize = reader->readUnsignedInt();
	FILE_CONTAINER_ERROR_TEST(headerSize < computeHeaderSize(numberOfFiles) || headerSize >= fileSize,
		"KM3 header size is out of range (" << headerSize << ")!");

	std::vector<unsigned int> sizes;
	reader->read(sizes, numberOfFiles);
	std::vector<unsigned short> ids;
	reader->read(ids, numberOfFiles);

	std::vector<PG_DisaFileEntry> contains;
	contains.reserve(numberOfFiles);

	unsigned int offset = headerSize;
	for (unsigned int i = 0; i < numberOfFiles; ++i) {
		PG_DisaFileEntry entry = PG_DisaFileEntry(this);
		PG_DisaFileInfo info;
		info.setOffset(offset);
		info.setSize(sizes[i]);

		FILE_CONTAINER_ERROR_TEST(sizes[i] == 0, "Entry " << i << " is empty!");
		FILE_CONTAINER_ERROR_TEST(sizes[i] > fileSize - offset,
			"Entry " << i << " is outside the range (" << offset << " + " << sizes[i] << " > " << fileSize << ")!");

		info.setName(buildEffectName(ids[i]));
		info.setType(PG_DisaFileInfo::GEO);

		entry.setExtraData(ids[i]);
		entry.setInfo(std::move(info));
		if (visitor) visitor(entry);

		contains.push_back(std::move(entry));
		offset += sizes[i];
	}

	FILE_CONTAINER_ERROR_TEST(offset != fileSize, "Entry sizes do not add up to the file size (" << offset << " != " << fileSize << ")!");

	set(std::move(contains));

	return SUCCESS;
}

bool PG_KM3::save(std::shared_ptr<PG::STREAM::Out> writer) const {
	const unsigned int startPos = writer->pos();
	m_originalReader = createReaderOriginal();

	const unsigned int numberOfFiles = size();
	const unsigned int headerSize = computeHeaderSize(numberOfFiles);

	writer->writeInt(numberOfFiles);
	writer->writeInt(headerSize);

	const unsigned int startTable = writer->pos();
	std::vector<unsigned int> sizeTable(numberOfFiles);
	std::vector<unsigned short> idTable(numberOfFiles);
	writer->writeData(sizeTable);
	writer->writeData(idTable);

	if (const unsigned int written = writer->pos() - startPos; written < headerSize) {
		writer->writeData(std::vector<char>(headerSize - written, 0));
	}

	std::vector<char> data;
	auto sizeIt = sizeTable.begin();
	auto idIt = idTable.begin();
	for (const auto& entry : *this) {
		auto entryReader = entry.createReader();
		if (!entryReader || !entryReader->isOpen()) {
			doneSaving();
			return FAILURE;
		}

		const unsigned int entryStart = writer->pos();
		entryReader->read(data, entryReader->size());
		writer->writeData(data);

		const unsigned int written = writer->pos() - entryStart;
		if (const unsigned int padding = alignUp(written) - written; padding) {
			writer->writeData(std::vector<char>(padding, 0));
		}

		*sizeIt = writer->pos() - entryStart;
		*idIt = getEffectId(entry);
		++sizeIt;
		++idIt;
	}

	const unsigned int endOffset = writer->pos();
	writer->seek(startTable);
	writer->writeData(sizeTable);
	writer->writeData(idTable);
	writer->seek(endOffset);

	doneSaving();
	return SUCCESS;
}
