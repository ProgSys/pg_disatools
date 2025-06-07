#include "DisaDataFunctions.h"
#include "FunctionPatcher.h"
#include <Managers/DisaCustomBaseModManager.h>
#include <Managers/DisaCustomCharacterManager.h>
#include <ranges>

#include <Util/PG_Dir.h>
#include <Util/PG_StringUtil.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_DataSerializer.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamInOutByteVector.h>
#include <Stream/PG_StreamInByteArray.h>
#include <Disa/fileformats/PG_ShiftJisEncoder.h>

std::pair<unsigned short, const char*> DisaSpriteOffsetListArchiveData::getDataEntry(int index) const {
    return {
        getIdsBegin()[index],
        (const char*)this + getAddressBegin()[index] //resolved address
    };
}

char* __fastcall FUN_004de880_getSpriteData(unsigned int spriteId) {
    //CUSTOM CODE START
    if (char* cunstomSpriteAddress = DisaCustomCharacterManager::get()->getSpriteData(spriteId)) {
        return cunstomSpriteAddress;
    }
    //CUSTOM CODE END

    DisaSpriteLoaderInfo* spriteBucketBegin = &FP::getData<DisaSpriteLoaderInfo>(0x00749afc - sizeof(DisaSpriteLoaderInfo) * 3); //0x00749abf
    DisaSpriteLoaderInfo* spriteBucketEnd = &FP::getData<DisaSpriteLoaderInfo>(0x00749afc);
    //filled at 004B80DB-> FUN_004de740 -> FUN_004de780
    for (DisaSpriteLoaderInfo& info : std::ranges::subrange(spriteBucketBegin, spriteBucketEnd + 1) | std::views::reverse ) {
        if (info.count == 0) continue;
        auto idsRange = std::ranges::subrange(info.spriteIds, info.spriteIds + info.count);
        auto findIt = std::ranges::find(idsRange, spriteId);
        if (findIt == idsRange.end()) continue;
        //each processed data has a fixed length of 0x1200
        const unsigned int offset = std::distance(idsRange.begin(), findIt) * 0x1200;
        return info.data + offset;
    }

    return nullptr;
}

void DisaSpriteLoaderInfo::FUN_004de780_processANM(int param_2) {
	FP::callThisFarA<void>(0x004de780, this, param_2);
}

// CONCAT44 is a concatenation operation for two 32-bit integers
static inline uint32_t CONCAT31(uint32_t x, uint32_t y) {
    return (x << 16) | (y & 0xFF);
}



PACK(struct SaveHeader {
    unsigned short field_0x0;
    unsigned char field_0x2;
    unsigned char field_0x3;
    unsigned int field_0x4;
    unsigned int fileSize;
    unsigned char field_0x0C;
    std::array<char, 35> str;

    unsigned char field_0x30;
    unsigned char field_0x31;

    std::string getName() const {
        return shiftJisToString(toShiftJisString(str));
    }
};);

bool __stdcall FUN_00442260_Save(char* data, size_t fileSize, char* filename) {
    /*
    * Uncopressed save file
    PG::STREAM::OutByteFile writer("D:/Steam/SteamApps/common/Disgaea PC/work2/save.dat");
    writer.write(data, fileSize);
    writer.close();
    */

    char* compressedData = nullptr;
    size_t compressedSize = 0;

    { //BEGIN custom code
        PG_DataSerializer metaData;

        const auto addCharacter = [&metaData](unsigned short characterId) {
            if (!PG_DisaIdentifier::isCustomId(characterId)) return;
            PG_DisaIdentifier identifier = DisaCustomCharacterManager::get()->getCharacterIdentifier(characterId);
            if (!identifier) return;
            auto charMeta = metaData["characters"];
            if (auto info = charMeta[std::to_string(characterId)]; !info.isSet()) {
                info = identifier.getIdentifier();
            }
         };

        {
            auto* playerUnits = (PG_DisaPartyCharacter*)(data + 3000);
            auto playerUnitsSize = *(unsigned short*)(data + 3000 + 277624);
            for (const PG_DisaPartyCharacter& c : std::ranges::subrange(playerUnits, playerUnits + playerUnitsSize)) {
                addCharacter(c.characterId);
            }
        }

        {
            const auto& assemblyUnitInfos = *(std::array<DisaAssemblyUnitInfo, 288>*)(data + 696);
            for (const auto disaAssemblyUnitInfo : assemblyUnitInfos) {
                addCharacter(disaAssemblyUnitInfo.characterId);
            }
        }

        if (!metaData.isEmpty()) {
            PG::STREAM::InOutByteVector customData;
            customData.write(data, fileSize);
            customData.writeString("PGMD");
            metaData.save(&customData);
            customData.writeInt(fileSize);
            FP::callFastFarA(0x0043e490, &compressedData, &compressedSize, customData.getData().data(), customData.size()); //compress data
        }
        else {  //END custom code
            FP::callFastFarA(0x0043e490, &compressedData, &compressedSize, data, fileSize); //compress data
        }
    } 

    FP::callThisFarA<unsigned int>(0x005064d0, FP::getData<int*>(0x006e80ec), filename, compressedData, compressedSize);
    if (compressedData != nullptr) {
        FP::callFastFarA<unsigned int>(0x00433380, compressedData);
    }  
    return true;
}

bool __fastcall FUN_0043e600_decompressSave(char*& decompressedData, uint& decompressedSize, char* data) {
    struct DisaCompressedSaveHeader{
        std::array<uint, 8> field_0x0;
        uint decruptKey;
        uint field_0x24;
        uint data4ByteSize; //0x28 is dataByteSize/4
        uint dataByteSize; //0x30
        //next file data
    };
    DisaCompressedSaveHeader* header = (DisaCompressedSaveHeader*)(data);
    unsigned int size = header->data4ByteSize * 4;
    char* _Dst = FP::callFastFarA<char*>(0x004333f0, size); //alloc
    memcpy(_Dst, data + 0x30, size);

    uint decruptSize = (uint)(_Dst + size + (3 - (int)_Dst)) >> 2;
    uint decruptSize2 = (size + 3) / 4;
    if (_Dst + size < _Dst) {
        decruptSize = 0;
    }

    const uint decruptKey = header->decruptKey;
    for (uint& it : std::ranges::subrange( (uint*)_Dst, ((uint*)_Dst) + decruptSize)) {
        it = it ^ decruptKey;
    }

    FP::callVariadicThisFarA<void, char*, char*&, uint&, uint>(0x00415600, _Dst, decompressedData, decompressedSize, 0xffffffff);
    if (decompressedData){ //BEGIN custom code
        unsigned int customDataStart = *(unsigned int*)(decompressedData + decompressedSize - sizeof(unsigned int));
        if (customDataStart < decompressedSize && customDataStart > 3000 &&
            decompressedData[customDataStart] == 'P' && decompressedData[customDataStart + 1] == 'G' && decompressedData[customDataStart + 2] == 'M' && decompressedData[customDataStart + 3] == 'D') {
            customDataStart += 4;
            PG::STREAM::InByteArray reader(decompressedData + customDataStart, decompressedSize - customDataStart);
            decompressedSize = customDataStart - 4;
            PG_DataSerializer metaData;
            metaData.open(&reader);

            auto charMeta = metaData["characters"];
            if(charMeta.isValid()){
                auto* playerUnits = (PG_DisaPartyCharacter*)(decompressedData + 3000);
                auto playerUnitsSize = *(unsigned short*)(decompressedData + 3000 + 277624);
                for (PG_DisaPartyCharacter& c : std::ranges::subrange(playerUnits, playerUnits + playerUnitsSize)) {
                    if (!PG_DisaIdentifier::isCustomId(c.characterId)) continue;

                    PG_DisaIdentifier identifier(charMeta[std::to_string(c.characterId)].asString());
                    auto displayInfo = DisaCustomCharacterManager::get()->getDisplayInfo(identifier);

                    if (displayInfo.characterId == 0) {
                        OUT_ERROR("Save file contains unknown character \"" << identifier.getIdentifier() << "\"!");
                        c.characterId = 2210;
                        c.spriteId = 2210;
                        c.paletteIndex = 0;
                        c.portraitColor = 5;
                        c.portraitImageId = 43;
                        c.charDatIndex = 0;
                        continue;
                    }

                    c.characterId = displayInfo.characterId;
                    c.spriteId = displayInfo.spriteId;
                    c.setCustomPortraitId(displayInfo.faceId);
                    c.charDatIndex = displayInfo.charDatIndex;
                }

                {
                    auto& assemblyUnitInfos = *(std::array<DisaAssemblyUnitInfo, 288>*)(decompressedData + 696);
                    for (auto& disaAssemblyUnitInfo : assemblyUnitInfos) {
                        if (!PG_DisaIdentifier::isCustomId(disaAssemblyUnitInfo.characterId)) continue;
                        PG_DisaIdentifier identifier(charMeta[std::to_string(disaAssemblyUnitInfo.characterId)].asString());
                        auto displayInfo = DisaCustomCharacterManager::get()->getDisplayInfo(identifier);
                        if (displayInfo.characterId == 0) {
                            OUT_ERROR("Save file contains unknown character \"" << identifier.getIdentifier() << "\"!");
                            disaAssemblyUnitInfo.characterId = 2210;
                        }
                        else {
                            disaAssemblyUnitInfo.characterId = displayInfo.characterId;
                        }
                    }
                }
            }
        }

    } //END custom data


    FP::callFastFarA(0x00433380, _Dst);
    return decompressedData != nullptr;
}

__declspec(naked) void FUN_0043e600_decompressSave_NoneStd(char* data) {
    //for some reason the original assebly expects __fastcall to NOT destroy the stack
    __asm {
        push[esp + 4]

        call FUN_0043e600_decompressSave
        ret
    }
}