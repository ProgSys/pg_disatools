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

#include "PG_Script.h"

#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>

#include <Util/Debug/PG_DebugOut.h>

/// header is the script count, a zero, then a address and a ID per script
static inline unsigned int headerSizeOf(unsigned int numberOfScripts) {
	return sizeof(unsigned int) + numberOfScripts * sizeof(unsigned int) * 2;
}

std::string PG_Script::buildScriptName(unsigned int scriptId) {
	std::stringstream o;
	o.fill('0');
	o.width(7);
	o << scriptId << ".SCRIPT";
	return o.str();
}

unsigned int PG_Script::buildScriptId(const std::string& filename) {
	//a file name is whatever the user typed, so a name that is no number carries no ID
	try {
		const long long value = std::stoll(PG_File(filename).getName());
		if (value < 0 || value > std::numeric_limits<unsigned int>::max()) return 0;
		return (unsigned int)value;
	} catch (...) {
		return 0;
	}
}

unsigned int PG_Script::getScriptId(const PG_DisaFileEntry& entry) {
	//a entry read from a SCRIPT.DAT keeps its ID, one the user added only has the name it was given
	if (const PG_Variety& extraData = entry.getExtraData(); extraData.is<unsigned int>()) {
		return extraData.get<unsigned int>();
	}
	return buildScriptId(entry.getInfo().getName().getFile());
}

bool PG_Script::open(std::shared_ptr<PG::STREAM::In> reader, const std::function<void(PG_DisaFileEntry&)>& visitor) {
	FILE_CONTAINER_ERROR_TEST(!reader || !reader->isOpen(), "Reader is not open!");

	const unsigned int fileSize = reader->size();
	const unsigned int numberOfScripts = reader->readUnsignedInt();

	FILE_CONTAINER_ERROR_TEST(numberOfScripts == 0 || numberOfScripts > 90000, "SCRIPT.DAT has a invalid number of scripts (" << numberOfScripts << ")!");

	const unsigned int headerSize = headerSizeOf(numberOfScripts);
	FILE_CONTAINER_ERROR_TEST(headerSize >= fileSize, "Header is bigger then the file itself! (" << headerSize << " >= " << fileSize << ")");
	const unsigned int dataSize = fileSize - headerSize;

	std::vector<unsigned int> addresses;
	reader->read(addresses, numberOfScripts);
	std::vector<unsigned int> ids;
	reader->read(ids, numberOfScripts);

	//a script stores no length, so it reaches until the next script starts
	std::vector<unsigned int> bounds = addresses;
	std::ranges::sort(bounds);
	bounds.erase(std::unique(bounds.begin(), bounds.end()), bounds.end());

	std::vector<PG_DisaFileEntry> contains;
	contains.reserve(numberOfScripts);


	for (unsigned int i = 0; i < numberOfScripts; ++i) {
		const unsigned int address = addresses[i];
		FILE_CONTAINER_ERROR_TEST(address >= dataSize, "Script " << i << " is outside the range (" << address << " >= " << dataSize << ")!");

		const auto nextIt = std::ranges::upper_bound(bounds, address);
		const unsigned int end = (nextIt == bounds.end()) ? dataSize : *nextIt;

		PG_DisaFileEntry entry = PG_DisaFileEntry(this);
		PG_DisaFileInfo info;
		info.setOffset(headerSize + address);
		info.setSize(end - address);
		info.setType(PG_DisaFileInfo::SCRIPT);
		info.setName(buildScriptName(ids[i]));
		entry.setInfo(std::move(info));
		entry.setExtraData(ids[i]);
		if (visitor) visitor(entry);

		contains.push_back(std::move(entry));
	}

	set(std::move(contains));

	return SUCCESS;
}

bool PG_Script::save(std::shared_ptr<PG::STREAM::Out> writer) const {
	if (!writer || empty()) return FAILURE;

	m_originalReader = createReaderOriginal();

	writer->writeInt(size());

	//write a empty header, the addresses are only known once the data is written
	const unsigned int startFileTable = writer->pos();
	std::vector<unsigned int> addressTable(size());
	std::vector<unsigned int> idTable(size());
	writer->writeData(addressTable);
	writer->writeData(idTable);

	const unsigned int dataStart = writer->pos(); //addresses are relative to the end of the header

	std::vector<char> data;
	auto addressIt = addressTable.begin();
	auto idIt = idTable.begin();
	for (const PG_DisaFileEntry& entry : *this) {
		*addressIt = writer->pos() - dataStart;
		*idIt = getScriptId(entry);

		auto entryReader = entry.createReader();
		if (!entryReader || !entryReader->isOpen()) {
			OUT_ERROR("Failed to read script '" << entry.getName() << "'!");
			doneSaving();
			return FAILURE;
		}
		entryReader->read(data, entryReader->size());
		writer->writeData(data);

		++addressIt;
		++idIt;
	}

	const unsigned int endOffset = writer->pos();
	writer->seek(startFileTable);
	writer->writeData(addressTable);
	writer->writeData(idTable);
	writer->seek(endOffset);

	doneSaving();
	return SUCCESS;
}
