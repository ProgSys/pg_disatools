#include "DisaFunctions.h"
#include <Managers/DisaCustomCampaignManager.h>
#include <Managers/DisaCustomCharacterManager.h>
#include <Managers/DisaCustomRenderManager.h>
#include <DisaCharacterFunctions.h>
#include <Windows.h>
#undef max
#undef min

#include "FunctionPatcher.h"

#include <Util/Color/RGBA.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_StringUtil.h>
#include <Disa/fileformats/PG_MPD.h>
#include <Stream/PG_StreamInByteArray.h>
#include <ranges>

#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/constants.hpp> 

void DisaCameraInfo::move(glm::vec3 targetPosition, float speed) {
    eyeOffsetSpeed = speed;
    eyeOffsetTarget = targetPosition;

    //instent movement
    if (glm::isnan(speed) || speed == 0.f) {
        eyeOffset = targetPosition;
        processTick(); //default camera processing tick (will proccess position, rotation, zoom, etc.)
        return;
    }

    const float transition = 1.f / speed;
    eyeOffsetMovement = (targetPosition - eyeOffset) * transition;
    //BEGIN custom code
    //clear values if near zero
    const glm::bvec3 isNearZero = glm::epsilonEqual(eyeOffsetMovement, glm::vec3(0.0f), 0.0001f);
    eyeOffsetMovement = glm::mix(eyeOffsetMovement, glm::vec3(0.0f), isNearZero);
    eyeOffset = glm::mix(eyeOffset, eyeOffsetTarget, isNearZero);
    //END custom code
}

void DisaCameraInfo::rotate(glm::vec3 targetRotation, float speed) {
    rotationSpeed = speed;
    rotationTarget = targetRotation;

    //instent rotation
    if (glm::isnan(speed) || speed == 0.f) {
        rotation = targetRotation;
        field_0x30 = targetRotation; //not sure why the game needs one more value...
        processTick(); //default camera processing tick (will proccess position, rotation, zoom, etc.)
        return;
    }

    //loop rotation in valid range
    const auto clampRotation = [](glm::vec3& vec) {
        for (float& v : std::ranges::subrange(&vec.x, &vec.z + 1)) {
            if (v < -glm::pi<float>()) {
                v += glm::two_pi<float>();
            }
            else if (v > glm::pi<float>()) {
                v -= glm::two_pi<float>();
            }
        }
    };

    glm::vec3 diff = targetRotation - rotation;
    clampRotation(diff);

    const float transition = 1.f / speed;
    rotationMovement = diff * transition;
    clampRotation(rotationMovement);
}

void DisaCameraInfo::zoom(float _zoomTarget, float speed) {
    zoomSpeed = speed;
    zoomTarget = _zoomTarget;
    if (glm::isnan(speed) || speed == 0.f) {
        zoomOffset = zoomTarget;
        processTick(); //default camera processing tick (will proccess position, rotation, zoom, etc.)
        return;
    }
    zoomMovement = (zoomTarget - zoomOffset) * (1.f / speed);
}

void DisaCameraInfo::processTick() {
    FP::callFastFarA<void>(0x004f8800, this);
}

void FUN_LoadDungeonsCategories() {
    unsigned char* globalStateIt = &FP::getData<unsigned char>(0x00619330);

    const BasicList<DisaDungeonDAT>& disaDungeonDatList = FP::getData<BasicList<DisaDungeonDAT>>(aDungeonDATEntries);
    ArrayList<unsigned short>& disaDisplayDungeonList = FP::getData<ArrayList<unsigned short>>(aNumberOfDungeonEntries); //memory of the list that will be displayed

    disaDisplayDungeonList.clear();

    //BEGIN custom code
    DisaCustomCampaignManager::get()->diactivate();
    if (!DisaCustomCampaignManager::get()->isEmpty()) {
        disaDisplayDungeonList.push_back(vCustomCampaignsCategoryIdentifier); //CustomCampaigns entry
    }
    //END custom code

    for (DisaDungeon* dungeonsListIt = &FP::getData<DisaDungeon>(aDungeonEntries); (int)dungeonsListIt <= 0x653fa3; std::advance(dungeonsListIt, 1)) {
        if (!dungeonsListIt->active) continue;

        //find the corresponding DungeonDAT table entry
        auto findDungeonDatIt = std::ranges::find(disaDungeonDatList, dungeonsListIt->id, &DisaDungeonDAT::dungeonId);
        if (findDungeonDatIt == disaDungeonDatList.end() || findDungeonDatIt == nullptr) continue; //DAT entry not found

        if (findDungeonDatIt->filter.isEmpty()) continue; //entry is not valid
        if (findDungeonDatIt->filter.isSet(globalStateIt)) continue; //check if entry should be displayed, based on current game state (maybe save file)

        unsigned short newEntryId = dungeonsListIt->getMainCategoryId();

        //is entry already present?
        if (std::ranges::any_of(disaDisplayDungeonList, [newEntryId](unsigned short v) { return v == newEntryId; })) {
            continue;
        }
        disaDisplayDungeonList.push_back(newEntryId);
    }
}

constexpr int renderFlag = 0xbef8; //not sure what this does


void _cdecl renderSimpleBgBox(unsigned int x, unsigned int y, int width, int height) {
    FP::callFastFarA(fRenderBgBox, x, y, renderFlag, width, height, -1);
}

struct Cutout {
    unsigned int y = 0, x = 0, width = 0, height = 0;
};

struct MenuOptionStyle {
    unsigned int keyIndex;
    unsigned int stringIndex;
    Cutout textCutout;
    bool hover = false;
};

static std::array<MenuOptionStyle, 4> gMenuOptionStyles{
    MenuOptionStyle{ //exit
        DisaSettings::KeyAssigned::Cancel,
        8,
        Cutout{0, 0, 0x30, 0x18}
    },
    MenuOptionStyle{ //skipStory
        DisaSettings::KeyAssigned::Menu,
        10,
        Cutout{108, 0, 104, 0x18}
    },
    MenuOptionStyle{ //select
        DisaSettings::KeyAssigned::Select,
        9,
        Cutout{48, 0, 60, 0x18}
    },  
    MenuOptionStyle{ //select
        DisaSettings::KeyAssigned::Select,
        0,
        Cutout{0, 0, 0, 0}
    }
};

static inline std::pair<int,int> getMenuStringWidth(const std::vector<EMenuOption>& options) {
    int count = 0;
    int stringWidth = 0;

    for (EMenuOption menuOption : options) {
        const auto& style = gMenuOptionStyles[menuOption];
        if (style.stringIndex == 0) continue;
        byte* stringPtr = *(byte**)(FP::getData<unsigned int>(0x006e6f58) + style.stringIndex * 4);
        stringWidth = std::max(stringWidth, FP::callFastFarA<int>(fGetWidthOfString, stringPtr, 0, -1, (int*)0x0, -1));
        ++count;
    }
    stringWidth = stringWidth + 11; //add margin
    return { count, stringWidth };
}

static inline int getMenuStringWidth(const std::string& str) {
    int stringWidth = 0;

    const byte* stringPtr = (const byte*)str.c_str();
    stringWidth = std::max(stringWidth, FP::callFastFarA<int>(fGetWidthOfString, stringPtr, 0, -1, (int*)0x0, -1));

   
    stringWidth = stringWidth + 11; //add margin
    return stringWidth;
}

EMenuOption processMenuBox(DisaSettings* settings, const std::vector<EMenuOption>& options) {
    if (options.empty()) return EMenuOption::MenuOption_None;

    constexpr unsigned int margin = 4;
    constexpr unsigned int rowHeight = 16; //24
    constexpr unsigned int yBottom = 216 + 24;

    const auto [count, stringWidth] = getMenuStringWidth(options);
    unsigned int height = rowHeight * count; //of content
    unsigned int y = yBottom - height - margin * 2;
    int xPos = -stringWidth;

    DisaKeyStates& keyStates = FP::getData<DisaKeyStates>(0x00778d98);

    EMenuOption selectedOption = EMenuOption::MenuOption_None;
    for (EMenuOption menuOption : options) {
        auto& style = gMenuOptionStyles[menuOption];
      
        bool isHover = false;
        if (style.stringIndex != 0) { //has display?
            isHover = FP::callFastFarA<bool>(fIsMouseCursorInAABB, xPos + 448, y, stringWidth + xPos + 448, y + rowHeight, '\0');
            if (style.hover != isHover) {
                style.hover = isHover;
                if (isHover) FP::callFastFarA(fPlaySound, 3, FP::getData<int>(0x00677274), 0, 0, 0); //play sound
            }

            y += rowHeight;
        }

        DisaKeyStates::Key key = DisaSettings::toKeyState((DisaSettings::KeyAssigned)style.keyIndex);

        if ((key && keyStates.isKeyPressed(key)) || (isHover && FP::callFastFarA<bool>(fIsLeftMouseBtnClicked, '\0'))) {
            selectedOption = menuOption;
            style.hover = false;
            keyStates.clear();
        }

    }

    return selectedOption;
}

static inline Cutout getCutout(const DisaSettings* settings, unsigned int key) {
    Cutout cutout;
    int mode = 2;
    if (*(((char*)settings) + 0x52c) == '\0') {
        mode = 1;
    }
    if ((FP::getData<char>(0x00778d5b) != '\0') && (mode == 2)) {
        mode = 0;
    }

    if (mode == 0) {   //controller 
        unsigned int choosenSymbol = settings->assignedControllerKeys[key];
        if (choosenSymbol > 0x1b) {
            choosenSymbol -= 0xc;
        }
        cutout = (&FP::getData<Cutout>(0x00597b98))[choosenSymbol];
    }
    else if (mode == 1) {  //keyboard

        //find letter to display to exit screen
        unsigned int choosenLetter = settings->assignedKeys[key]; //[0x005d7c40]+  + 0x81;
        unsigned int letterIndex = 0;
        unsigned int* lettersIt = &FP::getData<unsigned int>(0x005973f0);

        while (!((*lettersIt == 0xffff) && (*(int*)(lettersIt + 1) == -1))) {

            if (*lettersIt == choosenLetter) {//found
                cutout = *(Cutout*)&((&FP::getData<unsigned int>(0x005973f4))[letterIndex]);
                break;
            }

            ++letterIndex;
            ++lettersIt;
        }
    }
    else if (mode == 2) { //number?
        unsigned int choosenSymbol = settings->assignedControllerKeys[key];
        if (choosenSymbol > 0x1b) {
            choosenSymbol -= 0xc;
        }
        cutout = (&FP::getData<Cutout>(0x005972bc))[choosenSymbol];
    }

    return cutout;
}

void renderMenuBox(DisaSettings* settings, const std::vector<EMenuOption>& options) {
    if (options.empty()) return;

    constexpr unsigned int margin = 4;
    constexpr unsigned int rowHeight = 16; //24
    constexpr unsigned int yBottom = 216 + 24;

    auto [count, stringWidth] = getMenuStringWidth(options);
    unsigned int height = rowHeight * count; //of content
    unsigned int y = yBottom - height - margin * 2;
    int xPos = -stringWidth;

    renderSimpleBgBox(xPos + 448, y, stringWidth, (height + margin * 2) );

    //new style
    if (settings->field_0x22 == '\0' || settings->field_0x30 != 1) {

        for (EMenuOption menuOption : options) {
            const auto& style = gMenuOptionStyles[menuOption];
            if (style.stringIndex == 0) continue;

            byte* stringPtr = *(byte**)(FP::getData<unsigned int>(0x006e6f58) + style.stringIndex * 4);

            //old style
            int* transfredStr[65];
            int settingVar = settings->field_0x30;
            if (!((settingVar == 1) || (settingVar == 0))) {
                stringWidth = 0x4716f4;
                FP::callCdeclFarA(0x004e42c0, transfredStr, stringPtr);
                settingVar = settings->field_0x30; //not sure why
                stringPtr = (byte*)transfredStr;
            }

            //render text
            if (settings->field_0x21 == '\0') {
                FP::callFastFarA(fRenderTextOld, xPos + 0x1c6, y + (221 - 216), 0xc7a7, &FP::getData<int>(0x005ad538), stringPtr, '\0', settingVar);
            }
            else {
                FP::callFastFarA(fRenderTextNew, xPos + 0x1c6, y + (223 - 216), 0xc7a7, &FP::getData<int>(0x005ad538), stringPtr, 0.3f, 0.3f, stringWidth, height, settingVar);
            }

            y += rowHeight;
        }

    } else {

        for (EMenuOption menuOption : options) {
            const auto& style = gMenuOptionStyles[menuOption];
            if (style.stringIndex == 0) continue;
            //text
            const int textHeight = style.hover ? 0xc + 2: 0xc;
            const int textY = style.hover ? y + 6: y + 7;
            FP::callFastFarA(fRenderCutout, &FP::getData<float>(0x0074c850), &FP::getData<glm::rgba>(0x005ad534), (float)(xPos + 473), (float)(textY), 0xbef8, style.textCutout.width/2, textHeight,
                style.textCutout.y, style.textCutout.x, style.textCutout.width, style.textCutout.height);

            Cutout cutout = getCutout(settings, style.keyIndex);

            if (cutout.width > 0 && cutout.height > 0) {
                glm::rgba colloring = glm::rgba(255, 255, 255, 255);// 0xffffffff;
                FP::callFastFarA(fRenderCutout, &FP::getData<float>(0x74c8d8), &colloring, (float)(xPos + 0x1c8), (float)(y + 5), 0xbef8, 0xe, 0xe, cutout.y, cutout.x, cutout.width, cutout.height);
            }

            y += rowHeight;
        }
    }

}


bool processSingleLineTextBox(const DisaSettings* settings, glm::ivec2 pos, const std::string& text, bool& isHover, DisaSettings::KeyAssigned key, unsigned int width) {
    if (text.empty()) return false;

    constexpr unsigned int rowHeight = 16; //24
    const auto stringWidth = width != 0 ? width : getMenuStringWidth(text);
    unsigned int boxWidth = stringWidth;
    constexpr unsigned int margin = 4;

    if (key != DisaKeyStates::Key::Invalid) {
        Cutout cutout = getCutout(settings, key);
        if (cutout.width > 0 && cutout.height > 0) {
            if (width == 0) boxWidth += 14 + margin;
        }
    }
    static bool globalIsHover = false;
    isHover = FP::callFastFarA<bool>(fIsMouseCursorInAABB, pos.x, pos.y, pos.x + boxWidth, pos.y + rowHeight, '\0');

    if (globalIsHover != isHover) {
        globalIsHover = isHover;
        if (isHover) FP::callFastFarA(fPlaySound, 3, FP::getData<int>(0x00677274), 0, 0, 0); //play sound
    }
   
    DisaKeyStates& keyStates = FP::getData<DisaKeyStates>(0x00778d98);

    if ((key != DisaSettings::KeyAssigned::Invalid && keyStates.isKeyPressed(DisaSettings::toKeyState(key))) || (isHover && FP::callFastFarA<bool>(fIsLeftMouseBtnClicked, '\0'))) {
        keyStates.clear();
        return true;
    }

    return false;
}

void renderSingleLineTextBox(const DisaSettings* settings, glm::ivec2 pos, const std::string& text, bool isHover, DisaSettings::KeyAssigned key, unsigned int width) {
    if (text.empty()) return;

    constexpr unsigned int rowHeight = 16; //24
    auto stringWidth = width != 0 ? width : getMenuStringWidth(text);
    auto boxWidth = stringWidth;
    constexpr unsigned int margin = 4;


    unsigned int iconDisplayOffset = 0;
    Cutout cutout;

    if (key != DisaSettings::KeyAssigned::Invalid) {
        cutout = getCutout(settings, key);
        if (cutout.width > 0 && cutout.height > 0) {
            iconDisplayOffset = 14 + margin;
            if (width != 0) boxWidth += iconDisplayOffset;
        }
    }

    renderSimpleBgBox(pos.x, pos.y, stringWidth, (rowHeight + margin * 2));

    if (cutout.width > 0 && cutout.height > 0) {
        glm::rgba colloring = glm::rgba(255, 255, 255, 255);// 0xffffffff;
        FP::callFastFarA(fRenderCutout, &FP::getData<float>(0x74c8d8), &colloring, (float)(pos.x + margin + 2), (float)(pos.y + margin), 0xbef8, 0xe, 0xe, cutout.y, cutout.x, cutout.width, cutout.height);
    }


    const byte* stringPtr = (const byte*)text.c_str();

    //old style
    int* transfredStr[65];
    int settingVar = settings->field_0x30;
    if (!((settingVar == 1) || (settingVar == 0))) {
        //stringWidth = 0x4716f4;
        FP::callCdeclFarA(0x004e42c0, transfredStr, stringPtr);
        settingVar = settings->field_0x30; //not sure why
        stringPtr = (byte*)transfredStr;
    }

    //render text
    if (settings->field_0x21 == '\0') {
        FP::callFastFarA(fRenderTextOld, pos.x + margin + iconDisplayOffset, pos.y + margin + 2, 0xc7a7, &FP::getData<int>(0x005ad538), stringPtr, '\0', settingVar);
    }
    else {
        FP::callFastFarA(fRenderTextNew, pos.x + margin + iconDisplayOffset, pos.y + margin + 2, 0xc7a7, &FP::getData<int>(0x005ad538), stringPtr, 0.3f, 0.3f, stringWidth, rowHeight, settingVar);
    }
}

void FUN_RenderDungeonScreeen(void) { //00471240

    int** ppiVar3;

    unsigned short* ppiVar12;
    int* transferredStr[65];
    unsigned short* local_11c = nullptr;

    //unsigned int local_c = FP::getData<int>(0x005ac900) ^ (unsigned int)&local_11c;
    if (-1 < FP::getData<int>(0x0067728c)) {
        //draw image (background): this_ptr, x, y, ?, width, height, ?, ?, imageWidth(?), imageHeight(?)
        FP::callVariadicThisFarA(fRenderImg, &FP::getData<int>(0x007495d8), 0, 0, 0xbaa1, 0x1e0, 0x110, 2, 2, FP::getData<int>(0x00749626), FP::getData<int>(0x00749628));
    }

    DisaSettings* settings = (DisaSettings*)(FP::getData<unsigned int>(0x005d7c40));

    if (unsigned int v = settings->field_0x30;  v == 1 || v == 0) {
        //title box (area select)
        FP::callCdeclFarA(fRenderTextBox, 0xf0, 0xc, -1, 1, 0xbef8, (unsigned short*)&FP::getData<unsigned short>(0x005ae3e0), 99, *(unsigned int*)(FP::getData<unsigned int>(0x006e6f58) + 0x178));
    }
    else {
        FP::callCdeclFarA(0x004e42c0, transferredStr, *(unsigned short**)(FP::getData<int>(0x006e6f58) + 0x178));
        FP::callCdeclFarA(fRenderTextBox, 0xf0, 0xc, -1, 1, 0xbef8, (int*)&FP::getData<int>(0x005ae3e0), 99, (unsigned int)transferredStr);
    }

    int numberOfDungeonsOnScreen = (int)FP::getData<int>(aNumberOfDungeonEntries);
    if (10 < numberOfDungeonsOnScreen) {
        numberOfDungeonsOnScreen = 10;
    }

    int dungeonsBgBoxWidth, dungeonsBgBoxHeight;
    {  //render Dungeons Bg box
        unsigned short* stringPtr; //used to compute the size of box
        if (settings->field_0x30 == 1) {
            stringPtr = *(unsigned short**)(FP::getData<int>(0x006e6f58) + 0x10c);
        }
        else if (settings->field_0x30 == 0) {
            stringPtr = *(unsigned short**)(FP::getData<int>(0x006e6f58) + 0x108);
        }
        else {
            stringPtr = *(unsigned short**)(FP::getData<int>(0x006e6f58) + 0xd8); //small?
        }

        dungeonsBgBoxWidth = FP::callFastFarA<int>(fGetWidthOfString, stringPtr, 0, -1, nullptr, -1);
        dungeonsBgBoxWidth = dungeonsBgBoxWidth + 0xb;
        dungeonsBgBoxHeight = numberOfDungeonsOnScreen * 0x10 + 8;

        FP::callFastFarA(fRenderBgBox, 0x20, 0x30, 0xbef8, dungeonsBgBoxWidth, dungeonsBgBoxHeight, -1); //render box
    }

    //render Dungeons text entries
    if (numberOfDungeonsOnScreen > 0) {
        int yPos = 53;// 0x35;
        unsigned short* stringPtr = nullptr; //pointer to swift jis string
        for (int index = 0; index < numberOfDungeonsOnScreen; ++index, yPos += 16) {
            int settingVar = settings->field_0x30;
            int entryIndex = FP::getData<int>(aDisplayDungeonEntriesScrollOffset) + index;
            int dungeonIndex = (&FP::getData<unsigned short>(aDisplayDungeonEntries))[entryIndex]; //apply string offset
            //BEGIN custom code
            if (dungeonIndex == vCustomCampaignsCategoryIdentifier) {
                //"Custom Campaigns" in ShiftJis
                stringPtr = (unsigned short*)"\x82\x62\x82\x95\x82\x93\x82\x94\x82\x8f\x82\x8d\x81\x40\x82\x62\x82\x81\x82\x8d\x82\x90\x82\x81\x82\x89\x82\x87\x82\x8e\x82\x93";
            }
            else
                //END custom code
                if ((settingVar == 1) || (settingVar == 0)) {
                    stringPtr = (unsigned short*)(&FP::getData<unsigned int>(0x005aca90))[dungeonIndex]; //retrive string
                }
                else {
                    //get it from file? Or maybe language option?
                    ppiVar3 = transferredStr;
                    ppiVar12 = (unsigned short*)0x4713ea;
                    FP::callCdeclFarA(0x004e42c0, ppiVar3, (unsigned short*)(&FP::getData<int>(0x005aca90))[dungeonIndex]);
                    settingVar = settings->field_0x30; //not sure why
                    stringPtr = (unsigned short*)transferredStr;
                }

            if (settings->field_0x21 != '\0') {
                //Render text new: x, y, strange bit mask??, ?, pointer to string , ?, ?, ?, ?, spacing (bool)
                FP::callFastFarA(fRenderTextNew, 0x26, yPos + 2, 0xc7a7, &FP::getData<int>(0x005ad538), stringPtr, 0.3f, 0.3f, dungeonsBgBoxWidth, dungeonsBgBoxHeight, settingVar);
            }
            else {
                //render text old
                ppiVar3 = (int**)0x0;
                FP::callFastFarA(fRenderTextOld, 0x26, yPos, 0xc7a7, &FP::getData<int>(0x005ad538), stringPtr, '\0', settingVar);
            }
        }
    }
    //up and down arrows
    FP::callFastFarA(0x004b4b10, 0x20, 0x30, dungeonsBgBoxWidth, dungeonsBgBoxHeight, FP::getData<int>(aDisplayDungeonEntriesScrollOffset), numberOfDungeonsOnScreen, (int)FP::getData<int>(aNumberOfDungeonEntries));

    //render cursor
    {
        float* f;
        unsigned short cutoutX, cutoutY, cutoutWidth, cutoutHeight; //coords of cutout
        if (settings->field_0x22 == '\0') {
            //SD cursor data
            cutoutHeight = 0x10;
            cutoutWidth = 0x10;
            cutoutX = 0x60;  //cutout pos X
            cutoutY = 0x3c;  //cutout pos y
            f = &FP::getData<float>(0x0074bfd0);
        }
        else {
            //HD cursor data
            cutoutHeight = 0x28; //height
            cutoutWidth = 0x2a; //width
            cutoutX = 0x5a; //cutout pos X
            cutoutY = 0x5a;  //cutout pos Y
            f = &FP::getData<float>(0x0074d048);
        }
        //render hand cusor
        FP::callFastFarA(fRenderCutout, f, &FP::getData<glm::rgba>(0x005ad534), 22.0f, (float)(FP::getData<int>(aDisplayDungeonEntriesHoverIndex) * 16 + 53), 0xc7a9, 16, 16, cutoutY, cutoutX, cutoutWidth, cutoutHeight);
    }

    renderMenuBox(settings);

    FP::callFastFarA(0x004b3d40, *(int***)(FP::getData<unsigned int>(0x006e6f58) + 0x17c), 0); //bottom info box
    //FP::callFastFarA(0x0052376a, local_c ^ (unsigned int)&local_11c); //runtime check
}


void FUN_ProccessCategoryDungeonsScreen() { //00471780
    //BEGIN custom code
    if (DisaCustomCampaignManager::get()->process()) return; //take over control to procces and render Custom Campaigns menu
    //END custom code

    byte* stringPtr;
    if (*(int*)(FP::getData<int>(0x005d7c40) + 0x30) == 1) {
        stringPtr = *(byte**)(FP::getData<int>(0x006e6f58) + 0x10c);
    }
    else {
        stringPtr = *(byte**)(FP::getData<int>(0x006e6f58) + 0x108);
    }
    int iVar2 = FP::callFastFarA<int>(fGetWidthOfString, stringPtr, 0, -1, (int*)0x0, -1);

    const ArrayList<unsigned short>& disaDisplayDungeonList = FP::getData<ArrayList<unsigned short>>(aNumberOfDungeonEntries); //memory of the list that will be displayed
    int lastSelection = FP::getData<int>(aDisplayDungeonEntriesHoverIndex);
    short numberOfDisplayEntries = (short)std::min((int)disaDisplayDungeonList.size, 10); //there can be a max of 10 dungeon entries

    int iVar5 = FP::callFastFarA<int>(fGetWidthOfString, *(byte**)(FP::getData<int>(0x006e6f58) + 0x20), 0, -1, (int*)0x0, -1);
    unsigned int uVar11 = 0x1c0 - (iVar5 + 0xb);
    FP::callFastFarA(0x004f6a40, uVar11, 0xd8, 0xbef8, iVar5 + 0xb, 0x18);
    FP::callFastFarA(0x0050fb90, 0x20, 0x30, iVar2 + 0xb, 0x10, numberOfDisplayEntries, &FP::getData<int>(aDisplayDungeonEntriesHoverIndex), '\0'); //hand cursor position
    FP::callFastFarA(0x0050fc10, 0x20, 0x30, iVar2 + 0xb, 0x10, numberOfDisplayEntries, &FP::getData<int>(aDisplayDungeonEntriesHoverIndex), FP::getData<int>(aDisplayDungeonEntriesScrollOffset), disaDisplayDungeonList.size);

    //play sound if selection changed
    const bool selectionHasChanged = lastSelection != FP::getData<int>(aDisplayDungeonEntriesHoverIndex);
    if (selectionHasChanged) {
        FP::callFastFarA(fPlaySound, 3, FP::getData<int>(0x00677274), 0, 0, 0); //play sound
    }

    if (disaDisplayDungeonList.size < 1) {
        //disable scrolling
        FP::getData<int>(aDisplayDungeonEntriesScrollOffset) = 0;
        FP::getData<int>(aDisplayDungeonEntriesHoverIndex) = 0;
    }
    else {
        const int endScrollIndex = std::max((int)disaDisplayDungeonList.size - 10, 0); //we can not scroll past this index
        const byte selectionChangedByte = selectionHasChanged ? 2 : 0; //not sure why it needs to be a byte
        FP::callFastFarA(fHandleListScrolling, &FP::getData<int>(aDisplayDungeonEntriesHoverIndex), &FP::getData<int>(aDisplayDungeonEntriesScrollOffset), numberOfDisplayEntries, endScrollIndex, selectionChangedByte); //handle scrolling
    }

    auto x0 = FP::getData<char>(0x00778da5);

    //exit triggered
    DisaKeyStates& keyStates = FP::getData<DisaKeyStates>(0x00778d98);
    if (keyStates.isKeyPressed(DisaKeyStates::Cancel) || FP::callFastFarA<bool>(fIsExitPressed, 0) ||
        (FP::callFastFarA<bool>(fIsMouseCursorInAABB, uVar11, 0xd8, iVar5 + 0xb + uVar11, 0xe8, '\0') && FP::callFastFarA<bool>(fIsLeftMouseBtnClicked, '\0'))) {

        int iVar4 = FP::getData<int>(0x00677274);
        *(int*)(FP::getData<int>(0x005d7c70) + 0x30543c) = 0;
        keyStates.clear();
        FP::callFastFarA<void>(fPlaySound, 7, iVar4, 0, 0, 0);
        FP::getData<int>(0x0088abd0) = 0;
        int* piVar10 = (int*)0xa;
        if (FP::getData<int>(0x00653a9c) / 100 != 0x33) {
            piVar10 = (int*)0x7d0;
        }
        FP::callFastFarA<unsigned int>(0x004bd930, piVar10, (int*)0x0);
    }

    //check if an entry was clicked
    if ( keyStates.isKeyPressed(DisaKeyStates::Select) ||
        (FP::callFastFarA<bool>(fIsMouseCursorInAABB, 0x20, 0x30, iVar2 + 0x2b, (numberOfDisplayEntries + 3) * 0x10, '\0') && FP::callFastFarA<bool>(fIsLeftMouseBtnClicked, false)) ||
        (*(char*)(FP::getData<int>(0x005d7c70) + 0x305438) == '\0' && *(char*)(FP::getData<int>(0x005d7c70) + 0x30543a) != '\0' && ((FP::getData<int>(0x00701c58) & 0x2000000) != 0))) {

        iVar2 = FP::getData<int>(0x00677274);
        int iVar9 = FP::getData<int>(0x005d7c70);
        *(int*)(FP::getData<int>(0x005d7c70) + 0x30543c) = 0;
        keyStates.clear();
        FP::callFastFarA(fPlaySound, 1, iVar2, 0, iVar9, 0);
        FP::getData<int>(0x0088abd0) = 0;

        int selectedEntryIndex = FP::getData<int>(aDisplayDungeonEntriesScrollOffset) + FP::getData<int>(aDisplayDungeonEntriesHoverIndex);
        const unsigned short categoryId = disaDisplayDungeonList[selectedEntryIndex];
        //BEGIN custom code
        if (categoryId == vCustomCampaignsCategoryIdentifier) {
            //custom campains selected
            DisaCustomCampaignManager::get()->activate();
        }
        else {
            //END custom code
            FP::getData<unsigned short>(0x0065db44) = categoryId;
            FP::callFastFarA(0x004bd930, (int*)0xe, (int*)0x0);
        }
    }
    //FP::callStdFarA(0x00471240); //render screen
    FUN_RenderDungeonScreeen();
}


void _stdcall FUN_ProccessDungeonsScreen() { //00472a40
    unsigned int uVar1;
    byte bVar2;
    int iVar3;
    unsigned int uVar4;
    int uVar5;
    unsigned int uVar6;
    int uVar7;
    byte* pbVar8;
    int extraout_ECX = 0;
    int iVar9;
    int extraout_ECX_00 = 0;
    int extraout_ECX_01 = 0;
    int extraout_ECX_02 = 0;
    int extraout_ECX_03 = 0;
    int extraout_ECX_04 = 0;
    int extraout_ECX_05 = 0;
    int extraout_ECX_06 = 0;
    int extraout_ECX_07 = 0;
    int extraout_ECX_08 = 0;
    int extraout_ECX_09 = 0;
    int extraout_ECX_10 = 0;
    int extraout_ECX_11 = 0;
    int extraout_ECX_12 = 0;
    int extraout_ECX_13 = 0;
    int extraout_ECX_14 = 0;
    int extraout_ECX_15 = 0;
    int extraout_ECX_16 = 0;
    int extraout_ECX_17 = 0;
    int extraout_ECX_18 = 0;
    unsigned int uVar10;
    int iVar11;
    bool bVar12;
    unsigned int local_14;
    int local_c;

    iVar9 = FP::getData<int>(0x0088abf4);
    iVar11 = (int)FP::getData<int>(0x0088ac80);
    if (8 < iVar11) {
        iVar11 = 8;
    }
    if (*(int*)(FP::getData<unsigned int>(0x005d7c40) + 0x30) == 7) {
        local_c = FP::callFastFarA<int>(fGetWidthOfString, *(byte**)(FP::getData<unsigned int>(0x006e6f58) + 0x20), 0, -1, (int*)0x0, -1);
        pbVar8 = *(byte**)(FP::getData<unsigned int>(0x006e6f58) + 0x20);
    }
    else {
        local_c = FP::callFastFarA<int>(fGetWidthOfString, *(byte**)(FP::getData<unsigned int>(0x006e6f58) + 0x28), 0, -1, (int*)0x0, -1);
        pbVar8 = *(byte**)(FP::getData<unsigned int>(0x006e6f58) + 0x28);
    }
    local_c = local_c + 0xb;
    iVar3 = FP::callFastFarA<int>(fGetWidthOfString, pbVar8, 0, -1, (int*)0x0, -1);
    uVar10 = 0x1c0 - (iVar3 + 0xb);
    FP::callFastFarA(fRenderBgBox, uVar10, 0xb8, 0xbef8, iVar3 + 0xb, 0x38, -1);
    iVar3 = FP::callFastFarA<int>(fGetWidthOfString, *(byte**)(FP::getData<unsigned int>(0x006e6f58) + 0x184), 0, -1, (int*)0x0, -1);
    uVar4 = (0x1d5 - iVar3) / 2;
    if ((FP::getData<unsigned int>(0x005d7c4e) != '\0') || (local_14 = 100, *(char*)(FP::getData<unsigned int>(0x005d7c40) + 0x22) != '\0')) {
        local_14 = 0x67;
    }
    iVar3 = FP::callFastFarA<int>(fGetWidthOfString, *(byte**)(FP::getData<unsigned int>(0x006e6f58) + 0x184), 0, -1, (int*)0x0, -1);
    uVar1 = FP::getData<unsigned int>(0x0088abd4);

    DisaKeyStates& keyStates = FP::getData<DisaKeyStates>(0x00778d98);

    if (FP::getData<unsigned int>(0x0088abd0) == 0) {
        FP::callFastFarA(0x0050fb90, 0x20, 0x30, 0x80, 0x10, iVar11, &FP::getData<unsigned int>(0x0088abf4), '\0');
        FP::callFastFarA(0x0050fc10, 0x20, 0x30, 0x80, 0x10, iVar11, &FP::getData<unsigned int>(0x0088abf4), FP::getData<unsigned int>(0x0088abf8), (int)FP::getData<unsigned int>(0x0088ac80));
        bVar12 = iVar9 != FP::getData<unsigned int>(0x0088abf4);
        if (bVar12) {
            FP::callFastFarA(fPlaySound, 3, FP::getData<unsigned int>(0x00677274), 0, extraout_ECX, 0);
        }
        iVar9 = (int)FP::getData<unsigned int>(0x0088ac80);
        if (iVar9 < 1) {
            FP::getData<unsigned int>(0x0088abf8) = 0;
            FP::getData<unsigned int>(0x0088abf4) = 0;
        }
        else {
            iVar3 = iVar9;
            if (8 < iVar9) {
                iVar3 = 8;
            }
            bVar2 = 0;
            iVar9 = iVar9 + -8;
            if (iVar9 < 0) {
                iVar9 = 0;
            }
            if (bVar12) {
                bVar2 = 2;
            }
            FP::callFastFarA(fHandleListScrolling, &FP::getData<unsigned int>(0x0088abf4), &FP::getData<unsigned int>(0x0088abf8), iVar3, iVar9, bVar2);
            iVar9 = extraout_ECX_00;
        }

        //go back
        if (keyStates.isKeyPressed(DisaKeyStates::Cancel) || FP::callFastFarA<bool>(fIsExitPressed, 0) ||
            (FP::callFastFarA<bool>(fIsMouseCursorInAABB, uVar10, 0xd8, local_c + uVar10, 0xe8, '\0') && FP::callFastFarA<bool>(fIsLeftMouseBtnClicked, '\0'))) {

            iVar3 = FP::getData<unsigned int>(0x00677274);
            *(unsigned int*)(FP::getData<unsigned int>(0x005d7c70) + 0x30543c) = 0;
            keyStates.clear();

            FP::callFastFarA(fPlaySound, 7, iVar3, 0, iVar9, 0);
            FP::getData<unsigned int>(0x0088abcc) = 1;
            iVar9 = extraout_ECX_04;
        }

        //dungeon selected
        if (keyStates.isKeyPressed(DisaKeyStates::Select) ||
            (FP::callFastFarA<bool>(fIsMouseCursorInAABB, 0x20, 0x30, 0xa0, (iVar11 + 3) * 0x10, '\0') && FP::callFastFarA<bool>(fIsLeftMouseBtnClicked, '\0')) ||
            (FP::callFastFarA<bool>(fIsMouseCursorInAABB, uVar10, 0xb8, local_c + uVar10, 200, '\0') && FP::callFastFarA<bool>(fIsLeftMouseBtnClicked, '\0')) ||
            FP::callFastFarA<bool>(0x0050f790)){

            iVar11 = FP::getData<unsigned int>(0x00677274);
            *(unsigned int*)(FP::getData<unsigned int>(0x005d7c70) + 0x30543c) = 0;
            keyStates.clear();
            FP::callFastFarA(fPlaySound, 1, iVar11, 0, iVar9, 0);

            filterPlayStory.clear(&FP::getData<unsigned char>(aGlobalStateBits));

            unsigned int index = FP::getData<unsigned short>(0x0088ac00) + FP::getData<unsigned int>(0x0088abf8) + FP::getData<unsigned int>(0x0088abf4);
            FP::getData<unsigned short>(0x0065db1a) = index;
            FP::getData<DisaDungeon>(0x00653a98) = *(&FP::getData<DisaDungeon>(aDungeonEntries) + index);

            auto retUnused = FP::callFastFarA<int*>(0x004bd930, (int*)0xb, (int*)0x0);
            //FUN_004a59c0 x
               // FUN_004d5060 x
            iVar9 = extraout_ECX_08;
        }

        if (keyStates.isKeyPressed(DisaKeyStates::Menu) ||
            (FP::callFastFarA<bool>(fIsMouseCursorInAABB, uVar10, 200, local_c + uVar10, 0xd8, '\0') && FP::callFastFarA<int>(fIsLeftMouseBtnClicked, '\0'))) {

            iVar11 = FP::getData<unsigned int>(0x00677274);
            *(unsigned int*)(FP::getData<unsigned int>(0x005d7c70) + 0x30543c) = 0;
            keyStates.clear();
            FP::callFastFarA(fPlaySound, 1, iVar11, 0, iVar9, 0);
            FP::getData<unsigned int>(0x0088abd0) = 1;
            FP::getData<unsigned int>(0x0088abd4) = 1;
            FP::callStdFarA(0x00471a80);
        }

    }
    else {
        //skip story conform dialog
        FP::callFastFarA(0x0050fb90, uVar4, local_14, iVar3 + 0xb, 0x10, 2, (int*)&FP::getData<unsigned int>(0x0088abd4), '\0');
        uVar10 = iVar3 + 0xb + uVar4;

        //confirm
        if (FP::callFastFarA<bool>(fIsMouseCursorInAABB, uVar4, local_14, uVar10, local_14 + 0x10, '\0') && FP::callFastFarA<bool>(fIsLeftMouseBtnClicked, '\0')) {
            keyStates.clear();
            FP::callFastFarA(fPlaySound, 1, FP::getData<unsigned int>(0x00677274), 0, 0, 0);

            filterPlayStory.set(&FP::getData<unsigned char>(aGlobalStateBits)); //0x00619443

            unsigned int index = FP::getData<unsigned short>(0x0088ac00) + FP::getData<unsigned int>(0x0088abf8) + FP::getData<unsigned int>(0x0088abf4);
            FP::getData<unsigned short>(0x0065db1a) = index;
            FP::getData<DisaDungeon>(0x00653a98) = *(&FP::getData<DisaDungeon>(aDungeonEntries) + index);

            FP::callFastFarA<int*>(0x004bd930, (int*)0xb, (int*)0x0);
        }

        //cancel
        if (keyStates.isKeyPressed(DisaKeyStates::Cancel) || FP::callFastFarA<bool>(fIsExitPressed, 0) || 
            (FP::callFastFarA<int>(fIsMouseCursorInAABB, uVar4, local_14 + 0x10, uVar10, local_14 + 0x20, '\0') && FP::callFastFarA<bool>(fIsLeftMouseBtnClicked, '\0'))) {
            keyStates.clear();
            FP::callFastFarA(fPlaySound, 7, FP::getData<unsigned int>(0x00677274), 0, 0, 0);
            FP::getData<unsigned int>(0x0088abd0) = 0;
        }

        if ((FP::getData<unsigned int>(0x00778d9c) == '\x02') || (FP::getData<unsigned int>(0x00778d9e) == '\x02')) {
            *(unsigned int*)(FP::getData<unsigned int>(0x005d7c70) + 0x30543c) = 0;
            FP::getData<unsigned int>(0x0088abd4) = (unsigned int)(FP::getData<unsigned int>(0x0088abd4) == 0);
        }
        if (uVar1 != FP::getData<unsigned int>(0x0088abd4)) {
            FP::callFastFarA(fPlaySound, 3, FP::getData<unsigned int>(0x00677274), 0, 0, 0);
        }

        uVar5 = 0;
        if (keyStates.isKeyPressed(DisaKeyStates::Select) ||
            (FP::callFastFarA<bool>(fIsMouseCursorInAABB, 0x20, 0x30, 0xa0, (iVar11 + 3) * 0x10, '\0') && FP::callFastFarA<bool>(fIsLeftMouseBtnClicked, '\0')) ||
             FP::callFastFarA<bool>(0x0050f790)) {

            keyStates.clear();

            if (FP::getData<unsigned int>(0x0088abd4) == 0) {
                FP::callFastFarA(fPlaySound, 1, FP::getData<unsigned int>(0x00677274), 0, uVar5, 0);

                filterPlayStory.set(&FP::getData<unsigned char>(aGlobalStateBits)); //0x00619443

                unsigned int index = FP::getData<unsigned short>(0x0088ac00) + FP::getData<unsigned int>(0x0088abf8) + FP::getData<unsigned int>(0x0088abf4);
                FP::getData<unsigned short>(0x0065db1a) = index;
                FP::getData<DisaDungeon>(0x00653a98) = *(&FP::getData<DisaDungeon>(aDungeonEntries) + index);

                FP::callFastFarA<int*>(0x004bd930, (int*)0xb, (int*)0x0);
                FP::callStdFarA(0x00471a80);
                return;
            }
            FP::callFastFarA(fPlaySound, 7, FP::getData<unsigned int>(0x00677274), 0, uVar5, 0);
            FP::getData<unsigned int>(0x0088abd0) = 0;
        }

    }
    FP::callStdFarA(0x00471a80);
    return;
}

struct  Local_c {
    union
    {
        struct { char a, b, c, d; };
        unsigned int i;
    };

};

void FUN_0050a1f0_GetFileData(int param_1, int** fileData, bool param_4) {
    int* piVar1;
    int** ppiVar2;
    void* pvVar3;
    unsigned int uVar4;
    int local_24;
    void* local_20;

    void* local_14;
    int* puStack_10;
    Local_c local_c; //never referenced?!

    try {
        local_20 = (void*)0x0;
        local_c.i = 0;
        if (param_4) {
            piVar1 = FP::callStdFarA<int*>(0x004195e0, &local_24, (&FP::getData<int>(0x0055f9f0))[param_1], 1);
            local_c.a = 1;
        }
        else {
            int i = (&FP::getData<int>(0x00555e48))[param_1];
            piVar1 = FP::callStdFarA<int*>(0x004195e0, &local_24, i, 0, 0, 0, 0, 0, 0);
            local_c.a = 2;
        }
        FP::callThisFarA(0x00417e90, &local_20, &local_24);
        local_c.i = (unsigned int)local_c.c << 8;
        if (local_24 != 0) {
            FP::callFastFarA(0x00417750, local_24);
        }
        auto thiss = local_20;
        if (*fileData == nullptr) {
            if (local_20 == (void*)0x0) {
                uVar4 = 0;
            }
            else {
                uVar4 = *(unsigned int*)((int)local_20 + 0x10);
            }
            ppiVar2 = FP::callFastFarA<int**>(0x004333f0, uVar4);
            *fileData = (int*)ppiVar2;
        }
        unsigned int fileSize = 0;
        if (thiss != nullptr) {
            fileSize = *(unsigned int*)((int)thiss + 0x10);
        }

        FP::callThisFarA(0x00417470, thiss, *fileData, (int*)0x0, fileSize, 0, (int*)0x0, 0, 0);
        FP::callFastFarA(0x004176f0, thiss);
        local_c.i = 0xffffffff;
        if (thiss) {
            FP::callFastFarA(0x00417750, thiss);
        }
    }
    catch (...) {
        fileData = nullptr;
    }

    return;
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


// Custom rounding function that returns a 64-bit unsigned integer from a float.
uint64_t FUN_00523fc6_RoundToUInt64(float value) {
    const double floorVal = std::floor(value);
    const double diff = value - floorVal;

    // Bias: if exactly .5, do NOT round up (i.e. towards zero)
    if (diff == 0.5 || diff == -0.5) {
        return static_cast<uint64_t>(floorVal);
    }

    // Regular rounding otherwise
    return static_cast<uint64_t>(std::round(value));
    /*
    uint64_t rounded = static_cast<uint64_t>(roundf(value));
    uint32_t low = static_cast<uint32_t>(rounded);
    float high = static_cast<float>(rounded >> 32);

    float diff = value - static_cast<float>(rounded);
    if (low != 0 || (rounded & 0x7fffffff00000000ULL) != 0) {
        if (value < 0.0f) {
            // Adjust upward if needed (handles precision edge)
            if ((uint32_t)(diff) ^ 0x80000000 > 0x80000000)
                rounded++;
        }
        else {
            // Adjust downward if needed
            if ((uint32_t)(diff) > 0x80000000) {
                uint32_t carry = 1;
                if (low < carry) high -= 1.0f;
                low -= carry;
                rounded = (static_cast<uint64_t>(static_cast<uint32_t>(high)) << 32) | low;
            }
        }
    }

    return rounded;
    */
}

// CONCAT44 is a concatenation operation for two 32-bit integers
static inline uint64_t CONCAT44(uint32_t x, uint32_t y) {
    return (static_cast<uint64_t>(x) << 32) | static_cast<uint64_t>(y);
}

uint64_t FUN_00523fc6_V2(float param_1)
{
    uint64_t uVar1;
    uint uVar2;
    float fVar3;
    uint local_20;
    float fStack_1c;

    uVar1 = (uint64_t)std::floor(param_1);
    local_20 = (uint)uVar1;
    fStack_1c = (float)(uVar1 >> 0x20);
    fVar3 = param_1;
    if ((local_20 != 0) || (fVar3 = fStack_1c, (uVar1 & 0x7fffffff00000000) != 0)) {
        if ((int)fVar3 < 0) {
            uVar1 = uVar1 + (0x80000000 < ((uint)(param_1 - (float)uVar1) ^ 0x80000000));
        }
        else {
            uVar2 = (uint)(0x80000000 < (uint)(param_1 - (float)uVar1));
            uVar1 = 0;
            uVar1 = CONCAT44((int)fStack_1c - (uint)(local_20 < uVar2) , local_20 - uVar2);
        }
    }
    return uVar1;
}

uint64_t FUN_00523fc6_V3(float x) {
    return std::nearbyint(x);
}

uint64_t ConvertFloatToUInt64V3(float param_1) {
    // Convert float to integer using rounding
    double floatVal = static_cast<double>(param_1);
    int64_t intVal = static_cast<int64_t>(round(floatVal));

    // If the float was zero, return 0
    if (intVal == 0) {
        // But double-check if the float was -0.0 or 0.0 with a non-zero exponent
        int64_t check = *reinterpret_cast<int64_t*>(&floatVal);
        if ((check & 0x7FFFFFFFFFFFFFFF) != 0) {
            // Go back to processing if not pure zero
            goto redo;
        }
        return 0;
    }

redo:
    if (intVal < 0) {
        // Negative number handling (flip sign and adjust)
        intVal = ~intVal + 1;  // Equivalent to abs and adjust
        return static_cast<uint64_t>(intVal);
    }
    else {
        return static_cast<uint64_t>(intVal);
    }
}

// Function to initialize the level table
void FUN_0048cc90_GenerateLevelExperienceTable() {
    constexpr int kMaxLevel = 10000;
    std::vector<uint64_t> levelTable(kMaxLevel);

    constexpr float softcapStart = 98.0f;
    constexpr float softcapDivisor = 20.0f;
    constexpr float capModifier = 103.0f;
    constexpr float maxCurveValue = 4.920000076293945f;
    constexpr float curveDivisor = 25.0f;

    auto& levels = FP::getData<std::array<unsigned long long, kMaxLevel>>(0x005E9B28);

    for (int level = 0; level < kMaxLevel; ++level) {
        float fLevel = static_cast<float>(level);
        float growthFactor = fLevel / curveDivisor + 1.f;

        float scaledLevel = fLevel;
        if (fLevel > softcapStart) {
            float over = fLevel - softcapStart;
            scaledLevel = (over / (over / softcapDivisor + 1.f)) + capModifier;
        }

        // Clamp growth factor
        growthFactor = std::min(growthFactor, maxCurveValue);

        // Final experience formula
        float result = (fLevel + 1.f) * (scaledLevel + 1.f) * (2.f + fLevel) * growthFactor * 0.25f;

        //levelTable[level] = FUN_00523fc6(result); //convert float to  64-bit (function is very strange)

        //unsigned long long r = FUN_00523fc6_V3(result);
        //unsigned long long c = levels[level];
        levelTable[level] = std::floor(result);//FUN_00523fc6_RoundToUInt64(result);

    }



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
                ((unsigned int)*(byte*)((int)copyFrom + 0x72) + (unsigned int)*(byte*)((int)copyFrom + 0x71) * 0x28) *
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

        byte local_20[4] = { 0,0,0,0 };
        byte local_18[4] = { 0,0,0,0 };
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

void __fastcall FUN_004d6a30(DisaDungeon* dungeon) {

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

void __fastcall FUN_004b6420(unsigned int param_1, int param_2) {
    auto code = *(byte**)(param_1 + 0x10);
    unsigned char opCode = *code;
    auto arguments = (unsigned char*)(code + 2);

    if (opCode == 0x65) {
        long long a = FP::callFastFarA<long long>(0x004b5550, param_1, arguments, 2);
        long long b = FP::callFastFarA<long long>(0x004b5550, param_1, arguments + 4, 2);
        short c = FP::callFastFarA<short>(0x0046df10, arguments + 6);
        short d = FP::callFastFarA<short>(0x0046df10, arguments + 8);
        short f = FP::callFastFarA<short>(0x0046df10, arguments + 10);
        float fVar47 = (float)(unsigned  int)*(byte*)(*(int*)(param_1 + 0x10) + 0xc);
        auto iVar15 = *(int*)(param_1 + 0x30);
        auto i = 0;
    }
    FP::callFastFarA<void>(0x004b6420, param_1, param_2);
}

void __fastcall FUN_00446e20(int param_1, int param_2) {

    //BEGIN custom code
    //TODO save progress
    //END custom code

    FP::getData<unsigned int>(0x008895e0) = (unsigned int)FP::getData<short>(0x0065db26);
    FP::getData<int>(0x008895dc) = FP::getData<int>(0x008895e0) * 100 + (unsigned int)FP::getData<short>(0x0065db24);
    FP::getData<int>(0x008895d4) = -1;
    FP::getData<int>(0x008895d8) = 0;
    FP::getData<int>(0x008896a0) = 0;
    FP::getData<int>(0x00889618) = 1000;//open endscreen

    //peform some reset
    byte* pbVar3 = &FP::getData<byte>(0x0061a52b);
    for (short i = 0; i < FP::getData<short>(0x0065db10); ++i) {
        *(int*)(pbVar3 + -0x387) = 0;
        FP::callFastFarA(0x004db610, (long long*)(pbVar3 + -0x693), (unsigned int)*pbVar3);
        pbVar3 = pbVar3 + 0x6b8;
    }

    /*
    if (0 < FP::getData<short>(0x0065db10)) {
        pbVar3 = &FP::getData<byte>(0x0061a52b);
        do {
            *(int*)(pbVar3 + -0x387) = 0;
            FP::callFastFarA(0x004db610, (long long*)(pbVar3 + -0x693), (unsigned int)*pbVar3);
            pbVar3 = pbVar3 + 0x6b8;
            iVar4 = iVar4 + 1;
        } while (iVar4 < FP::getData<short>(0x0065db10));
    }
    */

    unsigned long long uVar5 = FP::callFastFarA<unsigned long long>(0x00523f90, ((float)(*(short*)(&FP::getData<int>(0x00545a52) + FP::getData<int>(0x008b7db8) * 0x10) * 0x27d8 >> 0xf)));
    FP::getData<int>(0x00778dfc) = (int)uVar5;
    FP::getData<int>(0x00778e00) = FP::getData<int>(0x00778dfc);
    uVar5 = FP::callFastFarA<unsigned long long>(0x00523f90, (float)FP::getData<int>(0x00778dfc) * FP::getData<int>(0x00778df0));
    int iVar4 = FP::getData<int>(0x007c8e70);
    FP::getData<int>(0x00778df4) = (int)uVar5;
    FP::getData<int>(0x00778df8) = FP::getData<int>(0x00778df4);
    if (FP::getData<int>(0x007c8e70) != 0) {
        char* pcVar1 = (char*)(FP::getData<unsigned int>(0x007c8e70) + 0x94);
        short sVar2 = (short)uVar5;
        *(short*)(FP::getData<unsigned int>(0x007c8e70) + 100) = sVar2;
        if ((*pcVar1 == '\0') && (*(char*)(iVar4 + 0x95) == '\0')) {
            FP::getData<unsigned int>(0x006b7574) = FP::getData<unsigned int>(0x006b7574) + 1;
            *(unsigned short*)(iVar4 + 0x66) = (unsigned short)*(byte*)(iVar4 + 0x62);
            *(short*)(iVar4 + 0x68) = sVar2;
            *(int*)(iVar4 + 0x6a) = 0x1e;
            return;
        }
        *(int*)(iVar4 + 0x7c) = (int)sVar2;
        *(short*)(iVar4 + 0x6a) = 0x1e;
    }
    FP::getData<unsigned int>(0x006b7574) = FP::getData<unsigned int>(0x006b7574) + 1;
    return;
}

void __stdcall FUN_004710d0() {

    //BEGIN custom code
    //go back to custom map screen
    if (DisaDungeon& dungeon = FP::getData<DisaDungeon>(0x00653a98); dungeon.getMainCategoryId() >= 654) {
        FP::getData<int>(0x0088abcc) = 1;
        FP::getData<int>(0x0067728c) = 0;
        FP::getData<int>(0x0088abd0) = 0;
        DisaCustomCampaignManager::get()->reactivate();
        dungeon.id = 0;
        return;
    }
    //END custom code

    FP::getData<int>(0x0067728c) = 0;
    FP::getData<int>(0x0088abcc) = 0xb; // sets state to FUN_ProccessDungeonsScreen in game loop
    FP::getData<int>(0x0088abd0) = 0;
    FP::callStdFarA(0x00470fa0); //FUN_PopulateDungeaonsListEntries
    FP::getData<int>(0x0088ac80) = 0; //number of dungeaons entries to display

    unsigned char* globalStateIt = &FP::getData<unsigned char>(0x00619330);
    const BasicList<DisaDungeonDAT>& disaDungeonDatList = FP::getData<BasicList<DisaDungeonDAT>>(aDungeonDATEntries);

    int counter = 0;
    int index = 0;

    for (const DisaDungeon currentDungeon : FP::getData<std::array<DisaDungeon, 160>>(0x00653aa0)) {
        if (currentDungeon.active) {
            if (auto findDungeonDatIt = std::ranges::find(disaDungeonDatList, currentDungeon.id, &DisaDungeonDAT::dungeonId); findDungeonDatIt != disaDungeonDatList.end()) {

                if (findDungeonDatIt != disaDungeonDatList.end() && 
                    !findDungeonDatIt->filter.isSet(globalStateIt) &&
                    currentDungeon.getMainCategoryId() == (unsigned short)FP::getData<char>(0x0065db44)) {

                    (&FP::getData<short>(0x0088ac00))[counter++] = index; //dungeaons entries
                }
            }
        }
        ++index;
    }

    FP::getData<short>(0x0088ac80) = (short)counter; //number of dungeaons entries to display

    FP::getData<int>(0x0088abd8) = 0;
    FP::getData<int>(0x0088abd4) = 0;
    if (counter > 0) {
        const int numberOfListEntries = std::min(counter, 8);
        const int lastScrollIndex = std::max(counter - 8, 0);

        FP::callFastFarA(fHandleListScrolling, &FP::getData<int>(0x0088abf4), &FP::getData<int>(0x0088abf8), numberOfListEntries, lastScrollIndex, 2);
    }
}

void _stdcall FUN_glViewport(int x, int y, int width, int height) {
    DisaCustomRenderManager::get()->glViewportReplace(x, y, width, height);
}

void _stdcall FUN_glDrawArrays(unsigned int mode, int first, int count) {
    DisaCustomRenderManager::get()->glDrawArraysReplace(mode, first, count);
}

void __fastcall FUN_RenderEnd(int param_1) {
    DisaCustomRenderManager::get()->lastReplace();
}