#include "PG_Exe.h"

#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamOutByteFile.h>

#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_Exception.h>
#include <algorithm>

void PG_Exe::clear() {
	dosHeader.clear(); //64
	dosStub.clear(); //194
	ntHeaderRest.clear(); //248 - sizeof(ntHeader)
	sections.clear();
	inbetween.clear();
	relocationTable.clear();
	dataAfter.clear();
}

bool PG_Exe::open(const std::string& file) {
	if (file.empty()) {
		OUT_ERROR("Given filename is empty!");
		return FAILURE;
	}

	return open(PG_File(file));
}


bool PG_Exe::open(const PG_File& file) {
	if (!file.exists()) {
		clear();
		OUT_ERROR("Target file doesn't exist!");
		return FAILURE;
	}

	try {
		PG::STREAM::InByteFile reader(file);
		const bool result = open(&reader);
		reader.close();
		return result;
	}
	catch (PG_Exception& e) {
		OUT_ERROR("Failed to import EXE! : " << e.what());
		clear();
		return FAILURE;
	}
	catch (...) {
		OUT_ERROR("Failed to import EXE!");
		clear();
		return FAILURE;
	}
}


bool PG_Exe::open(PG::STREAM::In* reader) {
	unsigned int fileSize = reader->size();

	if (fileSize < 100) {
		throw PG_Exception("File size too small!");
	}
	reader->read(dosHeader, 64);
	reader->read(dosStub, 192 + 16);
	reader->read(ntHeader);
	reader->read(ntHeaderRest, 248 - sizeof(NtHeaderStup));

	if (ntHeader.numberOfSections > 100) {
		throw PG_Exception("Too many sections!");
	}
	reader->read(sections, ntHeader.numberOfSections);

	//find .reloc secion
	auto findIt = std::ranges::find(sections, ".reloc", &SectionHeader::getName);
	if (findIt == sections.end()) {
		throw PG_Exception("Failed to find '.reloc' section!");
	}
	if (fileSize <= findIt->pointerToRawData) {
		throw PG_Exception("VirtualTableAddress is smaller then fileSize!");
	}
	inbetweenAddress = reader->pos();
	reader->read(inbetween, findIt->pointerToRawData - inbetweenAddress);

	relocationTableAddress = reader->pos();
	//read relocationTable
	while (reader->pos() < fileSize - 100) {
		BaseReloc reloc;
		reloc.virtualAddress = reader->readUnsignedInt();
		if (reloc.virtualAddress == 0) {
			reader->seek(reader->pos() - sizeof(int));
			break;
		}
		reloc.sizeOfBaseReloc = reader->readUnsignedInt();
		unsigned int numberOfBlocks = (reloc.sizeOfBaseReloc - sizeof(unsigned int) * 2) / 2;
		if (numberOfBlocks > 1000000) {
			throw PG_Exception("Too many blocks in BaseReloc!");
		}
		reader->read(reloc.blocks, numberOfBlocks);
		relocationTable.push_back(std::move(reloc));
	}
	relocationTableSize = reader->pos() - relocationTableAddress;

	reader->read(dataAfter, fileSize - reader->pos());
	return SUCCESS;
}

bool PG_Exe::save(const PG_File& file) const {
	try {
		PG::STREAM::OutByteFile writer(file);
		if (!writer.isOpen()) return FAILURE;
		const bool result = save(&writer);
		writer.close();
		return result;
	}
	catch (PG_Exception& e) {
		OUT_ERROR("Failed to export EXE! : " << e.what());
		return FAILURE;
	}
	catch (...) {
		OUT_ERROR("Failed to export EXE!");
		return FAILURE;
	}
}

bool PG_Exe::save(PG::STREAM::Out* writer) const {
	writer->writeData(dosHeader);
	writer->writeData(dosStub);
	writer->writeData(ntHeader);
	writer->writeData(ntHeaderRest);
	writer->writeData(sections);
	writer->writeData(inbetween);

	for (const auto& entry : relocationTable) {
		writer->writeInt(entry.virtualAddress);
		writer->writeInt(entry.sizeOfBaseReloc);
		writer->writeData(entry.blocks);
	}
	writer->writeData(dataAfter);
	return true;
}

bool PG_Exe::addSectionHeader(const SectionHeader& section) {
	sections.push_back(section);
	inbetween.erase(inbetween.begin(), inbetween.begin() + sizeof(SectionHeader));
	inbetweenAddress += sizeof(SectionHeader);
	++ntHeader.numberOfSections;
	return true;
}

bool PG_Exe::addToRelocTable(unsigned int physicalAddress) {
	auto findSectionIt = std::ranges::find(sections, ".text", &SectionHeader::getName);
	if (findSectionIt == sections.end()) {
		OUT_ERROR("Failed to find .text section!");
		return false;
	}
	unsigned int virtualAddress = physicalAddress - findSectionIt->pointerToRawData + findSectionIt->virtualAddress;
	auto findTableIt = std::ranges::find_if(relocationTable, [physicalAddress](const BaseReloc& e) { return e.virtualAddress > physicalAddress; });
	if (findTableIt == relocationTable.end()) {
		OUT_ERROR("Failed to find a valid relocation table entry!");
		return false;
	}
	--findTableIt;
	unsigned int offset = virtualAddress - findTableIt->virtualAddress;

	findTableIt->blocks.emplace_back(offset, 3);
	findTableIt->sizeOfBaseReloc += sizeof(BaseRelocBlock);
	std::ranges::sort(findTableIt->blocks, [](unsigned int a, unsigned int b) {
		if (a == 0 && a != b) return false;
		if (b == 0 && a != b) return true;
		return a < b;
		}, [](const BaseRelocBlock& b)->unsigned int { return b.offset; });
	relocationTableSize += sizeof(BaseRelocBlock);
	dataAfter.erase(dataAfter.begin(), dataAfter.begin() + sizeof(BaseRelocBlock));
	OUT_INFO("Added relocation table entry for virtual " << virtualAddress << " physical " << physicalAddress);
	return true;
}