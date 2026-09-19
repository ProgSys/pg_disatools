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

#include "PG_DisaFileInfo.h"
#include <Stream/PG_StreamIn.h>
#include <Disa/fileformats/PG_DisaFileTests.h>
#include <array>


PG_DisaFileInfo::PG_DisaFileInfo(const PG_File& externalFile) : m_name(externalFile), m_size(externalFile.size()) {
    determinTypeFromFileExtention();
}

void PG_DisaFileInfo::setType(const std::string& extention) {
	if (extention == "mpd") setType(MPD);
	else if (extention == "mpp") setType(MPP);
	else if (extention == "json") setType(JSON);
	else if (extention == "png") setType(PNG);
	else if (extention == "tx2") setType(TX2);
	else if (extention == "geo") setType(GEO);
	else if (extention == "km3") setType(KM3);
	else if (extention == "ogg") setType(OGG);
	else if (extention == "tscript") setType(TALK_SCRIPT);
	else if (extention == "script") setType(SCRIPT);
	else if (extention == "sh") setType(SH);
	else if (extention == "zip") setType(ZIP);
}

std::string_view PG_DisaFileInfo::toString(Type type) {
    switch (type) {
    case PSPFS_V1: return "PSPFS_V1";
    case OLA: return "OLA";
    case COLA: return "COLA";
    case SOLA: return "SOLA";
    case KM3: return "KM3";
    case TX2: return "TX2";
    case IMY: return "IMY";
    case MPD: return "MPD";
    case MPP: return "MPP";
    case SH: return "SH";
    case SH2: return "SH2";
    case GEO: return "GEO";
    case LZS: return "LZS";
    case PNG: return "PNG";
    case JSON: return "JSON";
    case TALK_SCRIPT: return "TALK_SCRIPT";
    case SCRIPT: return "SCRIPT";
    case ZIP: return "ZIP";
    case OGG: return "OGG";
    default:
    case UNKNOWN: return "UNKNOWN";
    }
}

std::string_view PG_DisaFileInfo::toFileExtension(Type type) {
    switch (type) {
        case OLA: return "OLA";
        case COLA: return "COLA";
        case SOLA: return "SOLA";
        case KM3: return "KM3";
        case TX2: return "TX2";
        case IMY: return "IMY";
        case MPD: return "MPD";
        case MPP: return "MPP";
        case SH: return "SH";
        case SH2: return "SH2";
        case GEO: return "GEO";
        case LZS: return "LZS";
        case PNG: return "PNG";
        case JSON: return "JSON";
        case TALK_SCRIPT: return "TALK_SCRIPT";
        case SCRIPT: return "SCRIPT";
        case ZIP: return "ZIP";
        case OGG: return "OGG";
        default:
        case PSPFS_V1: 
        case UNKNOWN: return {};
    }
}

PG_DisaFileInfo::Type PG_DisaFileInfo::typeFromFileFormat(PG::STREAM::In* reader) {
    if (!reader) return UNKNOWN;
    const unsigned int startPos = reader->pos();

    using TestFunc = bool(*)(PG::STREAM::In* reader);
    using PairType = std::pair<Type, TestFunc>;
    //the first test that passes wins, so a loose one goes after the tests it could take a file from
    constexpr static std::array<std::pair<Type, TestFunc>, 11> tests = {
        PairType{Type::PSPFS_V1, &PG_Disa::isPSPFS},
        PairType{Type::IMY, &PG_Disa::isIMY},
        PairType{Type::COLA, &PG_Disa::isIMYPackage},
        PairType{Type::KM3, &PG_Disa::isKM3}, //before the offset list archives, a size table can pass as an offset table
        PairType{Type::SOLA, &PG_Disa::isSOLA},
        PairType{Type::SCRIPT, &PG_Disa::isScript},
        PairType{Type::TALK_SCRIPT, &PG_Disa::isTalkScript},
        PairType{Type::TX2, &PG_Disa::isTX2},
        PairType{Type::GEO, &PG_Disa::isGeo},
        PairType{Type::OLA, &PG_Disa::isOLA},
        PairType{Type::SH, &PG_Disa::isSH},
    };

    for (const auto [type, testFunc] : tests) {
        bool result = std::invoke(testFunc, reader);
        reader->seek(startPos);
        if (result) {
            return type;
        }
    }

    return UNKNOWN;
}

void PG_DisaFileInfo::determinTypeFromFileFormat(PG::STREAM::In* reader) {
    if (auto type = typeFromFileFormat(reader); type != UNKNOWN) {
        m_type = type;
    }
}