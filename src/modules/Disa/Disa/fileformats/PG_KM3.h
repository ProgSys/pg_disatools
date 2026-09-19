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

#include <Disa/fileformats/PG_DisaFileContainer.h>

/**
* @brief Archive of effect models, used by EFFECT.KM3. Every entry is a GEO.
* @details Unlike an Offset List Archive it stores the byte size of each entry instead of its
* offset, so an offset is the header size plus every size before it. The header size is stored,
* it is not implied by the entry count.
*
* After the size table follows one id (unsigned short) per entry, which acts like a filename.
* The ids ascend but are sparse, so an effect keeps its id when another one is removed.
* The id is kept in the entry's extra data.
*/
class PG_KM3 : public PG_DisaFileContainer {
public:
    using PG_DisaFileContainer::PG_DisaFileContainer;

    using PG_DisaFileContainer::open;
    bool open(std::shared_ptr<PG::STREAM::In> reader, const std::function<void(PG_DisaFileEntry&)>& visitor) final;
    bool save(std::shared_ptr<PG::STREAM::Out> writer) const final;

    /// Size of the header for the given entry count, the entry data starts right after it
    static unsigned int computeHeaderSize(unsigned int numberOfFiles);
};
