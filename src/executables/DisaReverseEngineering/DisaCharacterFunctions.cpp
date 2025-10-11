#include "DisaCharacterFunctions.h"

#include <sstream>
#include <iomanip>
#include <ranges>
#include "FunctionPatcher.h"
#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_StringUtil.h>
#include <Disa/fileformats/PG_ShiftJisEncoder.h>
#include <Disa/fileformats/PG_JsonUtil.h>
#include <DisaDataFunctions.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Managers/DisaCustomCharacterManager.h>

using namespace PG_JsonUtil;

template<typename T, uint32_t AddressOrgData, uint32_t AddressOrgSize>
struct DisaDataRemap {
    std::vector<T> customData;
    T* originalPtr = nullptr;
    uint32_t originalSize = 0;

    void peformSteel() {
        if (!originalPtr) {
            originalPtr = FP::getData<T*>(AddressOrgData);
            originalSize = FP::getData<uint32_t>(AddressOrgSize);
        }
        customData.clear();
        customData.reserve(originalSize + 10);
        auto range = std::ranges::subrange(originalPtr, originalPtr + originalSize);
        customData.insert(customData.end(), range.begin(), range.end());
        FP::getData<T*>(AddressOrgData) = customData.data();
    }

    T& addNewEntry() {
        T& back = customData.emplace_back();
        FP::getData<T*>(AddressOrgData) = customData.data();
        FP::getData<uint32_t>(AddressOrgSize) = customData.size();
        return back;
    }
};

static DisaDataRemap<DisaDataCharacterInfo, 0x005d9cdc, 0x00663ff8> gDisaDataCharacterRemap;
static DisaDataRemap<DisaDataMagicInfo, 0x0066402c, 0x00664030> gDisaDataMagicRemap;


void DisaDataCharacterInfo::steal() {
    gDisaDataCharacterRemap.peformSteel();
}

DisaDataCharacterInfo& DisaDataCharacterInfo::addNewEntry() {
    return gDisaDataCharacterRemap.addNewEntry();
}

unsigned int DisaDataCharacterInfo::getSize() {
    return gDisaDataCharacterRemap.customData.size(); //FP::getData<unsigned int>(0x00663ff8);
}

DisaDataCharacterInfo& DisaDataCharacterInfo::getEntry(unsigned int index) {
    return gDisaDataCharacterRemap.customData[index];
}

std::vector<DisaDataCharacterInfo>::iterator DisaDataCharacterInfo::begin() {
    return gDisaDataCharacterRemap.customData.begin();
}
std::vector<DisaDataCharacterInfo>::iterator DisaDataCharacterInfo::end() {
    return gDisaDataCharacterRemap.customData.end();
}

void DisaDataMagicInfo::steal() {
    gDisaDataMagicRemap.peformSteel();
}

DisaDataMagicInfo& DisaDataMagicInfo::addNewEntry() {
    return gDisaDataMagicRemap.addNewEntry();
}
unsigned int DisaDataMagicInfo::getSize() {
    return gDisaDataMagicRemap.customData.size();
}

DisaDataMagicInfo& DisaDataMagicInfo::getEntry(unsigned int index) {
    return gDisaDataMagicRemap.customData[index];
}

std::vector<DisaDataMagicInfo>::iterator DisaDataMagicInfo::begin() {
    return gDisaDataMagicRemap.customData.begin();
}
std::vector<DisaDataMagicInfo>::iterator DisaDataMagicInfo::end() {
    return gDisaDataMagicRemap.customData.end();
}

std::string DisaDataMagicInfo::globalToString() {
    std::stringstream stream;

    stream << " == Global magic/skills with " << DisaDataMagicInfo::getSize() << " items == \n";
    for (const DisaDataMagicInfo& info : std::ranges::subrange(DisaDataMagicInfo::begin(), DisaDataMagicInfo::end())) {
        stream << " - 0x" << std::hex << &info << std::dec << ": " << info.toString() << "\n";
    }

    return stream.str();
}


std::string DisaPartyCharacterInfo::playerPartyToString() {
    std::stringstream stream;
    const unsigned int unitsOwnedSize = FP::getData<unsigned short>(aUnitsOwned);
    stream << " == Player Party with " << unitsOwnedSize << " units == \n";
    const auto* info = &FP::getData<DisaPartyCharacterInfo>(0x0061a45c - 0x171 * 4);
    for (int i = 0; i < unitsOwnedSize; ++i) {
        stream << " - 0x" << std::hex << info << std::dec << ": " << info->toString() << "\n";
        ++info;
    }

    return stream.str();
}

std::string DisaPartyCharacterInfo::playerPartyToJson() {
    nlohmann::json j;
    j["version"] = 1;
    j["buildDate"] = std::string(__DATE__);
    const unsigned int unitsOwnedSize = FP::getData<unsigned short>(aUnitsOwned);
    if (unitsOwnedSize == 0) return j.dump(4);

    nlohmann::json& charcters = j["characters"];
    DisaPartyCharacterInfo* info = &FP::getData<DisaPartyCharacterInfo>(0x0061a45c - 0x171 * 4);
    static_assert(sizeof(DisaPartyCharacterInfo) == DisaPartyCharacterInfo::byteSize, "Has invalid size! Should be DisaPartyCharacterInfo::byteSize!");
    for (int i = 0; i < unitsOwnedSize; ++i) {
        charcters.push_back(info->toJson());
        ++info;
    }

    return j.dump(4);
}

DisaPartyCharacterInfo& DisaPartyCharacterInfo::addNewEntry() {
    unsigned short& unitsOwnedSize = FP::getData<unsigned short>(aUnitsOwned);
    DisaPartyCharacterInfo* ret = (&FP::getData<DisaPartyCharacterInfo>(0x0061a45c - 0x171 * 4) + unitsOwnedSize);
    ++unitsOwnedSize;
    memset((char*)ret, 0, sizeof(DisaPartyCharacterInfo));
    return *ret;
}

unsigned int DisaPartyCharacterInfo::getSize() {
    return FP::getData<unsigned short>(aUnitsOwned);
}
DisaPartyCharacterInfo& DisaPartyCharacterInfo::getEntry(unsigned int index) {
    return *(&FP::getData<DisaPartyCharacterInfo>(0x0061a45c - 0x171 * 4) + index);
}

DisaPartyCharacterInfo* DisaPartyCharacterInfo::begin() {
    return &FP::getData<DisaPartyCharacterInfo>(0x0061a45c - 0x171 * 4);
}

DisaPartyCharacterInfo* DisaPartyCharacterInfo::end() {
    return (&FP::getData<DisaPartyCharacterInfo>(0x0061a45c - 0x171 * 4) + getSize());
}

void __fastcall FUN_0047f080_ProccessStatusMenu(unsigned int param_1) {
    constexpr unsigned int aStatusMenuCursorIndex = 0x0088ad2c;
    constexpr unsigned int aStatusMenuScrollOffset = 0x0088ad30;

    int& cursorIndex = FP::getData<int>(aStatusMenuCursorIndex); //cursor index
    const int oldCursorIndex = cursorIndex; //used to check chenged
    int& scrollOffset = FP::getData<int>(aStatusMenuScrollOffset);

    constexpr unsigned int menuEntries = 8u;
    const unsigned int unitsOwnedSize = FP::getData<unsigned short>(aUnitsOwned);
    const unsigned int numberOfDisplayEntries = std::min(menuEntries, unitsOwnedSize);


    DisaKeyStates& keyStates = FP::getData<DisaKeyStates>(0x00778d98);
    //EXIT
    if (keyStates.isKeyPressed(DisaKeyStates::Cancel) || FP::callFastFarA<bool>(fIsExitPressed, 0)) {

        *(int*)(FP::getData<unsigned int>(0x005d7c70) + 0x30543c) = 0;
        int iVar1 = FP::getData<int>(0x00677274);
        keyStates.clear();
        FP::callFastFarA<void>(fPlaySound, 7, iVar1, 0, param_1, 0);
        FP::getData<int>(0x0088ace0) = 0;
    }

    //ENTER details for unit
    if (keyStates.isKeyPressed(DisaKeyStates::Select) ||
        (FP::callFastFarA<bool>(fIsMouseCursorInAABB, 0x20, 0x16, 0x13e, (numberOfDisplayEntries + 1) * 0x16, '\0') && FP::callFastFarA<bool>(fIsLeftMouseBtnClicked, false)) ||
        (*(char*)(FP::getData<unsigned int>(0x005d7c70) + 0x305438) == '\0' && *(char*)(FP::getData<unsigned int>(0x005d7c70) + 0x30543a) != '\0' && ((FP::getData<unsigned int>(0x00701c58) & 0x2000000) != 0))) {

        *(int*)(FP::getData<unsigned int>(0x005d7c70) + 0x30543c) = 0;
        int iVar2 = FP::getData<int>(0x00677274);
        int iVar1 = FP::getData<int>(0x005d7c70);
        keyStates.clear();
        FP::callFastFarA<void>(fPlaySound, 1, iVar2, 0, iVar1, 0);

        FP::getData<int>(0x0088ace0) = FP::getData<int>(0x0088ace4); //set menu to open (6)
        FP::getData<int>(0x0088ad34) = scrollOffset + cursorIndex; //selected unit index


        FP::getData<int>(0x0088ace8) = 0;
        FP::getData<int>(0x0088ad10) = 0;
        FP::getData<int>(0x0088acfc) = 0;
        FP::getData<int>(0x0088acf4) = 0;
        FP::getData<int>(0x0088ad00) = 0;
        FP::getData<int>(0x0088acf8) = 0;
        FP::getData<int>(0x0088ad38) = 0;
    }


    FP::callFastFarA<void>(0x0050fb90, 0x20, 0x16, 0x11e, 0x16, numberOfDisplayEntries, &cursorIndex, '\0'); //hand cursor position
    FP::callFastFarA<void>(0x0050fc10, 0x20, 0x16, 0x11e, 0x16, numberOfDisplayEntries, &cursorIndex, scrollOffset, (int)FP::getData<short>(0x0065db10));
    const bool hoverChanged = oldCursorIndex != cursorIndex;

    if (hoverChanged) {
        FP::callFastFarA<void>(fPlaySound, 3, FP::getData<int>(0x00677274), 0, 0, 0);
    }

    if (unitsOwnedSize > 0) {
        const int endScrollIndex = unitsOwnedSize <= menuEntries ? 0 : unitsOwnedSize - menuEntries;
        const unsigned char selectionChangedByte = hoverChanged ? 2 : 0;

        //HoverIndex, EntriesScrollOffset, numberOfDisplayEntries, endScrollIndex, selectionChangedByte
        FP::callFastFarA(fHandleListScrolling, &cursorIndex, &scrollOffset, numberOfDisplayEntries, endScrollIndex, selectionChangedByte); //handle scrolling
    }
    else {
        scrollOffset = 0;
        cursorIndex = 0;
    }

    FP::callFastFarA<void>(0x0047ac30, 0);//render status screen
}


int64_t FUN_00523f900_floatToInt64Truncated(float param_1) {
    if (FP::getData<unsigned int>(0x005b3160) != 0) {
        // Fast path: uses truncating float-to-int conversion
        return static_cast<int64_t>(static_cast<int32_t>(param_1));
    }

    // Slow path: emulate truncating behavior carefully
    double fval = static_cast<double>(param_1);

    // Convert to int64 using rounding (based on FPU)
    int64_t rounded = static_cast<int64_t>(std::nearbyint(fval));
    double back = static_cast<double>(rounded);

    // Check if original float was not a perfect integer
    if (rounded != 0) {
        double diff = back - fval;

        if (diff == 0.0)  return rounded;

        if (fval < 0.0) {
            // Correct rounding toward zero for negatives
            if (rounded > std::numeric_limits<int64_t>::min())
                rounded += 1;
        }
        else {
            // Correct rounding toward zero for positives
            if (rounded > std::numeric_limits<int64_t>::min())
                rounded -= 1;
        }

    }

    return rounded;
}

inline int positiveMod(int a, int mod) {
    if (mod <= 0) return 0;
    a %= mod;
    if (a < 0) a += mod;
    return a;
}

void FUN_0048cd40_BuildAssemblyUnitInfosLookup() {
    unsigned short& assemblyUnitInfosLookupSize = FP::getData<unsigned short>(aAssemblyUnitInfosLookupSize);
    assemblyUnitInfosLookupSize = 0;
    const auto& assemblyUnitInfos = FP::getData<std::array<DisaAssemblyUnitInfo, 288>>(aAssemblyUnitInfos);
    auto& assemblyUnitInfosLookup = FP::getData<std::array<unsigned short, 288>>(aAssemblyUnitInfosLookup);

    //build lookup table for every set character
    for (unsigned short i = 0; i < assemblyUnitInfos.size(); ++i) {
        if (assemblyUnitInfos[i].characterId == 0) continue;
        assemblyUnitInfosLookup[assemblyUnitInfosLookupSize++] = i;
    }
}

/**
* @brief Copy a shiftJis string from source to dist
* About the same as std::strcpy
*/
char* __fastcall FUN_004e3530_shiftJisCpy(char* dist, char* source) {

    if (!dist || !source) return nullptr;

    char* origDist = dist;
    int len = 0;
    while (len <= 255) {
        *dist = *source;
        *(dist + 1) = *(source + 1);
        if ((!*source && !*(source + 1))) break;
        dist += 2;
        source += 2;
        len += 2;
    }

    return origDist;
}

/**
* @brief Compulte length of a shiftJis string
* About the same as std::strlen
*/
unsigned int __fastcall FUN_004e37b0_shiftJisLen(unsigned short* str) {
    if (!str) return 0;

    unsigned int len = 0;
    while (unsigned short v = *str) {
        if (v == 0 || v == 0x20 || v == 0x3000 || (0xff00 < v && v < 0xff5e) || v == 0x2018 || v == 0x2019 || v == 0x201c || v == 0x201d || v == 0x2461) {
            ++len;
        }
        else {
            len += 2;
        }
        ++str;
    }
    return len;
}

struct DisaAssemblyInfo {
    const int& unitCount;
    const unsigned short* unitVarianceCount;
    const DisaAssemblyUnitInfo* assemblyUnitInfos; //288
    const unsigned short* assemblyUnitInfosLookup;


    static inline bool isCustomDisplay = false;
    static inline int customUnitCount = 0;
    static inline std::vector<unsigned short> customUnitVarianceCount;
    static inline std::vector<DisaAssemblyUnitInfo> customAssemblyUnitInfos;
    static inline std::vector<unsigned short> customAssemblyUnitInfosLookup;
    static inline std::vector<unsigned int> customAssemblyLookUp;

    static void buildCustom() {
        static bool customCharsAreBuild = false;

        if (customCharsAreBuild) return;

        std::map<std::string, std::vector<unsigned short>> groups;

        for (auto& mod : DisaCustomCharacterManager::get()->getMods()) {
            for (auto& modChar : mod.characters) {
                if (!modChar.identifier || !modChar.isRecruitable) continue;
                if (modChar.recruitGroupName.empty()) {
                    groups[modChar.recruitGroupName].push_back(modChar.charData.characterId);
                }
                else {
                    groups[mod.namespaceMod + modChar.recruitGroupName].push_back(modChar.charData.characterId);
                }
            }
        }

        //add all untis that belong to a group
        for (const auto& [name, ids] : groups) {
            if (name.empty()) continue;

            customUnitVarianceCount.push_back(ids.size());
            customAssemblyLookUp.push_back(customAssemblyUnitInfos.size());
            for (auto id : ids) {
                customAssemblyUnitInfosLookup.push_back(customAssemblyUnitInfos.size());
                customAssemblyUnitInfos.emplace_back(0, id, 0);
            }
            ++customUnitCount;
        }

        //add all units that have no group
        if (auto it = groups.find(std::string()); it != groups.end()) {
            for (unsigned short id : it->second) {
                customUnitVarianceCount.push_back(1);
                customAssemblyLookUp.push_back(customAssemblyUnitInfos.size());
                customAssemblyUnitInfosLookup.push_back(customAssemblyUnitInfos.size());
                customAssemblyUnitInfos.emplace_back(0, id, 0);
                ++customUnitCount;
            }
        }


        customCharsAreBuild = true;
    }

    static DisaAssemblyInfo getFromDisa() {
        return { .unitCount = FP::getData<int>(0x0088bd54),
                .unitVarianceCount = &FP::getData<std::array<unsigned short, 64>>(0x0088c060).front(),
            .assemblyUnitInfos = &FP::getData<std::array<DisaAssemblyUnitInfo, 288>>(aAssemblyUnitInfos).front(),
            .assemblyUnitInfosLookup = &FP::getData<std::array<unsigned short, 288>>(aAssemblyUnitInfosLookup).front()
        };
    }

    static DisaAssemblyInfo getFromCustom() {
        return { .unitCount = customUnitCount,
            .unitVarianceCount = customUnitVarianceCount.data(),
            .assemblyUnitInfos = customAssemblyUnitInfos.data(),
            .assemblyUnitInfosLookup = customAssemblyUnitInfosLookup.data(),
        };
    }

    static DisaAssemblyInfo get() {
        return isCustomDisplay ? getFromCustom() : getFromDisa();
    }

    static unsigned short getLookUpIndex(unsigned short unitIndex, unsigned short unitVarianceIndex) {
        if (isCustomDisplay) {
            return customAssemblyLookUp[unitIndex] + unitVarianceIndex;
        }
        return (&FP::getData<unsigned short>(0x0088bd60))[unitVarianceIndex + unitIndex * 6];
    }
};

void FUN_0048dfd0_ProccessAssemblyMenu() {
    //FP::callFarA<void>(0x0048dfd0);//render status screen
    //return;

    char* pcVar1;
    char cVar2;
    int iVar6;
    int iVar7;
    char* pcVar8;
    unsigned int uVar10;
    int iVar11;
    int local_44;


    DisaKeyStates& keyStates = FP::getData<DisaKeyStates>(0x00778d98);
    int& unitIndex = FP::getData<int>(0x0088adbc);
    int& unitVarianceIndex = FP::getData<int>(0x0088adc4);
    DisaTransform transform;
    DisaCameraInfo& cameraInfo = FP::getData<DisaCameraInfo>(0x00703280);
    const DisaSettings* settings = FP::getData<DisaSettings*>(0x005d7c40);
    auto& creationCharacter = FP::getData<DisaPartyCharacterInfo>(0x0088ae40); //target data of the new char

    DisaAssemblyInfo::buildCustom();
    const auto& [unitCount, unitVarianceCount, assemblyUnitInfos, assemblyUnitInfosLookup] = DisaAssemblyInfo::get();
    unitIndex = std::min(unitIndex, unitCount - 1);

    int& movement = FP::getData<int>(0x0088adc0); //on pressing W or S, is negative when going back
    if (movement == 0) {
        int btnState = FP::getData<int>(0x00778d64);
        short joystickState = FP::getData<short>(0x00778d62); //maybe
        if ((btnState & 0x10) || joystickState < -0x46) {
            movement = 4;
        }

        if ((btnState & 0x40) || !(joystickState < 0x47)) {
            movement = -4;
        }

        //on first transition pause before continuing to next unit
        //this prevents a "unit skip" if player holds the button longer then 4 frames
        int& pauseCounter = FP::getData<int>(0x0088c630);
        constexpr int pauseTicks = 19;
        if (movement == 0) {
            pauseCounter = 0;
        }
        else {
            ++pauseCounter;
            if ((pauseCounter == 1) || (pauseCounter > pauseTicks)) {
                FP::callFastFarA<void>(fPlaySound, 3, FP::getData<int>(0x00677274), 0, 0, 0);
                pauseCounter = std::min(pauseCounter, pauseTicks); //I guess they are afraid that player holds the button longer then INT_MAX :P
            }
            else {
                movement = 0; //pause transision
            }
        }

    }
    else if (movement < 0) {
        ++movement;
        if (movement % 4 == 0) {
            --unitIndex;
            if (unitIndex < 0) {
                unitIndex = unitCount - 1; //loop around
            }
        }
    }
    else if (movement > 0) {
        --movement;
        if (movement % 4 == 0) {
            ++unitIndex;
            if (unitIndex >= unitCount) {
                unitIndex = 0; // loop around
            }
        }
    }

    if (keyStates.isKeyPressed(DisaKeyStates::Left) && unitVarianceIndex > 0) {
        --unitVarianceIndex;
        FP::callFastFarA<void>(fPlaySound, 3, FP::getData<int>(0x00677274), 0, 0, 0);
    }
    else if (keyStates.isKeyPressed(DisaKeyStates::Right) && unitVarianceIndex < static_cast<int>(unitVarianceCount[unitIndex])) {
        ++unitVarianceIndex;
        FP::callFastFarA<void>(fPlaySound, 3, FP::getData<int>(0x00677274), 0, 0, 0);
    }
    unitVarianceIndex = glm::clamp(unitVarianceIndex, 0, static_cast<int>(unitVarianceCount[unitIndex]) - 1);
    iVar11 = unitVarianceIndex;

    unsigned int selectedUnitLookupIndex = DisaAssemblyInfo::getLookUpIndex(unitIndex, unitVarianceIndex);
    const DisaAssemblyUnitInfo& selectedUnitInfo = assemblyUnitInfos[assemblyUnitInfosLookup[selectedUnitLookupIndex]];

    FP::callFastFarA<void>(0x004dafc0, &creationCharacter, selectedUnitInfo.characterId, 1, 0, 0); //load char info

    cameraInfo.move(glm::vec3(unitVarianceIndex * 21.f, 0.f, 0.f), 5.f);

    int counter = 0;
    float xOffset = -75.f;
    constexpr int unitRows = 5; //5
    constexpr int unitRowOffset = 3; //3
    for (int unitOffsetIndex = 0; unitOffsetIndex <= unitRows; ++unitOffsetIndex) {

        int currentUnitIndex = unitIndex + unitOffsetIndex - unitRowOffset;
        currentUnitIndex %= unitCount;
        if (currentUnitIndex < 0) currentUnitIndex += unitCount; //loop around

        unsigned short unitLookupIndex = DisaAssemblyInfo::getLookUpIndex(currentUnitIndex, 0);
        for (int variantIndex = 0; variantIndex < unitVarianceCount[currentUnitIndex]; ++variantIndex) {
            unsigned int unitLookUpSpriteIdIndex = assemblyUnitInfosLookup[unitLookupIndex]; //lookup table for unit index
            unsigned int characterId = assemblyUnitInfos[unitLookUpSpriteIdIndex].characterId;
            unsigned int* unitMemory = (unsigned int*)(&FP::getData<unsigned char>(0x00738718) + counter * 0x10b);
            FP::callThisFarA<void>(0x0046b1e0, unitMemory + -2, characterId); //load unit into memory

            //retrives some unknown data
            unsigned int spriteId = *(short*)(unitMemory + 0x8a);
            unsigned int someMemory = (unsigned int)FUN_004de880_getSpriteData(spriteId);
            unitMemory[-2] = someMemory;
            if (someMemory != 0) {
                unsigned int someSize = *(short*)(someMemory + 4);
                uVar10 = someSize;
                short* someValue = (short*)(*(int*)(someMemory + 0x1c) + 2);
                int foundIndex = 0;
                for (; foundIndex < someSize; ++foundIndex) {
                    if (*someValue == 1) goto LAB_FoundValue; //found
                    someValue += 2;
                }
                foundIndex = 0;//not found
            LAB_FoundValue:
                unsigned int someAddress = *(int*)(someMemory + 0x2c) + *(short*)(*(int*)(someMemory + 0x1c) + foundIndex * 4) * 8;
                unitMemory[-1] = someAddress;
                unitMemory[1] = 1;
                unitMemory[2] = unitLookUpSpriteIdIndex;
                unitMemory[3] = 0;
            }


            unitMemory[0x54] = 0;
            unitMemory[0x6b] = 0;
            *(unsigned char*)((int)unitMemory + 0x261) = 0;
            *(unsigned short*)((int)unitMemory + 0x236) = 0xffa6;
            *(unsigned short*)((int)unitMemory + 0x232) = 0xffa6;


            float animatedXOffset = 0.f;
            //use movement value to create a transition scroll animation between units
            const int absMovement = abs(movement / 5) + 1;
            if (movement > 0) {
                animatedXOffset = static_cast<float>((movement + absMovement * -5) * 5);
            }
            else if (movement < 0) {
                animatedXOffset = static_cast<float>((movement + absMovement * 5) * 5);
            }
            animatedXOffset += xOffset;

            ((float*)unitMemory)[0x55] = animatedXOffset; //x
            ((float*)unitMemory)[0x53] = float(variantIndex) * 21.f; //z
            unitMemory[0x70] = 0xff00ff; //color
            unitMemory[0x6f] = 0xff00ff; //color
            if (*unitMemory != 0) { //Custom memory check
                FP::callStdFastFarA<int, unsigned int*, unsigned int&>(0x0046c830, unitMemory - 2, uVar10, &cameraInfo, 0);
                FP::callStdFastFarA<void>(0x0046cf80, unitMemory - 2, uVar10, &cameraInfo, 0, 0, '\0', 41000, '\0');
            }

            ++unitLookupIndex;
            ++counter;
        }

        xOffset += 25.f;
    }



    { //render ground cicle
        transform.position.x = (float)(unitVarianceIndex * 21);
        unsigned int& anim = FP::getData<unsigned int>(0x0088c0e0);
        FP::callThisFarA<void>(fRender3DObject, &FP::getData<unsigned int>(0x007492f8), (float)anim, &cameraInfo, &transform, (float*)0x0, (float*)0x0, 0, 0, 0);
        ++anim;
        if (anim >= FP::getData<unsigned int>(0x00749330)) anim = 0;
    }

    { //render selection prisim
        transform.position.x -= 1.0;
        transform.position.y = -17.0;
        transform.position.z = -3.0;
        unsigned int& anim = FP::getData<unsigned int>(0x0088bd58);
        FP::callThisFarA<void>(fRender3DObject, &FP::getData<unsigned int>(0x007493e0), (float)anim, &cameraInfo, &transform, (float*)0x0, (float*)0x0, 45000.f, 0x50, 0);
        ++anim;
        if (anim >= FP::getData<unsigned int>(0x00749418)) anim = 0;
    }

    { //find full character info to display them
        DisaDataCharacterInfo* characterInfo = nullptr;
        const unsigned int characterInfoSize = FP::getData<unsigned int>(0x00663ff8);
        auto* beginIt = FP::getData<DisaDataCharacterInfo*>(0x005d9cdc);
        auto characterInfos = std::ranges::subrange(beginIt, beginIt + characterInfoSize);
        if (auto findIt = std::ranges::find(characterInfos, selectedUnitInfo.characterId, &DisaDataCharacterInfo::characterId); findIt != characterInfos.end()) {
            characterInfo = findIt;
        }
        //render all on top stats and character infos
        FP::callFastFarA<void>(0x0048dba0, characterInfo, &selectedUnitInfo);
    }

    //BEGIN custom code
    //render menu item to switch to modded characters
    if (DisaAssemblyInfo::customUnitCount > 0) {
        auto customCharsStr = DisaAssemblyInfo::isCustomDisplay ? stringToShiftJis("Default Characters") : stringToShiftJis("Modded Characters");
        bool isHover = false;
        const bool isSelected = processSingleLineTextBox(settings, glm::ivec2(32, 164), customCharsStr, isHover, DisaSettings::GeoEffectVisibility, 141);
        renderSingleLineTextBox(settings, glm::ivec2(32, 164), customCharsStr, isHover, DisaSettings::GeoEffectVisibility, 141);
        if (isSelected) {
            FP::callFastFarA<void>(fPlaySound, 1, FP::getData<int>(0x00677274), 0, 0, 0);
            static int prevUnitIndex = 0;
            static int prevtVarianceIndex = 0;
            std::swap(unitIndex, prevUnitIndex);
            std::swap(unitVarianceIndex, prevtVarianceIndex);

            DisaAssemblyInfo::isCustomDisplay = !DisaAssemblyInfo::isCustomDisplay;
            return;
        }
    }
    //END custom code

    bool isMouseInsideRootTile = false;

    if (settings->isMouseActive) {
        transform.clear();
        glm::vec3 hoverWorldPos = glm::vec3(0.f);
        FP::callFastFarA<void>(0x004d1070, &hoverWorldPos, FP::getData<unsigned int>(0x00701c80), FP::getData<unsigned int>(0x00701c84), 0.f, '\x01');

        /* ORIGINAL
        //wtf is this code bro. Its the most ugliest thing I have ever seen :(
        const float maxDepth = unitVarianceIndex * 21;
        const bool isInXRange = (maxDepth - 10.5f < hoverWorldPos.x) && (hoverWorldPos.x < maxDepth + 10.f);
        const bool isInZRange = (-5.f < hoverWorldPos.z) && (hoverWorldPos.z < 15.f);
        const bool isMouseClicked = FP::callFastFarA<bool>(fIsLeftMouseBtnClicked, false);
        isMouseInsideRootTile = isInXRange && isInZRange;
        if (isMouseClicked && !isMouseInsideRootTile) {

            int xGridIndex = FUN_00523f900_floatToInt64Truncated(hoverWorldPos.x / 21.f); //variance

            if (xGridIndex >= 0 && xGridIndex < unitVarianceCount[unitIndex]) {

                int zInt = FUN_00523f900_floatToInt64Truncated(hoverWorldPos.z); //unit row
                if (zInt < 0) zInt -= 10;
                movement = (zInt / 21) * 4;
                unitVarianceIndex = xGridIndex;
                FP::callFastFarA<void>(fPlaySound, 3, FP::getData<int>(0x00677274), 0, 0, 0);
            }
        }
        */

        //BEGIN custom code
        glm::vec3 clickAtWorld = glm::vec3(0.f);
        FP::callFastFarA<void>(0x004d1070, &clickAtWorld, FP::getData<unsigned int>(0x00701c80), FP::getData<unsigned int>(0x00701c84), 0.f, '\x01');

        constexpr float tileSize = 21.f;
        glm::ivec2 hoverTile = glm::floor((glm::vec2(clickAtWorld.x, clickAtWorld.z) + glm::vec2(tileSize / 2.f, 0.f)) / tileSize);
        //find the minimal unit variance from current and target. Yes, you need to do two clicks to move from a lower to higher variance size, 
        //as scroll animation would need to be rewritten too, too bad :(
        const int minUnitVarianceCount = std::min<int>(unitVarianceCount[unitIndex] - 1, unitVarianceCount[positiveMod(unitIndex + hoverTile.y, unitCount - 1)]);
        hoverTile.x = glm::clamp<int>(hoverTile.x, 0, minUnitVarianceCount);

        isMouseInsideRootTile = hoverTile.x == unitVarianceIndex && hoverTile.y == 0; //is used later too
        if (!isMouseInsideRootTile) {
            //player hovers a diffrent unit then selected
            if (FP::callFastFarA<bool>(fIsLeftMouseBtnClicked, false)) {
                //player selected a unit
                FP::callFastFarA<void>(fPlaySound, 3, FP::getData<int>(0x00677274), 0, 0, 0);
                unitVarianceIndex = hoverTile.x;
                movement = hoverTile.y * 4;
            }

            //render hover indicator
            unsigned int& anim = FP::getData<unsigned int>(0x0088c0e0);
            transform.position.x = hoverTile.x * tileSize;
            transform.position.z = hoverTile.y * (tileSize + 3.f);
            transform.scale = glm::vec4(glm::vec3(0.7f), 1.f);
            FP::callThisFarA<void>(fRender3DObject, &FP::getData<unsigned int>(0x007492f8), (float)anim, &cameraInfo, &transform, (float*)nullptr, (float*)nullptr, 0x0, 0x0, 0x0);
            transform.position.z = 0.f;
            transform.scale = glm::vec4(glm::vec3(1.0f), 1.f);
        }

        //END custom code
    }

    auto* assemblyParentUnit = FP::getData<DisaPartyCharacterInfo*>(0x0088ae34); //the unit who creates the new character
    if (keyStates.isKeyPressed(DisaKeyStates::Select) || (isMouseInsideRootTile && FP::callFastFarA<bool>(fIsLeftMouseBtnClicked, false))) {
        keyStates.clear();

        if (assemblyParentUnit->mana == 0) {
            FP::callFastFarA<void>(fPlaySound, 4, FP::getData<int>(0x00677274), 0, 0, 0);
        }
        else {
            FP::callFastFarA<void>(fPlaySound, 1, FP::getData<int>(0x00677274), 0, 0, 0);



            FP::getData<const DisaAssemblyUnitInfo*>(0x0088b4f8) = &selectedUnitInfo;
            FP::callFastFarA<void>(0x004dafc0, &creationCharacter, selectedUnitInfo.characterId, 1, 0, 0);//load char info

            //BEGIN custom code
            if (PG_DisaIdentifier::isCustomId(selectedUnitInfo.characterId)) {
                const auto& info = DisaCustomCharacterManager::get()->getCharacterInfo(selectedUnitInfo.characterId);
                auto displayInfo = DisaCustomCharacterManager::get()->getDisplayInfo(info.identifier);
                if (PG_DisaIdentifier::isCustomId(displayInfo.faceId)) {
                    creationCharacter.setCustomPortraitId(displayInfo.faceId);
                }
                else {
                    creationCharacter.portraitImageId = displayInfo.faceId;
                }
            }
            //END custom code

            FP::getData<unsigned int>(0x0088add8) = 0;
            FP::getData<unsigned int>(0x0088ada4) = 10;

            int strType = assemblyParentUnit->field_0x6b0 == 0 ? settings->field_0x30 : assemblyParentUnit->field_0x6ac;

            if (*(char*)(FP::getData<unsigned int>(0x0088c624) + 2) == '8') {
                //code block used for debugging?
                //this will create the same unit as assemblyParentUnit
                //also why is the if statment different then the bottom one !?
                if (strType != settings->field_0x30 || strType == 0 || strType == 1) {
                    std::strcpy(creationCharacter.charName.data(), assemblyParentUnit->charName.data());
                    std::strcpy(&FP::getData<char>(0x0088b500), assemblyParentUnit->charName.data());
                    std::strcpy(&FP::getData<char>(0x0088b540), assemblyParentUnit->charName.data());
                    FP::getData<unsigned int>(0x0088adb8) = std::strlen(&FP::getData<char>(0x0088b500)) / 2;
                }
                else {
                    FUN_004e3530_shiftJisCpy(creationCharacter.charName.data(), assemblyParentUnit->charName.data());
                    FUN_004e3530_shiftJisCpy(&FP::getData<char>(0x0088b500), assemblyParentUnit->charName.data());
                    FUN_004e3530_shiftJisCpy(&FP::getData<char>(0x0088b540), assemblyParentUnit->charName.data());
                    FP::getData<unsigned int>(0x0088adb8) = FUN_004e37b0_shiftJisLen(&FP::getData<unsigned short>(0x0088b500)) / 2;
                }

                creationCharacter.field_0x6ac = strType;
                creationCharacter.field_0x691 = assemblyParentUnit->field_0x691;
                creationCharacter.field_0x690 = assemblyParentUnit->field_0x690;
                creationCharacter.field_0x69a = assemblyParentUnit->field_0x69a;
                creationCharacter.field_0x6a0 = assemblyParentUnit->field_0x6a0;

            }
            else {
                //copy default randome name
                if (strType == 0 || strType == 1) {
                    std::strcpy(&FP::getData<char>(0x0088b540), creationCharacter.charName.data());
                }
                else {
                    FUN_004e3530_shiftJisCpy(&FP::getData<char>(0x0088b540), creationCharacter.charName.data());
                }
                creationCharacter.field_0x6ac = settings->field_0x30;
                creationCharacter.field_0x690 = (unsigned char)FP::getData<unsigned int>(0x0088ae18);
            }
        }
    }

    if (keyStates.isKeyPressed(DisaKeyStates::Cancel) || FP::callFastFarA<bool>(fIsExitPressed, 0)) {
        keyStates.clear();

        assemblyParentUnit->mana = assemblyParentUnit->mana + FP::getData<unsigned int>(0x0088ae2c);
        FP::getData<unsigned int>(0x0088ada4) = 0x1f;
        FP::callFastFarA<void>(fPlaySound, 7, FP::getData<unsigned int>(0x00677274), 0, 0, 0);
    }
}

void __fastcall FUN_004b5490(unsigned int portraitImageId, unsigned int portraitColor, int param_3, int param_4, int param_5, int param_6, int param_7) {
    const DisaSettings* settings = FP::getData<DisaSettings*>(0x005d7c40);

    //CUSTOM CODE START
    if (portraitColor & 0x80) { //indicator that custom
        portraitColor = portraitColor & ~0x80;//remove indicator bit
        unsigned short id = portraitImageId | portraitColor << 8;
        if (DisaCustomCharacterManager::FaceHeader* data = DisaCustomCharacterManager::get()->getFaceData(id, settings->field_0x21)) {
            const unsigned int imgSize = data->isHD ? 160 : 40;
            FP::callFastFarA<void>(0x0050eb30, data->header.data(), (float*)&FP::getData<float>(0x005ad534), (float)param_3, (float)param_4, param_5, param_6, param_7, 0, 0, imgSize, imgSize, portraitColor);
        }
        return;
    }
    //CUSTOM CODE END

    unsigned int imageOffset = portraitImageId;
    unsigned int imgSize = settings->field_0x21 ? 160 : 40;

    if (portraitColor != 0 && settings->field_0x21 != 0) {
        //they hard coded the locations of all the portrait files when coloring is involved
        if (portraitImageId < 10) {
            imageOffset = (portraitImageId + 14) * 5;
        }
        else if (portraitImageId < 24) {
            imageOffset = portraitImageId * 5 + 71;
        }
        else if (portraitImageId < 58) {
            imageOffset = portraitImageId * 5 + 72;
        }
        else {
            imageOffset = portraitImageId * 5 + 74;
        }
        imageOffset += portraitColor;
    }
    char* imgHeader;
    //CUSTOM CODE START
    if (DisaCustomCharacterManager::FaceHeader* data = DisaCustomCharacterManager::get()->getFaceData(imageOffset, settings->field_0x21)) {
        imgHeader = data->header.data();
        imgSize = data->isHD ? 160 : 40;
    }
    else { //CUSTOM CODE END
        imgHeader = FP::getData<char*>(0x00664098) + imageOffset * 0x88;//PortraitImageHeaders
    }


    FP::callFastFarA<void>(0x0050eb30, imgHeader, (float*)&FP::getData<float>(0x005ad534), (float)param_3, (float)param_4, param_5, param_6, param_7, 0, 0, imgSize, imgSize, portraitColor);
}

__declspec(naked) void FUN_004b5490_NoneStd(int param_3, int param_4, int param_5, int param_6, int param_7) {
    //for some reason the original assebly expects __fastcall to NOT destroy the stack
    __asm {
        push [esp + 20]
        push [esp + 20]
        push [esp + 20]
        push [esp + 20]
        push [esp + 20]

        call FUN_004b5490
        ret
    }
}

void __stdcall FUN_004cf3c0() {
    FP::callStdFarA<void>(0x004cf3c0); //builds face index list

    DisaCustomCharacterManager::get()->loadCharacters();
}