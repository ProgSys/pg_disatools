#include "DisaFunctions.h"
#include <Managers/DisaCustomCampaignManager.h>
#include <Managers/DisaCustomModificationsManager.h>
#include <Managers/DisaCustomRenderManager.h>
#include <Scene/PG_ModelNode.h>
#include <Managers/DisaCustomTranslationManager.h>
#include <DisaCharacterFunctions.h>
#include <Windows.h>
#undef max
#undef min

#include "FunctionPatcher.h"
#include <DisaRendering.h>

#include <Util/Color/RGBA.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_StringUtil.h>
#include <Disa/fileformats/PG_MPD.h>
#include <Disa/fileformats/PG_ShiftJisEncoder.h>
#include <Stream/PG_StreamInByteArray.h>
#include <ranges>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/constants.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <Scene/Gui/PG_GuiText.h>


DisaKeyToAtlasMapping* DisaKeyToAtlasMapping::getKeyboard() {
    return &FP::getData<DisaKeyToAtlasMapping>(0x005973f0);
}

DisaCutout* DisaCutout::getXBoxController() {
    return &FP::getData<DisaCutout>(0x00597b98);
}

DisaCutout* DisaCutout::getPlayController() {
    return &FP::getData<DisaCutout>(0x00597b98);
}

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
        if (findDungeonDatIt->filter.isClear(globalStateIt)) continue; //check if entry should be displayed, based on current game state (maybe save file)

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


struct MenuOptionStyle {
    unsigned int keyIndex;
    unsigned int stringIndex;
    DisaCutout textCutout;
    bool hover = false;
};

static std::array<MenuOptionStyle, 4> gMenuOptionStyles{
    MenuOptionStyle{ //exit
        DisaSettings::KeyAssigned::Cancel,
        8,
        DisaCutout{0, 0, 0x30, 0x18}
    },
    MenuOptionStyle{ //skipStory
        DisaSettings::KeyAssigned::Menu,
        10,
        DisaCutout{108, 0, 104, 0x18}
    },
    MenuOptionStyle{ //select
        DisaSettings::KeyAssigned::Select,
        9,
        DisaCutout{48, 0, 60, 0x18}
    },  
    MenuOptionStyle{ //select
        DisaSettings::KeyAssigned::Select,
        0,
        DisaCutout{0, 0, 0, 0}
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

static inline DisaCutout getKeyMapCutout(const DisaSettings* settings, unsigned int key) {
    DisaCutout cutout{0, 0, 0, 0};
    int mode = 1;
    if (!settings->isController()) {
        mode = 2;
    }
    if ((FP::getData<char>(0x00778d5b) != '\0') && (mode == 2)) {
        mode = 0;
    }

    if (mode == 2) {  //keyboard
        //find letter to display to exit screen
        unsigned int choosenLetter = settings->assignedKeys[key]; //[0x005d7c40]+  + 0x81;
        DisaKeyToAtlasMapping* keysIt = DisaKeyToAtlasMapping::getKeyboard();
        while (keysIt->key != -1 && keysIt->cutout.x != -1) {
            if (keysIt->key == choosenLetter) {
                cutout = keysIt->cutout;
                break;
            }
            ++keysIt;
        }
        return cutout;
    }

    unsigned int choosenSymbol = settings->assignedControllerKeys[key];
    if (choosenSymbol > 27) choosenSymbol -= 0xc;
    if (mode == 1) {
        cutout = (&FP::getData<DisaCutout>(0x00597b98))[choosenSymbol]; //controller 
    }
    else {
        cutout = (&FP::getData<DisaCutout>(0x005972b0))[choosenSymbol]; //0x005972bc
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

            DisaCutout cutout = getKeyMapCutout(settings, style.keyIndex);

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
        DisaCutout cutout = getKeyMapCutout(settings, key);
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
    DisaCutout cutout;

    if (key != DisaSettings::KeyAssigned::Invalid) {
        cutout = getKeyMapCutout(settings, key);
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

static char* g_translationStr = nullptr;
static std::array<char, 512> g_translateBuffer = { 0 };

bool __fastcall renderTranslatedString(char*& jis_string, int& mode) {
    auto manager = DisaCustomTranslationManager::get();

    g_translationStr = manager->translate(jis_string);

    if (!g_translationStr && manager->hasMarkes(jis_string)) {
        g_translationStr = jis_string;
        if (mode == 1) {
            for (int i = 0; i < g_translateBuffer.size() - 1; ++i) {
                g_translateBuffer[i] = g_translationStr[i];
                if (g_translationStr[i] == 0) break;
            }
            g_translationStr = g_translateBuffer.data();
            shiftJisToHalf((unsigned char*)g_translationStr);
        }
    }

    return false;
}

struct DisaTextIcon {
    unsigned int* texture;
    DisaCutout cutout;
    glm::vec2 offset;
    int size;
};

struct DisaText {
    int usage;
    PG_Model model;
    bool isController;
    std::vector<DisaTextIcon> icons;
};

static std::map<std::string, DisaText> g_fontGeometry;

static const std::map<size_t, std::pair<uint8_t, uint8_t>> g_charcodeToKeyboardSetting = {
    {9633, {3, 1}},//White Square
    {9651, {0, 1}},//White Up-Pointing Triangle
    {215,  {1, 1}},//Multiplication Sign
    {9675, {2, 1}},//White Circle
    {76,   {4, 2}},//Latin Capital Letter L
    {82,   {5, 2}},//Latin Capital Letter R
};

void createFontGeometry(PG_Model& model, std::vector<DisaTextIcon>& icons, DisaSettings* settings, const std::string& str) {
    icons.clear();
    auto* manager = DisaCustomTranslationManager::get();
    auto chars = manager->getFont().toCharInfosUtf8(str);

    struct GeoData {
        std::vector<glm::vec3> position;
        std::vector<glm::vec2> uv;
        std::vector<unsigned int> indices;
    };
    std::map<int32_t, GeoData> datas;

    auto baselines = PG_GuiText::createBaseLine(chars, manager->getFont(), 24.f, glm::vec2(0));
    const glm::vec2 padding = manager->getFont().getAtlasPadding();
    for (PG_GuiText::Baseline& baseline : baselines) {
        for (auto it = baseline.chars.begin(); it != baseline.chars.end(); ++it) {
            PG_GuiText::Char& c = *it;
            const glm::vec3 basePos((baseline.start.x + c.offset.x), (baseline.start.y - c.offset.y + baseline.size.y / 2.f), 0.0f);

            //White Square && White Up-Pointing Triangle
            if (auto findIt = g_charcodeToKeyboardSetting.find(c.charcode); findIt != g_charcodeToKeyboardSetting.end()) {
                //special case for L1 and R1
                auto nextIt = std::next(it);
                if ((findIt->second.second == 2)) {
                    if (nextIt != baseline.chars.end() && nextIt->charcode == 49) {
                        //is L1 or R2
                        nextIt->textureIndex = -1;
                        baseline.start.x -= nextIt->size.x;
                    }
                    else {
                        //normal L or R
                        findIt = g_charcodeToKeyboardSetting.end();
                    }
                }

                if (findIt != g_charcodeToKeyboardSetting.end()) {

                    auto cutout = getKeyMapCutout(settings, findIt->second.first);
                    if (cutout.x > 0) {
                        constexpr std::array<std::pair<int, float>, 3> styles = { 
                            std::pair<int, float>{14, 25.f}, //size, offset
                            std::pair<int, float>{14, 26.f},
                            std::pair<int, float>{14, 27.f}, //L1 R1
                        };
                        const auto& style = styles[settings->isController()? findIt->second.second : 0];

                        icons.emplace_back(
                            &FP::getData<unsigned int>(0x0074c8d8),
                            cutout,
                            glm::vec2(basePos.x - 2.f, 2.f), style.first
                        );
                        float nextCharDist = nextIt != baseline.chars.end() ? nextIt->offset.x - c.offset.x : c.size.x;
                        //float nextCharDist = c.size.x;
                        baseline.start.x += style.second - nextCharDist;
                        continue;
                    }
                }
            }

            //handle special icons Circled Digit 1 to 12
            //mapping to items
            if (c.charcode >= 9312 && c.charcode <= 9331) {
                constexpr static std::array<DisaCutout, 20> locations = {
                    DisaCutout(0, 0, 32, 32), DisaCutout(32, 0, 32, 32), DisaCutout(32 * 2, 0, 32, 32), DisaCutout(32 * 3, 0, 32, 32),
                    DisaCutout(32 * 4, 0, 32, 32), DisaCutout(32 * 5, 0, 32, 32), DisaCutout(32 * 6, 0, 32, 32), DisaCutout(128, 255, 32, 32),
                    DisaCutout(160, 160, 32, 32), DisaCutout(192, 160, 32, 32), DisaCutout(224, 160, 32, 32), DisaCutout(256, 160, 32, 32),
                    DisaCutout(288, 160, 32, 32), 
                    //geo symbols
                    DisaCutout(424, 264, 24, 24), DisaCutout(376, 240, 24, 24), DisaCutout(400, 240, 24, 24),
                    DisaCutout(424, 240, 24, 24), DisaCutout(448, 240, 24, 24), DisaCutout(376, 264, 24, 24), DisaCutout(400, 264, 24, 24)
                };
                constexpr static std::array<float, 20> yOffset = {
                    3.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 3.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f
                };
                int index = c.charcode - 9312;
                icons.emplace_back(
                    &FP::getData<unsigned int>(0x0074cc08),
                    locations[index],
                    glm::vec2(basePos.x, yOffset[index]), index >= 13 ? 14: 15
                );
                constexpr float iconsDist = 30.f;
                constexpr float iconsSmallDist = 24.f;
                auto nextIt = std::next(it);
                float nextCharDist = nextIt != baseline.chars.end() ? nextIt->offset.x - c.offset.x : c.size.x;
                baseline.start.x += (index >= 13 ? iconsSmallDist : iconsDist) - nextCharDist;
                continue;
            }
            if (c.textureIndex < 0) continue;

            auto& [position, uv, indices] = datas[c.textureIndex];
            const unsigned int i = position.size();
            /*
            *  1-----2
            *  |     |
            *  |     |
            *  |     |
            *  0-----3
            */
            position.insert(position.end(), {
                basePos - glm::vec3(padding, 0.f),
                basePos + glm::vec3(0.f, -c.size.y, 0.f) - glm::vec3(padding, 0.f),
                basePos + glm::vec3(c.size.x , -c.size.y , 0.f) - glm::vec3(padding, 0.f),
                basePos + glm::vec3(c.size.x, 0.f, 0.f) - glm::vec3(padding, 0.f),
                });
            uv.insert(uv.end(), {
                c.texOffset + glm::uvec2(0, c.texSize.y), c.texOffset ,  c.texOffset + glm::uvec2(c.texSize.x, 0) , c.texOffset + c.texSize
                });
            indices.insert(indices.end(), { i, i + 3, i + 1,   i + 1, i + 3, i + 2 });
        }
    }

    for (auto& dat : datas) {
        PG_Mesh geoInfo(
            {
                PG_Mesh::DataWrapper("position", std::move(dat.second.position)),
                PG_Mesh::DataWrapper("uv", std::move(dat.second.uv)),
                PG_Mesh::DataWrapper("indices",std::move(dat.second.indices), PG_Mesh::Index),
            }
            );
        auto geo = PG_Geometry(PG_UNIQUE_RES("D_TextGeo"), std::move(geoInfo));
        geo.expedite();
        auto mat = PG_Material("M_DisaFont");
        mat.makeInstance();
        mat.set("tex", manager->getFont().getTexture(dat.first));
        model.load(PG_UNIQUE_RES("D_DisaText"), { {std::move(mat), std::move(geo)}});
        break;
    }
}

/*☺
static std::array<char, 21 * 2> repString = {
    0x87, 0x3f + 1, 0x87, 0x3f + 2, 0x87, 0x3f + 3, 0x87, 0x3f + 4, 0x87, 0x3f + 5,
     0x87, 0x3f + 6, 0x87, 0x3f + 7, 0x87, 0x3f + 8, 0x87, 0x3f + 9, 0x87, 0x3f + 10,
      0x87, 0x3f + 12, 0x87, 0x3f + 12, 0x87, 0x3f + 13, 0x87, 0x3f + 14, 0x87, 0x3f + 15,
       0x87, 0x3f + 16, 0x87, 0x3f + 17, 0x87, 0x3f + 18, 0x87, 0x3f + 19, 0x87, 0x3f + 20, 
       0x0,0x0
};
static std::array<char, 18 * 2> repString2 = {
    0x81, 0x5f + 10,
    0x81, 0x9e + 2,
    0x81, 0x5f + 11,
    0x81, 0x5f + 10,
    0x81, 0x9e + 4, 
    0x81, 0x5f + 11,
    0x81, 0x6f + 15, 
    0x81, 0x90 + 11, 
    0x87, 0x4f + 5, //I
    0x87, 0x4f + 6, //II
    0x87, 0x4f + 7,
    0x87, 0x4f + 8, //IV
    0x90, 0xbe + 4,

    0x82, 0x5f + 12,
    0x82, 0x4f + 1,
    0x82, 0x6f + 2,
    0x82, 0x4f + 1,
     0x0, 0x0, 
};
*/

bool __fastcall renderTranslatedStringOverride(int x, int y, int param_3, glm::rgba* color, char*& jis_string, float scaleX, float scaleY, int stringWidth, int height, int mode) {
    auto* manager = DisaCustomTranslationManager::get();

    DisaSettings* settings = (DisaSettings*)(FP::getData<unsigned int>(0x005d7c40));
    const bool isController = settings->isController();

    if (manager->getFont().isLoaded()) {
        constexpr float scaleFactor = 71.f;
        const float scale = 24.f / scaleFactor;
        float baselineHeight = ((manager->getFont().getScaledHeight() * 0.69f + 12.f) * scale);

        const auto [hash, translation] = manager->tranlatedUtf8(jis_string);
        auto cc = translation.data();
        if (translation.empty() || !manager->isEnabled()) return false;

        auto& [usage, model, isIconsController, icons] = g_fontGeometry[hash];
        if ((!model.isValid() && usage == 0) || (!icons.empty() && isIconsController != isController)) {
            createFontGeometry(model, icons, settings, translation);
        }
        usage = 3;
        isIconsController = isController;

        constexpr float staticScaleX = 0.3f * 1.8f;
        const glm::vec2 fontScale = manager->getFontScale() * glm::vec2(staticScaleX, 1.8f);
        glm::vec2 fontPos = glm::vec2(x, y) + manager->getFontOffset();
        if (model.isValid() && model.getMaterial(0).isValid()) {
            auto* renderManager = DisaCustomRenderManager::get();
            //the same text may be drawn at several places in one frame, each one needs a node of its own
            auto* node = renderManager->getNode<PG_ModelNode>(renderManager->makeFrameNodeId("font:" + hash), DisaCustomRenderManager::EGroup::Screen);
            if (node->getModel() != model) node->setModel(model);
            node->setPropertyValue("disaGuiTransProjection", PG_Any::from(glm::translate(glm::mat4(1.f), glm::vec3(fontPos.x, fontPos.y, 0.f))));
            node->setPropertyValue("disaGuiScaleProjection", PG_Any::from(glm::scale(glm::mat4(1.f), glm::vec3(fontScale.x, scaleY * fontScale.y, 1.f))));
            node->setPropertyValue("disaFontColor", PG_Any::from(glm::vec4(*color) / 255.f));
            renderManager->submit(49999.0f, node, -1, false, false, 0, 0, '\0', false);
        }

        fontPos += manager->getIconOffset();
        for (const auto& [tex, cutout, offset, size] : icons) {
            FP::callFastFarA(fRenderCutout, tex, &FP::getData<glm::rgba>(0x005ad534), (float)fontPos.x + fontScale.x * offset.x, ((float)fontPos.y - offset.y), param_3, size, size,
                cutout.y, cutout.x, cutout.width, cutout.height);
        }

        //FUN_00425df0_SubmitVertexBasic(49999.0,piVar2,geometryData,triangleCount,3,-1,(Mat4 *)0x0,false,false,0,0,'\0',false);
        return true;
    }

    g_translationStr = manager->translate(jis_string);

    if (!g_translationStr && manager->hasMarkes(jis_string)) {
        g_translationStr = jis_string;
        if (mode == 1) {
            for (int i = 0; i < g_translateBuffer.size() - 1; ++i) {
                g_translateBuffer[i] = g_translationStr[i];
                if (g_translationStr[i] == 0) break;
            }
            g_translationStr = g_translateBuffer.data();
            shiftJisToHalf((unsigned char*)g_translationStr);
        }
    }

    return false;
}

void __declspec(naked) __fastcall FUN_RederTextNew(int x, int y, int param_3, int* param_4, unsigned short* jis_string, float scaleX, float scaleY, int stringWidth, int height, int mode) {
    __asm {
        // Save fastcall registers (x in ECX, y in EDX)
        push ebp
        mov  ebp, esp
        sub  esp, 12                 // locals:
        // [ebp-4]  = saved x
        // [ebp-8]  = saved y
        // [ebp-12] = temp char* for char*&

        // save fastcall registers
        mov[ebp - 4], ecx            // x
        mov[ebp - 8], edx            // y

        // temp = (char*)jis_string;
        mov  eax, [ebp + 16]           // incoming jis_string
        mov[ebp - 12], eax

        // call:
        // bool __fastcall renderTranslatedStringOverride(
        //     ecx=x, edx=y,
        //     param_3, param_4, char*& temp, scaleX, scaleY, stringWidth, height, mode)

        push dword ptr[ebp + 36]      // mode
        push dword ptr[ebp + 32]      // height
        push dword ptr[ebp + 28]      // stringWidth
        push dword ptr[ebp + 24]      // scaleY
        push dword ptr[ebp + 20]      // scaleX
        lea  eax, [ebp - 12]
        push eax                     // char*& jis_string  -> pass &temp
        push dword ptr[ebp + 12]      // param_4
        push dword ptr[ebp + 8]       // param_3
        call renderTranslatedStringOverride
        // __fastcall callee cleans these 8 stack args

        test al, al
        jnz  _return_ok

        // restore fastcall regs for original render
        mov  ecx, [ebp - 4]            // x
        mov  edx, [ebp - 8]            // y

        // optional:
        // if you want fallback to use a possibly modified string from override,
        // write temp back into the stack argument slot:
        mov  eax, [ebp-12]
        mov  [ebp+16], eax

        push dword ptr[ebp + 36]      // mode
        push dword ptr[ebp + 32]      // height
        push dword ptr[ebp + 28]      // stringWidth
        push dword ptr[ebp + 24]      // scaleY
        push dword ptr[ebp + 20]      // scaleX
        push dword ptr[ebp + 16]      // jis_string
        push dword ptr[ebp + 12]      // param_4
        push dword ptr[ebp + 8]       // param_3

        call fRenderTextNew
        add     esp, 32

     _return_ok :
        mov  esp, ebp
        pop  ebp
        ret
    }
}

bool __fastcall translateString(char* str, char* outStr, size_t* outLength) {
    if (!g_translationStr) return false;
    return DisaCustomTranslationManager::get()->toUtf8(g_translationStr, outStr, outLength);
}

constexpr unsigned int fToUtf8 = 0x0050c210; //func
void __declspec(naked) __fastcall FUN_0050c210_toUtf8(char* str, char* outStr, size_t* outLength) {
   
    __asm {
        push ecx
        push edx
        push[esp + 8 + 4] //out length
        call translateString

        test    al, al
        jnz     short _return_ok

        mov     ecx, [esp + 4]            // restore str
        mov     edx, [esp]                // restore outStr
        push    [esp + 12] //out length
        call fToUtf8 //original
        add     esp, 4

    _return_ok:
        add     esp, 8                    // drop saved edx, ecx
        ret
    }
    
}

void __fastcall translateReplace(char*& jis_string) {
    auto* translator = DisaCustomTranslationManager::get();
    auto str = translator->translate(jis_string);
    if (str) {
        translator->replaceSpecial(str);
        jis_string = str;
    }
}

int __declspec(naked) __fastcall FUN_GetWidthOfString(char* str, int param_2, int param_3, int* param_4, int param_5) {
    __asm {
        // Save fastcall registers (x in ECX, y in EDX)
        push ecx
        push edx

        lea  ecx, [esp + 4]
        call translateReplace
        // Restore x,y for the final target
        pop edx
        pop ecx

        jmp fGetWidthOfString
    }
}

void FUN_00471240_RenderDungeonScreeen(void) {

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
                FUN_RederTextNew(0x26, yPos + 2, 0xc7a7, &FP::getData<int>(0x005ad538), stringPtr, 0.3f, 0.3f, dungeonsBgBoxWidth, dungeonsBgBoxHeight, settingVar);
                //FP::callFastFarA(fRenderTextNew, 0x26, yPos + 2, 0xc7a7, &FP::getData<int>(0x005ad538), stringPtr, 0.3f, 0.3f, dungeonsBgBoxWidth, dungeonsBgBoxHeight, settingVar);
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


void FUN_00471780_ProccessCategoryDungeonsScreen() {
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
    FUN_00471240_RenderDungeonScreeen();
}


void _stdcall FUN_00472a40_ProccessDungeonsScreen() {
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

void FUN_0050a1f0_GetFileData(int fileIndex, int** fileData, bool param_4) {
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
            piVar1 = FP::callStdFarA<int*>(0x004195e0, &local_24, (&FP::getData<int>(0x0055f9f0))[fileIndex], 1);
            local_c.a = 1;
        }
        else {
            int i = (&FP::getData<int>(0x00555e48))[fileIndex];
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
    FP::getData<int>(0x0088abcc) = 0xb; // sets state to FUN_00472a40_ProccessDungeonsScreen in game loop
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
                    !findDungeonDatIt->filter.isClear(globalStateIt) &&
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

struct GeometryBatchKey {
    unsigned short textureIndex;
    unsigned char textureAdditive;

    bool operator<(const GeometryBatchKey& other) const{
        return textureIndex < other.textureIndex || (textureIndex == other.textureIndex && textureAdditive < other.textureAdditive);
    }
};

static std::map<std::pair<geofileMeshHeader*, GeometryBatchKey>, std::pair<byte*, bool>> g_geometryMemoryBuffer;
static std::map<std::pair<geofileMeshHeader*, GeometryBatchKey>, std::pair<int, std::vector<byte>>> g_geometryMemoryMeshData; //key - used count, mesh data

void _stdcall FUN_004201d0_RenderClearBegin() {
    FP::callStdFarA(0x004201d0);
    g_geometryMemoryBuffer.clear();
    for (auto it = g_geometryMemoryMeshData.begin(); it != g_geometryMemoryMeshData.end();) {
        --it->second.first; //deplete use counter
        if (it->second.first <= 0) {
            //memory is no longer in use
            it = g_geometryMemoryMeshData.erase(it);
        }
        else {
            ++it;
        }
    }
    for (auto it = g_fontGeometry.begin(); it != g_fontGeometry.end();) {
        --it->second.usage; //deplete use counter
        if (it->second.usage <= 0) {
            //memory is no longer in use
            it = g_fontGeometry.erase(it);
        }
        else {
            ++it;
        }
    }
}

void __cdecl FUN_00424ae0_FrameBegin() {
    static bool firstCall = false;
    if (!firstCall) {
        firstCall = true;
        //the mods register with the resource manager, which has to live on this thread
        DisaCustomRenderManager::get()->init();
        auto* manager = DisaCustomModificationsManager::get();
        manager->waitForModThread();
        manager->loadModifications();
    }

    DisaCustomRenderManager::get()->onFrameBegin();
    FP::callFarA(0x00424ae0); //ages the palette slots of every texture, still has to happen
}

void _stdcall FUN_glViewport(int x, int y, int width, int height) {
    DisaCustomRenderManager::get()->glViewportReplace(x, y, width, height);
}

void _stdcall FUN_glDrawArrays(unsigned int mode, int first, int count) {
    DisaCustomRenderManager::get()->glDrawArraysReplace(mode, first, count);
}

static inline void copyIntoMemory(DisaObjectModel* context, byte* memory, const size_t stride, const GeometryBatchKey key) {

    uint32_t dataStart = 0;
    uint32_t dataNextLength = 0;
    for (const geofileSurface& surface : std::ranges::subrange(context->m_geofileSurfaces, context->m_geofileSurfaces + context->m_numberOfSurfaces)) {
        dataStart += dataNextLength;
        dataNextLength = surface.length;

        if (surface.index != key.textureIndex || surface.additive != key.textureAdditive) {
            continue; //We could collect the start index during batch computation
        }

        //compute color
        glm::rgba color;
        {  // Compute final rgba and pack it into an sizeof(int)
            glm::frgba colorAsFloat(
                (float)surface.rMult * context->m_colorMultiplier->r,
                (float)surface.gMult * context->m_colorMultiplier->g,
                (float)surface.bMult * context->m_colorMultiplier->b,
                (float)surface.aMult * context->m_colorMultiplier->a * context->m_alphaScale
            );
            colorAsFloat = glm::round(colorAsFloat);
            colorAsFloat = glm::clamp(colorAsFloat, 0.f, 255.f); //clamp to byte
            color = glm::rgba(colorAsFloat);
        }

        //write into memory
        auto trianglesIt = context->m_triangles + dataStart;
        if (stride == sizeof(VertexSolidColor)) {
            VertexSolidColor* outIt = (VertexSolidColor*)memory;
            for (int i = 0; i < surface.length; ++i) {
                outIt->color = *((uint32_t*)&color.r);
                outIt->pos = *trianglesIt;
                ++trianglesIt;
                ++outIt;
            }
            memory = (byte*)outIt;
            continue;
        }
        auto uvsIt = context->m_uvs + dataStart;
        //we have texture so we need to compute uv scrolling if needed
        if (surface.animated && key.textureIndex > -1 && (context->m_uvScrollObject->uvScrollSpeedX || context->m_uvScrollObject->uvScrollSpeedY)) {
            double factor = FP::getData<double>(0x00598258);
            context->m_uvScrollSpeed = glm::vec2(*context->m_uvScrollObject->uvScrollSpeedX, *context->m_uvScrollObject->uvScrollSpeedY) * (float)factor;
            *context->m_uvScrollObject->uvScrollX += context->m_uvScrollSpeed.x;
            *context->m_uvScrollObject->uvScrollY += context->m_uvScrollSpeed.y;
            context->m_uvScroll = glm::vec2(*context->m_uvScrollObject->uvScrollX, *context->m_uvScrollObject->uvScrollY);

            // Wrap if we moved out of [0,1] over the tile grid
            //custom as original code does strange voodo for no reason at all
            context->m_uvScroll = glm::fract(context->m_uvScroll);
            *context->m_uvScrollObject->uvScrollX = context->m_uvScroll.x;
            *context->m_uvScrollObject->uvScrollY = context->m_uvScroll.y;
            /*Original code that looks stupid
            glm::vec2 firstUVValue = *uvsIt + context->m_uvScroll;
            if (firstUVValue.x < 0.f || firstUVValue.x > 1.f) {
                const float offsetValue = firstUVValue.x <= 1.f ? 1.f : -1.f;
                context->m_uvScroll.x += offsetValue;
                *context->m_uvScrollObject->uvScrollX = context->m_uvScroll.x;
            }

            if (firstUVValue.y < 0.f || firstUVValue.y > 1.f) {
                const float offsetValue = firstUVValue.y <= 1.f ? 1.f : -1.f;
                context->m_uvScroll.y += offsetValue;
                *context->m_uvScrollObject->uvScrollY = context->m_uvScroll.y;
            }
            */
        }

        if (stride == sizeof(VertexBasic)) {
            VertexBasic* outIt = (VertexBasic*)memory;
            for (int i = 0; i < surface.length; ++i) {
                outIt->uv = *uvsIt + context->m_uvScroll;
                outIt->color = *((uint32_t*)&color.r);
                outIt->pos = *trianglesIt;
                ++uvsIt;
                ++trianglesIt;
                ++outIt;
            }
            memory = (byte*)outIt;
            continue;
        }

        auto tangentAndBitangentIt = context->m_tangentAndBitangent + dataStart * 2;

        if (stride == sizeof(VertexWithNormal)) {
            VertexWithNormal* outIt = (VertexWithNormal*)memory;
            for (int i = 0; i < surface.length; ++i) {
                outIt->uv = *uvsIt + context->m_uvScroll;
                outIt->color = *((uint32_t*)&color.r);
                outIt->pos = *trianglesIt;
                outIt->tangent = *tangentAndBitangentIt;
                ++tangentAndBitangentIt;
                outIt->bitangent = *tangentAndBitangentIt;
                ++tangentAndBitangentIt;
                ++uvsIt;
                ++trianglesIt;
                ++outIt;
            }
            memory = (byte*)outIt;
            continue;
        }

        assert(false); //unknown format
    }

}

byte* DisaMemeoryManager::FUN_00425bb0(int requestedMemorySize) {
    return FP::callThisFarA<byte*>(0x00425bb0, this, requestedMemorySize);
}


void __fastcall FUN_004ef8a0(DisaObjectModel* context, glm::mat4* modelMatrix) {
    if (context->m_numberOfSurfaces == 0) return;

    // Compute batches. i.e. merge all surfaces into on
    std::map<GeometryBatchKey, uint32_t> batches; //texture index, addative
    for (const geofileSurface& surface : std::ranges::subrange(context->m_geofileSurfaces, context->m_geofileSurfaces + context->m_numberOfSurfaces)) {
        batches[{surface.index, surface.additive}] += surface.length;
    }

    /*
    byte* memoryManager =  FP::getData<byte*>(0x005bb02c);
    uint32_t* byteCount = (uint32_t*)(memoryManager + 0x76038);
    uint32_t* memoryIndex = (uint32_t*)(memoryManager + 0x76040);
    uint32_t* memoryBank = (uint32_t*)(memoryManager + 0x76038 + *memoryIndex * 4);
    */

   
    for (auto& [key, totalLength] : batches) {
        if (totalLength == 0) continue;

        bool hasNormalMap = false;
        if (key.textureIndex > 0) { //does it even has a texture
            if (geofileTextureHeader* texture = context->m_textures[key.textureIndex -1]) {
                hasNormalMap = texture->hasNormal; // (*(size_t*)texture) & 0x80000000
            }
        }

        const size_t stride = key.textureIndex > 0 ? hasNormalMap ? sizeof(VertexWithNormal) : sizeof(VertexBasic): sizeof(VertexSolidColor);
        const size_t bytes = (totalLength * 3) * stride;


        size_t& arraySize = FP::getData<size_t>(0x005bae84);
        auto arrayRoot = FP::getData<size_t*>(0x005bae88);
        //iVar15 = *(int*)(&DAT_005bae88 + DAT_005bae84 * 4);

        size_t* memoryPtr = arrayRoot + arraySize;
       
        //CUSTOM memory reusage if we already build same geometry 
        auto& [memory, custom] = g_geometryMemoryBuffer[{context->m_geofileHeader, key}];
        //byte* memory = nullptr;
        if (!memory) {
            //perform custom render
            if (totalLength > 1024) {
                if (stride != sizeof(VertexBasic)) 
                    continue;  //other mesh types are not supported
                
                auto& [usage, mashData] = g_geometryMemoryMeshData[{context->m_geofileHeader, key}];
                if (usage == 0) {
                    mashData.resize(bytes + 32); //the size plus a buffer
                    memory = mashData.data();
                    copyIntoMemory(context, memory, stride, key);
                }
                else {
                    memory = mashData.data();
                }
                usage = 3;
                custom = true;
                /*
                auto sum = bytes + 32;
                g_geometryMemoryTotal += sum;
                if (g_geometryMemoryTotal >= g_geometryMemorySize) {
                    continue;
                }
                byte* dump = FP::callStdFarA<byte*>(0x00425c10, 0x20, 0x20);
                memset(dump, 0xFF, 0x20);
                memory = g_geometryMemory.data() + g_geometryMemoryAt;
                g_geometryMemoryAt += sum;
                */
            } else if (arrayRoot + arraySize == 0) {
                memory = FP::callStdFarA<byte*>(0x00425c10, 0x20, bytes);
                if (!memory) return; //CUSTOM out of memory check
                copyIntoMemory(context, memory, stride, key);
            }
            else {
                auto uVar18 = *(uint*)((uint)memoryPtr + 0x20) >> 0x1d & 1;
                if (*(int*)((uint)memoryPtr + 8 + uVar18 * 4) == 0) {
                    memory = nullptr;
                }
                else {
                    auto uVar13 = *(int*)((uint)memoryPtr + uVar18 * 4) + 0x1fU & 0xffffffe0;
                    *(uint*)((uint)memoryPtr + uVar18 * 4) = uVar13 + bytes;
                    memory = (byte*)(*(int*)((uint)memoryPtr + 0x10 + (*(uint*)((uint)memoryPtr + 0x20) >> 0x1d & 1) * 4) + uVar13);
                }
                copyIntoMemory(context, memory, stride, key);
            }
        }

        DisaRenderObjectManager* manager = DisaRenderObjectManager::get();
        const float depth = static_cast<float>(key.textureAdditive != 0 ? 45000 : 41000);
        const int blendMode = key.textureAdditive > 2 ? -1 : key.textureAdditive;
        switch (stride) {
            case sizeof(VertexSolidColor) : {
                manager->FUN_00426040_submitSolidColor(depth, (VertexSolidColor*)memory, totalLength, 3, -1, modelMatrix, true, 0.f);
            } break;
            case sizeof(VertexBasic) : {
                auto* texture = (DisaTextureSPtr*)context->m_textureDatas[key.textureIndex - 1];
                if (custom) {
                    manager->submitVertexBasicCustom(depth, texture, (VertexBasic*)memory, totalLength, 3, blendMode, modelMatrix, true, key.textureAdditive != 0, context->m_poligonOffset, 0, 0, false);
                }
                else {
                    manager->FUN_00425df0_submitVertexBasic(depth, texture, (VertexBasic*)memory, totalLength, 3, blendMode, modelMatrix, true, key.textureAdditive != 0, context->m_poligonOffset, 0, 0, false);
                }
            } break;
            case sizeof(VertexWithNormal) : {
                auto* texture = (DisaTextureSPtr*)context->m_textureDatas[key.textureIndex - 1];
                auto* normalMap = (DisaTextureSPtr*)context->m_textureHDDatas[key.textureIndex - 1];
                manager->FUN_004f3450_submitVertexWithNormalRef(depth, texture, normalMap, (VertexWithNormal*)memory, totalLength, 3, blendMode, modelMatrix, true, key.textureAdditive != 0, context->m_poligonOffset, 0, 0, false);
            } break;
        }
         
    }
}

/// The per frame allocator FUN_004ef8a0 inlines twice. Returns nullptr when the active bank has no buffer.
static byte* FUN_004ef8a0_allocFrameMemory(uint32_t bytes) {
    byte* slot = (&FP::getData<byte*>(0x005bae88))[FP::getData<int>(0x005bae84)];
    if (!slot) return FP::callStdFarA<byte*>(0x00425c10, 0x20, bytes);

    const uint32_t bank = *(uint32_t*)(slot + 0x20) >> 0x1d & 1;
    if (*(int*)(slot + 0x08 + bank * 4) == 0) return nullptr;

    const uint32_t offset = *(uint32_t*)(slot + bank * 4) + 0x1f & 0xffffffe0; //32 byte aligned
    *(uint32_t*)(slot + bank * 4) = offset + bytes;
    return *(byte**)(slot + 0x10 + bank * 4) + offset;
}

/**
* @brief FUN_0046ab50, packs the truncated surface color into RGBA bytes.
* If any RGB channel is above 255, all three are clamped to 511 and halved, keeping the hue. Alpha is clamped alone.
*/
static uint32_t FUN_0046ab50_packColor(int r, int g, int b, int a) {
    const uint32_t divisor = r > 0xff || g > 0xff || b > 0xff ? 2 : 1;
    const auto channel = [divisor](int value) { return (uint32_t)(uint8_t)((uint32_t)std::min(value, 0x1ff) / divisor); };
    const uint32_t alpha = a > 0xff ? 0xff : (uint8_t)a;
    return channel(r) | channel(g) << 8 | channel(b) << 16 | alpha << 24;
}

/**
* @brief The uv scroll step of FUN_004ef8a0.
* @param firstUV First uv of the animated surface. The scroll wraps by one when it lands outside [0,1].
*/
static void FUN_004ef8a0_advanceUVScroll(DisaObjectModel* self, const glm::vec2& firstUV) {
    DisaObjectScrollInfo* scroll = self->m_uvScrollObject;
    const double factor = FP::getData<double>(0x00598258); //0.0007326

    self->m_uvScrollSpeed = glm::vec2(*scroll->uvScrollSpeedX, *scroll->uvScrollSpeedY);
    *scroll->uvScrollX = (float)(self->m_uvScrollSpeed.x * factor + *scroll->uvScrollX);
    *scroll->uvScrollY = (float)(self->m_uvScrollSpeed.y * factor + *scroll->uvScrollY);
    self->m_uvScroll = glm::vec2(*scroll->uvScrollX, *scroll->uvScrollY);

    const glm::vec2 first = self->m_uvScroll + firstUV;
    if (first.x < 0.f || first.x > 1.f) {
        self->m_uvScroll.x += first.x > 1.f ? -1.f : 1.f;
        *scroll->uvScrollX = self->m_uvScroll.x;
    }
    if (first.y < 0.f || first.y > 1.f) {
        self->m_uvScroll.y += first.y > 1.f ? -1.f : 1.f;
        *scroll->uvScrollY = self->m_uvScroll.y;
    }
}

/**
* @brief FUN_004ef8a0 as close to the game's code as possible, without any memory reuse.
* @details Surfaces are merged into batches by texture and additive mode in first seen order. Each textured batch is
* copied into frame memory and submitted once. Untextured surfaces get their own memory and are submitted one by one.
* A batch goes to the translucent layer (45000, no depth writes) when its last surface is additive or has alpha <= 128.
*/
void __fastcall FUN_004ef8a0_original(DisaObjectModel* context, glm::mat4* modelMatrix) {
    DisaObjectModel* self = context->m_self;
    const glm::frgba* colorMultiplier = context->m_colorMultiplier;
    const auto surfaces = std::ranges::subrange(self->m_geofileSurfaces, self->m_geofileSurfaces + self->m_numberOfSurfaces);

    struct Batch {
        int textureIndex; /// 1 based, 0 is untextured
        int additive;
        int length;
    };
    std::vector<Batch> batches;
    for (const geofileSurface& surface : surfaces) {
        auto batchIt = std::ranges::find_if(batches, [&surface](const Batch& batch) { return batch.textureIndex == surface.index && batch.additive == surface.additive; });
        if (batchIt == batches.end()) {
            batches.push_back({ surface.index, surface.additive, surface.length });
        }
        else {
            batchIt->length += surface.length;
        }
    }

    constexpr auto toBlendMode = [](int additive) { return additive == 1 || additive == 2 ? additive : -1; };
    DisaRenderObjectManager* manager = DisaRenderObjectManager::get();

    bool scrolled = false; //the uv scroll advances once per call, on the first animated textured surface
    for (const Batch& batch : batches) {
        const int textureSlot = batch.textureIndex - 1;
        const bool hasNormalMap = textureSlot >= 0 && self->m_textures[batch.textureIndex]->hasNormal;

        //untextured batches get memory too, it stays unused
        byte* memory = FUN_004ef8a0_allocFrameMemory(batch.length * (hasNormalMap ? sizeof(VertexWithNormal) : sizeof(VertexBasic)));
        if (!memory) continue; //the original writes to it anyway and crashes

        bool translucent = false;
        int totalLength = 0;
        uint32_t vertexStart = 0;
        for (const geofileSurface& surface : surfaces) {
            const uint32_t surfaceStart = vertexStart;
            vertexStart += surface.length;
            if (surface.index != batch.textureIndex || surface.additive != batch.additive) continue;

            const uint32_t color = FUN_0046ab50_packColor(
                (int)(colorMultiplier->r * (double)surface.rMult),
                (int)((double)surface.gMult * colorMultiplier->g),
                (int)((double)surface.bMult * colorMultiplier->b),
                (int)((double)surface.aMult * self->m_alphaScale * colorMultiplier->a));
            const glm::vec3* positions = self->m_triangles + surfaceStart;
            const glm::vec2* uvs = self->m_uvs + surfaceStart;

            const DisaObjectScrollInfo* scroll = self->m_uvScrollObject;
            if (!scrolled && surface.animated && textureSlot >= 0 && (scroll->uvScrollSpeedX || scroll->uvScrollSpeedY)) {
                scrolled = true;
                FUN_004ef8a0_advanceUVScroll(self, *uvs);
            }

            if (textureSlot < 0) {
                auto* vertices = (VertexSolidColor*)FUN_004ef8a0_allocFrameMemory(surface.length * sizeof(VertexSolidColor));
                if (!vertices) continue;
                for (uint32_t i = 0; i < surface.length; ++i) {
                    vertices[i] = { color, positions[i] };
                }
                manager->FUN_00426040_submitSolidColor(41000.f, vertices, (int)surface.length, 3, toBlendMode(surface.additive), modelMatrix, true, 0.f);
                continue;
            }

            translucent = color >> 24 <= 0x80 || surface.additive != 0;
            if (self->m_textures[surface.index]->hasNormal) {
                VertexWithNormal* vertices = (VertexWithNormal*)memory + totalLength;
                const glm::vec3* tangents = self->m_tangentAndBitangent + surfaceStart * 2;
                for (uint32_t i = 0; i < surface.length; ++i) {
                    vertices[i] = { uvs[i] + self->m_uvScroll, color, positions[i], tangents[i * 2], tangents[i * 2 + 1] };
                }
            }
            else {
                VertexBasic* vertices = (VertexBasic*)memory + totalLength;
                for (uint32_t i = 0; i < surface.length; ++i) {
                    vertices[i] = { uvs[i] + self->m_uvScroll, color, positions[i] };
                }
            }
            totalLength += surface.length;
        }

        if (batch.length <= 0 || totalLength <= 0 || textureSlot < 0) continue;

        const float depth = translucent ? 45000.f : 41000.f;
        const int blendMode = toBlendMode(batch.additive);
        auto* texture = (DisaTextureSPtr*)self->m_textureDatas[textureSlot];
        if (hasNormalMap) {
            auto* normalMap = (DisaTextureSPtr*)self->m_textureHDDatas[textureSlot];
            manager->FUN_004f3450_submitVertexWithNormalRef(depth, texture, normalMap, (VertexWithNormal*)memory, totalLength, 3, blendMode, modelMatrix, true, translucent,
                context->m_poligonOffset, 0, 0, false);
        }
        else {
            manager->FUN_00425df0_submitVertexBasic(depth, texture, (VertexBasic*)memory, totalLength, 3, blendMode, modelMatrix, true, translucent, context->m_poligonOffset,
                0, 0, false);
        }
    }
}

void __thiscall DisaObjectModel::FUN_004e2180(glm::mat4* modelMatrix, glm::frgba* colorMultiplier, uint32_t param_3, float poligonOffset) {
    //no need to render geometry if its alpha is zero
    if (glm::isnan(colorMultiplier->a) || colorMultiplier->a == 0.f) return;

    m_colorMultiplier = colorMultiplier;
    m_field0x4 = param_3;
    m_self = this;
    m_poligonOffset = poligonOffset;

    //0 game code, 1 recreation with memory reuse (breaks particle systems), 2 recreation without optimizations
    static int renderFunction = 2;
    switch (renderFunction) {
        case 0: FP::callFastFarA(0x004ef8a0, this, modelMatrix); break;
        case 1: FUN_004ef8a0(this, modelMatrix); break;
        default: FUN_004ef8a0_original(this, modelMatrix); break;
    }
}