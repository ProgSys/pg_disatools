#include "DisaCustomScriptManager.h"
#include "DisaCustomCampaignManager.h"
#include "DisaScriptManager.h"
#include <Managers/DisaCustomCharacterManager.h>
#include <DisaFunctions.h>
#include "FunctionPatcher.h"

template<typename T>
PG_DisaIdentifier getScriptName(const std::string* scriptIdentifier, const DisaCustomCampaignManager::TalkScripts* scripts, T valuePtr, const std::string& defaultScriptName) {
    if (!scriptIdentifier || scriptIdentifier->empty()) return defaultScriptName;
    return PG_DisaIdentifier((scripts && !(scripts->*valuePtr).empty()) ? (*scriptIdentifier + "::" + (scripts->*valuePtr)) : defaultScriptName);
}

int DisaCustomScriptManager::readTalkScript(unsigned char*& dataLocation, unsigned int scriptId, unsigned int dungeonId, unsigned int stateFlag) {
	if (dungeonId / 100 >= 655) {
		auto [scriptIdentifier, scripts] = DisaCustomCampaignManager::get()->getActiveTalkScripts();

        unsigned char* globalStateIt = &FP::getData<unsigned char>(aGlobalStateBits);

        switch (stateFlag)
        {
        case 1: {
            return PG_DisaScriptsModel::readTalkScript(dataLocation, getScriptName(scriptIdentifier, scripts,
                filterPlayStory.isSet(globalStateIt) ? &DisaCustomCampaignManager::TalkScripts::intro : &DisaCustomCampaignManager::TalkScripts::introSkipStory,
                "genericDungeonIntro"));
        }break;
        case 2: {
            return PG_DisaScriptsModel::readTalkScript(dataLocation, getScriptName(scriptIdentifier, scripts,
                filterPlayStory.isSet(globalStateIt) ? &DisaCustomCampaignManager::TalkScripts::exit : &DisaCustomCampaignManager::TalkScripts::exitSkipStory,
                "genericDungeonExit"));
        }break;
        default: return -1;
        }
	}

	PG_DisaIdentifier identifier = PG_DisaIdentifier("Disa::" + std::to_string(scriptId) + "_" + std::to_string(dungeonId) + "_" + std::to_string(stateFlag));
	return PG_DisaScriptsModel::readTalkScript(dataLocation, identifier);
}

int DisaCustomScriptManager::readScript(unsigned int*& dataLocation, int scriptId) {
    PG_DisaIdentifier functionName;
    if (scriptId == PG_DisaIdentifier::customScriptIdStart + 1) {
        functionName = "Base::genericDungeonIntro";
    }
    else {
        functionName = "Disa::" + std::to_string(scriptId);
    }

    return readScript(dataLocation, functionName);
}


int DisaCustomScriptManager::readDisaTalkScript(unsigned char*& dataLocation, uint32_t scriptId, uint32_t dungeonId, uint32_t stateFlag) {
    return DisaScriptManager::get().readDisaTalkScript(dataLocation, scriptId, dungeonId, stateFlag);
}

int DisaCustomScriptManager::readDisaScript(unsigned int*& dataLocation, uint32_t id) {
   return DisaScriptManager::get().readDisaScript(dataLocation, id);
}

DisaCustomScriptManager* DisaCustomScriptManager::get() {
	return reinterpret_cast<DisaCustomScriptManager*>(DisaCustomCharacterManager::get()->getDisaScriptsModel());
}
