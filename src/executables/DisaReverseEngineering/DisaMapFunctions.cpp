#include "DisaMapFunctions.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstring>
#include <ranges>

#include <FunctionPatcher.h>
#include <DisaMapChunk.h>
#include <DisaMath.h>
#include <DisaCharacterFunctions.h>
#include <Managers/DisaCustomCampaignManager.h>
#include <Managers/DisaCustomModificationsManager.h>
#include <Disa/modifications/PG_DisaModCharactersModel.h>
#include <Managers/DisaCustomRenderManager.h>
#include <Disa/fileformats/PG_MPD.h>
#include <Disa/fileformats/PG_MPDCustom.h>
#include <Resources/PG_ResourceName.h>
#include <Stream/PG_StreamInByteArray.h>
#include <Util/PG_DataSerializer.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_MathGeometry.h>

static_assert(sizeof(DisaPartyCharacterInfo) == 0x6b8);
static_assert(offsetof(PG_DisaPartyCharacter, charName) == 0x248);

DisaMapUnit* DisaMapUnit::get(int index) {
    return &(&FP::getData<DisaMapUnit>(0x00738710))[index];
}

void DisaMapUnit::FUN_0046cab0_translateTo(float x, float y, float z, float frames) {
    moveTarget = glm::vec3(x, y, z);
    if (frames == 0.f) {
        position = moveTarget;
        moveStep = glm::vec3(0.f);
        moveFramesLeft = 0;
        return;
    }
    /* Closer to disassembly
    const float inverseFrames = static_cast<float>(1.0 / frames);
    moveStep = glm::vec3(
        static_cast<float>((double(x) - position.x) * inverseFrames),
        static_cast<float>((double(y) - position.y) * inverseFrames),
        static_cast<float>((double(z) - position.z) * inverseFrames));
    moveFramesLeft = static_cast<int16_t>(static_cast<int>(frames));
    */
    const float inverseFrames = 1.f / frames;
    moveStep = (glm::vec3(x,y,z) - position) * inverseFrames;
    moveFramesLeft = static_cast<int16_t>(frames);
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
    //models from the resources of the campaign, drawn where the marker object of the chunk is
    std::vector<PG_MPDCustomModel> customModels;

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

                auto info = DisaCustomModificationsManager::get()->getDisplayInfo(identifier);
                if (info.characterId == 0) {
                    OUT_ERROR("Failed to find custom character \"" << identifier.getIdentifier() << "\"! Replacing with prinny!");
                    charInfo.id = 2210;
                    continue;
                }
                charInfo.id = info.characterId;
            }
        }

        //only the first chunk of the file is copied, so only its models count
        customModels = PG_MPDCustom::readModels(metaData, 0);
    }

    std::vector<DisaCustomMapModel> loadedModels;
    if (!customModels.empty()) {
        const std::string nameSpace = DisaCustomCampaignManager::get()->getActiveResourceNameSpace();
        if (nameSpace.empty()) {
            OUT_WARNING("The map has " << customModels.size() << " custom models but no campaign is active to load them from!");
        } else {
            loadedModels.reserve(customModels.size());
            DisaCustomRenderManager* renderManager = DisaCustomRenderManager::get();
            for (const PG_MPDCustomModel& customModel : customModels) {
                loadedModels.push_back({ PG_Model(PG_ResourceName::join(nameSpace, customModel.name)), customModel.object, renderManager->createUniqueNodeId() });
            }
        }
    }
    //an empty list too, so a map without custom models replaces those of the last one.
    //chunkFrom was moved on to find the actors, the chunk copied into is the one targetMemOffset names
    getCustomMapModels()[targetMemOffset / static_cast<int>(sizeof(DisaMapChunk))] = std::move(loadedModels);
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
            const PG_DisaIdentifier identifier = DisaCustomModificationsManager::get()->getCharactersModel()->getCharacterIdentifier(charInfo.characterId);
            if (!identifier) continue;
            auto displayInfo = DisaCustomModificationsManager::get()->getDisplayInfo(identifier);

           
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

float __fastcall FUN_004d2be0_getGroundHeight(int* chunkIndex, int unused, float x, float z) {
    const auto& isChunkInWorldGrid = FP::getData<std::array<uint8_t, DisaMapChunk::count>>(0x0065db48);

    float lowest = 1000000.f;
    int lowestIndex = -1;
    for (int i = 0; i < DisaMapChunk::count; ++i) {
        const DisaMapChunk* chunk = DisaMapChunk::get(i);
        if (!chunk->isActive || !isChunkInWorldGrid[i]) continue;

        const float height = chunk->FUN_004a2af0_getHeight(x, z);
        if (height < lowest) {
            lowest = height;
            lowestIndex = i;
        }
    }

    if (chunkIndex) *chunkIndex = lowestIndex;
    return lowestIndex >= 0 ? lowest : 0.f;
}

glm::vec3* __fastcall FUN_004d1070_screenToWorld(glm::vec3* out, int screenX, int screenY, float height, bool followTerrain) {
    const glm::mat4 inverse = glm::inverse(FP::getData<glm::mat4>(0x00819200));
    const auto unproject = [&inverse, screenX, screenY](float depth) {
        const glm::vec4 point = inverse * glm::vec4(static_cast<float>(screenX), static_cast<float>(screenY), depth, 1.f);
        return glm::vec3(point) / point.w;
    };
    const glm::vec3 nearPoint = unproject(0.f);
    const glm::vec3 rayDirection = glm::normalize(nearPoint - unproject(1.f));

    const float rayLength = -nearPoint.y / rayDirection.y; //cut with the plane y = 0
    const glm::vec3 groundPoint = nearPoint + rayDirection * rayLength;

    const glm::vec3 viewDirection = glm::normalize(groundPoint - FP::getData<DisaCameraInfo>(0x00730980).eye);

    if (followTerrain) {
        //CUSTOM CODE START
        //intersects the drawn tile faces instead of sampling the height, falls back to the sampling when nothing is hit
        static bool customSolver = true;
        if (customSolver) {
            const auto& isChunkInWorldGrid = FP::getData<std::array<uint8_t, DisaMapChunk::count>>(0x0065db48);
            const glm::vec3 intoScene = -rayDirection;

            std::optional<DisaMapChunk::CustomRayHit> nearestFace;
            for (int i = 0; i < DisaMapChunk::count; ++i) {
                const DisaMapChunk* chunk = DisaMapChunk::get(i);
                if (!chunk->isActive || !isChunkInWorldGrid[i]) continue;
                if (auto face = chunk->customIntersectRay(nearPoint, intoScene); face && (!nearestFace || face->distance < nearestFace->distance)) nearestFace = face;
            }

            std::optional<float> nearest;
            if (nearestFace) nearest = nearestFace->distance;
            else nearest = PG_MathGeometry::intersectPlane(nearPoint, intoScene, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));

            if (nearest) {
                const glm::vec3 hit = nearPoint + intoScene * *nearest;
                *out = glm::vec3(hit.x, -hit.y, hit.z); //like the original, the height is returned with the sign of the drawn height flipped

                /* A side face lies exactly on the edge between two tiles, so the cursor flips between them. The wall
                 * belongs to the higher tile, so the hit is moved into it and onto its ground. */
                if (nearestFace && std::abs(nearestFace->normal.y) < 0.5f) {
                    constexpr float offset = 1.f;
                    const glm::vec2 across = glm::normalize(glm::vec2(nearestFace->normal.x, nearestFace->normal.z)) * offset;
                    const glm::vec2 sideA = glm::vec2(hit.x, hit.z) + across;
                    const glm::vec2 sideB = glm::vec2(hit.x, hit.z) - across;
                    const float heightA = FUN_004d2be0_getGroundHeight(nullptr, 0, sideA.x, sideA.y);
                    const float heightB = FUN_004d2be0_getGroundHeight(nullptr, 0, sideB.x, sideB.y);
                    //negative is up
                    const glm::vec2 higher = glm::distance2(heightA, hit.y) < glm::distance2(heightB, hit.y) ? sideA : sideB;
                    *out = glm::vec3(higher.x, -hit.y, higher.y);
                }
                OUT_DEBUG("Hit:" << *out);
                return out;
            }
        }
        //CUSTOM CODE END

        //no wonder the mouse feels so bad, they just kinda gessing it...
        const glm::dvec3 step = glm::dvec3(viewDirection.x, -viewDirection.y, viewDirection.z) * 2.0;
        constexpr double tolerance = 0.6f;
        for (int i = 100; i > 0; --i) {
            const float stepCount = static_cast<float>(i);
            const float sampleX = static_cast<float>(groundPoint.x - step.x * stepCount);
            const float rayHeight = static_cast<float>(step.y * stepCount);
            const float sampleZ = static_cast<float>(groundPoint.z - step.z * stepCount);

            const float groundHeight = FUN_004d2be0_getGroundHeight(nullptr, 0, sampleX, sampleZ);
            if (groundHeight >= rayHeight - tolerance && groundHeight <= rayHeight + tolerance) {
                *out = glm::vec3(sampleX, -rayHeight, sampleZ);
                OUT_DEBUG("Hit:" << *out);
                return out;
            }
        }
    }

    //whole steps, the original truncates the distance to an integer
    const float steps = static_cast<float>(static_cast<int>(-height / double(viewDirection.y)));
    *out = glm::vec3(static_cast<float>(groundPoint.x - double(steps) * viewDirection.x), height, static_cast<float>(groundPoint.z - double(steps) * viewDirection.z));
    return out;
}

__declspec(naked) void FUN_004d1070_screenToWorld_NoneStd(int screenY, float height, bool followTerrain) {
    //out and screenX stay in ECX and EDX, the stack arguments are pushed again for the fastcall to pop
    __asm {
        push [esp + 12]
        push [esp + 12]
        push [esp + 12]
        call FUN_004d1070_screenToWorld
        ret
    }
}

void __fastcall FUN_004d1500_updateMapCursor(DisaCameraInfo* camera, DisaMapUnit* cursor, int unused, char useRecordedInput) {
    if (cursor->moveFramesLeft != 0) {
        cursor->velocity.x = 0.f;
        cursor->velocity.z = 0.f;
        return;
    }

    //map bounds in tiles
    const int minX = FP::getData<int>(0x00664770);
    const int minZ = FP::getData<int>(0x00664774);
    const int maxX = FP::getData<int>(0x00664768);
    const int maxZ = FP::getData<int>(0x0066476c);
    const uint32_t buttons = FP::getData<uint32_t>(0x00778d64);
    const auto recordFastMove = []() {
        uint8_t& recorded = FP::getData<uint8_t>(0x006d4aca);
        recorded += (recorded != 0) * 10;
    };

    const float cursorX = cursor->position.x;
    const float cursorZ = cursor->position.z;
    const int tileX = static_cast<int>(cursorX * double(1.f / 12.f));
    const int tileZ = static_cast<int>(cursorZ * double(1.f / 12.f));
    const int centerX = tileX * 12 + 6;
    const int centerZ = tileZ * 12 + 6;

    const int direction = FP::callFastFarA<int>(0x004d0db0, camera, tileZ, static_cast<int>(useRecordedInput));
    const bool isStickLocked = (FP::getData<uint8_t>(0x006194d9) & 1) != 0;

    const DisaSettings* settings = FP::getData<DisaSettings*>(0x005d7c40);
    if (direction == -1 && !isStickLocked && settings->isMouseActive && !FP::callFarA<bool>(0x0050ff80) && !FP::callFarA<bool>(0x0050ffb0)
        && FP::getData<bool>(0x00701cc6)) {

        const glm::ivec2 mouse = FP::getData<glm::ivec2>(0x00701c80);
        if (mouse.x == 0 || mouse.y == 0 || mouse.x == 480 || mouse.y == 272) return; //on the screen edge

        glm::vec2& lastMouse = FP::getData<glm::vec2>(0x00730ab0);
        if (lastMouse == static_cast<glm::vec2>(mouse)) return;

        glm::vec3 target;
        FUN_004d1070_screenToWorld(&target, mouse.x, mouse.y, cursor->position.y, true);
        if (buttons & 0x4000) recordFastMove();

        //CUSTOM CODE START
        /* The snap below moves every tile edge 2.4 units up, which fit the early stopping sampler of FUN_004d1070. With
         * the exact hits of the custom solver it sends the low edge of a tile to the previous tile. Snapping to the
         * center of the hit tile first fixes that, the snap below keeps the center of any tile with index 0 or more. */
        static bool customSnap = true;
        if (customSnap) {
            target.x = std::floor(target.x / 12.f) * 12.f + 6.f;
            target.z = std::floor(target.z / 12.f) * 12.f + 6.f;
        }
        //CUSTOM CODE END

        target.x = static_cast<float>(6 - 12 * static_cast<int>(target.x / -12.0 + 0.2));
        target.z = static_cast<float>(6 - 12 * static_cast<int>(target.z / -12.0 + 0.2));
        if (target.z <= static_cast<float>(minZ * 12)) target.z = static_cast<float>(minZ * 12 + 6);
        if (target.x <= static_cast<float>(minX * 12)) target.x = static_cast<float>(minX * 12 + 6);
        if (target.z > static_cast<float>(maxZ * 12)) target.z = static_cast<float>(maxZ * 12 + 6);
        if (target.x > static_cast<float>(maxX * 12)) target.x = static_cast<float>(maxX * 12 + 6);

        cursor->FUN_0046cab0_translateTo(target.x, target.y, target.z, 5.f);
        lastMouse = static_cast<glm::vec2>(mouse);
        return;
    }

    int frames = 10;
    if (useRecordedInput == 1) {
        int8_t& recorded = FP::getData<int8_t>(0x006d4acb);
        if (recorded / 10 != 0) {
            recorded = static_cast<int8_t>(recorded % 10);
            frames = 5;
        }
    }
    else if (buttons & 0x4000) {
        recordFastMove();
        frames = 5;
    }

    switch (direction) {
        case 0:
            if (tileZ <= minZ) return;
            cursor->FUN_0046cab0_translateTo(static_cast<float>(centerX), 0.f, static_cast<float>(centerZ - 12), static_cast<float>(frames));
            cursor->facing = 0;
            return;
        case 1:
            if (tileX <= minX) return;
            cursor->FUN_0046cab0_translateTo(static_cast<float>(centerX - 12), 0.f, static_cast<float>(centerZ), static_cast<float>(frames));
            cursor->facing = 90;
            return;
        case 2:
            if (tileZ >= maxZ) return;
            cursor->FUN_0046cab0_translateTo(static_cast<float>(centerX), 0.f, static_cast<float>(centerZ + 12), static_cast<float>(frames));
            cursor->facing = 180;
            return;
        case 3:
            if (tileX >= maxX) return;
            cursor->FUN_0046cab0_translateTo(static_cast<float>(centerX + 12), 0.f, static_cast<float>(centerZ), static_cast<float>(frames));
            cursor->facing = -90;
            return;
        case -1:
            break;
        default:
            return;
    }

    //analog stick
    glm::vec2 stick(0.f);
    if (!isStickLocked) {
        stick = glm::vec2(FP::getData<int16_t>(0x00778d60), FP::getData<int16_t>(0x00778d62));
        if (stick.x != 0.f || stick.y != 0.f) FP::getData<uint8_t>(0x0067726b) = 1;
    }
    const float stickLength = std::sqrt(static_cast<float>(double(stick.x) * stick.x + double(stick.y) * stick.y));
    if (stickLength < 70.f) { //dead zone
        cursor->velocity.x = 0.f;
        cursor->velocity.z = 0.f;
    }
    else {
        const float yaw = useRecordedInput == 1 ? FP::getData<float>(0x006e57b8) : camera->field_0x30.y;
        FUN_004e32a0_rotateXZ(static_cast<float>(yaw * double(57.29674f)), static_cast<float>(stick.x * double(0.01f)),
            static_cast<float>(stick.y * double(0.01f)));
        const float rotatedX = FP::getData<float>(0x006b791c);
        const float rotatedZ = FP::getData<float>(0x006b7a60);
        if (frames == 5) {
            cursor->velocity.x = static_cast<float>(rotatedX * 2.5);
            cursor->velocity.z = static_cast<float>(-rotatedZ * 2.5);
        }
        else {
            cursor->velocity.x = rotatedX;
            cursor->velocity.z = -rotatedZ;
        }
    }

    float nextX = cursorX + cursor->velocity.x;
    float nextZ = cursorZ + cursor->velocity.z;
    const float maxXEdge = static_cast<float>(maxX * 12 + 12);
    const float maxZEdge = static_cast<float>(maxZ * 12 + 12);
    if (nextX > maxXEdge) {
        nextX = maxXEdge;
        cursor->velocity.x = 0.f;
    }
    if (nextZ > maxZEdge) {
        nextZ = maxZEdge;
        cursor->velocity.z = 0.f;
    }
    if (static_cast<float>(minX * 12) > nextX) cursor->velocity.x = 0.f;
    if (static_cast<float>(minZ * 12) > nextZ) cursor->velocity.z = 0.f;
}

__declspec(naked) void FUN_004d1500_updateMapCursor_NoneStd(int unused, char useRecordedInput) {
    //camera and cursor stay in ECX and EDX, the stack arguments are pushed again for the fastcall to pop
    __asm {
        push [esp + 8]
        push [esp + 8]
        call FUN_004d1500_updateMapCursor
        ret
    }
}
