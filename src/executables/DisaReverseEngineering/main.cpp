
#include <FunctionPatcher.h>
#include <Managers/DisaCustomCampaignManager.h>
#include <Managers/DisaCustomBaseModManager.h>
#include <Managers/DisaCustomCharacterManager.h>
#include <DisaFunctions.h>
#include <DisaTalkScriptFunctions.h>
#include <DisaScriptFunctions.h>
#include <DisaCharacterFunctions.h>
#include <DisaDataFunctions.h>
#include <Windows.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_StringUtil.h>
#include <OS/Functions/Clipboard.h>
#include <Base/PG_Clipboard.h>
#include <Resources/PG_ResourceManager.h>
#undef max
#undef min

#include <iostream>
#include <fstream>
#include <thread> 
#include <sstream>
#include <vector>
#include <ranges>
#include <mutex>

using FUN_glViewportF = void(_stdcall*)(int, int, int, int);
static FUN_glViewportF FUN_glViewportPointer;


using FUN_glDrawArraysF = void(_stdcall*)(unsigned int, int, int);
static FUN_glDrawArraysF FUN_glDrawArraysPointer;


struct HockManager {
    struct Entry {
        enum Type {
            Replace,
            ReplaceFunctionIndirectCall
        };
        Type type;
        std::string name;
        std::string description;
        unsigned int originalCallAdress;
        unsigned int overrideFunctionAdress;
        unsigned int originalFunctionAdress = 0;
        bool replaced = false;
    };

    std::vector<Entry> entries;

    void init() {
        
        replaceFunction("LoadDungeonsCategories", "", 0x0047108a, FUN_LoadDungeonsCategories);
        replaceFunction("LoadDungeonsCategories", "", 0x004710f6, FUN_LoadDungeonsCategories);

        replaceFunction("ProccessCategoryDungeonsScreen", "", 0x0048c74b, FUN_ProccessCategoryDungeonsScreen);
        replaceFunction("ProccessDungeonsScreen", "", 0x0048c741, FUN_ProccessDungeonsScreen);

        replaceFunction("LoadMPP", "", 0x004b6eea, FUN_004a59c0_LoadMPP); //load dungeon assets
        replaceFunction("LoadMPD", "", 0x004b6ecb, FUN_004d5060_LoadMPD);

        //hdTexture
        replaceFunction("LoadMPP", "", 0x004816b4, FUN_004a59c0_LoadMPP); //load dungeon assets
        replaceFunction("LoadMPD", "", 0x004816c3, FUN_004d5060_LoadMPD);

        replaceFunction("LoadMPP", "", 0x004816ef, FUN_004a59c0_LoadMPP); //load dungeon assets
        replaceFunction("LoadMPD", "", 0x0048170a, FUN_004d5060_LoadMPD);

        //status menu
        replaceFunction("statusMenu", "", 0x004803D1, FUN_0047f080_ProccessStatusMenu);

        replaceFunction("renderPortrait", "Renders the face (Portrait) of a character", 0x004a7455, FUN_004b5490_NoneStd); //render portrait
        replaceFunction("renderPortrait", "Renders the face (Portrait) of a character", 0x004a6f91, FUN_004b5490_NoneStd); //render portrait
        replaceFunction("renderPortrait", "Renders the face (Portrait) of a character", 0x004a7573, FUN_004b5490_NoneStd); //render portrait
        replaceFunction("renderPortrait", "Renders the face (Portrait) of a character", 0x004a7f85, FUN_004b5490_NoneStd); //render portrait
        replaceFunction("renderPortrait", "Renders the face (Portrait) of a character", 0x004a94a3, FUN_004b5490_NoneStd); //render portrait
        replaceFunction("renderPortrait", "Renders the face (Portrait) of a character", 0x004aac77, FUN_004b5490_NoneStd); //render portrait

        replaceFunction("assemblyMenu", "The dark assembly menu", 0x004a0d72, FUN_0048dfd0_ProccessAssemblyMenu);
        replaceFunction("assemblyMenu", "The dark assembly menu", 0x0049f06c, FUN_0048cd40_BuildAssemblyUnitInfosLookup);
        replaceFunction("assemblyMenu", "The dark assembly menu", 0x004a0bfc, FUN_0048cd40_BuildAssemblyUnitInfosLookup);
        replaceFunction("enemyUnitKilled", "Records history when enemy unit killed", 0x004764ea, FUN_004d63b0_EnemyUnitKilled);


        replaceFunction("getSpriteData", "Finds loaded sprite data with given sprite id", 0x0046b262, FUN_004de880_getSpriteData);
        replaceFunction("getSpriteData", "Finds loaded sprite data with given sprite id", 0x0048e461, FUN_004de880_getSpriteData);
        replaceFunction("getSpriteData", "Finds loaded sprite data with given sprite id", 0x00490875, FUN_004de880_getSpriteData);
        replaceFunction("getSpriteData", "Finds loaded sprite data with given sprite id", 0x004d53c1, FUN_004de880_getSpriteData);
        replaceFunction("getSpriteData", "Finds loaded sprite data with given sprite id", 0x004d5e87, FUN_004de880_getSpriteData);
        replaceFunction("getSpriteData", "Finds loaded sprite data with given sprite id", 0x004de915, FUN_004de880_getSpriteData);

        replaceFunction("afterCharDataLoad", "Used to init custom characters", 0x0048afc7, FUN_004cf3c0);

        replaceFunction("save", "Create new save file", 0x0043f6a7, FUN_00442260_Save);
        replaceFunction("loadSave", "Load save file", 0x00442495, FUN_0043e600_decompressSave_NoneStd); //Is run to decompress a save file, usualy after load
        replaceFunction("loadSave", "Load save file", 0x0043f8a9, FUN_004422c0_LoadSave);

        {
            auto f = &DisaTScriptManager::FUN_004bf7e0_RunTalkScript; //find talk Dat
            unsigned int i = *((unsigned int*)&f);
            replaceFunction("RunTalkScript", "", 0x004b6b93, i);
            replaceFunction("RunTalkScript", "", 0x00463f7a, i);
        }

        replaceFunction("RunScript", "", 0x004c0286, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004b6d41, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004b6c85, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004b6ccb, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004b6dbf, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004b6d70, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004b6d9e, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004680c1, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004fbbeb, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x00464608, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004734a2, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004741ef, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x0048c4c6, FUN_004bd930_RunScript); //gameloop
        replaceFunction("RunScript", "", 0x00496251, FUN_004bd930_RunScript);
        //replaceFunction("RunScript", "", 0x00496a1b, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x0049efd6, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x0049ff33, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004b4abd, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004c305d, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004c6f15, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004c93be, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004c957e, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004c965a, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004ca778, FUN_004bd930_RunScript);
        replaceFunction("RunScript", "", 0x004cb2fd, FUN_004bd930_RunScript);
       
        replaceFunction("ScriptLoop", "", 0x0043cd28, FUN_004f7e70_ScriptLoop);
        replaceFunction("ScriptLoop", "", 0x00489ad1, FUN_004f7e70_ScriptLoop);
       
        //patch_FUN_004bd814_OrgAddress = FP::replaceFunction(0x004bd814, FUN_004b6420);
        replaceFunction("TalkScriptExec", "", 0x004bdef1, FUN_004bfa60_TalkScriptExec);

        //map end
        replaceFunction("MapEnd", "", 0x00458558, FUN_00446e20);
        replaceFunction("MapEnd", "", 0x004660b2, FUN_00446e20);
        replaceFunction("MapEnd", "", 0x0045da28, FUN_00446e20);

        //gameloop map exit
        replaceFunction("MapEndGameloop", "", 0x0048c71c, FUN_004710d0);

        FUN_glViewportPointer = &FUN_glViewport;
        FUN_glDrawArraysPointer = &FUN_glDrawArrays;

        replaceFunctionIndirectCall("glViewportPointer", "", 0x00420396, FUN_glViewportPointer); //render begin
        replaceFunctionIndirectCall("glDrawArraysPointer", "", 0x0042111c, FUN_glDrawArraysPointer); //render begin
        replaceFunction("RenderEnd", "", 0x00509d44, FUN_RenderEnd); //render begin

        /*
        {
            auto f = &DisaObjectRenderContext::FUN_004e2180; 
            unsigned int i = *((unsigned int*)&f);
            replaceFunction("TEST", "", 0x004E1E08, i);
            replaceFunction("TEST", "", 0x00508ba5, FUN_004201d0_RenderClearBegin);
        }
        */
        
        replaceDefaultCase();
    }

    template<typename T>
    void replaceFunction(std::string&& name, std::string&& description, unsigned int originalFuncAddress, T&& replacementFunction) {
        replaceFunction(std::move(name), std::move(description), originalFuncAddress, reinterpret_cast<unsigned int>(std::addressof(replacementFunction)));
    }

    void replaceFunction(std::string&& name, std::string&& description, unsigned int originalFuncAddress, unsigned int replacementFunction) {
        entries.emplace_back(
            Entry::Replace,
            std::move(name),
            std::move(description),
            originalFuncAddress,
            replacementFunction
            );
    }

    template<typename T>
    void replaceFunctionIndirectCall(std::string&& name, std::string&& description, unsigned int originalFuncAddress, T&& replacementFunction) {
        replaceFunctionIndirectCall(std::move(name), std::move(description), originalFuncAddress, reinterpret_cast<unsigned int>(std::addressof(replacementFunction)));
    }

    void replaceFunctionIndirectCall(std::string&& name, std::string&& description, unsigned int originalFuncAddress, unsigned int replacementFunction) {
        entries.emplace_back(
            Entry::ReplaceFunctionIndirectCall,
            std::move(name),
            std::move(description),
            originalFuncAddress,
            replacementFunction
        );
    }


    void hook(const std::string& name = "") {

        for (auto& entry : entries) {
            if (entry.replaced) continue;
            if (!name.empty() && !PG_StringUtil::isEqualCaseless(entry.name, name)) continue;
            OUT_INFO("Replacing function call \"" << entry.name << "\" at 0x"<< entry.originalCallAdress<< ".");
            switch (entry.type)
            {
            case Entry::Replace: {
                entry.originalFunctionAdress = FP::replaceFunction(entry.originalCallAdress, entry.overrideFunctionAdress);
            }break;
            case Entry::ReplaceFunctionIndirectCall: {
                entry.originalFunctionAdress = FP::replaceFunctionIndirectCall(entry.originalCallAdress, entry.overrideFunctionAdress);
            }break;
            default:
                break;
            }
            entry.replaced = true;
        }
        OUT_INFO("Successfully injected custom code!");
    }

    void unhook(const std::string& name = "") {
        for (auto& entry : entries) {
            if (!entry.replaced) continue;
            if (!name.empty() && !PG_StringUtil::isEqualCaseless(entry.name, name)) continue;
            OUT_INFO("Replacing function call \"" << entry.name << "\" back to original at 0x" << entry.originalCallAdress << ".");
            switch (entry.type)
            {
            case Entry::Replace: {
                FP::replaceFunction(entry.originalCallAdress, entry.originalFunctionAdress);
            }break;
            case Entry::ReplaceFunctionIndirectCall: {
               FP::replaceFunctionIndirectCall(entry.originalCallAdress, entry.originalFunctionAdress);
            }break;
            default:
                break;
            }
            entry.replaced = false;
        }
        OUT_INFO("Successfully unloaded custom code!");
    }

    void info() {
        for (auto& entry : entries) {
            OUT_INFO("Registered replacement \"" << entry.name << "\" at 0x" << entry.originalCallAdress << ".");
        }
    }

};

HockManager gHockManager;

void hook() {
    gHockManager.hook();   
}

void hook(const std::string& name) {
    gHockManager.hook(name);
}

void unhook() {
    gHockManager.unhook();
}

void unhook(const std::string& name) {
    gHockManager.unhook(name);
}

void playTest() {
    DisaCustomCampaignManager::get()->runTestMap();
}

struct CommandTerminal {

    void openConsole() {
        if (coutBuffOld) return;
     
        AllocConsole();
        coutBuffOld = std::cout.rdbuf(); // save pointer to std::cout buffer

//#ifndef _DEBUG
        std::cout.rdbuf(std::cout.rdbuf());
        std::cin.rdbuf(std::cin.rdbuf());
        std::cerr.rdbuf(std::cerr.rdbuf());
        std::cout.flush();
//#endif // !_DEBUG

        FILE* pCout;
        freopen_s(&pCout, "CONIN$", "r", stdin);
        freopen_s(&pCout, "CONOUT$", "w", stdout);
        freopen_s(&pCout, "CONOUT$", "w", stderr);

        std::ios::sync_with_stdio(true);

        OUT_INFO("Console opened! Build Date "<< __DATE__<<".");
        auto path = DisaCustomBaseModManager::getPath();
        OUT_INFO("Current working path: "<< path);

    }


    void closeConsole() {
        if (!coutBuffOld) return;

        std::cout.rdbuf(coutBuffOld);
        coutBuffOld = nullptr;
        // Close the console
        FreeConsole();
    }

    void toggleConsole() {
        isConsoleOpen = !isConsoleOpen;
    }

    void sendMsg(std::string msg) {
        auto split = PG_StringUtil::splitOnce(msg, ':');
        if (split.empty()) return;
        msg = std::move(split.front());
        std::lock_guard g(mutex);
        if (msg == "e" || msg == "exit") {
            isRunning = false;
        }
        else if (msg == "h" || msg == "hook") {
            if (split.size() == 2) {
                hook(split[1]);
            }
            else {
                hook();
            }
        }
        else if (msg == "u" || msg == "unhook") {
            if (split.size() == 2) {
                unhook(split[1]);
            }
            else {
                unhook();
            }
        }
        else if (msg == "p" || msg == "playtest") {
            playTest();
        }
        else if (msg == "i" || msg == "info") {
            gHockManager.info();
        }
        else if (msg == "ps" || msg == "printScript") {
            DisaCustomBaseModManager::get()->setIsPrintScript(!DisaCustomBaseModManager::get()->isPrintScript());
            std::cout << "Printing script: " << DisaCustomBaseModManager::get()->isPrintScript() << std::endl;
        }
        else if (msg == "pp" || msg == "printParty") {
            if (split.size() == 2 && (split.back() == "j" || split.back() == "json")) {
                OS::Clipboard::textToClipboard(DisaPartyCharacterInfo::playerPartyToJson());
                std::cout << "Written player party into clipboard!" << std::endl;
            }
            else {
                std::cout << DisaPartyCharacterInfo::playerPartyToString() << std::endl;
            }
        }
        else if (msg == "pc" || msg == "printCharacters") {
            std::cout << DisaDataCharacterInfo::globalToString() << std::endl;
        }
        else if (msg == "pm" || msg == "printMagic") {
            std::cout << DisaDataMagicInfo::globalToString() << std::endl;
        }
        else if (msg == "ac" || msg == "addCharacter") {
            if (split.size() != 2) {
                std::cout << "Error: addCharacter needs arguments!" << std::endl;
                return;
            }
          
            auto args = PG_StringUtil::split(split.back(), ",");
            if (args.size() < 1) {
                std::cout << "Error: addCharacter needs at least an identifier!" << std::endl;
                return;
            }

            PG_DisaIdentifier identifier(args.front());
            if (!identifier.isValid()) {
                std::cout << "Error: Invalid identifier!" << std::endl;
                return;
            }

            DisaCustomCharacterManager::get()->addCharacterToPary(identifier, 
                args.size() > 1 ? std::max(1, PG_StringUtil::toInt(args[1])) : 1,
                args.size() > 2 ? glm::clamp(-1,128, PG_StringUtil::toInt(args[2])): -1
            );
        }
        else if (msg == "help") {
            std::cout << "* hook:name (h:name) - Inject custom code. Optional second argument is a target name." << std::endl;
            std::cout << "* unhook:name (u:name) - Remove injected custom code. Optional second argument is a target name." << std::endl;
            std::cout << "* info (i) - Info about all registerd functions." << std::endl;
            std::cout << "* playtest (p) - Run playtest map from 'customCampainsTest'." << std::endl;
            std::cout << "* printScript (ps) - Will print scripts into console when they are executed." << std::endl;
            std::cout << "* printParty:mode (pp:mode) - Will print the current party as info (default) or dump a full json into clipboard (pp:json)." << std::endl;
            std::cout << "* printCharacters (pc) - Will print the current loaded character data." << std::endl;
            std::cout << "* printMagic (pm) - Will print the current loaded magic/skills data." << std::endl;
            std::cout << "* addCharacter:identifier,level,mentorIndex (ac:id,l,m) - Add a new character to your party. Example to add Etna with level 42: 'ac:Disa::2,42'. Level and mentor are optional." << std::endl;
            std::cout << "* exit (e) - Close injection thread" << std::endl;
        }
        else {
            std::cout << "'" << msg << "'" << " not a valid input! Enter 'help' for instructions!" << std::endl;
        }
    }

    void run() {
        while (isRunning) {
            if (isConsoleOpen) {
                openConsole();
            }
            else {
                closeConsole();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            std::string input;
            std::cin >> input;

            sendMsg(input);
        }
    }

    bool isRunning = true;
    std::atomic_bool isConsoleOpen = false;
    std::streambuf* coutBuffOld = nullptr;
    std::mutex mutex;
};


CommandTerminal commandTerminal;
UINT g_playTestMessage;
WNDPROC g_originalWndProc = 0;

// Window procedure to handle messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == g_playTestMessage) {
        commandTerminal.sendMsg("p");
        return 0;
    }

    if (uMsg == WM_KEYDOWN && wParam == VK_F2) {
        commandTerminal.toggleConsole();
    }

    if (g_originalWndProc) {
        return CallWindowProc(g_originalWndProc, hwnd, uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/**
* Build settings:
* Build Events -> Post-Build Events -> copy /Y "$(TargetDir)$(ProjectName).dll" "D:\Steam\SteamApps\common\Disgaea PC\j.dll"
* (Only Debug) C++ -> Code Generation -> Basic Runtime Check -> /RTCu
* Debugging -> Command -> D:/Steam/SteamApps/common/Disgaea PC/dis1_st.exe
*/
DWORD WINAPI startMain() {
#ifdef _DEBUG
    commandTerminal.openConsole();
#endif // _DEBUG

    FP::init();
    if (!FP::get()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        commandTerminal.closeConsole();
        return 0;
    }

    PG_ResourceManager::create();
    gHockManager.init();

    g_playTestMessage = RegisterWindowMessage(TEXT("START_PLAY_TEST"));
    if (g_playTestMessage == 0) {
        std::cout << "Failed to register message!" << std::endl;
    }
    else {
        HWND hwnd = FindWindow(NULL, TEXT("DISGAEA"));
        if (hwnd == NULL) {
            std::cerr << "Error: FindWindow failed with error code " << GetLastError() << "!" << std::endl;
        }
        else {
            g_originalWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WindowProc);
        }
    }

    hook();
    DisaCustomCampaignManager::get()->loadCampaigns();


    commandTerminal.run();

    std::cout << "Exit!" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    commandTerminal.closeConsole();

    return 0;

}

bool IsVCppRedistInstalled()
{
    HKEY hKey;
    // Visual Studio 2015–2022 Redistributable (x86)
    const wchar_t* subkey = L"SOFTWARE\\Microsoft\\VisualStudio\\14.0\\VC\\Runtimes\\x86";

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, subkey, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return false;

    DWORD installed = 0, size = sizeof(installed);
    RegQueryValueExW(hKey, L"Installed", nullptr, nullptr, reinterpret_cast<LPBYTE>(&installed), &size);
    RegCloseKey(hKey);

    return installed == 1;
}


BOOL APIENTRY DllMain(HINSTANCE, DWORD  reason, LPVOID) {
    /*
    if (!IsVCppRedistInstalled()) {
        MessageBoxW(nullptr,
            L"Microsoft Visual C++ Redistributable (2015–2022) X86 (32bit) is not installed!\n\n"
            L"Please download and install it from microsoft!",
            L"Missing Runtime",
            MB_ICONERROR | MB_OK);
        return 1;
    }
    */

    if (reason == DLL_PROCESS_ATTACH) {
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)startMain, NULL, 0, NULL);
    }

    return TRUE;
}