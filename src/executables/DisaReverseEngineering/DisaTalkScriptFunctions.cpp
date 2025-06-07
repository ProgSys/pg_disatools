#include "DisaTalkScriptFunctions.h"
#include "FunctionPatcher.h"
#include <Managers/DisaCustomBaseModManager.h>
#include "DisaScriptFunctions.h"
#include <iostream>
#include <Util/Debug/PG_DebugOut.h>

using byte = unsigned char;
using ushort = unsigned short;
using uint = unsigned int;
using undefined = char;
using undefined2 = short;
using undefined4 = int;
using undefined8 = long long;


void DisaTScriptManager::clearCharDisplay() {
    std::fill(std::begin(charDisplaySlot), std::begin(charDisplaySlot) + 3, -1);
    std::fill(std::begin(charDisplayXOffset), std::begin(charDisplayXOffset) + 3, 0);
    std::fill(std::begin(charDisplayYOffset), std::begin(charDisplayYOffset) + 3, 0);
    std::fill(std::begin(dataMid), std::begin(dataMid) + 8, 0); //not all are cleard
    std::fill(std::begin(loadedCharIds), std::begin(loadedCharIds) + 9, 0);
}

int __thiscall DisaTScriptManager::FUN_004bf7e0_RunTalkScript(unsigned int scriptId, unsigned int dungeonId, unsigned int unknownFlag, unsigned int param_4) {
   // return FP::callThisFarA<int>(0x004bf7e0, this, scriptId, dungeonId, unknownFlag, param_4);
    if (isActive != 0 || totalScripts <= 0) return -1;

    unsigned int index = 0;
    //@see FUN_LoadDungeonsCategories
    //BEGIN custom code
    
    if (dungeonId / 100 >= 655) { //is custom dungeon?
        this->resolvedScriptId = DisaCustomBaseModManager::get()->readTalkScript(this->scriptPointer, scriptId, dungeonId, unknownFlag);
        if (this->scriptPointer == nullptr) return -1;
        OUT_INFO("Found custom talk script with dungeonId " << std::dec << dungeonId);
    } //END custom code
    else {

        const DisaTalkDatScriptEntry* startIt = scriptsEntries;
        const DisaTalkDatScriptEntry* endIt = startIt + totalScripts;

        //search for script
        auto findIt = std::find_if(startIt, endIt, [&](const DisaTalkDatScriptEntry& entry) ->bool {

            //test if script fits
            if (!((scriptId == 0 || scriptId == entry.scriptId) &&
                (dungeonId == 0 || dungeonId == entry.dungeonId) &&
                (unknownFlag == 0 || unknownFlag == entry.unknownFlag)
                )) return false;

            //Tests if flag IS set
            unsigned char* globalStateIt = &FP::getData<unsigned char>(aGlobalStateBits);
            for (int i = 0; i < entry.gameStateFilterSizePositive; ++i) {
                if (entry.gameStateFilterPositive[i].isSet(globalStateIt)) return false;
            }
            //Tests if flag is NOT set
            for (int i = 0; i < entry.gameStateFilterSizeNegative; ++i) {
                if (!entry.gameStateFilterNegative[i].isSet(globalStateIt)) return false;
            }

            return true;
            });

        if (findIt == endIt) return -1;
        index = std::distance(startIt, findIt);
        OUT_INFO("Found disa talk script at index " << std::dec << index);
        this->scriptPointer = this->scriptsData + findIt->address;//the script to play
        this->resolvedScriptId = findIt->scriptId;
        DisaCustomBaseModManager::get()->readTalkScript(this->scriptPointer, scriptId, dungeonId, unknownFlag);
    }

    setupScript(scriptId, dungeonId, unknownFlag, param_4);
    FP::callStdFarA(0x004bde60); //used by tutorial cutscene (and maybe others things)
    return index;
}


int __thiscall DisaTScriptManager::call(const std::string& scriptName, unsigned int param_4) {
    if (isActive != 0) return -1;

    this->resolvedScriptId = DisaCustomBaseModManager::get()->readTalkScript(this->scriptPointer, scriptName);

    if (this->scriptPointer == nullptr) {
        return -1;
    }

    setupScript(990, 0, 0, param_4);
    return 990 + this->resolvedScriptId;
}

void DisaTScriptManager::setupScript(unsigned int scriptId, unsigned int dungeonId, unsigned int unknownFlag, unsigned int param_4) {
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

#include <Stream/PG_StreamInByteFile.h>
#include <Util/PG_ImageLoad.h>

void __fastcall FUN_004bfa60_TalkScriptExec(DisaTScriptManager* disaTScriptManager){
    const short speechBubbleAnimationFrame = disaTScriptManager->speechBubbleAnimationFrame;
    if ((speechBubbleAnimationFrame < 16) && (0 < speechBubbleAnimationFrame)) {
        disaTScriptManager->speechBubbleAnimationFrame = disaTScriptManager->speechBubbleAnimationFrame + disaTScriptManager->speechBubbleAnimationSpeed;
        return;
    }

    if (disaTScriptManager->voiceId) {
        disaTScriptManager->voiceRunFlag++;
        if (disaTScriptManager->voiceRunFlag < 301) {
            if (FP::getData<char>(0x00818ef5) != '\x01') return;
          
            FP::callFastFarA(0x00505250, (int)disaTScriptManager->voiceId, 127);//soundId, volume
        }
        disaTScriptManager->voiceId = 0;
    }

    int iVar18;
    uint uVar9;
    ushort uVar4 = disaTScriptManager->field_0x47fc;
    byte waitUserImputMode = disaTScriptManager->waitUserImputMode;

    //BEGIN custom code
    //now I have no idea what the other sub blocks to, nor do I care to much for now
    //so I just call the original function in that case
    if (waitUserImputMode != 0) {
       FP::callFastFarA(0x004bfa60, disaTScriptManager);
       return;
    }
    //END custom code

    if (waitUserImputMode == 1) { //confirm
        if (((disaTScriptManager->data2[0x306c] == 2) || ((uVar4 & 0xd) != 0)) ||
            (uVar9 = FP::callFastFarA<int>(0x0050f7c0, '\0'), (char)uVar9 != '\0')) {
            FP::callFastFarA(0x00510ad0, (int*)0x1);
            disaTScriptManager->voiceId = 0;
            *(undefined2*)(disaTScriptManager->data2 + 0x30d0) = 0;
            disaTScriptManager->dialogStringLength = 0;
            disaTScriptManager->dialogStringCurrentRow = 0;
            disaTScriptManager->field16_0x16f2 = 1;
            *(undefined*)disaTScriptManager->field_0x16f8 = 0;
            if ((*(int*)(FP::getData<uint>(0x005d7c40) + 0x30) == 6) || (*(int*)(FP::getData<uint>(0x005d7c40) + 0x30) == 7)) {
                *(undefined*)((int)disaTScriptManager->field_0x16f8 + 1) = 0;
            }
            disaTScriptManager->waitUserImputMode = 0;
            disaTScriptManager->field_0x18[0] = 0;
            disaTScriptManager->field_0x4814 = 0;
        }
        return;
    }
  
    if (waitUserImputMode == 2) { //confirm close
        if (((disaTScriptManager->data2[0x306c] != 2) && ((uVar4 & 0xd) == 0)) &&
            (uVar9 = FP::callFastFarA<int>(0x0050f7c0, '\0'), (char)uVar9 == '\0')) {
            return;
        }
        FP::callFastFarA(0x00510ad0, (int*)0x1);
        disaTScriptManager->voiceId = 0;
        disaTScriptManager->speechBubbleAnimationSpeed = -1;
        disaTScriptManager->speechBubbleAnimationFrame = 0xf;
        if (disaTScriptManager->speechBubbleType == 1) {
            disaTScriptManager->speechBubbleAnimationSpeed = -2;
            disaTScriptManager->speechBubbleAnimationFrame = 0xe;
        }
        iVar18 = FP::getData<int>(0x005d7c40);
        *(undefined*)disaTScriptManager->field_0x16f8 = 0;
        if ((*(int*)(iVar18 + 0x30) == 6) || (*(int*)(iVar18 + 0x30) == 7)) {
            *(undefined*)((int)disaTScriptManager->field_0x16f8 + 1) = 0;
        }
        disaTScriptManager->waitUserImputMode = 0;
        disaTScriptManager->field_0x4814 = 0;
        return;
    }

    short sVar8;
    uint uVar10;
    undefined4 uVar11;
    uint uVar14;

    if (waitUserImputMode == 3) { //procces menu selection
        short* pbVarA = &disaTScriptManager->field21_0x4800;
        sVar8 = *(short*)(disaTScriptManager->data + 0x654);
        iVar18 = *(short*)(disaTScriptManager->data + 0x652) + 0xd;
        uVar9 = 0x1c0 - iVar18;
        short sVar5 = *(short*)pbVarA;
        FP::callFastFarA(0x0050fb90, uVar9, 0xc, iVar18, 0x10, (int)sVar8, (int*)pbVarA, '\0');
        if (disaTScriptManager->data2[0x3062] != 0) {
            *(undefined2*)(disaTScriptManager->data2 + 0x306b) = 0;
            *pbVarA = *pbVarA -1;
        }
        if (disaTScriptManager->data2[0x3064] != 0) {
            *(undefined2*)(disaTScriptManager->data2 + 0x306b) = 0;
            *pbVarA = *pbVarA + 1;
        }
        if (*pbVarA < 0) {
            *pbVarA = *(short*)(disaTScriptManager->data + 0x654) + -1;
        }
        if (*(short*)(disaTScriptManager->data + 0x654) <= *(short*)pbVarA) {
            *pbVarA = 0;
        }

        undefined4 extraout_ECX = 0;
        uint extraout_ECX_00 = 0;
        uint extraout_ECX_01 = 0;
        uint extraout_ECX_02 = 0;
        uint extraout_ECX_03 = 0;

        if (sVar5 != *pbVarA) {
            FP::callFastFarA(fPlaySound, 3, FP::getData<int>(0x00677274), 0, extraout_ECX, 0);
        }

        if ((disaTScriptManager->data2[0x306c] == 2) ||
            (uVar10 = FP::callFastFarA<bool>(fIsLeftMouseBtnClicked, '\0'), uVar14 = extraout_ECX_00, (char)uVar10 != '\0')) {

            uVar11 = FP::callFastFarA<undefined4>(0x0050fe30, uVar9, 0xc, iVar18 + uVar9, sVar8 * 0x10 + 0xc, '\x01');
            uVar9 = extraout_ECX_01;

            if (((char)uVar11 == '\0') &&
                ((iVar18 = FP::callThisFarA<undefined4>(0x0043c6c0, &FP::getData<unsigned int>(0x00701c50), 0x2000000, 0), uVar9 = extraout_ECX_02,
                    (char)iVar18 == '\0' && (uVar14 = extraout_ECX_02, disaTScriptManager->data2[0x306c] != 2)
                    ))) {

                goto LAB_004bfda5;

            }else{


                iVar18 = FP::getData<int>(0x00677274);

                FP::getData<byte>(0x00778d98) = 0;
                FP::getData<byte>(0x00778d9c) = 0;
                FP::getData<int>(0x00778da0) = 0;
                FP::getData<byte>(0x00778da4) = 0;

                disaTScriptManager->data2[0x306b] = 0;
                FP::callFastFarA(fPlaySound, 1, iVar18, 0, uVar9, 0);
                *(undefined2*)(disaTScriptManager->data + 0x64a) = 0;
                disaTScriptManager->speechBubbleAnimationFrame = 0;
                disaTScriptManager->dialogStringLength = 0;
                disaTScriptManager->dialogStringCurrentRow = 0;

                const short sValue = *pbVarA;
                disaTScriptManager->waitUserImputMode = 0;
                uVar14 = (int)sValue + 0x15U & 7;
                (&FP::getData<byte>(aGlobalStateBits))[(int)((int)sValue + 0x15U) >> 3] = (&FP::getData<byte>(aGlobalStateBits))[(int)((int)sValue + 0x15U) >> 3] | '\x01' << (signed char)uVar14;
                disaTScriptManager->field_0x18[0] = 0;
                disaTScriptManager->scriptPointer = *(byte**)(disaTScriptManager->data + sValue * 4 + 4);
            }
        }

        if ((disaTScriptManager->data2[0x306b] == 2) || (uVar9 = FP::callFastFarA<unsigned int>(0x0050f950, 0), uVar14 = extraout_ECX_03, (char)uVar9 != '\0')) {

        LAB_004bfda5:

            FP::getData<byte>(0x00778d98) = 0;
            FP::getData<byte>(0x00778d9c) = 0;
            FP::getData<int>(0x00778da0) = 0;
            FP::getData<byte>(0x00778da4) = 0;

            FP::callFastFarA(fPlaySound, 7, FP::getData<int>(0x00677274), 0, uVar14, 0);
            const short sValue = *(short*)(disaTScriptManager->data + 0x654) + -1;

            *(undefined2*)(disaTScriptManager->data + 0x64a) = 0;
            disaTScriptManager->speechBubbleAnimationFrame = 0;
            disaTScriptManager->dialogStringLength = 0;
            disaTScriptManager->dialogStringCurrentRow = 0;
            disaTScriptManager->waitUserImputMode = 0;
            (&FP::getData<byte>(aGlobalStateBits))[sValue + 0x15 >> 3] = (&FP::getData<byte>(aGlobalStateBits))[sValue + 0x15 >> 3] | '\x01' << ((byte)(sValue + 0x15) & 7);

            disaTScriptManager->field_0x18[0] = 0;
            disaTScriptManager->scriptPointer = *(byte**)(disaTScriptManager->data + sValue * 4 + 4);
            return;
        }
    }

    if (waitUserImputMode == 4) {
        if (disaTScriptManager->field5_0x58 < 0) {

            const DisaSettings* settings2 = FP::getData<DisaSettings*>(0x005d7c40);
            DisaSettings* settings = (DisaSettings*)(FP::getData<unsigned int>(0x005d7c40));

            if (disaTScriptManager->field5_0x5a == 0) {
                FP::callFastFarA(0x00510ad0, (int*)0x1);
                disaTScriptManager->voiceId = 0;
                disaTScriptManager->speechBubbleAnimationSpeed = -1;
                disaTScriptManager->speechBubbleAnimationFrame = 0xf;
                if (disaTScriptManager->speechBubbleType == 1) {
                    disaTScriptManager->speechBubbleAnimationSpeed = -2;
                    disaTScriptManager->speechBubbleAnimationFrame = 0xe;
                }

                disaTScriptManager->field_0x16f8[0] = 0;

                if ((*(int*)(settings + 0x30) == 6) || (*(int*)(settings + 0x30) == 7)) {
                    disaTScriptManager->field_0x16f8[1] = 0;
                }
            }
            else {
                FP::callFastFarA(0x00510ad0, (int*)0x1);
                disaTScriptManager->voiceId = 0;
                *(undefined2*)(disaTScriptManager->data2 + 0x30d0) = 0;
                disaTScriptManager->dialogStringLength = 0;
                disaTScriptManager->dialogStringCurrentRow = 0;
                disaTScriptManager->field16_0x16f2 = 1;

                if ((*(int*)(settings + 0x30) == 6) || (*(int*)(settings + 0x30) == 7)) {
                    disaTScriptManager->field_0x16f8[1] = 0;
                }
                else {
                    disaTScriptManager->field_0x16f8[0] = 0;
                }
                disaTScriptManager->field_0x18[0] = 0;
            }
            disaTScriptManager->field_0x4814 = 0;
            disaTScriptManager->waitUserImputMode = 0;
            disaTScriptManager->field5_0x58 = 0;
            return;
        }
        --disaTScriptManager->field5_0x58;
        return;
    }

    byte bVar17;
    char cVar1;
    byte bVar2;
    byte bVar3;

    undefined4* puVar15;
    int iVar16;
    byte* pbVar21;
    uint local_c = 0;
    uVar9 = local_c;

    //run talk script
    byte opCode;
    while (true) {
        if (disaTScriptManager->dialogStringLength == 0) {
            opCode = *disaTScriptManager->scriptPointer;
            disaTScriptManager->scriptPointer++;
        }
        else {
            opCode = 1;
        }

        switch (opCode) {
        case 1: { //dialog
            //if (disaTScriptManager->dialogStringLength == 0) {
               // std::cout << "Dialog(" << ");" << std::endl;
            //}

            if (disaTScriptManager->field16_0x16f2 != 0) {
                disaTScriptManager->field16_0x16f2 = 0;
                disaTScriptManager->dialogStringLength = 0;
                disaTScriptManager->dialogStringCurrentRow = 0;
            }

            if (disaTScriptManager->speechBubbleAnimationFrame == 0) {
                disaTScriptManager->speechBubbleAnimationSpeed = 1;
                disaTScriptManager->speechBubbleAnimationFrame = 1;
                if (disaTScriptManager->speechBubbleType == 1) {
                    disaTScriptManager->speechBubbleAnimationSpeed = 2;
                    disaTScriptManager->speechBubbleAnimationFrame = 2;
                }
                disaTScriptManager->dialogStringLength = 0;
                disaTScriptManager->dialogStringCurrentRow = 0;
                disaTScriptManager->field14_0x16ec = 0;
            }

            if ((*(int*)(FP::getData<uint>(0x005d7c40) + 0x30) == 1) || (*(int*)(FP::getData<uint>(0x005d7c40) + 0x30) == 0)) { //settings

                bVar17 = *disaTScriptManager->scriptPointer;
                byte lowerByte = *disaTScriptManager->scriptPointer;
                disaTScriptManager->scriptPointer++;
                /*
                bVar17 = *disaTScriptManager->scriptPointer;
                iVar18 = (int)(char)bVar17;
                iVar18 &= 0x000000FF;
                disaTScriptManager->scriptPointer++;
                */
                if (lowerByte != 0) {
                    if (disaTScriptManager->field14_0x16ec < disaTScriptManager->dialogStringCurrentRow) {
                        disaTScriptManager->field14_0x16ec = disaTScriptManager->field14_0x16ec + 1;
                    }
                    unsigned char* pbVar13 = disaTScriptManager->scriptPointer;
                    byte upperByte = *disaTScriptManager->scriptPointer;
                    disaTScriptManager->scriptPointer++;


                    if (0x60 < lowerByte && lowerByte < 0x7b || 0x40 < lowerByte && lowerByte < 0x5b) {
                        disaTScriptManager->scriptPointer = pbVar13; //rewind?
                        iVar16 = lowerByte;
                        upperByte = lowerByte;
                    }

                    if (lowerByte == 0x23) {
                        iVar18 = (lowerByte + -0x30) * 0x38;
                        undefined4* puVar19 = (undefined4*)(&FP::getData<uint>(0x006b75b0) + iVar18);
                        puVar15 = puVar19;

                        do {
                            cVar1 = *(char*)puVar15;
                            puVar15 = (undefined4*)((int)puVar15 + 1);
                        } while (cVar1 != '\0');

                        pbVar13 = disaTScriptManager->field17_0x16f4 +
                            disaTScriptManager->dialogStringLength * 2 +
                            disaTScriptManager->dialogStringCurrentRow * 0x81 + 5;

                        do {
                            pbVar21 = pbVar13 + 1;
                            pbVar13 = pbVar13 + 1;
                        } while (*pbVar21 != 0);

                        undefined4* puVar20 = puVar19;

                        for (uVar9 = (uint)((int)puVar15 - (int)puVar19) >> 2; uVar9 != 0; --uVar9) {
                            *(undefined4*)pbVar13 = *puVar20;
                            puVar20 = puVar20 + 1;
                            pbVar13 = pbVar13 + 4;
                        }

                        for (uVar9 = (int)puVar15 - (int)puVar19 & 3; uVar9 != 0; --uVar9) {
                            *pbVar13 = *(byte*)puVar20;
                            puVar20 = (undefined4*)((int)puVar20 + 1);
                            pbVar13 = pbVar13 + 1;
                        }

                        do {
                            cVar1 = *(char*)puVar19;
                            puVar19 = (undefined4*)((int)puVar19 + 1);
                        } while (cVar1 != '\0');

                        disaTScriptManager->dialogStringLength =
                            disaTScriptManager->dialogStringLength +
                            (short)((uint)((int)puVar19 - (int)(&FP::getData<uint>(0x006b75b1) + iVar18)) >> 1);

                        disaTScriptManager->field_0x18[0] = 1;
                        return;
                    }

                    disaTScriptManager->field_0x16f8[
                        disaTScriptManager->dialogStringLength * 2 +
                            disaTScriptManager->dialogStringCurrentRow * 0x81] = lowerByte;

                    disaTScriptManager->field_0x16f8[
                        disaTScriptManager->dialogStringLength * 2 +
                            disaTScriptManager->dialogStringCurrentRow * 0x81 + 1] = (byte)upperByte;

                        disaTScriptManager->field_0x16f8[disaTScriptManager->dialogStringLength * 2 + disaTScriptManager->dialogStringCurrentRow * 0x81 + 2] = 0;

                        disaTScriptManager->dialogStringLength++;
                        disaTScriptManager->field_0x18[0] = 1;
                        return;
                }

                if (disaTScriptManager->dialogStringCurrentRow == (short)(char)bVar17) {
                    disaTScriptManager->data2[0x57] = bVar17;
                    disaTScriptManager->data2[0xd8] = bVar17;
                }

                *(undefined*)
                    ((int)disaTScriptManager->field_0x16f8 +
                        disaTScriptManager->dialogStringLength * 2 + disaTScriptManager->dialogStringCurrentRow * 0x81) = 0;
            }
            else {
                unsigned char* pbVar13 = disaTScriptManager->scriptPointer;
                bVar17 = *pbVar13;
                disaTScriptManager->scriptPointer = pbVar13 + 1;
                bVar3 = pbVar13[1];
                disaTScriptManager->scriptPointer = pbVar13 + 2;

                if (bVar17 != 0 || bVar3 != 0) {
                    sVar8 = disaTScriptManager->dialogStringCurrentRow;
                    if (disaTScriptManager->field14_0x16ec < sVar8) {
                        disaTScriptManager->field14_0x16ec++;
                        sVar8 = disaTScriptManager->dialogStringCurrentRow;
                    }

                    if ((bVar17 == 0x23) && (bVar3 == 0)) {
                        FP::callFastFarA(0x004e3660, (int)disaTScriptManager->field_0x16f8 + disaTScriptManager->dialogStringLength * 2 + sVar8 * 0x81, &FP::getData<char>(0x006b6b30));
                        disaTScriptManager->dialogStringLength += FP::callFastFarA<short>(0x6b6b30);
                        disaTScriptManager->field_0x18[0] = 1;
                        return;
                    }

                    *(byte*)((int)disaTScriptManager->field_0x16f8 +
                        disaTScriptManager->dialogStringLength * 2 + sVar8 * 0x81) = bVar17;
                    *(byte*)((int)disaTScriptManager->field_0x16f8 +
                        disaTScriptManager->dialogStringLength * 2 +
                        disaTScriptManager->dialogStringCurrentRow * 0x81 + 1) = bVar3;
                    *(undefined*)
                        ((int)disaTScriptManager->field_0x16f8 +
                            disaTScriptManager->dialogStringLength * 2 + disaTScriptManager->dialogStringCurrentRow * 0x81 + 2) = 0;
                    *(undefined*)
                        ((int)disaTScriptManager->field_0x16f8 +
                            disaTScriptManager->dialogStringLength * 2 + disaTScriptManager->dialogStringCurrentRow * 0x81 + 3) = 0;

                    disaTScriptManager->dialogStringLength++;
                    disaTScriptManager->field_0x18[0] = 1;
                    return;
                }

                if (disaTScriptManager->dialogStringCurrentRow == 0) {
                    *(undefined2*)(disaTScriptManager->data2 + 0x57) = 0;
                    *(undefined2*)(disaTScriptManager->data2 + 0xd8) = 0;
                }
                *(undefined*)
                    ((int)disaTScriptManager->field_0x16f8 +
                        disaTScriptManager->dialogStringLength * 2 + disaTScriptManager->dialogStringCurrentRow * 0x81) = 0
                    ;
                *(undefined*)
                    ((int)disaTScriptManager->field_0x16f8 +
                        disaTScriptManager->dialogStringLength * 2 + disaTScriptManager->dialogStringCurrentRow * 0x81 + 1)
                    = 0;
            }
            disaTScriptManager->dialogStringCurrentRow = disaTScriptManager->dialogStringCurrentRow + 1;
            disaTScriptManager->dialogStringLength = 0;
            return;
        }
        case 2:
            //std::cout << "Confirm(" << ");" << std::endl;
            disaTScriptManager->waitUserImputMode = 1;
            *(undefined2*)(disaTScriptManager->data2 + 0x30d8) = 0x3c;
            disaTScriptManager->field_0x4814 = 1;
            return;
        case 3: //Confirm
            //std::cout << "ConfirmClose(" << ");" << std::endl;
            disaTScriptManager->waitUserImputMode = 2;
            *(undefined2*)(disaTScriptManager->data2 + 0x30d8) = 0x3c;
            disaTScriptManager->field_0x4814 = 1;
            return;
        case 4: //CharLeft
        case 0xe: //CharMid
        case 0x18: { //CharRight
            uint charPos = 0;
            if (opCode == 4) {
                charPos = 1;
            }
            else if (opCode == 0xe) {
                charPos = 0;
            }
            else if (opCode == 0x18) {
                charPos = 2;
            }
            else {
                charPos = uVar9; //dosn't make sence
            }
            short slot = *(short*)disaTScriptManager->scriptPointer; //Char slot
            disaTScriptManager->scriptPointer += 2;
            short animationTypeAndSpeed = *(unsigned short*)disaTScriptManager->scriptPointer; //animation type
            disaTScriptManager->scriptPointer += 2;
            disaTScriptManager->field_0x47be[charPos] = (short)*disaTScriptManager->scriptPointer; //somekind of flag
            disaTScriptManager->scriptPointer++;

            /*
            if (opCode == 4) {
                std::cout << "CharLeft";
            }
            else if (opCode == 0xe) {
                std::cout << "CharMid";
            }
            else if (opCode == 0x18) {
                std::cout << "CharRight";
            }
            std::cout << "(" << (int)slot << ", " << (int)animationTypeAndSpeed / 200 << ", " << (int)animationTypeAndSpeed % 100 << ", " << (int)disaTScriptManager->field_0x47be[charPos] << ");" << std::endl;
            */


            if (animationTypeAndSpeed == 0) {
                disaTScriptManager->charDisplayAnimationLength[charPos] = 0;
                disaTScriptManager->charDisplayAnimationType[charPos] = 0;
                disaTScriptManager->charDisplaySpeed[charPos] = 0;
            }
            else {
                disaTScriptManager->charDisplayAnimationLength[charPos] = 0x168;
                disaTScriptManager->charDisplaySpeed[charPos] = (short)(animationTypeAndSpeed / 100);
                disaTScriptManager->charDisplayAnimationType[charPos] = animationTypeAndSpeed % 100;
            }


            if (slot - 1 >= 8) {
                slot = -1;
            }

            int i = 0;
            uVar9 = charPos;
            if (slot < 1) {
                disaTScriptManager->charDisplayIsFree[charPos] = 1;
            }
            else {
                disaTScriptManager->charDisplayXOffset[charPos] = 0;
                disaTScriptManager->charDisplayYOffset[charPos] = 0;

                struct DisaImageOffsetEntry {
                    int id;
                    short x, y;
                };
                //some images have a predefined offset
                DisaImageOffsetEntry* it = &FP::getData<DisaImageOffsetEntry>(0x005acba0);
                while (it->id != 9999) {
                    if (disaTScriptManager->loadedCharIds[slot] == it->id) {
                        disaTScriptManager->charDisplayXOffset[charPos] = it->x;
                        disaTScriptManager->charDisplayYOffset[charPos] = it->y;
                        break;
                    }
                    ++it;
                }

                disaTScriptManager->charDisplaySlot[charPos] = slot - 1;
                disaTScriptManager->charDisplayIsFree[charPos] = 0;
            }
        } break;
        case 5: { //Portrait
            short portraitId = *(short*)disaTScriptManager->scriptPointer;
            disaTScriptManager->scriptPointer += 2;

            //std::cout << "Portrait(" << (int)portraitId << ");" << std::endl;
            char portraitColorOld = disaTScriptManager->portraitColor;
            //transforms the portraitId (file id) to index
            //this index is used at 0x004bf40e -> 0x004c1cd1 to find a loaded image from DAT_00664098
            int result = FP::callFastFarA<int, int, int&, int&>(0x0046dc60, portraitId, disaTScriptManager->portraitIndex, disaTScriptManager->portraitColor);

            if (result < 0) {
                disaTScriptManager->portraitIndex = -1;
                disaTScriptManager->portraitColor = portraitColorOld;
            }
            //not used
            /*
            // Some leftover experiments to replace an image
            struct DisaImageInfo {
                int field_0x0[12];
                unsigned char* imgPtr;
                unsigned char* imgPtr2;
                unsigned char* imgColorTalblePtr;
                unsigned char* imgPixelPtr;
                int field_0x40[2];
                char u, i;
                short v;
                short a2, b2;
                short u3;
                short billScaleX, billScaleY;
                short b4;
                int field_0x41[12];

            };
            auto at = disaTScriptManager->portraitIndex * 0x88;
            DisaImageInfo* img = (DisaImageInfo*)(at + FP::getData<unsigned int>(0x00664098));
            */

        } break;
        case 6: { //Voice
            unsigned short voiceId = *(unsigned short*)disaTScriptManager->scriptPointer;
            disaTScriptManager->scriptPointer += 2;
            //std::cout << "Voice(" << voiceId << ");" << std::endl;
            FP::callFastFarA(0x005051a0, voiceId);
            disaTScriptManager->voiceId = voiceId;
            disaTScriptManager->voiceRunFlag = 0;
            return;
        }
        case 7: { //SpeechBubble
            unsigned char bubbleType = *disaTScriptManager->scriptPointer;
            disaTScriptManager->scriptPointer++;
            unsigned char arrowPosition = *disaTScriptManager->scriptPointer;
            disaTScriptManager->scriptPointer++;

            //std::cout << "SpeechBubble(" << (int)bubbleType << ", " << (int)arrowPosition << ");" << std::endl;

            disaTScriptManager->speechBubbleType = bubbleType - 1;
            disaTScriptManager->speechBubbleArrowPosition = arrowPosition;

        }break;
        case 8: { //NameLabel
            short nameId = *(short*)disaTScriptManager->scriptPointer; //name id
            disaTScriptManager->scriptPointer += 2;
            byte labelPosition = *disaTScriptManager->scriptPointer; //label position (bVar17)
            disaTScriptManager->scriptPointer++;

            //std::cout << "NameLabel(" << nameId << ", " << (int)labelPosition << ");" << std::endl;

            if (nameId == 0 || nameId == 0xffff) {
                disaTScriptManager->labelPosition = 0;
                disaTScriptManager->labelTextId = 0;
            }
            else {
                if (disaTScriptManager->labelTextId != nameId) {
                    disaTScriptManager->labelTextId = nameId;
                    if (nameId > 0) {
                        FP::callFastFarA<int>(0x004c0db0, (int)nameId);
                    }
                }
                disaTScriptManager->labelPosition = (ushort)labelPosition;
            }
            uVar9 = local_c;
        }break;
        case 9: {//Set
            DisaGameStateFilter gameStateFilter = *(DisaGameStateFilter*)disaTScriptManager->scriptPointer; //game state
            disaTScriptManager->scriptPointer += 2;

            //std::cout << "Set(" << gameStateFilter.index << ", " << gameStateFilter.bitIndex << ");" << std::endl;
            unsigned char* globalStateIt = &FP::getData<unsigned char>(aGlobalStateBits);
            gameStateFilter.set(globalStateIt);

            FP::callFastFarA(0x004bfa60, disaTScriptManager);
            return;
            uVar9 = local_c;
        } break;
        case 10: { //Clear
            DisaGameStateFilter gameStateFilter = *(DisaGameStateFilter*)disaTScriptManager->scriptPointer; //game state
            disaTScriptManager->scriptPointer += 2;

            unsigned char* globalStateIt = &FP::getData<unsigned char>(aGlobalStateBits);
            //std::cout << "Clear(" << gameStateFilter.index << ", " << gameStateFilter.bitIndex << ");" << "// is set " << !gameStateFilter.isSet(globalStateIt) << std::endl;
            gameStateFilter.clear(globalStateIt);

            //FP::callFastFarA(0x004bfa60, disaTScriptManager);
            //return;
            uVar9 = local_c;
        }  break;
        case 0xb: { //Script
            unsigned int scriptId = *(unsigned int*)disaTScriptManager->scriptPointer;
            scriptId &= 0x00FFFFFF;
            disaTScriptManager->scriptPointer = disaTScriptManager->scriptPointer + 3;
            //std::cout << "Script(" << scriptId << ");" << std::endl;

            FUN_004bd930_RunScript(scriptId, 0);
            uVar9 = local_c;
            if (FP::getData<int>(0x0071fd34) != 0) {
                return;
            }
        } break;
        case 0xc: { //Call
            const uint scriptId = (*(uint*)disaTScriptManager->scriptPointer) & 0x00FFFFFF;
            disaTScriptManager->scriptPointer += 3;
            //std::cout << "Call(" << scriptId << ");" << std::endl;

            disaTScriptManager->isActive = 0;
            const uint param_4 = FP::getData<uint>(0x00677264);
            const int scriptIndex = disaTScriptManager->FUN_004bf7e0_RunTalkScript(scriptId, 0, 0, param_4);
            //int scriptIndex = FP::callThisFarA<int>(0x04bf7e0, disaTScriptManager, scriptId, 0, 0, param_4);
            if (scriptIndex > -1) {
                return;
            }
            disaTScriptManager->isActive = 1;
            return;
        }

                //BEGIN custom code
        case 0x0d: { //script custom
            std::string scriptName((char*)disaTScriptManager->scriptPointer);
            while (*disaTScriptManager->scriptPointer) {
                ++disaTScriptManager->scriptPointer;
            }
            ++disaTScriptManager->scriptPointer;//skipZero

            //std::cout << "ScriptCustom(" << scriptName << ");" << std::endl;
            runCustomScript(scriptName, 0);

            if (FP::getData<int>(0x0071fd34) != 0) {
                return;
            }
        } break;
        case 0x0f: { //call custom
            std::string scriptName((char*)disaTScriptManager->scriptPointer);
            while (*disaTScriptManager->scriptPointer) {
                ++disaTScriptManager->scriptPointer;
            }
            ++disaTScriptManager->scriptPointer;//skipZero
            //std::cout << "CallCustom(" << scriptName << ");" << std::endl;

            disaTScriptManager->isActive = 0;
            const uint param_4 = FP::getData<uint>(0x00677264);
            const int scriptIndex = disaTScriptManager->call(scriptName, param_4);
            if (scriptIndex > -1) {
                //TOOD release script
                return;
            }
            disaTScriptManager->isActive = 1;

        } break;
            //END custom code

        case 0x14: { //MenuStart or MenuEntry
            if (disaTScriptManager->field16_0x16f2 != 0) {
                disaTScriptManager->field16_0x16f2 = 0;
                disaTScriptManager->dialogStringLength = 0;
                disaTScriptManager->dialogStringCurrentRow = 0;
            }

            unsigned char numberOfEntries = *disaTScriptManager->scriptPointer;
            disaTScriptManager->scriptPointer = disaTScriptManager->scriptPointer + 1;

            //std::cout << "MenuStart(" << (int)numberOfEntries << ");" << std::endl;

            uint maxEntryWidth = 0;

            for (unsigned char i = 0; i < numberOfEntries; ++i) {
                if (i == 0) {
                    //skip everything until the very first one, that indicates the start entry string
                    while (*disaTScriptManager->scriptPointer != 1) {
                        ++disaTScriptManager->scriptPointer;
                    }
                    ++disaTScriptManager->scriptPointer;
                }
                else {
                    //search for next entry. Entry is marked as 0x1401
                    while (*disaTScriptManager->scriptPointer != 0x14 || *(disaTScriptManager->scriptPointer + 1) != 1) {
                        ++disaTScriptManager->scriptPointer;
                    }
                    disaTScriptManager->scriptPointer += 2;
                }

                constexpr int maxStringEntrySize = 129; //0x81
                uint entryWidth = 0;
                uint stringByteSize = 0;
                if ((*(int*)(FP::getData<uint>(0x005d7c40) + 0x30) == 1) || (*(int*)(FP::getData<uint>(0x005d7c40) + 0x30) == 0)) {
                    entryWidth = FP::callFastFarA<int>(fGetWidthOfString, disaTScriptManager->scriptPointer, 0, -1, (int*)0x0, -1);

                    byte* copyTargetIt = disaTScriptManager->data + i * maxStringEntrySize + 0x65c; ; //to where the string should be copied to
                    byte* copyTargetEndIt = copyTargetIt + maxStringEntrySize; //custom code
                    byte* copyFromIt = disaTScriptManager->scriptPointer; //from where the string should be copied from

                    while (char c = *copyFromIt) {
                        *copyTargetIt = c;
                        ++copyTargetIt;
                        ++copyFromIt;
                        //BEGIN custom code
                        //peform range check to be save
                        if (copyTargetIt >= copyTargetEndIt) {
                            exit(0);
                        }
                        //END custom code
                    }
                    stringByteSize = std::distance(disaTScriptManager->scriptPointer, copyFromIt);
                }
                else {
                    unsigned long long count = FP::callFastFarA<unsigned long long>(0x004e3790, (int)disaTScriptManager->scriptPointer);
                    entryWidth = (int)count * 0xd;
                    FP::callFastFarA(0x004e3530, (int)(disaTScriptManager->data + i * maxStringEntrySize + 0x65c), (char*)(count >> 0x20));
                    stringByteSize = (int)count * 2;
                }

                maxEntryWidth = std::max(maxEntryWidth, entryWidth);

                *(byte**)(disaTScriptManager->data + i * 4 + 4) = disaTScriptManager->scriptPointer + stringByteSize + 1; //branch path
            }

            //set script pointer to dialog end
            while (*disaTScriptManager->scriptPointer != 0xfd) {
                ++disaTScriptManager->scriptPointer;
            }
            ++disaTScriptManager->scriptPointer;

            FP::getData<char>(0x00619332) = FP::getData<char>(0x00619332) & 0x1f;
            FP::getData<char>(0x00619333) = FP::getData<char>(0x00619333) & 0xe0;
            disaTScriptManager->waitUserImputMode = 3;
            *(undefined2*)(disaTScriptManager->data + 0x64a) = 1;
            *(short*)(disaTScriptManager->data + 0x652) = (short)maxEntryWidth;
            *(ushort*)(disaTScriptManager->data + 0x654) = (ushort)numberOfEntries;
            disaTScriptManager->field21_0x4800 = 0;
            disaTScriptManager->dialogStringCurrentRow = disaTScriptManager->field14_0x16ec % 3;
            return;
        }
        case 100: { //load img
            const uint value = (*(uint*)disaTScriptManager->scriptPointer) & 0x00FFFFFF;
            disaTScriptManager->scriptPointer += 3;

            const uint slot = (value / 10000) - 1;
            const uint imageId = value % 10000; //BU**  image id

            //std::cout << "LoadImg(" << slot << ", " << imageId << ");" << std::endl;

            if ((imageId - 0x65 < 0x26ab) && slot < 8) {
                disaTScriptManager->loadedCharIds[slot] = (short)imageId;

                int* piVar6 = (int*)(&FP::getData<int>(0x006b77d0))[slot];
                (&FP::getData<int>(0x00702330))[slot * 0x22] = (int)piVar6;
                byte* slotPtr = &FP::getData<byte>(0x00702300) + slot * 0x88;

                undefined4 in_stack_ffffffc0 = 0;
                undefined4 in_stack_ffffffc4 = 0;
                FP::callFastFarA(0x0050b3f0, (int)(slotPtr), imageId, piVar6, in_stack_ffffffc0, in_stack_ffffffc4, slotPtr);

                return;
            }
        }break;
        case 0x65: { //ConfirmAuto 
            int delay = *(short*)disaTScriptManager->scriptPointer;
            disaTScriptManager->scriptPointer += 2;
            int delayAlt = *(short*)disaTScriptManager->scriptPointer;
            disaTScriptManager->scriptPointer += 2;
            int keepDialogBoxOpen = *disaTScriptManager->scriptPointer;
            disaTScriptManager->scriptPointer++;

            //std::cout << "ConfirmAuto(" << delay << ", " << delayAlt << ", " << keepDialogBoxOpen << ");" << std::endl;

            if (((FP::getData<char>(0x006194f3) & 4) != 0) && (delayAlt != 0)) {
                delay = delayAlt;
            }
            if (delay == 0) {
                return;
            }
            disaTScriptManager->field5_0x58 = delay;
            disaTScriptManager->field5_0x5a = keepDialogBoxOpen;
            disaTScriptManager->field_0x47fa = 0x3c;
            disaTScriptManager->waitUserImputMode = 4;
            disaTScriptManager->field_0x4814 = 1;
            return;
        }
        case 0xc9: { //Showbox
            const char value = *disaTScriptManager->scriptPointer;
            disaTScriptManager->scriptPointer++;
            //std::cout << "Showbox(" << (value == 0 ? "false" : "true") << ");" << std::endl;
            if (value == 0) {
                disaTScriptManager->bgBoxSpeed = 8;
            }
            else if (value == 1) {
                disaTScriptManager->bgBoxSpeed = -8;
            }
        } break;
        case 0xfa: //NOP
            //std::cout << "NOP(" << ");" << std::endl;
            disaTScriptManager->scriptPointer = disaTScriptManager->scriptPointer + 1;
            break;
        case 0xfd:  //MenuEnd
        case 0xff:  //return

            //std::cout << "MenuEnd(" << ");" << std::endl;
            iVar18 = 0;
            bVar17 = 1;
            do {
                iVar16 = iVar18 >> 3;
                cVar1 = -bVar17;
                iVar18 = iVar18 + 1;
                bVar17 = bVar17 << 1 | (char)bVar17 < '\0';
                (&FP::getData<byte>(0x00619330))[iVar16] = (&FP::getData<byte>(0x00619330))[iVar16] & cVar1 - 1U;
            } while (iVar18 < 100);

            //clear data
            *(undefined2*)(disaTScriptManager->data + 0x64a) = 0;
            disaTScriptManager->speechBubbleAnimationFrame = 0;

            std::fill(std::begin(disaTScriptManager->charDisplaySlot), std::begin(disaTScriptManager->charDisplaySlot) + 3, -1);
            std::fill(std::begin(disaTScriptManager->charDisplayXOffset), std::begin(disaTScriptManager->charDisplayXOffset) + 3, 0);
            std::fill(std::begin(disaTScriptManager->charDisplayYOffset), std::begin(disaTScriptManager->charDisplayYOffset) + 3, 0);
            std::fill(std::begin(disaTScriptManager->dataMid), std::begin(disaTScriptManager->dataMid) + 8, 0); //not all are cleard
            std::fill(std::begin(disaTScriptManager->loadedCharIds), std::begin(disaTScriptManager->loadedCharIds) + 9, 0);

            disaTScriptManager->labelPosition = 0;
            disaTScriptManager->labelTextId = 0;
            disaTScriptManager->field5_0x58 = 0;
            disaTScriptManager->isActive = 0;
            disaTScriptManager->bgBoxSpeed = 8;
            return;
        case 0xfe: { //?
            //std::cout << "Restart(" << ");" << std::endl;
            if (disaTScriptManager->field_0x18[0] != 0) {
                disaTScriptManager->scriptPointer--;
                disaTScriptManager->field_0x47fa = 0x3c;
                disaTScriptManager->waitUserImputMode = 1;
                disaTScriptManager->field_0x4814 = 1;
                return;
            }
            *(undefined2*)(disaTScriptManager->data + 0x64a) = 0;
            if (disaTScriptManager->speechBubbleAnimationFrame != 0) {
                disaTScriptManager->speechBubbleAnimationSpeed = -1;
                disaTScriptManager->speechBubbleAnimationFrame = 0xf;
                if (disaTScriptManager->speechBubbleType == 1) {
                    disaTScriptManager->speechBubbleAnimationSpeed = -2;
                    disaTScriptManager->speechBubbleAnimationFrame = 0xe;
                }
            }
            unsigned int param_4 = FP::getData<unsigned int>(0x00677264);
            disaTScriptManager->isActive = 0;
            //FP::callThisFarA<int>(0x04bf7e0, disaTScriptManager, disaTScriptManager->scriptId, disaTScriptManager->dungeonId, disaTScriptManager->unknownFlag, param_4);
            disaTScriptManager->FUN_004bf7e0_RunTalkScript(disaTScriptManager->scriptId, disaTScriptManager->dungeonId, disaTScriptManager->unknownFlag, param_4);
            return;
        }

        }
    }
}