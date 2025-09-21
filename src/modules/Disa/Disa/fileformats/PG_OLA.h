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

class PG_OLA : public PG_DisaFileContainer {
    PG_OBJECT(PG_OLA);
    PG_PARENT(PG_DisaFileContainer);
signals:
    void decompresProgress(float progress);
public:
    using PG_DisaFileContainer::PG_DisaFileContainer;

    void close();
    bool open(std::shared_ptr<PG::STREAM::In> reader, const std::function<void(PG_DisaFileEntry&)>& visitor) final;
    bool save(std::shared_ptr<PG::STREAM::Out> writer) const final;

    inline bool isHeaderSizeIncluded() const { return m_isHeaderSizeIncluded; }

    bool isCompressable() const final { return true; } ;
    bool isCompressed() const final { return m_isCompressed; }
    bool uncompress(PG::STREAM::Out* writer) final;

private:
    bool m_isHeaderSizeIncluded = false; //a typical Nis exception, files like SCRIPT.DAT don't include the file table size in file pointers
    bool m_isCompressed = false;
    uint m_decompressedSize = 0; // in bytes
};

