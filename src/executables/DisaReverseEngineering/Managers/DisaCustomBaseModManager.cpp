#include "DisaCustomBaseModManager.h"
#include "DisaCustomCampaignManager.h"

#include <ranges>
#include <algorithm> 
#include <array> 
#include <windows.h>

#include <Stream/PG_StreamInByteFile.h>
#include <Disa/PG_DisaTalkCompiler.h>
#include <Scene/Gui/Validator/PG_GuiTextValidators.h>
#include <Util/PG_StringUtil.h>
#include <Util/Debug/PG_DebugOut.h>
#include <DisaFunctions.h>
#include "FunctionPatcher.h"

static inline std::string getExecutablePath() {
    char path[MAX_PATH];
    // Get the handle of the executable that loaded this DLL (pass NULL)
    GetModuleFileName(NULL, path, MAX_PATH);
    return std::string(path);
}

std::string DisaCustomBaseModManager::getPath() {
#ifdef _DEBUG
    const char* dirPath = "D:/Steam/SteamApps/common/Disgaea PC/";
    return dirPath;
#else
    static std::string filePath;
    if (filePath.empty()) {
        filePath = PG_File(getExecutablePath()).getDir();
        if (filePath.empty() || filePath.size() < 4 || filePath.size() > 3400) exit(1);
        if (filePath.back() != '\\') filePath.push_back('\\');
        OUT_INFO("Found path: " << filePath);
    }
    return filePath.c_str();
#endif // _DEBUG
}

DisaCustomBaseModManager* DisaCustomBaseModManager::get() {
    static DisaCustomBaseModManager manager;
    return &manager;
}

/**
* @brief Keep all custom scrips stored in a rolling buffer
* Reason, I have no idea right now how to clear a scrips once it finished executing
*/
struct RollingScriptMemory {
    std::array<std::vector<unsigned char>, 64> scripts;
    unsigned int roll = 0;

    void next() {
        roll = (roll + 1) % scripts.size();
    }

    std::vector<unsigned char>& current() { return scripts[roll]; }
};

static RollingScriptMemory gRollingScriptMemory;


template<typename T>
std::string getScriptName(const std::string* scriptIdentifier, const DisaCustomCampaignManager::TalkScripts* scripts, T valuePtr, const std::string& defaultScriptName) {
    if (!scriptIdentifier || scriptIdentifier->empty()) return defaultScriptName;
    return (scripts && !(scripts->*valuePtr).empty()) ? (*scriptIdentifier + "::" + (scripts->*valuePtr)) : defaultScriptName;
}

int DisaCustomBaseModManager::readTalkScript(unsigned char*& dataLocation, unsigned int scriptId, unsigned int dungeonId, unsigned int stateFlag) {
    if (dungeonId / 100 >= 655) {
        auto [scriptIdentifier, scripts] = DisaCustomCampaignManager::get()->getActiveTalkScripts();

        unsigned char* globalStateIt = &FP::getData<unsigned char>(aGlobalStateBits);

        switch (stateFlag)
        {
        case 1: {
            return readTalkScript(dataLocation, getScriptName(scriptIdentifier, scripts,
                filterPlayStory.isSet(globalStateIt) ? &DisaCustomCampaignManager::TalkScripts::intro : &DisaCustomCampaignManager::TalkScripts::introSkipStory,
                "genericDungeonIntro"));
        }break;
        case 2: {
            return readTalkScript(dataLocation, getScriptName(scriptIdentifier, scripts,
                filterPlayStory.isSet(globalStateIt) ? &DisaCustomCampaignManager::TalkScripts::exit : &DisaCustomCampaignManager::TalkScripts::exitSkipStory,
                "genericDungeonExit"));
        }break;
        default: return -1;
        }
    }

    std::string functionName = "Disa::" + std::to_string(scriptId) + "_" + std::to_string(dungeonId) + "_" + std::to_string(stateFlag);
    return readTalkScript(dataLocation, functionName);
}




static inline std::pair<std::string, std::string> splitScriptName(const std::string& scriptNameStr) {
    auto split = PG_StringUtil::split(scriptNameStr, "::");
    std::string scriptNamespace;
    std::string scriptName;
    if (split.size() == 1) {
        scriptNamespace = "Base";
        scriptName = std::move(split.front());
    }
    else if (split.size() == 2) {
        scriptNamespace = std::move(split.front());
        scriptName = std::move(split.back());

        PG_SimpleWordValidator()(scriptNamespace);
    }
    else {
        return { std::move(scriptNamespace), std::move(scriptName) };
    }
    PG_SimpleWordValidator()(scriptName);
    return { std::move(scriptNamespace), std::move(scriptName) };
}

int DisaCustomBaseModManager::readTalkScript(unsigned char*& dataLocation, const std::string& scriptNameStr) {
    if (scriptNameStr.empty()) {
        dataLocation = nullptr;
        return 0;
    }

    auto [scriptNamespace, scriptName] = splitScriptName(scriptNameStr);

    if (scriptNamespace.empty() || scriptName.empty()) {
        dataLocation = nullptr;
        return 0;
    }


    std::string code;
    PG_File scriptFile;

    {
        scriptFile = getPath() + "mod/CustomTalkScript/" + scriptNamespace + "/" + scriptName + ".tscript";

        if (PG_StringUtil::isEqualCaseless(scriptNamespace, "disa") && !scriptFile.exists()) {
            OUT_INFO("No replacement talk script found at: " << scriptFile);
            return 0;
        }


        if (PG::STREAM::InByteFile streamFile(scriptFile); streamFile.isOpen()) {
            code.resize(streamFile.size());
            streamFile.read(code.data(), code.size());
        }
    }

    if (code.empty()) {
        scriptFile = "scripts/" + scriptName + ".tscript";
        code = DisaCustomCampaignManager::get()->readScript(scriptNamespace, scriptName, true);
    }


    if (code.size() < 3) {
        OUT_ERROR("Failed to read or find talk script at: " << scriptFile);
        dataLocation = nullptr;
        return 0;
    }

    std::vector<unsigned char> buffer;
    try
    {
        buffer = PG_DisaTalkScript::compile(code);
    }
    catch (const PG_CompileException& ex)
    {
        dataLocation = 0;
        OUT_ERROR("Failed to compile talk script: \"" << scriptFile << "\"! Reason: " << ex.what());
        return 0;
    }

    gRollingScriptMemory.next();
    gRollingScriptMemory.current() = std::move(buffer);
    dataLocation = gRollingScriptMemory.current().data();
    return 9001;
}


void DisaCustomBaseModManager::readScript(unsigned int*& dataLocation, int scriptId) {
    std::string functionName;
    if (scriptId == 9000001) {
        functionName = "genericDungeonIntro";
    }
    else {
        functionName = "Disa::" + std::to_string(scriptId);
    }

    readScript(dataLocation, functionName);

}

void DisaCustomBaseModManager::readScript(unsigned int*& dataLocation, const std::string& function) {
    if (function.empty()) return;
    auto split = PG_StringUtil::split(function, "::");
    if (split.empty() || split.size() > 2) return;

    auto [scriptNamespace, scriptName] = splitScriptName(function);

    if (scriptNamespace.empty() || scriptName.empty()) {
        OUT_ERROR("Invalid function syntax: " << function);
        dataLocation = nullptr;
        return;
    }


    std::string scriptHex;
    PG_File scriptFile;

    {
        scriptFile = (getPath() + "mod/CustomScript/" + scriptNamespace + "/" + scriptName + ".script");

        if (PG_StringUtil::isEqualCaseless(scriptNamespace, "disa") && !scriptFile.exists()) {
            OUT_INFO("No replacement script found at: " << scriptFile);
            return;
        }

        if (PG::STREAM::InByteFile streamFile(scriptFile); streamFile.isOpen()) {
            scriptHex.resize(streamFile.size());
            streamFile.read(scriptHex.data(), scriptHex.size());
        }
    }

    if (scriptHex.empty()) {
        scriptFile = "script/" + scriptName + ".script";
        scriptHex = DisaCustomCampaignManager::get()->readScript(scriptNamespace, scriptName, false);
    }

    if (scriptHex.empty()) {
        OUT_ERROR("Failed to read or find script at: " << scriptFile);
        dataLocation = nullptr;
        return;
    }

    gRollingScriptMemory.next();
    gRollingScriptMemory.current() = PG_StringUtil::hexToData(scriptHex);
    dataLocation = (unsigned int*)gRollingScriptMemory.current().data();
}