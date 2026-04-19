#include "DisaCustomCampaignManager.h"
#include <Managers/DisaCustomBaseModManager.h>
#include <Managers/DisaCustomRenderManager.h>
#include <Managers/DisaCustomScriptManager.h>
#include <DisaFunctions.h>
#include "FunctionPatcher.h"
#include <ranges>
#include <algorithm> 
#include <iostream> 
#include <windows.h>
#include <Disa/fileformats/PG_ShiftJisEncoder.h> 
#include <Disa/fileformats/PG_ZIPUtil.h>
#include <Disa/models/PG_DisaModLoadOrderListModel.h>
#include <Disa/utils/PG_DisaModLoadInfo.h>
#include <Util/Color/RGBA.h>
#include <Util/PG_ImageLoad.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_Dir.h>
#include <Util/PG_StringUtil.h>
#include <Stream/PG_StreamInByteFile.h>
#include <zlib.h>
#include <zip.h>
#include <nlohmann/json.hpp>

typedef unsigned char byte;



static inline void readFromJson(auto& j, const std::string& field, auto& target, unsigned int max = 128u) {
    if (auto id = j.find(field); id != j.end()) {
        id->get_to(target);
        if constexpr(std::is_same_v<decltype(target), std::string>)
            target.resize(std::min(target.size(), max));
    }
}


static inline bool readFromJsonMandatory(auto& j, const std::string& field, auto& target, unsigned int max = 128u) {
    readFromJson(j, field, target, max);

    if constexpr (std::is_same_v<decltype(target), std::string>) {
        if (target.empty()) {
            OUT_ERROR("'campaign.json' has no valid '"<< field  <<"' field!");
            return false;
        }
    }

    return true;
}

static inline void readCampaign(DisaCustomCampaignManager::Campaign& campain, const PG_DisaModLoadInfo& info) {
    auto& j = info.json;

    if (auto item = j.find("areas"); item == j.end() || !item->is_array() || item->empty()) {
        throw_Exception("'campaign.json' has no areas inside zip!");
    }

    if (!readFromJsonMandatory(j, "id", campain.id)) throw_Exception("'campaign.json' has no id inside zip!");
    if (!readFromJsonMandatory(j, "name", campain.name)) throw_Exception("'campaign.json' has no name inside zip!");
    readFromJson(j, "description", campain.description, 1024);
    readFromJson(j, "author", campain.author);
    readFromJson(j, "previewImage", campain.previewImage);
    readFromJson(j, "backgroundImage", campain.backgroundImage);

    readFromJson(j, "namesspace", campain.namespaceMod);
    if (campain.namespaceMod.empty()) {
        campain.namespaceMod = info.file.getName();
    }
    campain.namespaceMod = PG_StringUtil::toLowerCopy(campain.namespaceMod);

    campain.name = stringToShiftJis(campain.name);
    campain.description = stringToShiftJis(campain.description);
    campain.author = stringToShiftJis(campain.author);
    campain.zipFile = info.file;
}

void DisaCustomCampaignManager::Campaign::open() {
    if (archive) archive.reset();
    entries.clear();
    int error = 0;
    zip* ptr = zip_open(zipFile.c_str(), 0, &error);

    if (!ptr) {
        OUT_ERROR("Failed to open the zip '" << zipFile << "': error code " << error);
        return;
    }
    archive = zipUPtr(ptr, [](zip* a) { zip_close(a); });

    nlohmann::json j;
    try {
        j = PG_ZIPUtil::readJson(archive.get(), "campaign.json");
    }
    catch (const PG_RuntimeError& e) {
        OUT_ERROR("Failed to parse campaign json in zip '" << zipFile << "' with error: " << e.what());
        return;
    }
    catch (...) {
        OUT_ERROR("Failed to parse campaign json in zip '" << zipFile << "'!");
        return;
    }

    if (j.empty()) {
        OUT_ERROR("Failed to parse 'campaign.json' inside zip '" << zipFile << "'!");
        return;
    }

    for (const auto& jArea : j["areas"]) {
        DisaCustomCampaignManager::CampaignArea area;
        readFromJson(jArea, "name", area.name, 24);
        readFromJson(jArea, "backgroundImage", area.backgroundImage, 2024);

        area.name = stringToShiftJis(area.name);

        for (const auto& jDungeon : jArea["dungeons"]) {
            DisaCustomCampaignManager::CampaignDungeon dungeon;
            readFromJson(jDungeon, "name", dungeon.name, 24);
            readFromJson(jDungeon, "description", dungeon.description, 1024);
            readFromJson(jDungeon, "bonusRank", dungeon.bonusRank);
            readFromJson(jDungeon, "bgm", dungeon.bgm);
            readFromJson(jDungeon, "mpd", dungeon.mpd, 2024);
            readFromJson(jDungeon, "mpp", dungeon.mpp, 2024);
            if (dungeon.mpp.empty() && !dungeon.mpd.empty()) {
                dungeon.mpp = dungeon.mpd;
                PG_File::replaceFileExtension(dungeon.mpp, "mpp");
            }
            dungeon.mppHD = dungeon.mpp;
            PG_File::replaceFileName(dungeon.mppHD, "new_" + PG_File(dungeon.mpp).getName());

            if (dungeon.mpd.empty() || dungeon.mpp.empty()) {
                OUT_ERROR("Missing mpd or mpp in dungeon '"<<  dungeon.name << "' in zip '" << zipFile << "'!");
                continue;
            }

            if (auto findScriptsIt = jDungeon.find("talkScripts"); findScriptsIt != jDungeon.end()) {
                readFromJson(*findScriptsIt, "intro", dungeon.scripts.intro.value);
                readFromJson(*findScriptsIt, "introSkipStory", dungeon.scripts.introSkipStory.value);
                readFromJson(*findScriptsIt, "exit", dungeon.scripts.exit.value);
                readFromJson(*findScriptsIt, "exitSkipStory", dungeon.scripts.exitSkipStory.value);
                for (int i = 0; i < 4; ++i) {
                    auto& id = ((PG_DisaIdentifier*)(&dungeon.scripts))[i];
                    if (!id.build(namespaceMod)) id.clear();
                }
            }

            dungeon.name = stringToShiftJis(dungeon.name);
            dungeon.description = stringToShiftJis(dungeon.description);

            area.entries.push_back(std::move(dungeon));
        }
        entries.push_back(std::move(area));
    }

}


template<typename T>
void computeMaxWidth(int& width, const T* item) {
    //compute max width
    width = 0;
    for (const auto& entry : item->entries) {
        width = std::max(width, FP::callFastFarA<int>(fGetWidthOfString, entry.name.data(), 0, -1, nullptr, -1));
    }
    width += 11;
}

DisaCustomCampaignManager::DisaCustomCampaignManager(PG_DisaScriptsModel* scriptsModel) : m_scriptsModel(scriptsModel) {
    assert(m_scriptsModel);
}

DisaCustomCampaignManager::TalkScripts* DisaCustomCampaignManager::getActiveTalkScripts() const {
    if (m_entriesState.empty())  return nullptr;

    if (CampaignArea* area = std::get<CampaignArea*>(m_entriesState.back().entry)) {
        if (m_entriesState.back().hoverIndex < 0 || m_entriesState.back().hoverIndex >= area->entries.size()) return nullptr;
        return &area->entries[m_entriesState.back().hoverIndex].scripts;
    }

    return nullptr;
}

unsigned int DisaCustomCampaignManager::getBGM() const {
    if (m_entriesState.empty()) return 0;
    return m_entriesState.back().getBGM();
}


void DisaCustomCampaignManager::loadCampaigns() {
    const std::string customCampaignsDirPath = DisaCustomBaseModManager::get()->getCustomCampaignsDir().getDir();

    PG_Dir customCampaignsDir(customCampaignsDirPath);
    for (const auto& zipFile : customCampaignsDir.findFiles([](const PG_File& file)->bool {  return file.getFileExtensionFormated() == "zip"; }, 1)) {
        auto archive = PG_ZIPUtil::openZip(zipFile);
        if (!archive) continue;

        PG_DisaModLoadInfo info;
        info.file = zipFile;
        try {
            info.json = PG_ZIPUtil::readJson(archive, "campaign.json");
            if (info.json.empty()) {
                OUT_ERROR("Failed to parse 'campaign.json' inside zip '" << zipFile << "'!");
                continue;
            }

            if (!readFromJsonMandatory(info.json, "id", info.id)) {
                OUT_ERROR("'campaign.json' has no 'id' inside zip '" << zipFile << "'!");
                continue;
            }
        }
        catch (const std::exception& ex) {
            OUT_ERROR("Failed to parse 'campaign.json' inside zip '" << zipFile << "'! Reason: " << ex.what());
            continue;
        }
        catch (...) {
            OUT_ERROR("Failed to parse 'campaign.json' inside zip '" << zipFile << "'!");
            continue;
        }
      
        m_campaigns.push_back({});
        try {
            readCampaign(m_campaigns.back(), info);
            m_scriptsModel->loadScripts(&m_campaigns.back(), info);
        }
        catch (const std::exception& ex) {
            m_campaigns.pop_back();
            OUT_ERROR("Failed to parse 'campaign.json' inside zip '" << info.file << "'! Reason: " << ex.what());
        }
        catch (...) {
            m_campaigns.pop_back();
            OUT_ERROR("Failed to parse 'campaign.json' inside zip '" << info.file << "'!");
        }
    }
    { //remove dubliucate ids
        std::ranges::sort(m_campaigns, {}, &Campaign::id);
        auto range = std::ranges::unique(m_campaigns);
        m_campaigns.erase(range.begin(), range.end());
    }
    { //sort by load order
        PG_DisaModLoadOrderListModel orderModel;
        orderModel.open(customCampaignsDir.toFilePath("loadOrder.json"));
        auto mapping = orderModel.getOrderMapping();
        std::ranges::sort(m_campaigns, {}, [&mapping](const Campaign& c) {
                auto findIt = mapping.find(c.id);
                return findIt != mapping.end() ? findIt->second : std::numeric_limits<std::size_t>::max();
            });
        std::erase_if(m_campaigns, [&mapping, &orderModel](const Campaign& c) {
            auto findIt = mapping.find(c.id);
            return findIt != mapping.end() ? orderModel.get(findIt->second).disabled : false;
        });
    }

    OUT_INFO("Successfully loaded "<< m_campaigns.size() << " campaigns!");
}

bool DisaCustomCampaignManager::process() {
	if (!isActive()) return false;
	processUserEvents();
    if (!isActive()) return false;
	processUserRender();
	return true;
}

void DisaCustomCampaignManager::activate() {
    m_entriesState = {};

    EntriesState state;
    state.entry = this;
    state.type = CampaignList;
    state.size = m_campaigns.size();
    //compute max width
    state.width = 0;
    for (const auto& campain : m_campaigns) {
        state.width = std::max(state.width, FP::callFastFarA<int>(fGetWidthOfString, campain.name.data(), 0, -1, nullptr, -1));
    }
    state.width += 11;

    state.titleText = stringToShiftJis("Custom Area Select");
    state.infoText = stringToShiftJis("Select the custom campaign you want to go to");
    m_entriesState.push_back(std::move(state));
    m_selectEntry = m_entriesState.back().createChildEntry(m_entriesState.back().getIndex());
}

void DisaCustomCampaignManager::reactivate() {
    //empty
}

void DisaCustomCampaignManager::diactivate() {
    m_entriesState.clear();
    if (m_activeCampaign) m_activeCampaign->archive.reset();
}

static inline void startCutomMap() {
    DisaDungeon& dungeon = FP::getData<DisaDungeon>(0x00653a98);
    dungeon.difficulty = 0;
    dungeon.sx2 = 0;
    dungeon.id = 65501;
    dungeon.bonusRank = 15;
    dungeon.active = true;

    auto retUnused = FP::callFastFarA<int*>(0x004bd930, (int*)0xb, (int*)0x0);
}

void DisaCustomCampaignManager::processUserEvents() {
    EntriesState& entriesBox = m_entriesState.back();
    DisaSettings* settings = (DisaSettings*)(FP::getData<unsigned int>(0x005d7c40));

       const short numberOfDisplayEntries = (short)std::min(entriesBox.size, 10); //there can be a max of 10 dungeon entries
    {
        const int lastIndex = entriesBox.getIndex();
        const int lastHover = entriesBox.hoverIndex;

        FP::callFastFarA(0x0050fb90, 0x20, 0x30, entriesBox.width + 0xb, 0x10, numberOfDisplayEntries, &entriesBox.hoverIndex, '\0'); //hand cursor position
        FP::callFastFarA(0x0050fc10, 0x20, 0x30, entriesBox.width + 0xb, 0x10, numberOfDisplayEntries, &entriesBox.hoverIndex, entriesBox.scrollOffsetIndex, entriesBox.size);

        //play sound if selection changed
        const bool selectionHasChanged = lastHover != entriesBox.hoverIndex;
        if (selectionHasChanged) {
            FP::callFastFarA(fPlaySound, 3, FP::getData<int>(0x00677274), 0, 0, 0); //play sound
        }

        if (entriesBox.size <= 1) {
            //disable scrolling
            entriesBox.resetHover();
        }
        else {
            const int endScrollIndex = std::max(entriesBox.size - 10, 0); //we can not scroll past this index
            const byte selectionChangedByte = selectionHasChanged ? 2 : 0; //not sure why it needs to be a byte
            FP::callFastFarA(fHandleListScrolling, &entriesBox.hoverIndex, &entriesBox.scrollOffsetIndex, numberOfDisplayEntries, endScrollIndex, selectionChangedByte); //handle scrolling
        }

        //load entry into memory
        if (int newIndex = entriesBox.getIndex(); newIndex != lastIndex) {
            m_selectEntry = entriesBox.createChildEntry(newIndex);
        }
    }

    EMenuOption menuOption = EMenuOption::MenuOption_None;
    if (FP::callFastFarA<bool>(fIsExitPressed, 0)) {
        menuOption = EMenuOption::MenuOption_Exit;
    }else if (entriesBox.type == EMenuState::CampaignDungeonList) {
        menuOption = processMenuBox(settings, { MenuOption_Select, MenuOption_SkipStory, MenuOption_Exit });
    }
    else {
        menuOption = processMenuBox(settings, { MenuOption_SelectHidden, MenuOption_Exit });
    }

    //check if entry was clicked instead
    if (menuOption == EMenuOption::MenuOption_None &&
        ((FP::callFastFarA<bool>(fIsMouseCursorInAABB, 0x20, 0x30, entriesBox.width + 0x2b, (numberOfDisplayEntries + 3) * 16, '\0') && FP::callFastFarA<bool>(fIsLeftMouseBtnClicked, '\0')) ||
        (*(char*)(FP::getData<int>(0x005d7c70) + 0x305438) == '\0' && ((*(char*)(FP::getData<int>(0x005d7c70) + 0x30543a) != '\0' && ((FP::getData<int>(0x00701c58) & 0x2000000) != 0)))))) {
        menuOption = EMenuOption::MenuOption_Select;
    }
    

    switch (menuOption)
    {
    case MenuOption_Exit: {
        int soundEntry = FP::getData<int>(0x00677274);
        FP::callFastFarA<void>(fPlaySound, 7, soundEntry, 0, 0, 0);

        m_entriesState.pop_back();
        if (m_entriesState.empty()) {
            diactivate();
            return;
        }
        else {
            m_selectEntry = m_entriesState.back().createChildEntry(m_entriesState.back().getIndex());
        }
    }break;
    case MenuOption_SkipStory: {
        if (entriesBox.type == CampaignDungeonList) {
            filterPlayStory.set(&FP::getData<unsigned char>(aGlobalStateBits));//0x00619443
            startCutomMap();
        }
    }break;
    case MenuOption_SelectHidden:
    case MenuOption_Select: {
        int soundEntry = FP::getData<int>(0x00677274);
        int soundSomething = FP::getData<int>(0x005d7c70);
        FP::callFastFarA(fPlaySound, 1, soundEntry, 0, soundSomething, 0);


        if (entriesBox.type != CampaignDungeonList) {
            assert(m_selectEntry.type != None);
            if (m_selectEntry.type == None) return;
            m_entriesState.push_back(std::move(m_selectEntry));
            m_selectEntry = m_entriesState.back().createChildEntry(m_entriesState.back().getIndex());
        }
        else {
            filterPlayStory.clear(&FP::getData<unsigned char>(aGlobalStateBits));//0x00619443
            FP::getData<unsigned short>(0x0065db1a) = 0;
            startCutomMap();
        }
    } break;
    default:
        break;
    }
}


constexpr int renderFlag = 0xbef8; //not sure what this does
/**
* @brief Render text box
* @param x Coord from 0 to 480. Is center of box if centered is true
* @param y Coord from 0 to 272.
* @param width The with of box. -1 for auto, in that case the with will be calculated based on string size
*/
static inline void _cdecl renderSimpleTextBox(const std::string& str, unsigned int x, unsigned int y, int width = -1, bool centered = true) {
    unsigned int* fontAtlas = &FP::getData<unsigned int>(0x005ae3e0);
    FP::callCdeclFarA(fRenderTextBox, x, y, width, 1, renderFlag, fontAtlas, centered ? 99: 0, str.data());
   
}

static inline void renderInfoTextBox(const std::string& str) {
    FP::callFastFarA(0x004b3d40, str.data(), 0); //bottom info box
}

static inline void renderString(DisaSettings* settings, const std::string& str, unsigned int x, unsigned int y, int availableWidth = -1, int availableHeight = -1) {
    if (settings->field_0x21 != '\0') {
        //Render text new: x, y, strange bit mask??, ?, pointer to string , ?, ?, ?, ?, spacing (bool)
        //FP::callFastFarA(fRenderTextNew, x, y, 0xc7a7, &FP::getData<int>(0x005ad538), str.data(), 0.3f, 0.3f, availableWidth, availableHeight, settings->field_0x30);
        FUN_RederTextNew(x, y, 0xc7a7, &FP::getData<int>(0x005ad538), (unsigned short*)str.data(), 0.3f, 0.3f, availableWidth, availableHeight, settings->field_0x30);
    }
    else {
        //render text old
        FP::callFastFarA(fRenderTextOld, x, y, 0xc7a7, &FP::getData<int>(0x005ad538), str.data(), '\0', settings->field_0x30);
    }
}

static inline void renderCursor(DisaSettings* settings, float x, float y) {
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
    FP::callFastFarA(fRenderCutout, f, &FP::getData<glm::rgba>(0x005ad534), x, y, 0xc7a9, 16, 16, cutoutY, cutoutX, cutoutWidth, cutoutHeight);
}

static inline int getWidthOfString(const std::string& str) {
    return FP::callFastFarA<int>(fGetWidthOfString, str.data(), 0, -1, (int*)0x0, -1);
}

static inline void renderTextInBox(DisaSettings* settings, const std::string& text, unsigned int x, unsigned int y, int width, int height) {
    if (text.empty() || width < 1 || height < 1) return;

    auto splitText = splitShiftJisBySpace(text);

    constexpr int spaceWidth = 4;
    int yPos = y;
    std::string combinedString;
    int combinedWidth = 0;

    for (std::string str : splitText) {
        if ((yPos - y + 16) > height) return;
        if (str.empty()) {
            //is new line?
            if (!combinedString.empty()) {
                renderString(settings, combinedString, x, yPos, width, 16);
                yPos += 16;
                combinedString = std::move(str);
                combinedWidth = 0;
            }
            continue;
        }
        float strWidth = getWidthOfString(str);
        if (strWidth > width) {
            //string too long
            str = splitShiftJisClampToSize(str, 5) + "...";
            strWidth = getWidthOfString(str);
        }

        if (combinedWidth + spaceWidth + strWidth > width) {
            //start new line
            renderString(settings, combinedString, x, yPos, width, 16);
            yPos += 16;
            combinedString = std::move(str);
            combinedWidth = strWidth;
            continue;
        }
        if (!combinedString.empty()) {
            combinedString.push_back(0x81); //space
            combinedString.push_back(0x40);
        }
        combinedString += str;
        combinedWidth += spaceWidth + strWidth;
    }
    if (!combinedString.empty()) {
        renderString(settings, combinedString, x, yPos, width, 16);
    }

}

static inline PG_Texture getImageTexture(const std::string& id, const std::string& path, zip* archive) {
    if (path.empty()) return DisaCustomRenderManager::get()->getErrorTexture();
    PG_Texture tex;
    tex.load("T_DisaImg_" + id + "_" + path, [path, archive]() -> PG_Texture::ImageFormats {
        auto fullPath = "img/" + path;
        PG_StringUtil::removeAll(fullPath, "..");

        PG_RGBAImage img;
        try {
            img = PG_ZIPUtil::readPNGImage(archive, fullPath);
        }
        catch (const PG_RuntimeError& e) {
            OUT_ERROR("Failed to read png from zip '" << fullPath << "' with error: " << e.what());
        }
        catch (...) {
            OUT_ERROR("Failed to read png from zip '" << fullPath << "'!");
        }

        if (img.empty()) return {};
        return img;
        });
    return tex;
}

void DisaCustomCampaignManager::renderCustomImage(const std::string& id, const std::string& path) {
    if (id.empty()) return;

    auto* renderManager = DisaCustomRenderManager::get();
   
    PG_DisaModel disaModel(renderManager->getFullImageScreenModel());
    disaModel.additionalProperties["disaImage"] = PG_Any::from(getImageTexture(id, path, m_activeCampaign->archive.get()));

    renderManager->submit(49999.0f, std::move(disaModel), -1, nullptr, false, false, 0, 0, '\0', false);
}

void DisaCustomCampaignManager::renderCustomImage(const std::string& id, const std::string& path, const glm::vec2& pos, const glm::vec2& size) {
    if (id.empty()) return;

    auto* renderManager = DisaCustomRenderManager::get();

    PG_DisaModel disaModel(renderManager->getImageScreenModel());
    disaModel.additionalProperties["disaGuiTransProjection"] = PG_Any::from(glm::translate(glm::mat4(1.f), glm::vec3(pos, 0.f)));
    disaModel.additionalProperties["disaGuiScaleProjection"] = PG_Any::from(glm::scale(glm::mat4(1.f), glm::vec3(size, 1.f)));
    disaModel.additionalProperties["disaImage"] = PG_Any::from(getImageTexture(id, path, m_activeCampaign->archive.get()));

    renderManager->submit(49999.0f, std::move(disaModel), -1, nullptr, false, false, 0, 0, '\0', false);
}


void DisaCustomCampaignManager::processUserRender() {
    EntriesState& entriesBox = m_entriesState.back();

    //FP::callVariadicThisFarA(fRenderImg, &FP::getData<int>(0x007495d8), 0, 0, 0xbaa1, 0x1e0, 0x110, 2, 2, FP::getData<int>(0x00749626), FP::getData<int>(0x00749628));
    if (entriesBox.isCustomBackground) {
        renderCustomImage(m_activeCampaign->id, entriesBox.getBackgroundImagePath());
    }else{
        FP::callVariadicThisFarA(fRenderImg, &FP::getData<int>(0x007495d8), 0, 0, 0xbaa1, 0x1e0, 0x110, 2, 2, FP::getData<int>(0x00749626), FP::getData<int>(0x00749628));
    }

    DisaSettings* settings = (DisaSettings*)(FP::getData<unsigned int>(0x005d7c40));

    renderSimpleTextBox(entriesBox.titleText, 240, 12);

    if (isEmpty()) return;

    constexpr int maxEntries = 10;
    const int numberOfDungeonsOnScreen = std::min(maxEntries, (int)entriesBox.size);
    const int dungeonsBgBoxHeight = numberOfDungeonsOnScreen * 16 + 8;
    { //entires
        renderSimpleBgBox(32, 48, entriesBox.width, dungeonsBgBoxHeight);
        int yPos = 53;
        for (int i = entriesBox.scrollOffsetIndex; i < std::min((int)entriesBox.size, entriesBox.scrollOffsetIndex + maxEntries); ++i) {
            const std::string text = splitShiftJisClampToSize(entriesBox.getEntryName(i), 24) ;
            renderString(settings, text, 38, yPos + 2, entriesBox.width, dungeonsBgBoxHeight);
            yPos += 16;
        }
    }

    //up and down arrows
    FP::callFastFarA(0x004b4b10, 32, 48, entriesBox.width, dungeonsBgBoxHeight, entriesBox.scrollOffsetIndex, numberOfDungeonsOnScreen, entriesBox.size);

    //TODO preview image background
    constexpr int previewImgWidth = 220;
    //renderSimpleBgBox(448 - previewImgWidth, 48, previewImgWidth, 124);
    if (m_selectEntry.type == EMenuState::CampaignAreaList) {
        constexpr glm::ivec2 textBoxPos = glm::ivec2(448 - 144, 48);
        constexpr glm::ivec2 textBoxSize = glm::ivec2(144, 160);
        constexpr glm::ivec2 textBoxMargin= glm::ivec2(7);
        renderSimpleBgBox(textBoxPos.x, textBoxPos.y, textBoxSize.x, textBoxSize.y); //text bg
        renderTextInBox(settings, m_selectEntry.getDescription(), 
            textBoxPos.x + textBoxMargin.x, textBoxPos.y + textBoxMargin.y,
            textBoxSize.x - textBoxMargin.x * 2, textBoxSize.y - textBoxMargin.y * 2);

        constexpr glm::ivec2 imageBoxPos = glm::ivec2(177, 48);
        constexpr glm::ivec2 imageBoxSize = glm::ivec2(124, 70);
        constexpr glm::ivec2 imageBoxMargin = glm::ivec2(4);
        renderSimpleBgBox(imageBoxPos.x, imageBoxPos.y, imageBoxSize.x, imageBoxSize.y); //img bg
        renderCustomImage(m_activeCampaign->id, m_selectEntry.getPreviewImagePath(),
            glm::vec2(imageBoxPos) + glm::vec2(imageBoxMargin),
            glm::vec2(imageBoxSize) - glm::vec2(imageBoxMargin) * 2.f);
    }

    renderCursor(settings, 22.0f, float(entriesBox.hoverIndex * 16 + 53));
    if (entriesBox.type == EMenuState::CampaignDungeonList) {
        renderMenuBox(settings, { MenuOption_Select, MenuOption_SkipStory, MenuOption_Exit });
    }
    else {
        renderMenuBox(settings, { MenuOption_Exit });
    }
    renderInfoTextBox(entriesBox.infoText);
}


DisaCustomCampaignManager* DisaCustomCampaignManager::get() {
    static DisaCustomCampaignManager manager(DisaCustomBaseModManager::get()->getScriptsModel());
    return &manager;
}

const std::string& DisaCustomCampaignManager::EntriesState::getEntryName(int index) const {
    return std::visit([index](const auto* arg)->const std::string& {
        using T = std::decay_t<std::remove_const_t<std::remove_pointer_t<decltype(arg)>>>;
        if constexpr (std::is_same_v<T, DisaCustomCampaignManager>) {
            const Campaign* campain = &arg->m_campaigns[index];
            return campain->name;
        }
        else if constexpr (std::is_same_v<T, Campaign>) {
            const CampaignArea* area = &arg->entries[index];
            return area->name;
        }
        else if constexpr (std::is_same_v<T, CampaignArea>) {
            const CampaignDungeon* dungon = &arg->entries[index];
            return dungon->name;
        }
        else {
            std::cout << "Missing entry for " << typeid(T).name() << std::endl;
            static std::string missing = "missingNo.";
            return missing;
        }
        }, entry);
}

const std::string& DisaCustomCampaignManager::EntriesState::getDescription() const {
    return std::visit([](const auto* arg)->const std::string& {
        using T = std::decay_t<std::remove_const_t<std::remove_pointer_t<decltype(arg)>>>;

        if constexpr (std::is_same_v<T, Campaign>) {
            return arg->description;
        } else if constexpr (std::is_same_v<T, CampaignDungeon>) {
            return arg->description;
        }

        static std::string empty;
        return empty;
        }, entry);
}

const std::string& DisaCustomCampaignManager::EntriesState::getBackgroundImagePath() const {
    return std::visit([](const auto* arg)->const std::string& {
        using T = std::decay_t<std::remove_const_t<std::remove_pointer_t<decltype(arg)>>>;

        if constexpr (std::is_same_v<T, Campaign>) {
            if (arg->backgroundImage.empty()) return arg->previewImage;
            return arg->backgroundImage;
        }else if constexpr (std::is_same_v<T, CampaignArea>) {
            return arg->backgroundImage;
        }

        static std::string empty;
        return empty;
        }, entry);
}

const std::string& DisaCustomCampaignManager::EntriesState::getPreviewImagePath() const {
    return std::visit([](const auto* arg)->const std::string& {
        using T = std::decay_t<std::remove_const_t<std::remove_pointer_t<decltype(arg)>>>;

        if constexpr (std::is_same_v<T, Campaign>) {
            if (arg->previewImage.empty()) return arg->backgroundImage;
            return arg->previewImage;
        }

        static std::string empty;
        return empty;
        }, entry);
}

const std::string& DisaCustomCampaignManager::EntriesState::getAutor() const {
    return std::visit([](const auto* arg)->const std::string& {
        using T = std::decay_t<std::remove_const_t<std::remove_pointer_t<decltype(arg)>>>;

        if constexpr (std::is_same_v<T, Campaign>) {
            return arg->author;
        }
        static std::string empty;
        return empty;
        }, entry);
}

unsigned int DisaCustomCampaignManager::EntriesState::getBGM() const {
    return std::visit([this](const auto* arg)-> unsigned int {
        using T = std::decay_t<std::remove_const_t<std::remove_pointer_t<decltype(arg)>>>;
        if constexpr (std::is_same_v<T, CampaignArea>) {
            return hoverIndex < arg->entries.size()? arg->entries[hoverIndex].bgm: 0;
        }
        return 0;
        }, entry);
}

DisaCustomCampaignManager::EntriesState DisaCustomCampaignManager::EntriesState::createChildEntry(int index) {
    return std::visit([index](auto* arg)->DisaCustomCampaignManager::EntriesState {
        using T = std::decay_t<std::remove_const_t<std::remove_pointer_t<decltype(arg)>>>;
        DisaCustomCampaignManager::EntriesState state;
        if constexpr (std::is_same_v<T, DisaCustomCampaignManager>) {
            if (arg->m_activeCampaign) arg->m_activeCampaign->archive.reset();
            Campaign* campain = &arg->m_campaigns[index];
            campain->open();
            state.entry = campain;
            state.type = DisaCustomCampaignManager::CampaignAreaList;
            state.titleText = DisaCustomCampaignManager::CampaignAreaList;
            state.size = campain->entries.size();
            state.isCustomBackground = !campain->backgroundImage.empty();

            state.titleText = campain->name;
            state.infoText = stringToShiftJis("Select the campaign area you want to go to");
            computeMaxWidth(state.width, campain);
            arg->m_activeCampaign = campain;
        } else if constexpr (std::is_same_v<T, Campaign>) {
            CampaignArea* area = &arg->entries[index];
            state.entry = area;
            state.type = DisaCustomCampaignManager::CampaignDungeonList;
            state.titleText = DisaCustomCampaignManager::CampaignAreaList;
            state.size = area->entries.size();
            state.isCustomBackground = !area->backgroundImage.empty();

            state.titleText = area->name;
            state.infoText = stringToShiftJis("Select the dungon you want to go to");
            computeMaxWidth(state.width, area);
        }

        return state;
        }, entry);
}


static inline void readMapFile(int* dataLocation, const zipUPtr& archive, const std::vector<DisaCustomCampaignManager::EntriesState>& entriesState, auto&& strFunc) {
    if (!archive || entriesState.empty()) return;
    auto area = std::get_if<DisaCustomCampaignManager::CampaignArea*>(&entriesState.back().entry);
    if (!area) return;
    auto& dungeon = (*area)->entries[entriesState.back().getIndex()];

    auto copy = "dungeons/" + strFunc(dungeon);// .mpp;
    PG_StringUtil::removeAll(copy, "..");

    zip_file* zf = zip_fopen(archive.get(), copy.c_str(), 0);
    if (!zf) {
        OUT_ERROR("Failed to find '" << copy << "' in zip!");
        return;
    }


    char* target = (char*)dataLocation;
    zip_int64_t bytesRead;
    int i = 0;
    while ((bytesRead = zip_fread(zf, target, 1024)) > 0) {
        target += bytesRead;
        ++i;
        if (i > 1024 * 10) {
            OUT_ERROR("FATAIL ERROR: Injection file is too big!");
            zip_fclose(zf);
            exit(1);
            return;
        }
    }
    zip_fclose(zf);
}

void DisaCustomCampaignManager::readMPP(int* dataLocation, bool hdTextures) const {
    assert(m_activeCampaign);
    readMapFile(dataLocation, m_activeCampaign->archive, m_entriesState, [hdTextures](const DisaCustomCampaignManager::CampaignDungeon& d) {
        if (hdTextures && !d.mppHD.empty()) return d.mppHD;
        return d.mpp; 
        });
}

void DisaCustomCampaignManager::readMPD(int* dataLocation) const {
    assert(m_activeCampaign);
    readMapFile(dataLocation, m_activeCampaign->archive, m_entriesState, [](const DisaCustomCampaignManager::CampaignDungeon& d) { return d.mpd; });
}



void DisaCustomCampaignManager::runTestMap() {
    FP::getData<unsigned short>(0x0065db1a) = 0;
    DisaDungeon& dungeon = FP::getData<DisaDungeon>(0x00653a98);
    dungeon.difficulty = 0;
    dungeon.sx2 = 0;
    dungeon.id = 65500;
    dungeon.bonusRank = 15;
    dungeon.active = true;

    auto retUnused = FP::callFastFarA<int*>(0x004bd930, (int*)0xb, (int*)0x0);
}

void DisaCustomCampaignManager::readMPPTestMap(int* dataLocation, bool hdTextures) const {

    std::string path = DisaCustomBaseModManager::get()->getCustomCampaignsDir().toFilePath(hdTextures ? "new_debugMap.MPP" : "debugMap.MPP");

    PG::STREAM::InByteFile reader(path);
    if (!reader.isOpen() || reader.size() < 10) {
        OUT_ERROR("Failed to find test map file at: \"" << path << "\"! We will now crash!");
        exit(1);
    }
    unsigned int size = reader.size();
    reader.read((char*)dataLocation, size);
}

void DisaCustomCampaignManager::readMPDTestMap(int* dataLocation) const {
    //test map
    std::string path = DisaCustomBaseModManager::get()->getCustomCampaignsDir().toFilePath("debugMap.mpd");

    PG::STREAM::InByteFile reader(path);
    if (!reader.isOpen()) {
        OUT_ERROR("Failed to find test map file at: \"" << path << "\"! We will now crash!");
        exit(1);
    }
    unsigned int size = reader.size();
    reader.read((char*)dataLocation, size);
}