#include "DisaMapFunctions.h"

#include <array>
#include <cassert>
#include <cstring>
#include <ranges>

#include <FunctionPatcher.h>
#include <DisaMapChunk.h>
#include <DisaCharacterFunctions.h>
#include <Managers/DisaCustomCampaignManager.h>
#include <Managers/DisaCustomCharacterManager.h>
#include <Disa/fileformats/PG_MPD.h>
#include <Stream/PG_StreamInByteArray.h>
#include <Util/PG_DataSerializer.h>
#include <Util/Debug/PG_DebugOut.h>

static_assert(sizeof(DisaPartyCharacterInfo) == 0x6b8);
static_assert(offsetof(PG_DisaPartyCharacter, charName) == 0x248);

DisaMapUnit* DisaMapUnit::get(int index) {
    return &(&FP::getData<DisaMapUnit>(0x00738710))[index];
}

void __fastcall FUN_004d7970_clearMap(bool isFullClear) {
    //rewinds the buffer the MPP files are loaded into
    FP::getData<int*>(0x0067325c) = FP::getData<int*>(0x00668f10);

    for (int i = 0; i < 4; ++i) {
        DisaGeoModelSet::get(i)->isLoaded = false;
    }

    std::array<uint8_t, DisaMapChunk::count>& isChunkInWorldGrid = FP::getData<std::array<uint8_t, DisaMapChunk::count>>(0x0065db48);
    for (int i = 0; i < DisaMapChunk::count; ++i) {
        DisaMapChunk::get(i)->FUN_004a27f0_clearChunk(isFullClear);
        isChunkInWorldGrid[i] = 0;
    }

    if (isFullClear) FUN_004d3060_clearMapUnits();
}

void FUN_004d3060_clearMapUnits() {
    constexpr int temporaryCharacterCount = 64;
    const DisaPartyCharacterInfo* temporaryBegin = &FP::getData<DisaPartyCharacterInfo>(0x005fe0b0);
    const DisaPartyCharacterInfo* temporaryEnd = temporaryBegin + temporaryCharacterCount;

    for (DisaMapUnit& unit : std::ranges::subrange(DisaMapUnit::get(0), DisaMapUnit::get(DisaMapUnit::count))) {
        unit.isActive = false;
        //party members point into the party array and stay untouched
        if (unit.isTemporaryCharacter && unit.character >= temporaryBegin && unit.character < temporaryEnd) {
            unit.character->charName.fill(0);
            unit.character = nullptr;
        }
        unit.field0x280 = 1;
    }
}

void _stdcall FUN_004a59c0_LoadMPP(unsigned short param_1, unsigned short param_2) {

    int* mapAssetsLocation = FP::getData<int*>(0x0067325c);

    FP::callFastFarA(0x004a56b0, &FP::getData<int>(0x00703480));

    FP::getData<int>(0x00704bbd) = param_1;
    FP::getData<int>(0x00704bc0) = param_2;


    //BEGIN custom code
    if (param_1 >= 655) {
        DisaSettings* settings = (DisaSettings*)(FP::getData<unsigned int>(0x005d7c40));
        if (param_2 == 1) {
            DisaCustomCampaignManager::get()->readMPP(mapAssetsLocation, settings->isHDTextures);
        }
        else {
            //test map
            DisaCustomCampaignManager::get()->readMPPTestMap(mapAssetsLocation, settings->isHDTextures);
        }
    }
    //END custom code
    else {
        const int fileIndex = FP::callFastFarA<int>(0x0050a7b0, param_1, param_2);
        FUN_0050a1f0_GetFileData(fileIndex, &mapAssetsLocation, false);
    }

    FP::getData<int*>(0x007034b0) = mapAssetsLocation;


    FP::callFastFarA(0x004a5830, param_1, param_2);
    FP::callThisFarA(0x004a5ac0, &FP::getData<int>(0x00703480), 1);

    //FUN_0052376a(local_8 ^ (unsigned int)&stack0xfffffffc);
}

void __fastcall FUN_004d5060_LoadMPD(unsigned short chapterId, unsigned short mapId) {
    //LoadMPD actully has 4 arguments, BUT it dosn't clean its stack
    //however __fastcall is defined to be the one who cleans it
    //i fail to find any other convention that functions like __fastcall but dosn't clean the stack
    bool isHDTexturesInMPP, isDoNotUpdateActors;
    __asm {
        mov eax, [ebp + 8]     // Retrieve the third argument
        mov isHDTexturesInMPP, al

        mov eax, [ebp + 12]    // Retrieve the fourth argument
        mov isDoNotUpdateActors, al      
    }
    assert(FP::getData<bool>(0x00704bc8) == isHDTexturesInMPP);

    int* mapDataLocation = FP::getData<int*>(0x00668f14);
    //BEGIN custom code
    if (chapterId >= 655) {
        if (mapId == 1) {
            DisaCustomCampaignManager::get()->readMPD(mapDataLocation);
        }
        else {
            //test map
            DisaCustomCampaignManager::get()->readMPDTestMap(mapDataLocation);
            chapterId = 7;
        }
    }
    //END custom code
    else {
        const int fileIndex = FP::callFastFarA<int>(0x0050a390, chapterId, mapId);
        FUN_0050a1f0_GetFileData(fileIndex, &mapDataLocation, false);
        //FP::callFastFarA(0x0050a1f0, fileIndex, &mapDataLocation, 0, false); //this functions dosn't clean the stack?!
    }

    FP::getData<int*>(0x00668f14) = mapDataLocation;

    //FP::callFastFarA(0x004d4980, chapterId, mapId, 0, false);
    FUN_004d4980_CopyDungeonIntoActive(chapterId, mapId, isHDTexturesInMPP, isDoNotUpdateActors);

    //FP::callFastFarA(0x0052376a, FP::getData<unsigned int>(0x005ac900)); //check stack corruption (build in by compiler, now it does nothing)
}

void __fastcall FUN_004d4980_CopyDungeonIntoActive(int chapterId, int mapId, int isHDTexture, bool isDoNotLoadActors) {

    unsigned char* mpdFileData = FP::getData<unsigned char*>(0x00668f14); //MPD file data
    mpdfileHeader* headerFrom = (mpdfileHeader*)(mpdFileData);
    /*
    #include <Stream/PG_StreamInByteArray.h>
    PG::STREAM::InByteArray arrays((const char*)mpdFileData, 900000);
    PG_MPD debug;
    debug.open(&arrays);
     */

    const int number_of_chunks = headerFrom->number_of_chunks;
    FP::getData<int>(0x008acd40) = (int)headerFrom->number_of_actors;

    int* puVar9 = (int*)(mpdFileData + sizeof(mpdfileChunkDef) * number_of_chunks + 16);
    assert(puVar9 == (int*)((unsigned short*)mpdFileData + ((unsigned int)(number_of_chunks * 0x580) >> 1) + 8));

    mpdfileChunkHeader* chunkFrom = (mpdfileChunkHeader*)(mpdFileData + sizeof(mpdfileHeader));

    const int targetMemOffset = chunkFrom->index * 0x91a8; //mem offset

    int local_8 = *(int*)(&FP::getData<char>(0x007091c0) + targetMemOffset); //todo is never used o.0

    (&FP::getData<unsigned char>(0x00712366))[targetMemOffset] = isHDTexture;
    (&FP::getData<unsigned char>(0x007116a0))[targetMemOffset] = 1;

    chunkFrom->map_offset_w = 1.f;
    chunkFrom->wRot = 1.f;
    *(glm::vec4*)(&FP::getData<unsigned char>(0x007091d8) + targetMemOffset) = glm::vec4(chunkFrom->map_offset_x, chunkFrom->map_offset_y, chunkFrom->map_offset_z, chunkFrom->map_offset_w);
    *(glm::vec4*)(&FP::getData<unsigned char>(0x007091e8) + targetMemOffset) = glm::vec4(chunkFrom->xRot, chunkFrom->yRot, chunkFrom->zRot, chunkFrom->wRot);
    *(glm::vec3*)(&FP::getData<unsigned char>(0x00711684) + targetMemOffset) = glm::vec3(chunkFrom->map_offset2_x, chunkFrom->map_offset2_y, chunkFrom->map_offset2_z);


    (&FP::getData<unsigned char>(0x007116a1))[targetMemOffset] = *(mpdFileData + 0x41);
    (&FP::getData<unsigned char>(0x007116a3))[targetMemOffset] = 0;
    (&FP::getData<unsigned char>(0x00712365))[targetMemOffset] = *(mpdFileData + 0x511);

    {
        using ObjectArray = std::array<mpdfileChunkObject, 32>;
        ObjectArray* copyTo = (ObjectArray*)(&FP::getData<unsigned char>(0x007111fc) + targetMemOffset);
        ObjectArray* copyFrom = (ObjectArray*)(mpdFileData + 0x28 * 2);
        *copyTo = *copyFrom;  //memcpy(copyTo, copyFrom, sizeof(ObjectArray));
    }

    {
        using EventTileArray = std::array<mpdfileChunkEventTile, 16>;
        EventTileArray* copyTo = (EventTileArray*)(&FP::getData<unsigned char>(0x00712324) + targetMemOffset);
        EventTileArray* copyFrom = (EventTileArray*)(mpdFileData + 0x268 * 2);
        *copyTo = *copyFrom;  // memcpy(copyTo, copyFrom, sizeof(unsigned int) * 0x10); 
    }

    {

        mpdfileTile* copyTo = (mpdfileTile*)(&FP::getData<unsigned char>(0x007091fc) + targetMemOffset);
        mpdfileTile* copyFrom = (mpdfileTile*)puVar9;

        for (int i = 0; i < chunkFrom->number_of_tiles; ++i) {
            *(short*)(&FP::getData<unsigned char>(0x007116a4) +
                ((unsigned int)*(uint8_t*)((int)copyFrom + 0x72) + (unsigned int)*(uint8_t*)((int)copyFrom + 0x71) * 0x28) *
                2 + targetMemOffset) = (short)i + 1;

            memcpy(copyTo, copyFrom, sizeof(unsigned int) * (isDoNotLoadActors ? 0x1e : 0x20));

            ++copyTo;
            ++copyFrom;
        }
        puVar9 = (int*)copyFrom;
    }
    //skip other tiles to find actor address
    if (number_of_chunks > 1) {
        ++chunkFrom;
        for (int i = 0; i < (number_of_chunks - 1); ++i) {
            puVar9 = (int*)((char*)puVar9 + chunkFrom->number_of_tiles * sizeof(mpdfileTile));
            ++chunkFrom;
        }
    }
    FP::getData<int*>(0x008acd3c) = puVar9; //actors address

    //BEGIN custom code
    //remap custom actors id
    char* metaDataPtr = (char*)puVar9;
    metaDataPtr += headerFrom->number_of_actors * sizeof(mpdfileActor);
    if (*metaDataPtr == 'P' && *(metaDataPtr + 1) == 'G' && *(metaDataPtr + 2) == 'M' && *(metaDataPtr + 3) == 'D') {
        metaDataPtr += 4;
        PG::STREAM::InByteArray reader(metaDataPtr, 1024 * 1024 * 10);
        PG_DataSerializer metaData;
        if (!metaData.open(&reader)) {
            OUT_ERROR("Failed to read metadata!");
        }

        if (auto charsInfo = metaData["characters"]; charsInfo.isValid()) {
            mpdfileActor* charInfos = (mpdfileActor*)(puVar9);
            for (auto& charInfo : std::ranges::subrange(charInfos, charInfos + headerFrom->number_of_actors)) {
                if (!PG_DisaIdentifier::isCustomId(charInfo.id)) continue;
                //is custom character
                PG_DisaIdentifier identifier(charsInfo[std::to_string(charInfo.id)].asString());

                auto info = DisaCustomCharacterManager::get()->getDisplayInfo(identifier);
                if (info.characterId == 0) {
                    OUT_ERROR("Failed to find custom character \"" << identifier.getIdentifier() << "\"! Replacing with prinny!");
                    charInfo.id = 2210;
                    continue;
                }
                charInfo.id = info.characterId;
            }
        }
    }
    //END custom code

    FP::callFastFarA(0x004a6070, 0x703480);

    //reset some scripts? TODO there is some strange struff going on with "extraout_ECX"
    char* thiss = &FP::getData<char>(0x007091c0);
    do {
        FP::callThisFarA(0x004a3ab0, thiss, 0, '\0');
        thiss = thiss + 0x91a8;
    } while ((int)thiss < 0x71b510);

    FP::callFastFarA(0x004d50c0);


    {//find teleport pannel
        using EventTileArray = std::array<mpdfileChunkEventTile, (0x712364 - 0x00712324) / sizeof(mpdfileChunkEventTile)>;
        EventTileArray& data = FP::getData<EventTileArray>(0x00712324);
        if (auto findIt = std::ranges::find(data, 1, &mpdfileChunkEventTile::index); findIt != data.end()) {
            FP::getData<float>(fTeleportPannelX) = (float)((unsigned int)findIt->x * 0xc);
            FP::getData<float>(fTeleportPannelZ) = (float)((unsigned int)findIt->z * 0xc);
            findIt->index = 0;
        }
    }

    //override starting point 
    if (!((FP::getData<char>(0x0065db3a) != '\x05') || (mapId - 1U > 8))) {

        uint8_t local_20[4] = { 0,0,0,0 };
        uint8_t local_18[4] = { 0,0,0,0 };
        long long index;

        switch (mapId) {
        case 1:
            local_20[0] = 6;
            local_20[1] = 6;
            local_20[2] = 0xb;
            local_20[3] = 0xb;

            local_18[0] = 0xf;
            local_18[1] = 0xf;
            local_18[2] = 0x11;
            local_18[3] = 0xb;

            index = FP::callFastFarA<long long>(0x0046e1a0, 3);
            FP::getData<float>(fTeleportPannelX) = (float)((char)local_20[(char)index * 2] * 0xc);
            FP::getData<float>(fTeleportPannelZ) = (float)((char)local_20[(char)index * 2 + 1] * 0xc);
            index = FP::callFastFarA<long long>(0x0046e1a0, 3);
            FP::getData<float>(0x006e57a0) = (float)(*(char*)((int)&local_18 + (char)index * 2) * 0xc);
            FP::getData<float>(0x006e57a4) = (float)(*(char*)((int)&local_18 + (char)index * 2 + 1) * 0xc);
            break;
        case 2:
            local_20[0] = 10;
            local_20[1] = 5;
            local_20[2] = 6;
            local_20[3] = 6;
            index = FP::callFastFarA<long long>(0x0046e1a0, 3);
            FP::getData<float>(fTeleportPannelX) = (float)((char)local_20[(char)index * 2] * 0xc);
            FP::getData<float>(fTeleportPannelZ) = (float)((char)local_20[(char)index * 2 + 1] * 0xc);
            FP::getData<float>(0x006e57a0) = 120.0;
            FP::getData<float>(0x006e57a4) = 228.0;
            break;
        case 3: {
            //TODO not sure whats going on here

            local_20[0] = 10;
            local_20[1] = 4;
            local_20[2] = 5;
            local_20[3] = 0xb;

            local_18[0] = 0xe;
            local_18[1] = 0xb;
            local_18[2] = 9;
            local_18[3] = 0x13;

            index = FP::callFastFarA<long long>(0x0046e1a0, 2);
            FP::getData<float>(fTeleportPannelX) = (float)((char)local_20[(char)index * 2] * 0xc);
            FP::getData<float>(fTeleportPannelZ) = (float)((char)local_20[(char)index * 2 + 1] * 0xc);
            index = FP::callFastFarA<long long>(0x0046e1a0, 2);
            FP::getData<float>(0x006e57a0) = (float)(*(char*)((int)&local_18 + (char)index * 2) * 0xc);
            FP::getData<float>(0x006e57a4) = (float)(*(char*)((int)&local_18 + (char)index * 2 + 1) * 0xc);

        } break;
        case 4:
            local_20[0] = 5;
            local_20[1] = 6;
            local_20[2] = 0x13;
            local_20[3] = 0x12;

            local_18[0] = 5;
            local_18[1] = 0x12;
            local_18[2] = 0x13;
            local_18[3] = 6;

            index = FP::callFastFarA<long long>(0x0046e1a0, 3);
            FP::getData<float>(fTeleportPannelX) = (float)((char)local_20[(char)index * 2] * 0xc);
            FP::getData<float>(fTeleportPannelZ) = (float)((char)local_20[(char)index * 2 + 1] * 0xc);
            index = FP::callFastFarA<long long>(0x0046e1a0, 3);
            FP::getData<float>(0x006e57a0) = (float)(*(char*)((int)&local_18 + (char)index * 2) * 0xc);
            FP::getData<float>(0x006e57a4) = (float)(*(char*)((int)&local_18 + (char)index * 2 + 1) * 0xc);
            break;
        case 5:
            local_20[0] = 7;
            local_20[1] = 7;
            local_20[2] = 10;
            local_20[3] = 8;

            local_18[0] = 0x11;
            local_18[1] = 0x11;
            local_18[2] = 0x10;
            local_18[3] = 8;

            index = FP::callFastFarA<long long>(0x0046e1a0, 3);
            FP::getData<float>(fTeleportPannelX) = (float)((char)local_20[(char)index * 2] * 0xc);
            FP::getData<float>(fTeleportPannelZ) = (float)((char)local_20[(char)index * 2 + 1] * 0xc);
            index = FP::callFastFarA<long long>(0x0046e1a0, 3);
            FP::getData<float>(0x006e57a0) = (float)(*(char*)((int)&local_18 + (char)index * 2) * 0xc);
            FP::getData<float>(0x006e57a4) = (float)(*(char*)((int)&local_18 + (char)index * 2 + 1) * 0xc);
            break;
        case 6:
            local_20[0] = 6;
            local_20[1] = 6;
            local_20[2] = 0xf;
            local_20[3] = 0x10;

            index = FP::callFastFarA<long long>(0x0046e1a0, 4);
            FP::getData<float>(fTeleportPannelX) = (float)((char)local_20[(char)index * 2] * 0xc);
            FP::getData<float>(fTeleportPannelZ) = (float)((char)local_20[(char)index * 2 + 1] * 0xc);
            FP::getData<float>(0x006e57a0) = 120.0;
            FP::getData<float>(0x006e57a4) = 108.0;
            break;
        case 7:
            local_20[0] = 7;
            local_20[1] = 7;
            local_20[2] = 0x12;
            local_20[3] = 0x12;

            local_18[0] = 0x12;
            local_18[1] = 7;
            local_18[2] = 7;
            local_18[3] = 0x12;

            index = FP::callFastFarA<long long>(0x0046e1a0, 3);
            FP::getData<float>(fTeleportPannelX) = (float)((char)local_20[(char)index * 2] * 0xc);
            FP::getData<float>(fTeleportPannelZ) = (float)((char)local_20[(char)index * 2 + 1] * 0xc);
            index = FP::callFastFarA<long long>(0x0046e1a0, 3);
            FP::getData<float>(0x006e57a0) = (float)(*(char*)((int)&local_18 + (char)index * 2) * 0xc);
            FP::getData<float>(0x006e57a4) = (float)(*(char*)((int)&local_18 + (char)index * 2 + 1) * 0xc);
            break;
        case 8:
            local_20[0] = 0x10;
            local_20[1] = 9;
            local_20[2] = 8;
            local_20[3] = 0xd;

            local_18[0] = 7;
            local_18[1] = 0x12;
            local_18[2] = 0xf;
            local_18[3] = 0xe;

            index = FP::callFastFarA<long long>(0x0046e1a0, 3);
            FP::getData<float>(fTeleportPannelX) = (float)((char)local_20[(char)index * 2] * 0xc);
            FP::getData<float>(fTeleportPannelZ) = (float)((char)local_20[(char)index * 2 + 1] * 0xc);
            index = FP::callFastFarA<long long>(0x0046e1a0, 4);
            FP::getData<float>(0x006e57a0) = (float)(*(char*)((int)&local_18 + (char)index * 2) * 0xc);
            FP::getData<float>(0x006e57a4) = (float)(*(char*)((int)&local_18 + (char)index * 2 + 1) * 0xc);
            break;
        case 9:
            local_20[0] = 7;
            local_20[1] = 7;
            local_20[2] = 0xc;
            local_20[3] = 10;

            local_18[0] = 0xc;
            local_18[1] = 0x14;
            local_18[2] = 0x11;
            local_18[3] = 7;

            index = FP::callFastFarA<long long>(0x0046e1a0, 3);
            FP::getData<float>(fTeleportPannelX) = (float)((char)local_20[(char)index * 2] * 0xc);
            FP::getData<float>(fTeleportPannelZ) = (float)((char)local_20[(char)index * 2 + 1] * 0xc);
            index = FP::callFastFarA<long long>(0x0046e1a0, 3);
            FP::getData<float>(0x006e57a0) = (float)(*(char*)((int)&local_18 + (char)index * 2) * 0xc);
            FP::getData<float>(0x006e57a4) = (float)(*(char*)((int)&local_18 + (char)index * 2 + 1) * 0xc);
            break;
        }

    }



    //load actors
    if (!isDoNotLoadActors) {
        if (chapterId == 250) {
            FP::callFastFarA(0x004d5140, 1);
            FP::callStdFarA(0x004d5e20);
            return;
        }
        if (chapterId == 210) {
            FP::callFastFarA(0x004d5140, 0);
            return;
        }
        FP::callFastFarA(0x004d5140, 1);

        //BEGIN custom code
        //fix loaded characters (ids are already remapped)
        DisaPartyCharacterInfo* charInfos = FP::getData<DisaPartyCharacterInfo*>(0x00738994);
        if (!charInfos) return;
        unsigned int numberOfActors = FP::getData<unsigned int>(0x008acd40);
        for (auto& charInfo : std::ranges::subrange(charInfos, charInfos + numberOfActors)) {
            if (!PG_DisaIdentifier::isCustomId(charInfo.characterId)) continue;
            const auto& info = DisaCustomCharacterManager::get()->getCharacterInfoIndex(charInfo.characterId - PG_DisaIdentifier::customIdStart);
            if (!info.identifier) continue;
            auto displayInfo = DisaCustomCharacterManager::get()->getDisplayInfo(info.identifier);

           
            //is custom character
            if (displayInfo.spriteId > 0) charInfo.spriteId = displayInfo.spriteId;
            if (displayInfo.faceId > 0) charInfo.setCustomPortraitId(displayInfo.faceId);
        }
        //END custom code
    }
    return;


}

void __fastcall FUN_004d6a30_finalizeDungeonLoad(DisaDungeon* dungeon) {

    FP::callFastFarA(0x004d6a30, dungeon);
    return;
    short sVar1;
    unsigned int uVar2;
    int iVar3;
    unsigned int extraout_ECX;
    unsigned int extraout_ECX_00;
    unsigned int extraout_ECX_01;
    unsigned int extraout_ECX_02;
    int extraout_ECX_03;
    int extraout_ECX_04;
    unsigned int extraout_EDX;
    unsigned int extraout_EDX_00;
    int extraout_EDX_01;
    int extraout_EDX_02;
    int extraout_EDX_03;
    int extraout_EDX_04;
    int extraout_EDX_05;
    int uVar4;
    int extraout_EDX_06;
    int* puVar5;
    int iVar6;
    unsigned long long uVar7;
    unsigned long long uVar8;
    unsigned long long uVar9;
    unsigned long long uVar10;
    unsigned int dungeonCategoryId;

    dungeonCategoryId = dungeon->id / 100;
    int uVar12 = 0x4d6a53;
    uVar7 = FP::callFastFarA<unsigned int>(0x004a1a20, (unsigned int)dungeon->id, dungeonCategoryId);
    FP::getData<int>(0x006b7588) = 1;
    uVar7 = FP::callFastFarA<unsigned long long>(0x00523f90, uVar7);
    iVar6 = (int)uVar7;
    uVar7 = FP::callFastFarA<unsigned long long>(0x00523f90, extraout_ECX_00, (unsigned int)(uVar7 >> 0x20));
    FP::callFastFarA(0x004a23e0, &FP::getData<int>(0x00712368), (int)uVar7, iVar6);
    FP::getData<int>(0x0071a84b) = 3;
    puVar5 = &FP::getData<int>(0x007091c0);
    FP::getData<int>(0x0065db49) = 0;
    do {
        FP::callFastFarA(0x004a3620, (int)puVar5);
        uVar7 = FP::callFastFarA<unsigned long long>(0x00523f90, extraout_ECX_01, extraout_EDX_00);
        iVar6 = (int)uVar7;
        uVar7 = FP::callFastFarA<unsigned long long>(0x00523f90, extraout_ECX_02, (unsigned int)(uVar7 >> 0x20));
        FP::callFastFarA(0x004a3410, puVar5, (int)(uVar7 >> 0x20), (short*)uVar7, iVar6);
        puVar5 = puVar5 + 0x91a8;
    } while ((int)puVar5 < 0x71b510);
    FP::callFastFarA(0x004d50c0, extraout_ECX_03, extraout_EDX_01);
    uVar4 = extraout_EDX_02;
    if ((dungeonCategoryId != 0x33) && (dungeonCategoryId != 0xd2)) {
        FP::callFastFarA(0x004d2960);
        uVar4 = extraout_EDX_03;
        sVar1 = FP::getData<short>(0x0065db50);
        for (uVar2 = (unsigned int)(FP::getData<int>(0x0065db50) != 0); uVar2 != 0; uVar2 = uVar2 - 1) {
            iVar6 = 1000;
            do {
                iVar6 = iVar6 + -1;
                if (iVar6 < 1) goto LAB_004d6bc4;
                uVar8 = FP::callFastFarA<unsigned long long>(0x0046e1a0, 0x40);
                uVar9 = FP::callFastFarA<unsigned long long>(0x0046e1a0, 0x40);
                iVar3 = FP::callFastFarA<int>(0x004d9b50, (int)uVar8, (int)uVar9);
                uVar4 = extraout_EDX_04;
            } while (iVar3 != 0);
            uVar4 = extraout_ECX_04;
            uVar10 = FP::callFastFarA<unsigned long long>(0x0046e000, 4);
            iVar6 = FP::callFastFarA<int>(0x004d30e0, 0x8fc, '\x02', (int)sVar1, (float)((int)uVar8 * 0xc + 6), 0,
                (float)((int)uVar9 * 0xc + 6), (short)uVar10 * 0x5a + -0x5a, uVar4, iVar3);
            sVar1 = 0;
            FP::getData<int>(0x0065db50) = 0;
            (&FP::getData<float>(0x00738988))[iVar6 * 0x42c] = 1;
            uVar4 = extraout_EDX_05;
        LAB_004d6bc4:
            int i = 0;
        }
    }
    FP::callFastFarA(0x004f8500, (float*)&FP::getData<float>(0x00730980), uVar4, -0.52359, 0.785385, 0.0, 0.0);
    FP::getData<unsigned int>(0x00730a54) = 0;
    FP::getData<float>(0x00730a28) = (float)FP::getData<unsigned int>(0x0065db22);
    FP::getData<unsigned int>(0x00730a4c) = FP::getData<unsigned int>(0x00730a28);
    FP::callFastFarA(0x004f8800, (float*)&FP::getData<int>(0x00730980), extraout_EDX_06);
    FP::callFastFarA(0x0046cab0, &FP::getData<int>(0x00730ba0), FP::getData<float>(fTeleportPannelX) + 6.0, 0.0, FP::getData<float>(fTeleportPannelZ) + 6.0, 0.0);

}
