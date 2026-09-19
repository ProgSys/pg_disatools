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

#include "PG_DisaFileTests.h"
#include <algorithm>
#include <Stream/PG_StreamInByteFile.h>
#include <Util/Typedefs.h>
#include <Disa/fileformats/PG_TX2.h>
#include <Disa/fileformats/PG_GEO.h>
#include <Disa/fileformats/PG_KM3.h>
#include <Disa/fileformats/PG_SH.h>
#include <Disa/fileformats/PG_TalkScript.h>

template<typename T = bool>
inline T openFile(const PG_File& file, T (*func)(PG::STREAM::In*)) {
    PG::STREAM::InByteFile reader(file);

    if (reader.isOpen()) {
        const T b = func(&reader);
        reader.close();
        return b;
    }

    return T(FAILURE);
}


bool PG_Disa::isIMY(const PG_File& file) {
    return openFile(file, isIMY);
}
bool PG_Disa::isIMY(PG::STREAM::In* reader) {
    return reader->readString(3) == "IMY";
}



unsigned int PG_Disa::getIMYPackageSize(const PG_File& file) {
	return openFile<unsigned int>(file, getIMYPackageSize);
}

unsigned int PG_Disa::getIMYPackageSize(PG::STREAM::In* reader) {
	const unsigned int start = reader->pos();
	const unsigned int numberOfFiles = reader->readUnsignedInt();
	if (numberOfFiles == 0 || numberOfFiles > 9000) return 0;

	const unsigned int commpress_size = reader->readUnsignedInt();
	const unsigned int first_offset = reader->readUnsignedInt();
	if (first_offset > commpress_size) return 0;
	reader->seek(start + first_offset);

	if (PG_Disa::isIMY(reader)) {
		return numberOfFiles * commpress_size;
	}
	return 0;
}

/**
* @brief Tests the offset table that the offset list archives start with.
* @param numberOfFiles Entry count, already read from the stream, so the reader sits on the table.
* @param headerSize The earliest position an entry may start at.
* @details open() takes each entry's size from the gap to the next offset, so an offset may never
* run backwards or leave the file. Checking only a few scattered ones lets any table of unordered
* numbers through, a KM3 size table among them.
*/
static bool testOffsetTable(PG::STREAM::In* reader, unsigned int numberOfFiles, unsigned int headerSize) {
	const unsigned int readerSize = reader->size();
	if (headerSize >= readerSize) return FAILURE;

	constexpr unsigned int maxTested = 100; //a run this long already rules out anything that isn't an offset table
	const unsigned int tested = std::min(numberOfFiles, maxTested);

	std::vector<unsigned int> offsets;
	reader->read(offsets, tested);
	unsigned int previous = headerSize;
	for (unsigned int fileOffset : offsets) {
		if (fileOffset < previous || fileOffset >= readerSize) return FAILURE;
		previous = fileOffset;
	}

	if (numberOfFiles > tested) { //the far end of the table has to be sane too
		reader->skip(sizeof(unsigned int) * (numberOfFiles - tested - 1));
		const unsigned int lastOffset = reader->readUnsignedInt();
		if (lastOffset < previous || lastOffset >= readerSize) return FAILURE;
	}

	return SUCCESS;
}

bool PG_Disa::isOLA(const PG_File& file) {
	return openFile(file, isOLA);
}
bool PG_Disa::isOLA(PG::STREAM::In* reader) {
	const unsigned int numberOfFiles = reader->readUnsignedInt();
	if (numberOfFiles == 0 || numberOfFiles > 90000) return FAILURE;

	return testOffsetTable(reader, numberOfFiles, sizeof(unsigned int) + sizeof(unsigned int) * numberOfFiles);
}

bool PG_Disa::isSOLA(const PG_File& file) {
	return openFile(file, isSOLA);
}
bool PG_Disa::isSOLA(PG::STREAM::In* reader) {
	const unsigned int numberOfFiles = reader->readUnsignedInt();
	if (numberOfFiles == 0 || numberOfFiles > 90000) return FAILURE;

	return testOffsetTable(reader, numberOfFiles,
		sizeof(unsigned int) * (1 + numberOfFiles) + sizeof(unsigned short) * numberOfFiles);
}

bool PG_Disa::isKM3(const PG_File& file) {
	return openFile(file, isKM3);
}
bool PG_Disa::isKM3(PG::STREAM::In* reader) {
	const unsigned int readerSize = reader->size();
	const unsigned int numberOfFiles = reader->readUnsignedInt();
	if (numberOfFiles == 0 || numberOfFiles > 9000) return FAILURE;

	const unsigned int headerSize = reader->readUnsignedInt();
	if (headerSize != PG_KM3::computeHeaderSize(numberOfFiles) || headerSize >= readerSize) return FAILURE;

	//the entry sizes have to add up to exactly the rest of the file, that is what separates it from an OLA
	std::vector<unsigned int> sizes;
	reader->read(sizes, numberOfFiles);
	unsigned int total = headerSize;
	for (unsigned int entrySize : sizes) {
		if (entrySize == 0 || total > readerSize - entrySize) return FAILURE;
		total += entrySize;
	}

	return total == readerSize;
}

bool PG_Disa::isScript(const PG_File& file) {
	return openFile(file, isScript);
}
bool PG_Disa::isScript(PG::STREAM::In* reader) {
	const unsigned int scripts = reader->readUnsignedInt();
	const unsigned int zero = reader->readUnsignedInt();
	if (scripts == 0 || scripts > 90000 || zero != 0) return FAILURE;
	const unsigned int headerSize = sizeof(unsigned int) * 2 + sizeof(unsigned int) * scripts * 2; //offsets + ids
	const unsigned int readerSize = reader->size();

	if ((headerSize + sizeof(unsigned int)) >= readerSize) return FAILURE;

	std::vector<unsigned int> data(scripts); //address
	reader->read((char*)data.data(), scripts * sizeof(int));
	for (unsigned int a : data) {
		if ((headerSize + a) > readerSize) return FAILURE;
	}
	reader->read((char*)data.data(), scripts * sizeof(int));
	for (unsigned int a : data) {
		if (a > std::numeric_limits<unsigned int>::max() / 2) return FAILURE;
	}

	return SUCCESS;
}

bool PG_Disa::isTalkScript(const PG_File& file) {
	return openFile(file, isTalkScript);
}
bool PG_Disa::isTalkScript(PG::STREAM::In* reader) {
	const unsigned int scripts = reader->readUnsignedInt();
	const unsigned int scriptsAgain = reader->readUnsignedInt();
	if (scripts == 0 || scripts > 90000 || scripts != scriptsAgain) return FAILURE;

	const unsigned int headerSize = sizeof(unsigned int) * 2 + scripts * sizeof(talkScriptFileInfoHeader);
	const unsigned int readerSize = reader->size();
	if (headerSize >= readerSize) return FAILURE;

	//the first entry has to describe a script that starts right where the header ends
	talkScriptFileInfoHeader info;
	reader->read(info);
	if (info.startAddress != 0) return FAILURE;
	if (info.gameStateFilterPositiveSize > 4 || info.gameStateFilterNegativeSize > 4) return FAILURE;

	return SUCCESS;
}

bool PG_Disa::isSpriteSheetPackage(const PG_File& file) {
	return openFile(file, isSpriteSheetPackage);
}

bool PG_Disa::isSpriteSheetPackage(PG::STREAM::In* reader) {
	const unsigned int numberOfFiles = reader->readUnsignedInt();
	if (numberOfFiles == 0 || numberOfFiles > 90000) return FAILURE;

	const unsigned int firstFileOffset = reader->readUnsignedInt();

	const unsigned int chracterIdsOffset = numberOfFiles * 4 + 4;
	if (firstFileOffset < chracterIdsOffset + numberOfFiles * 2) return FAILURE;

	return SUCCESS;
}



bool PG_Disa::isTX2(const PG_File& file) {
	return openFile(file, isTX2);
}

bool PG_Disa::isTX2(PG::STREAM::In* reader) {
	const unsigned short width = reader->readUnsignedShort();
	const unsigned short height = reader->readUnsignedShort();

	const unsigned short type = reader->readUnsignedShort();
	const unsigned short same = reader->readUnsignedShort();

	const unsigned short colorTableSize = reader->readUnsignedShort();
	reader->skip(2);

	if (width == 0 || height == 0)
		return false;
	//if(reader.readUnsignedInt() != 65536) //width*height)
		//return false;

	switch (type) {
	case PG_TX2::DXT1:
	{
		if (width % 4 != 0 || height % 4 != 0 || colorTableSize != 0)
			return FAILURE;
	}
	break;
	case PG_TX2::DXT5:
	{
		if (width % 4 != 0 || height % 4 != 0 || colorTableSize != 0)
			return FAILURE;
	}
	break;
	case PG_TX2::BGRA:
	{
		if ((colorTableSize != 0 && colorTableSize != 16))
			return FAILURE;
	}
	break;
	case PG_TX2::COLORTABLE_RGBA256:
	{
		if (colorTableSize == 0 || colorTableSize > 500)
			return FAILURE;
	}
	break;
	case PG_TX2::COLORTABLE_BGRA256:
	{
		if (colorTableSize == 0 || colorTableSize > 500)
			return FAILURE;
	}
	break;
	case PG_TX2::COLORTABLE_BGRA16:
	{
		if (colorTableSize != 16 && colorTableSize != 256)
			return FAILURE;
	}
	break;
	case PG_TX2::COLORTABLE_RGBA16:
	{
		if (colorTableSize != 16 && colorTableSize != 256)
			return FAILURE;
	}
	break;
	default:
		return FAILURE;
		break;
	}

	return SUCCESS;
}


bool PG_Disa::isGeo(const PG_File& file) {
	return openFile(file, isGeo);
}

bool PG_Disa::isGeo(PG::STREAM::In* reader) {
	const long long fileStart = reader->pos();
	const long long fileSize = (long long)reader->size() - fileStart;
	if (fileSize < (long long)(sizeof(geofileHeader) + 32)) return FAILURE;

	geofileHeader header;
	reader->read(header);

	//the counts PG_GEO::open holds a file to, so what passes here is what it can go on to read
	if (header.number_of_transformations < 1 || header.number_of_transformations > 1024) return FAILURE;
	if (header.number_of_keyframes < 0 || header.number_of_keyframes > 1024) return FAILURE;
	if (header.number_of_nodes < 1 || header.number_of_nodes > 512) return FAILURE;
	if (header.number_of_meshes < 1 || header.number_of_meshes > 100) return FAILURE;

	/* The three tables sit between the header and the first mesh, so their sizes have to add up to no
	 * more than where that mesh says it begins. Four counts agreeing with an offset and with the file
	 * size is what makes this a test rather than a guess, the format carries no magic number. */
	const long long nodesStart = sizeof(geofileHeader)
		+ (long long)header.number_of_transformations * sizeof(geofileTransformation)
		+ (long long)header.number_of_keyframes * sizeof(geofileKeyframe);
	const long long tablesEnd = nodesStart + (long long)header.number_of_nodes * sizeof(geofileTreeNode);

	const long long firstMesh = header.offset_to_first_mesh;
	if (firstMesh < tablesEnd || firstMesh >= fileSize) return FAILURE;

	/* The nodes are a tree over those tables, so every index one holds has to land inside what the
	 * header counted. The offsets above put the whole table inside the file, so this reads no further
	 * than the first mesh.
	 * @see PG_DisaModel::parse for what each of these is read as */
	std::vector<geofileTreeNode> nodes;
	reader->seek((unsigned int)(fileStart + nodesStart));
	reader->read(nodes, header.number_of_nodes);

	bool hasRoot = false;
	for (const geofileTreeNode& node : nodes) {
		if (node.parent_index > header.number_of_nodes) return FAILURE;
		hasRoot |= node.parent_index < 1; //parent_index is one based, so anything below 1 is a root
		if (node.model_index >= header.number_of_meshes) return FAILURE;

		//an index is only read where there is a count to read it with, so only then does it have to be one
		if (node.number_of_transformations > 0 && (node.transformation_index < 0
			|| node.transformation_index + node.number_of_transformations > header.number_of_transformations))
			return FAILURE;
		if (node.number_of_keyframes > 0 && (node.keyframe_index < 0
			|| node.keyframe_index + node.number_of_keyframes > header.number_of_keyframes))
			return FAILURE;
	}

	return hasRoot; //a tree with nothing to hang off is not one
}

bool PG_Disa::isNISPACK(const PG_File& file) {
	return openFile(file, isNISPACK);
}
bool PG_Disa::isNISPACK(PG::STREAM::In* reader) {
	return reader->readString(7) == "NISPACK";
}

bool PG_Disa::isDSARC_FL(const PG_File& file) {
	return openFile(file, isDSARC_FL);
}

bool PG_Disa::isDSARC_FL(PG::STREAM::In* reader) {
	return reader->readString(8) == "DSARC FL";
}

/*
tx2Type getTX2CompressionType(PG::STREAM::InByteFile& reader) {
	reader.skip(4);
	const unsigned short type = reader.readUnsignedShort();

	switch (type) {
	case tx2Type::DXT1:
	{
		return tx2Type::DXT1;
	}
	break;
	case tx2Type::DXT5:
	{
		return tx2Type::DXT5;
	}
	break;
	case tx2Type::BGRA:
	{
		return tx2Type::BGRA;
	}
	break;
	case tx2Type::COLORTABLE_RGBA256:
	{
		return tx2Type::COLORTABLE_RGBA256;
	}
	break;
	case tx2Type::COLORTABLE_BGRA256:
	{
		return tx2Type::COLORTABLE_BGRA256;
	}
	break;
	case tx2Type::COLORTABLE_BGRA16:
	{
		return tx2Type::COLORTABLE_BGRA16;
	}
	break;
	case tx2Type::COLORTABLE_RGBA16:
	{
		return  tx2Type::COLORTABLE_RGBA16;
	}
	break;
	default:
	{
		PG_INFO_STREAM("Unknown texture type: " << type);
		return tx2Type::TX2ERROR;
	}
	break;
	}

	return tx2Type::TX2ERROR;
}

std::string asString(tx2Type type) {
	switch (type) {
	case tx2Type::DXT1:
	{
		return "DXT1";
	}
	break;
	case tx2Type::DXT5:
	{
		return "DXT5";
	}
	break;
	case tx2Type::BGRA:
	{
		return "BGRA";
	}
	break;
	case tx2Type::COLORTABLE_RGBA256:
	{
		return "COLORTABLE RGBA256";
	}
	break;
	case tx2Type::COLORTABLE_BGRA256:
	{
		return "COLORTABLE BGRA256";
	}
	break;
	case tx2Type::COLORTABLE_BGRA16:
	{
		return "COLORTABLE BGRA16";
	}
	break;
	case tx2Type::COLORTABLE_RGBA16:
	{
		return "COLORTABLE RGBA16";
	}
	break;
	default:
		return "";
		break;
	}
}

bool PG_Disa::isSpriteSheet(const PG_File& file) {
	return openFile(file, isSpriteSheet);
}

bool PG_Disa::isSpriteSheet(PG::STREAM::In* reader) {
	const unsigned int startPos = reader->pos();
	shfileHeader header;
	reader.read((char*)&header, sizeof(shfileHeader));

	std::vector<unsigned int> pointers(4);
	reader.read((char*)&pointers[0], 4 * sizeof(unsigned int));

	if (pointers[0] == 0 || pointers[1] == 0 || pointers[2] == 0 || pointers[3] == 0) return false;
	if (pointers[3] + startPos >= reader.size()) return false;

	return true;
}
*/

bool PG_Disa::isSH(const PG_File& file) {
	return openFile(file, isSH);
}
bool PG_Disa::isSH(PG::STREAM::In* reader) {
	const unsigned int startPos = reader->pos();
	shfileHeader header;
	if (reader->size() - startPos <= sizeof(shfileHeader) + sizeof(unsigned int) * 4) return false;
	reader->read((char*)&header, sizeof(shfileHeader));
	if (header.number_of_colortables == 0 || header.number_of_colortables > 255) return false;
	if (header.number_of_animations > 90000) return false;
	if (header.number_of_sheetInfos == 0 || header.number_of_sheetInfos > 512) return false;

	std::vector<unsigned int> pointers(4);
	reader->read((char*)&pointers[0], 4 * sizeof(unsigned int));

	if (pointers[0] == 0 || pointers[1] == 0 || pointers[2] == 0 || pointers[3] == 0) return false;
	if (pointers[3] + startPos >= reader->size()) return false;

	/* looking onto m_sheetsInfos would be the best approch
	reader->read((char*)m_animations.data(), m_animations.size() * sizeof(shfileAnimation));
	reader->read((char*)m_layers.data(), m_layers.size() * sizeof(shfileLayers));
	reader->read((char*)m_numberOfColortables.data(), m_numberOfColortables.size() * sizeof(unsigned int));
	reader->read((char*)m_sheetsInfos.data(), m_sheetsInfos.size() * sizeof(shfileSheetInfo));
	*/

	return true;
}

bool PG_Disa::isPSPFS(const PG_File& file) {
	return openFile(file, isPSPFS);
}

bool PG_Disa::isPSPFS(PG::STREAM::In* reader) {
	if (reader->readString(8) == "PSPFS_V1") {
		return SUCCESS;
	}
	return FAILURE;
}

bool PG_Disa::isANMD2(const PG_File& file) {
	return openFile(file, isANMD2);
}


bool PG_Disa::isANMD2(PG::STREAM::In* reader) {
	const unsigned int files = reader->readUnsignedInt();
	if (files > 0 && files < 9000) {
		if (reader->readUnsignedInt() != 0) return FAILURE;
		if (reader->readUnsignedInt() != 0) return FAILURE;
		if (reader->readUnsignedInt() != 0) return FAILURE;
		return SUCCESS;
	}
	return FAILURE;
}


