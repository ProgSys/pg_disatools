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

#include <array>
#include <Util/Typedefs.h>

#include <Stream/PG_StreamIn.h>
#include <Stream/PG_StreamOut.h>
#include <Util/PG_File.h>

PACK(struct NtHeaderStup {
	unsigned int signature;
	//fileheader
	unsigned short machine;
	unsigned short numberOfSections;
	unsigned int timeStamp;
	unsigned int pointerToSymbolTable;
	unsigned int numberOfSymbols;
	unsigned short sizeOfOptionalHeader;
});

struct SectionHeader {
	std::array<char, 8> name; //8byte
	unsigned int misc; //PhysicalAddress or VirtualSize
	unsigned int virtualAddress;
	unsigned int sizeOfRawData;
	unsigned int pointerToRawData;
	unsigned int pointerToRelocations;
	unsigned int pointerToLineNumbers;
	unsigned short numberOfRelocations;
	unsigned short numberOfLineNumbers;
	unsigned int characteristics; //bitmask

	std::string getName() const {
		return std::string(&*name.begin());
	}
};

PACK(struct BaseRelocBlock {
	unsigned short offset : 12; //virtualAddress + offset
	unsigned short type : 4; //type of offset in our case its 3
});


struct BaseReloc {
	unsigned int virtualAddress; //the virtual origin address
	unsigned int sizeOfBaseReloc; // sizeof(unsigned int) * 2 + sizeof(BaseRelocBlock) * blocks.size()
	std::vector<BaseRelocBlock> blocks; //offsets
};


struct PG_Exe {
	std::vector<unsigned char> dosHeader; //64
	std::vector<unsigned char> dosStub; //194
	NtHeaderStup ntHeader;
	std::vector<unsigned char> ntHeaderRest; //248 - sizeof(ntHeader)
	std::vector<SectionHeader> sections;
	unsigned int inbetweenAddress;
	std::vector<unsigned char> inbetween;
	unsigned int relocationTableAddress;
	std::vector<BaseReloc> relocationTable;
	unsigned int relocationTableSize;
	std::vector<unsigned char> dataAfter;

	bool isEmpty() const { return dosHeader.empty() || sections.empty(); }
	void clear();

	bool open(const std::string& file);
	bool open(const PG_File& file);
	bool open(PG::STREAM::In* reader);

	bool save(const PG_File& file) const;
	bool save(PG::STREAM::Out* writer) const;

	bool addSectionHeader(const SectionHeader& section);
	bool addToRelocTable(unsigned int physicalAddress);
};