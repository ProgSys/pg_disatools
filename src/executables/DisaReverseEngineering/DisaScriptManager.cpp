#include "DisaScriptManager.h"
#include "FunctionPatcher.h"
#include "DisaScriptFunctions.h"
#include <Managers/DisaCustomScriptManager.h>
#include <Util/Debug/PG_DebugOut.h>

using byte = unsigned char;
using ushort = unsigned short;
using uint = unsigned int;
using undefined = char;
using undefined2 = short;
using undefined4 = int;
using undefined8 = long long;

void DisaScriptManager::clearCharDisplay() {
    std::fill(std::begin(charDisplaySlot), std::begin(charDisplaySlot) + 3, -1);
    std::fill(std::begin(charDisplayXOffset), std::begin(charDisplayXOffset) + 3, 0);
    std::fill(std::begin(charDisplayYOffset), std::begin(charDisplayYOffset) + 3, 0);
    std::fill(std::begin(dataMid), std::begin(dataMid) + 8, 0); //not all are cleard
    std::fill(std::begin(loadedCharIds), std::begin(loadedCharIds) + 9, 0);
}

int __thiscall DisaScriptManager::FUN_004bf7e0_RunTalkScript(unsigned int scriptId, unsigned int dungeonId, unsigned int stateFlag, unsigned int param_4) {
    // return FP::callThisFarA<int>(0x004bf7e0, this, scriptId, dungeonId, unknownFlag, param_4);
    if (isActive != 0 || sizeTalkScripts <= 0) return -1;

    int index = this->resolvedScriptId = DisaCustomScriptManager::get()->readTalkScript(this->scriptPointer, scriptId, dungeonId, stateFlag);
    if (!this->scriptPointer) return -1;
    setupTalkScript(scriptId, dungeonId, stateFlag, param_4);
    FP::callStdFarA(0x004bde60); //used by tutorial cutscene (and maybe others things)
    return index;
}

int DisaScriptManager::readDisaTalkScript(unsigned char*& dataLocation, uint32_t scriptId, uint32_t dungeonId, uint32_t stateFlag) {
    const PG_DisaTalkDatScriptEntry* startIt = talkScriptsEntries;
    const PG_DisaTalkDatScriptEntry* endIt = startIt + sizeTalkScripts;

    //search for script
    auto findIt = std::find_if(startIt, endIt, [&](const PG_DisaTalkDatScriptEntry& entry) ->bool {

        //test if script fits
        if (!((scriptId == 0 || scriptId == entry.scriptId) &&
            (dungeonId == 0 || dungeonId == entry.dungeonId) &&
            (stateFlag == 0 || stateFlag == entry.searchFlag)
            )) return false;

        //Tests if flag IS set
        unsigned char* globalStateIt = &FP::getData<unsigned char>(aGlobalStateBits);
        for (int i = 0; i < entry.gameStateFilterPositiveSize; ++i) {
            if (entry.gameStateFilterPositive[i].isClear(globalStateIt)) return false;
        }
        //Tests if flag is NOT set
        for (int i = 0; i < entry.gameStateFilterNegativeSize; ++i) {
            if (!entry.gameStateFilterNegative[i].isClear(globalStateIt)) return false;
        }

        return true;
        });

    if (findIt == endIt) return -1;
    OUT_INFO("Found disa talk script at index " << std::dec << std::distance(startIt, findIt));
    dataLocation = this->scriptsData + findIt->startAddress;//the script to play
    return findIt->scriptId;
}

int DisaScriptManager::readDisaScript(unsigned int*& dataLocation, uint32_t scriptId) {
    dataLocation = nullptr;
    if (scriptId >= 9000000) return -1;
    /*
    uint32_t scriptIndex = <find scriptId in scriptsIdList>
    bate* scriptCode = scriptsOriginAdress + scriptsCodeAdresses[scriptIndex]
    */
    if (sizeScripts == 0) return -1;
    auto rng = std::ranges::subrange(scriptsIdList, scriptsIdList + sizeScripts);
    auto findIt = std::ranges::find(rng, scriptId);
    if (findIt == rng.end()) return -1;

    const int index = std::distance(rng.begin(), findIt);
    //OUT_INFO("Found disa script at index " << std::dec << index << " id: " << scriptId);
    dataLocation = (unsigned int*)(scriptsOriginAdress + scriptsCodeAdresses[index]);
    return index;
}


int __thiscall DisaScriptManager::callTalkScript(const std::string& scriptName, unsigned int param_4) {
    if (isActive != 0) return -1;

    this->resolvedScriptId = DisaCustomScriptManager::get()->readTalkScript(this->scriptPointer, scriptName);

    if (this->scriptPointer == nullptr) {
        return -1;
    }

    setupTalkScript(990, 0, 0, param_4);
    return 990 + this->resolvedScriptId;
}

void DisaScriptManager::setupTalkScript(unsigned int scriptId, unsigned int dungeonId, unsigned int unknownFlag, unsigned int param_4) {
    //reset states
    this->field16_0x16f2 = 0;
    *(undefined*)this->field_0x16f8 = 0;
    this->data2[0x57] = 0;
    this->data2[0xd8] = 0;

    DisaSettings* settings = (DisaSettings*)(FP::getData<unsigned int>(0x005d7c40));
    if (settings->field_0x1e == 6 || settings->field_0x1e == 7) {
        this->field_0x16f8[1] = 0;
        this->data2[0x58] = 0;
        this->data2[0xd9] = 0;
    }

    this->scriptId = scriptId;
    this->dungeonId = dungeonId;
    this->unknownFlag = unknownFlag;

    *(int*)((int)this + 0x4828) = 0x110;
    *(int*)((int)this + 0x482c) = 0;
    this->isActive = 1;
    this->field4_0x54 = 0;
    this->field5_0x58 = 0;

    this->speechBubbleType = 0;
    this->speechBubbleArrowPosition = 0;
    this->labelPosition = 0;
    this->labelTextId = 0;

    clearCharDisplay();

    FP::getData<unsigned int>(0x00677264) = param_4;
}

DisaScriptManager& DisaScriptManager::get() {
    return FP::getData<DisaScriptManager>(0x0071b510);
}