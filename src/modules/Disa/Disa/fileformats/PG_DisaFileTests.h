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


#include <Stream/PG_StreamIn.h>
#include <Util/PG_File.h>

namespace PG_Disa {

    bool isIMY(const PG_File& file);
    bool isIMY(PG::STREAM::In* reader);

    unsigned int getIMYPackageSize(const PG_File& file);
    unsigned int getIMYPackageSize(PG::STREAM::In* reader);

    inline bool isIMYPackage(const PG_File& file) { return getIMYPackageSize(file) > 0; } //COLA - Compressed Offset List Archive
    inline bool isIMYPackage(PG::STREAM::In* reader) { return getIMYPackageSize(reader) > 0; } //COLA - Compressed Offset List Archive

    bool isOLA(const PG_File& file); 
    bool isOLA(PG::STREAM::In* reader);

    bool isSOLA(const PG_File& file); //Sprite Offset List Archive
    bool isSOLA(PG::STREAM::In* reader); //Sprite Offset List Archive

    bool isKM3(const PG_File& file); //Effect model archive
    bool isKM3(PG::STREAM::In* reader); //Effect model archive

    bool isScript(const PG_File& file);
    bool isScript(PG::STREAM::In* reader);

    bool isTalkScript(const PG_File& file);
    bool isTalkScript(PG::STREAM::In* reader);

    bool isSpriteSheetPackage(const PG_File& file);
    bool isSpriteSheetPackage(PG::STREAM::In* reader);

    bool isTX2(const PG_File& file);
    bool isTX2(PG::STREAM::In* reader);

    bool isGeo(const PG_File& file); //model with its textures
    bool isGeo(PG::STREAM::In* reader); //model with its textures

    bool isNISPACK(const PG_File& file);
    bool isNISPACK(PG::STREAM::In* reader);

    bool isDSARC_FL(const PG_File& file);
    bool isDSARC_FL(PG::STREAM::In* reader);

    bool isSH(const PG_File& file);
    bool isSH(PG::STREAM::In* reader);

    bool isPSPFS(const PG_File& file);
    bool isPSPFS(PG::STREAM::In* reader);

    bool isANMD2(const PG_File& file);
    bool isANMD2(PG::STREAM::In* reader);
}