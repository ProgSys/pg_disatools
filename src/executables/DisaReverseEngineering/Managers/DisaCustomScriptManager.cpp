#include "DisaCustomScriptManager.h"
#include "DisaCustomCampaignManager.h"
#include "DisaScriptManager.h"
#include <Managers/DisaCustomCharacterManager.h>
#include <DisaFunctions.h>
#include "FunctionPatcher.h"

template<typename T>
PG_DisaIdentifier getScriptName(const DisaCustomCampaignManager::TalkScripts* scripts, T valuePtr, const std::string& defaultScriptName) {
    return PG_DisaIdentifier((scripts && !(scripts->*valuePtr).isValid()) ? (scripts->*valuePtr) : PG_DisaIdentifier(defaultScriptName));
}

int DisaCustomScriptManager::readTalkScript(unsigned char*& dataLocation, unsigned int scriptId, unsigned int dungeonId, unsigned int stateFlag) {
	if (dungeonId / 100 >= 655) {
		auto scripts = DisaCustomCampaignManager::get()->getActiveTalkScripts();

        unsigned char* globalStateIt = &FP::getData<unsigned char>(aGlobalStateBits);

        switch (stateFlag)
        {
        case 1: {
            return PG_DisaScriptsModel::readTalkScript(dataLocation, getScriptName(scripts,
                filterPlayStory.isSet(globalStateIt) ? &DisaCustomCampaignManager::TalkScripts::intro : &DisaCustomCampaignManager::TalkScripts::introSkipStory,
                "base::genericDungeonIntro"));
        }break;
        case 2: {
            return PG_DisaScriptsModel::readTalkScript(dataLocation, getScriptName(scripts,
                filterPlayStory.isSet(globalStateIt) ? &DisaCustomCampaignManager::TalkScripts::exit : &DisaCustomCampaignManager::TalkScripts::exitSkipStory,
                "base::genericDungeonExit"));
        }break;
        default: return -1;
        }
	}

	PG_DisaIdentifier identifier = PG_DisaIdentifier("Disa::" + std::to_string(scriptId) + "_" + std::to_string(dungeonId) + "_" + std::to_string(stateFlag));
	return PG_DisaScriptsModel::readTalkScript(dataLocation, identifier);
}

int DisaCustomScriptManager::readScript(unsigned int*& dataLocation, int scriptId) {
    PG_DisaIdentifier functionName;

    if (auto split = PG_DisaIdentifier::splitScriptId(scriptId); PG_DisaIdentifier::isCustomScriptId(split)) {
        switch (split.first) {
        case PG_DisaIdentifier::ScriptType_Magic: {
            auto* charManager = DisaCustomCharacterManager::get();
            const auto& info = charManager->getMagicInfo(split.second);
            if (!info.mod) return -1;

            auto findIt = std::ranges::find(info.mod->magics, info.identifier, &PG_DisaCustomCharactersModel::MagicInfo::identifier);
            if (findIt == info.mod->magics.end() || !findIt->scriptIdentifier.isValid()) {
                OUT_INFO("Failed to find a custom script for magic! (" << scriptId << ")");
                return -1;
            }

            functionName = findIt->scriptIdentifier;
        } break;
        case PG_DisaIdentifier::ScriptType_Custom: {
            functionName = getScriptInfo(split.second).identifier;
        } break;
        }
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
