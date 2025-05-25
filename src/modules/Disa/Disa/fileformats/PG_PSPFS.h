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
#pragma once

#include <functional>
#include <Disa/fileformats/PG_DisaFileContainer.h>

class PG_PSPFS: public PG_DisaFileContainer {
public:
	PACK(struct FileTableEntry //total of 64 bytes
	{
		char name[40] = { " " };
		unsigned int decompressedFileSize = 0;
		unsigned int fileSize = 0;
		unsigned int offset = 0ll;
	});
	using PG_DisaFileContainer::PG_DisaFileContainer;

    bool open(std::shared_ptr<PG::STREAM::In> reader) final;
	bool save(std::shared_ptr<PG::STREAM::Out> writer) const final;
	bool save() final;
	bool refresh(std::shared_ptr<PG::STREAM::In> reader) final;
};

