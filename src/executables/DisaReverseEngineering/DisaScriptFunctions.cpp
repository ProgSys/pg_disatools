#include "DisaScriptFunctions.h"
#include "FunctionPatcher.h"
#include <Managers/DisaCustomBaseModManager.h>
#include <Managers/DisaCustomCampaignManager.h>
#include <DisaTalkScriptFunctions.h>
#include <Scene/Gui/Validator/PG_GuiTextValidators.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Util/PG_StringUtil.h>
#include <Util/Debug/PG_DebugOut.h>
#include <ranges>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <windows.h> // Include the Windows API header

void __fastcall FUN_004b6420_ScriptExec(DisaScriptMemory* disaScriptMemory, int param_2) {
    
    DisaTScriptManager& tManager = FP::getData<DisaTScriptManager>(0x0071b510);
    if (disaScriptMemory->ux24 == 1) {
        if (tManager.field_0x47f2 == 1 || (tManager.field_0x47f2 & 0x4000U) != 0 || FP::callFastFarA<bool>(0x0050f7c0, false)) {
            tManager.field_0x47f2 = 0;
            tManager.data2[4092] = 0;
            tManager.data2[4093] = 0;
            tManager.field_0x4814 = 0;
            disaScriptMemory->ux24 = 0;
        }
        return;
    }

    if (0 < (int)disaScriptMemory->ux32) {
        if (FP::getData<unsigned char>(0x0066475b) != 0 &&
            ((tManager.field_0x47fc & 0x6000) != 0 || FP::callFastFarA<bool>(0x0050f7c0, false))) {
            disaScriptMemory->ux32 = 0;
            FP::getData<unsigned char>(0x0066475b) = 0;
            return;
        }
        --disaScriptMemory->ux32;
        if (disaScriptMemory->ux32 == 0) {
            FP::getData<unsigned char>(0x0066475b) = 0;
        }
        return;
    }

    //values
    float fVar42 = 1.0f;
    float fVar43 = 0.75f;
    bool running = true;

    while (running) {

        const auto* ptr = (unsigned char*)disaScriptMemory->codePointer;
        const unsigned char opCode = *ptr;
        const unsigned char commandSize = *(ptr + 1);

        switch (opCode)
        {
        case 0x13: { //custom script
            std::string path;
            path.resize(commandSize);
            memcpy(path.data(), ptr + 2, commandSize);

            FP::callStdFarA(0x0043c890, 0, fVar42, (float)fVar43);
            runCustomScript(path, disaScriptMemory->ux80);
        } break;
        default:
            running = false;
            break;
        }
    }
    FP::callFastFarA(0x004b6420, disaScriptMemory, param_2);
}

static unsigned int backJumpToWhileLoop = 0;

void __fastcall scriptDefaultCase(unsigned char opCode, DisaScriptMemory* disaScriptMemory) {
    unsigned char* args = (unsigned char*)disaScriptMemory->codePointer;
    unsigned char sizeArgs = *(args - 1);
    disaScriptMemory->codePointer = (unsigned int*)((unsigned char*)disaScriptMemory->codePointer + sizeArgs);

    backJumpToWhileLoop = 0x004b65fb - FP::usedBase + FP::get()->getBaseAdress();
    switch (opCode)
    {
    case 0x13: { //custom talk script

        std::string path;
        path.resize(sizeArgs);
        memcpy(path.data(), args, sizeArgs);

        DisaTScriptManager& tScriptManager = FP::getData<DisaTScriptManager>(0x0071b510);
        if (tScriptManager.call(path, 0) < 0) {
            FP::getData<char>(0x00619330) = FP::getData<char>(0x00619330) | 0x20;
        }
        else {
            FP::getData<char>(0x00619330) = FP::getData<char>(0x00619330) & 0xdf;
        }
        backJumpToWhileLoop = 0x004bcf14 - FP::usedBase + FP::get()->getBaseAdress();
    } break;
    case 0x14: { //custom script

        std::string path;
        path.resize(sizeArgs);
        memcpy(path.data(), args, sizeArgs);

        float fVar42 = 1.0f;
        float fVar43 = 0.75f;
        FP::callStdFarA(0x0043c890, 0, fVar42, (float)fVar43);
        runCustomScript(path, disaScriptMemory->ux80);
    } break;
    case 0xa0: { //custom map bgm
        unsigned int bgmMem = 0;// = FP::callFastFarA<unsigned int>(0x004d0d90, 10);
        
        if (sizeArgs == 2) {
            bgmMem = *args;
            bgmMem = bgmMem == 0 ? DisaCustomCampaignManager::get()->getBGM() : bgmMem;
            *((unsigned char*)disaScriptMemory->codePointer - 2) = bgmMem;
            *((unsigned char*)disaScriptMemory->codePointer - 4) = 0x91;
            disaScriptMemory->codePointer = (unsigned int*)((unsigned char*)disaScriptMemory->codePointer - 4);
            break;
        }

        //TODO dons't work and I don't know why
        const unsigned int volume = *args;
        bgmMem = bgmMem == 0 ? DisaCustomCampaignManager::get()->getBGM() : bgmMem;
        bgmMem = FP::callFastFarA<unsigned int>(0x004d0d90, bgmMem);
        FP::callFastFarA(0x004c1ee0, bgmMem, volume);
        backJumpToWhileLoop = 0x004b7d3b - FP::usedBase + FP::get()->getBaseAdress();
    } break;
    case 0xa1: { //print
        std::string str;
        str.resize(sizeArgs);
        memcpy(str.data(), args, sizeArgs);
        OUT_INFO("[0x" << std::hex << disaScriptMemory->codePointer - sizeArgs << "]: " << str);
    }break;
        /*
    case 0xa2: {
        //reverse 7
        unsigned char var1 = *args;
        ++args; //3
        unsigned char size = *args;
        unsigned char* end = args + sizeArgs + var1 * 0x100;
        ++args; //4

        char* compareResult = &FP::getData<char>(0x006e80f0);
        if (size > 0) {
            unsigned char counter = 0;
            do {
                int leftArg = FP::callFastFarA<int>(0x004b60d0, disaScriptMemory, args);
                args += 3;
                unsigned char opratorArg = *args;
                ++args;
                int rightArg = FP::callFastFarA<int>(0x004b60d0, disaScriptMemory, args);
                args += 3;
                *compareResult = false;
                switch (opratorArg)
                {
                case 10: {
                    *compareResult = leftArg == rightArg;
                } break;
                case 11: {
                    *compareResult = leftArg < rightArg;
                } break;
                case 12: {
                    *compareResult = leftArg > rightArg;
                } break;
                case 13: {
                    *compareResult = leftArg <= rightArg;
                } break;
                case 14: {
                    *compareResult = leftArg >= rightArg;
                } break;
                case 15: {
                    *compareResult = leftArg != rightArg;
                } break;
                }
                if (counter < (size - 1)) {

                    ++args;
                }

                ++compareResult;
                ++counter;
            } while (counter < size);
        }
        compareResult = &FP::getData<char>(0x006e80f0);

        bool isTrue = false;
        if (size == 1) {
            isTrue = (bool)*compareResult;
        }
        else if (size == 2) {
            if (compareResult[128] == '20' && ((bool)compareResult[0] && (bool)compareResult[1])) {
                isTrue = true;
            }

        }
        else {
            isTrue = false;
        }

    }break;
    */
    default: break;
    }
}


_declspec(naked) void injectDefaultJumpFunc()
{
    _asm
    {
        nop;
        mov esi, edx
        call scriptDefaultCase;
        jmp backJumpToWhileLoop;
    }
}

void replaceDefaultCase() {
    backJumpToWhileLoop = 0x004b65fb - FP::usedBase + FP::get()->getBaseAdress();
    FP::replaceAddress(0x004bd23c, injectDefaultJumpFunc);
}


struct DisaScriptInfo
{
    FP::FarCdeclFunction<void, DisaScriptInfo*> function;
    DisaScriptMemory* scriptMemory;
    DisaScriptInfo* nextScript;
    short functionSlot;
    short ux16;
    int ux18;
};


int runScriptIntern(int scriptId, unsigned int scriptIndex, unsigned int* code, int param_2) {
    //find free fucntion slot 2
    std::array<DisaScriptInfo, 1024>& scriptInfos = FP::getData<std::array<DisaScriptInfo, 1024>>(0x006cfa70);
    auto findScriptSlotIt = std::ranges::find(scriptInfos | std::views::reverse, nullptr, &DisaScriptInfo::function);  //stack? We go backwards
    if (findScriptSlotIt == scriptInfos.rend()) return -1;

    FP::FarCdeclFunction<void, DisaScriptInfo*>* callbackFunc = &findScriptSlotIt->function;
    *callbackFunc = [](DisaScriptInfo* param_1) {
        FP::callCdeclFarA<int>(0x004bd800, param_1); //__cdecl
        };

    DisaScriptMemory* scriptMemoryPtr = FP::callFastFarA<DisaScriptMemory*>(0x004333f0, 0x88);

    findScriptSlotIt->scriptMemory = scriptMemoryPtr;
    findScriptSlotIt->nextScript = nullptr;
    findScriptSlotIt->functionSlot = 10;
    findScriptSlotIt->ux16 = 0;
    findScriptSlotIt->ux18 = 0;

    //find free fucntion slot 2
    auto& functionSlotsArray = FP::getData<std::array<FP::FarCdeclFunction<void, DisaScriptInfo*>*, 0x100>>(0x0071b5bc);

    auto findSlotIt = std::ranges::find_if(functionSlotsArray, [](auto* ptr) { return ptr == nullptr; });
    if (findSlotIt == functionSlotsArray.end()) return -1;

    *findSlotIt = callbackFunc;

    scriptMemoryPtr->ux56 = std::distance(functionSlotsArray.begin(), findSlotIt);
    scriptMemoryPtr->ux52 = 0;
    scriptMemoryPtr->ux40 = 0;
    scriptMemoryPtr->index = scriptIndex;
    scriptMemoryPtr->scriptId = scriptId;
    scriptMemoryPtr->ux32 = 0;
    scriptMemoryPtr->ux20 = 0;
    scriptMemoryPtr->ux24 = 0;
    scriptMemoryPtr->ux44 = 0;
    scriptMemoryPtr->ux80 = param_2;

    FP::getData<DisaScriptMemory*>(0x008aaa00) = scriptMemoryPtr;
    scriptMemoryPtr->ux76 = FP::getData<int>(0x00730a20);

    if (param_2 == 0) {
        scriptMemoryPtr->ux48 = 0;
    }
    else {
        if (*(int*)param_2 == 0) {
            *(FP::FarCdeclFunction<void, DisaScriptInfo*>**)param_2 = callbackFunc;
        }
        scriptMemoryPtr->ux48 = (int)*(short*)(param_2 + 0x166);
    }

    scriptMemoryPtr->codePointer = code;

    scriptMemoryPtr->ux68 = 0;
    scriptMemoryPtr->ux64 = 0;
    scriptMemoryPtr->ux60 = 0;
    scriptMemoryPtr->ux84 = -1;
    scriptMemoryPtr->ux88 = -1;
    scriptMemoryPtr->ux92 = -1;
    scriptMemoryPtr->ux96 = -1;
    scriptMemoryPtr->ux100 = -1;
    scriptMemoryPtr->ux104 = -1;
    scriptMemoryPtr->ux108 = -1;
    scriptMemoryPtr->ux112 = -1;
    scriptMemoryPtr->ux116 = -1;
    scriptMemoryPtr->repeatLoopCounter = 0;
    scriptMemoryPtr->repeatLoopMax = 0;
    scriptMemoryPtr->repeatLoopStartCodePointer = -1;


    if ((FP::getData<char>(0x0065db3a) == '\x05') && (param_2 != 0)) {
        int iVar4 = 0;
        if (0 < *(short*)(param_2 + 0x154)) {
            short* psVar5 = (short*)(param_2 + 0x84);
            do {
                if ((unsigned int)(int)*psVar5 < 0x40) {
                    (&FP::getData<bool>(0x005d7c90))[*psVar5] = true;
                }
                iVar4 = iVar4 + 1;
                psVar5 = psVar5 + 1;
            } while (iVar4 < *(short*)(param_2 + 0x154));
        }

        iVar4 = 0;
        if (0 < *(short*)(param_2 + 0x156)) {
            short* psVar5 = (short*)(param_2 + 0x94);
            do {
                if ((unsigned int)(int)*psVar5 < 0x40) {
                    (&FP::getData<bool>(0x005d7c90))[*psVar5] = true;
                }
                iVar4 = iVar4 + 1;
                psVar5 = psVar5 + 1;
            } while (iVar4 < *(short*)(param_2 + 0x156));
        }
    }

    if (((int)scriptId < 0xf4241) || (1999999 < (int)scriptId)) {
        FP::getData<int>(0x0071fd34) = FP::getData<int>(0x0071fd34) + 1;
    }
    FP::callCdeclFarA<void>(0x004bd800, &*findScriptSlotIt);
    return scriptIndex;
}

std::string opCodeToComment(unsigned char op) {
    switch (op)
    {
    case 0x01: return "Sleep";
    case 0x02: return "Return";
    case 0x03: return "RepeatStart";
    case 0x04: return "RepeatEnd";
    case 0x05: return "LockUser";
    case 0x06: return "UnlockUser";
    case 0x07: return "If";
    case 0x09: return "Nop";
    case 0x0A: return "FadeOut";
    case 0x11: return "LoadTalkScript";
    case 0x12: return "RunScript";
    case 0x27: return "SetCursorPos";
    case 0x29: return "SetCameraZoom";
    case 0x28: return "SetCameraMode";
    case 0x2C: return "SetCameraPos";
    case 0x2E: return "SetCameraRotation";
    case 0x65: return "SetActorPos";
    case 0x67: return "SetActorAnimation";
    case 0x6D: return "SetActor";
    case 0x73: return "RotateActor";
    case 0x7A: return "SetMultRGBA";
    case 0x87: return "PlayVoice";
    case 0x8C: return "PlaySound";
    case 0x91: return "PlayBGM";
    case 0xC8: return "RunFunction";
    default:
        break;
    }
    return {};
}

//call SCRIPT
int __fastcall FUN_004bd930_RunScript(int scriptId, int param_2) {
    unsigned int size = FP::getData<unsigned int>(0x0071b534);
    if (size == 0) return -1;

    const auto startIt = FP::getData<int*>(0x0071b53c);
    const auto endIt = startIt + size;
    const auto findIt = std::find(startIt, endIt, scriptId);
    if (findIt == endIt) return -1;

    const unsigned int index = std::distance(startIt, findIt);
    OUT_INFO("Found Script at index " << std::dec << index << " id: " << scriptId);

    if ((scriptId - 0xf4241 < 999999) && (*(bool*)(FP::getData<unsigned int>(0x0071b544) + index) != false))
        return -1; //script is in use?

    *(bool*)(FP::getData<unsigned int>(0x0071b544) + index) = true;

    unsigned int* codeLocation = nullptr;
    if (scriptId < 9000000) {
        codeLocation = (unsigned int*)(*(unsigned int*)(FP::getData<unsigned int>(0x0071b538) + index * 4) + FP::getData<unsigned int>(0x0071b540)); //code ptr
    }
    //BEGIN custom code
    DisaCustomBaseModManager::get()->readScript(codeLocation, scriptId);

    //print script
    if (DisaCustomBaseModManager::get()->isPrintScript() && codeLocation) {
        OUT_INFO("Script:");

        unsigned char* ptr = (unsigned char*)codeLocation;
        std::stringstream stream;
        stream << "\n";
        stream.fill('0');
        int killCount = 10000;
        while (--killCount > 0) {
            const unsigned char opCode = *ptr;
            if (opCode == 0) {
                stream << std::hex << std::setw(2) << (int)opCode << "\n";
                break;
            }
            unsigned int argsSize = *(ptr + 1);
            stream << std::hex << std::setw(2) << (int)opCode << " " << std::hex << std::setw(2) << (int)argsSize;
            ptr += 2;

            //is if statement
            if (opCode == 7) {
                const unsigned int operations = *(ptr + 1);
                argsSize = 2 + operations * 7;
            }

            for (int i = 0; i < argsSize; ++i) {
                stream << " " << std::hex << std::setw(2) << (int)*ptr;
                ++ptr;
            }
            if (auto comment = opCodeToComment(opCode); !comment.empty()) {
                stream << " #" << comment;
            }
            if (opCode == 0x02) 
                break;
            stream << "\n";
        }
        OUT_INFO(stream.str());
    }
    //END custom code

    return runScriptIntern(scriptId, index, codeLocation, param_2);
}


int runCustomScript(const std::string& scriptStr, int param_2) {
    if (scriptStr.empty()) return -1;

    unsigned int* codeLocation = nullptr;
    DisaCustomBaseModManager::get()->readScript(codeLocation, scriptStr);


    if (codeLocation == nullptr) return -1;

    //TODO think of a better id method
    return runScriptIntern(910000, 910000, codeLocation, param_2);
}

void __fastcall FUN_004f7e70_ScriptLoop(short param_1, short param_2) {

    //clean

    std::array<DisaScriptInfo*, 128>& functionSlots = FP::getData<std::array<DisaScriptInfo*, 128>>(0x008b69d0);
    functionSlots.fill(nullptr);

    std::array<DisaScriptInfo, 1024>& disaScriptInfos = FP::getData<std::array<DisaScriptInfo, 1024>>(0x006cfa70);

    for (auto& disaScriptInfo : disaScriptInfos) {
        if (disaScriptInfo.function == nullptr) {
            disaScriptInfo.nextScript = nullptr;
        }
        else {
            //CUSTOM CODE
            if (disaScriptInfo.functionSlot >= functionSlots.size()) {
                //disaScriptInfo.functionSlot = functionSlots.size() - 1;
                MessageBox(
                    NULL,                      
                    "Invalid script function slot detected in ScriptLoop!",  
                    "Fatal Error",                   
                    MB_ICONERROR | MB_OK       
                );
                exit(1);
            }
            //CUSTOM CODE END

            disaScriptInfo.nextScript = functionSlots[disaScriptInfo.functionSlot];
            functionSlots[disaScriptInfo.functionSlot] = &disaScriptInfo;
        }
    }

    for (int i = param_1; i < param_2; ++i) {
        for (DisaScriptInfo* info = functionSlots[i]; info; info = info->nextScript) {
            if (info->function && info->ux16 == 0) {
                info->function(info);
            }
        }
    }
}