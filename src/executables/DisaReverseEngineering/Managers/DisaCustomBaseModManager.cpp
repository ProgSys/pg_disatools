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
#include <Managers/DisaCustomScriptManager.h>
#include <Managers/DisaCustomTranslationManager.h>

DisaCustomBaseModManager::DisaCustomBaseModManager():
    PG_DisaMainBaseModel(), m_scriptsModel(new DisaCustomScriptManager()), m_translationModel(new DisaCustomTranslationManager()){}

static inline std::string getExecutablePath() {
    char path[MAX_PATH];
    // Get the handle of the executable that loaded this DLL (pass NULL)
    GetModuleFileName(NULL, path, MAX_PATH);
    return std::string(path);
}

PG_File DisaCustomBaseModManager::getExePath() const {
#ifdef _DEBUG
    const char* dirPath = "D:/Steam/SteamApps/common/Disgaea PC/";
    return PG_File(dirPath);
#else
    static PG_File filePath;
    if (filePath.isEmpty()) {
        filePath = PG_File(getExecutablePath()).getDir();
        if (filePath.isEmpty() || filePath.getPath().size() < 4 || filePath.getPath().size() > 3400) exit(1);
        if (filePath.getPath().back() != '\\') filePath.getPath().push_back('\\');
        OUT_INFO("Found path: " << filePath);
    }
    return filePath;
#endif // _DEBUG
}

