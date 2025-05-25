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
#include <Disa/fileformats/PG_Def.h>


struct PG_TebleDATColumn{
    PG_DefDefinition::RowFormat::Type type = PG_DefDefinition::RowFormat::Type::INVALID;
    PG_Variety data;
};

struct PG_TebleDATRow {
    std::vector<PG_TebleDATColumn> columns;
};


class PG_TableDAT : public PG_DisaContainerInterface, public PG_ListModel<PG_TebleDATRow> {
public:
    using PG_DisaContainerInterface::PG_DisaContainerInterface;

    const PG_File& getDefFile() const { return m_defFile; }
    void setDefFile(const PG_File& file);

    bool open(std::shared_ptr<PG::STREAM::In> reader) final;
    bool save(std::shared_ptr<PG::STREAM::Out> writer) const final;

    int getHeaderIndex(const std::string& headerName) const;

    std::shared_ptr<PG::STREAM::In> createReader() const final;
    std::shared_ptr<PG::STREAM::In> createReaderOriginal()const  final;
private:
    PG_File m_defFile;
    PG_Def m_dataStructure;
};