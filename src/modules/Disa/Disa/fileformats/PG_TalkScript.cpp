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

#include "PG_TalkScript.h"

#include <algorithm>
#include <limits>
#include <sstream>
#include <unordered_set>

#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_StringUtil.h>

/// header is the script count twice, then a file table entry per script
static inline unsigned int headerSizeOf(unsigned int numberOfScripts) {
	return sizeof(unsigned int) * 2 + numberOfScripts * sizeof(PG_DisaTalkDatScriptEntry);
}

/// a file name is whatever the user typed, so a value that isn't a number is simply not there
static inline unsigned int toUnsignedInt(const std::string& str, unsigned int fallback = 0) {
	try {
		const long long value = std::stoll(str);
		if (value < 0 || value > std::numeric_limits<unsigned int>::max()) return fallback;
		return (unsigned int)value;
	} catch (...) {
		return fallback;
	}
}

static inline void appendFilters(std::stringstream& o, char sign, const std::array<PG_DisaGameStateFilter, 4>& filters, unsigned int count) {
	count = std::min(count, (unsigned int)filters.size());
	if (count == 0) return; //a filter list the game doesn't look at has no place in the name

	o << sign <<"(";
	for (unsigned int i = 0; i < count; ++i) {
		if (i != 0) o << ',';
		o << filters[i].getValue();
	}
	o << ")";
}

static inline void parseFilters(const std::string& list, std::array<PG_DisaGameStateFilter, 4>& filters, std::uint16_t& count) {
	count = 0;
	for (const std::string& value : PG_StringUtil::split(list, ',')) {
		if (value.empty()) continue;
		if (count >= filters.size()) break;
		filters[count++].setValue((std::uint16_t)toUnsignedInt(value));
	}
}

std::string PG_TalkScript::buildScriptName(const PG_DisaTalkDatScriptEntry& info) {
	std::stringstream o;
	o.fill('0');

	//without a ID the search values are the only thing that tells two scripts apart
	o.width(4);
	o << info.scriptId;
	o << '_';
	o.width(4);
	o << info.dungeonId;
	o << '_';
	o.width(4);
	o << info.searchFlag;
	appendFilters(o, '+', info.gameStateFilterPositive, info.gameStateFilterPositiveSize);
	appendFilters(o, '-', info.gameStateFilterNegative, info.gameStateFilterNegativeSize);


	o << ".TSCRIPT";
	return o.str();
}

PG_DisaTalkDatScriptEntry PG_TalkScript::buildScriptInfo(const std::string& filename) {
	PG_DisaTalkDatScriptEntry info{};

	const std::string name = PG_File(filename).getName();
	if (name.empty()) return info;

	//a name that is nothing but digits is a script ID, everything else is a search
	if (std::ranges::all_of(name, [](char c) { return c >= '0' && c <= '9'; })) {
		info.scriptId = toUnsignedInt(name);
		return info;
	}

	const std::size_t dungeonEnd = name.find('_');
	if (dungeonEnd == std::string::npos) return info; //not a name this container ever made
	info.dungeonId = (std::uint16_t)toUnsignedInt(name.substr(0, dungeonEnd));

	const std::size_t filtersStart = name.find_first_of("+-", dungeonEnd + 1);
	info.searchFlag = (std::uint16_t)toUnsignedInt(name.substr(dungeonEnd + 1, filtersStart - (dungeonEnd + 1)));

	//both filter lists are optional and may come in either order
	for (std::size_t pos = filtersStart; pos < name.size(); ) {
		const char sign = name[pos];
		++pos;
		const std::size_t end = name.find_first_of("+-", pos);
		const std::string list = name.substr(pos, end - pos);

		if (sign == '+') parseFilters(list, info.gameStateFilterPositive, info.gameStateFilterPositiveSize);
		else parseFilters(list, info.gameStateFilterNegative, info.gameStateFilterNegativeSize);

		if (end == std::string::npos) break;
		pos = end;
	}

	return info;
}

PG_DisaTalkDatScriptEntry PG_TalkScript::getScriptInfo(const PG_DisaFileEntry& entry) {
	if (const PG_Variety& extraData = entry.getExtraData(); extraData.is<PG_DisaTalkDatScriptEntry>()) {
		return extraData.get<PG_DisaTalkDatScriptEntry>();
	}
	//a entry added outside of this container has no file table entry yet, its name is all there is to go on
	return buildScriptInfo(entry.getInfo().getName().getFile());
}

bool PG_TalkScript::open(std::shared_ptr<PG::STREAM::In> reader, const std::function<void(PG_DisaFileEntry&)>& visitor) {
	FILE_CONTAINER_ERROR_TEST(!reader || !reader->isOpen(), "Reader is not open!");

	const unsigned int fileSize = reader->size();
	const unsigned int numberOfScripts = reader->readUnsignedInt();
	const unsigned int numberOfScriptsAgain = reader->readUnsignedInt();

	FILE_CONTAINER_ERROR_TEST(numberOfScripts == 0 || numberOfScripts > 90000, "TALK.DAT has a invalid number of scripts (" << numberOfScripts << ")!");
	FILE_CONTAINER_ERROR_TEST(numberOfScripts != numberOfScriptsAgain, "The script count is stored twice but the two don't match (" << numberOfScripts << " != " << numberOfScriptsAgain << ")!");

	const unsigned int headerSize = headerSizeOf(numberOfScripts);
	FILE_CONTAINER_ERROR_TEST(headerSize >= fileSize, "Header is bigger then the file itself! (" << headerSize << " >= " << fileSize << ")");
	const unsigned int dataSize = fileSize - headerSize;

	std::vector<PG_DisaTalkDatScriptEntry> fileTable;
	reader->read(fileTable, numberOfScripts);

	//a script stores no length, so it reaches until the next script starts
	std::vector<unsigned int> bounds;
	bounds.reserve(numberOfScripts);
	for (const PG_DisaTalkDatScriptEntry& scriptInfo : fileTable) bounds.push_back(scriptInfo.startAddress);
	std::ranges::sort(bounds);
	bounds.erase(std::unique(bounds.begin(), bounds.end()), bounds.end());

	std::vector<PG_DisaFileEntry> contains;
	contains.reserve(numberOfScripts);
	std::unordered_set<std::string> usedNames; //two scripts the game can't tell apart get the same name

	for (const PG_DisaTalkDatScriptEntry& scriptInfo : fileTable) {
		FILE_CONTAINER_ERROR_TEST(scriptInfo.startAddress >= dataSize, "A script is outside the range (" << scriptInfo.startAddress << " >= " << dataSize << ")!");
		FILE_CONTAINER_ERROR_TEST(scriptInfo.gameStateFilterPositiveSize > 4 || scriptInfo.gameStateFilterNegativeSize > 4,
			"A script has more game state filters then there is room for (" << scriptInfo.gameStateFilterPositiveSize << ", " << scriptInfo.gameStateFilterNegativeSize << ")!");

		const auto nextIt = std::ranges::upper_bound(bounds, scriptInfo.startAddress);
		const unsigned int end = (nextIt == bounds.end()) ? dataSize : *nextIt;

		std::string name = buildScriptName(scriptInfo);
		if (!usedNames.insert(name).second) {
			//nothing is lost, but exporting both of them writes to the same file
			OUT_WARNING("Two talk scripts carry the same search values, so both are named '" << name << "'!");
		}

		PG_DisaFileEntry entry = PG_DisaFileEntry(this);
		PG_DisaFileInfo info;
		info.setOffset(headerSize + scriptInfo.startAddress);
		info.setSize(end - scriptInfo.startAddress);
		info.setType(PG_DisaFileInfo::TALK_SCRIPT);
		info.setName(std::move(name));
		entry.setInfo(std::move(info));
		entry.setExtraData(scriptInfo);
		if (visitor) visitor(entry);

		contains.push_back(std::move(entry));
	}

	set(std::move(contains));

	return SUCCESS;
}

bool PG_TalkScript::save(std::shared_ptr<PG::STREAM::Out> writer) const {
	if (!writer || empty()) return FAILURE;

	m_originalReader = createReaderOriginal();

	writer->writeInt(size());
	writer->writeInt(size());

	//write a empty file table, the addresses are only known once the data is written
	const unsigned int startFileTable = writer->pos();
	std::vector<PG_DisaTalkDatScriptEntry> fileTable(size());
	writer->writeData(fileTable);

	const unsigned int dataStart = writer->pos(); //addresses are relative to the end of the header

	std::vector<char> data;
	auto fileTableIt = fileTable.begin();
	for (const PG_DisaFileEntry& entry : *this) {
		PG_DisaTalkDatScriptEntry scriptInfo = getScriptInfo(entry);
		scriptInfo.startAddress = writer->pos() - dataStart; //where the script ends up now
		scriptInfo.gameStateFilterPositiveSize = std::min<std::uint16_t>(scriptInfo.gameStateFilterPositiveSize, 4);
		scriptInfo.gameStateFilterNegativeSize = std::min<std::uint16_t>(scriptInfo.gameStateFilterNegativeSize, 4);
		*fileTableIt = scriptInfo;

		auto entryReader = entry.createReader();
		if (!entryReader || !entryReader->isOpen()) {
			OUT_ERROR("Failed to read talk script '" << entry.getName() << "'!");
			doneSaving();
			return FAILURE;
		}
		entryReader->read(data, entryReader->size());
		writer->writeData(data);

		++fileTableIt;
	}

	const unsigned int endOffset = writer->pos();
	writer->seek(startFileTable);
	writer->writeData(fileTable);
	writer->seek(endOffset);

	doneSaving();
	return SUCCESS;
}
