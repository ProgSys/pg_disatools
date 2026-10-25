#include "PG_DisaScriptsModel.h"

#include <Disa/fileformats/PG_JsonUtil.h>
#include <Disa/utils/PG_DisaModInfo.h>
#include <Disa/utils/PG_DisaModLoadInfo.h>
#include <Disa/models/PG_DisaMainModel.h>
#include <Disa/PG_DisaTalkCompiler.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamInByteArray.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_StringUtil.h>

using namespace PG_JsonUtil;

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

PG_DisaScriptsModel::PG_DisaScriptsModel() {
	m_scripts.emplace_back(nullptr, PG_DisaIdentifier("Base::genericDungeonIntro"));
	m_scriptsMapper[m_scripts.back().identifier] = 0;
}

void PG_DisaScriptsModel::loadScripts(PG_DisaModInfo* modInfo, const PG_DisaModLoadInfo& metaInfo) {
	if (!modInfo || modInfo->id.empty()) return;

	auto& j = metaInfo.json;

	auto addScript = [modInfo, &metaInfo](auto& jScripts, std::map<std::string, unsigned int>& mapper,
		std::vector<ScriptInfo>& infos, const char* extension) {
		ScriptInfo info{ modInfo };
		if (!readFromJsonMandatory(jScripts, "identifier", info.identifier.value) || !info.identifier.build(modInfo->namespaceMod)) {
			OUT_ERROR("'identifier' missing or is invalid for script in \"" << metaInfo.file << "\"!");
			return;
		}

		if (!readFromJsonMandatory(jScripts, "file", info.fileInZip) || info.fileInZip.isEmpty()) {
			OUT_ERROR("'file' missing for script in \"" << metaInfo.file << "\"!");
			return;
		}

		if (info.fileInZip.getFileExtensionFormated() != extension) {
			OUT_ERROR("'file' has invalid extension for script \"" << info.fileInZip << "\"! Should be ."<< extension<<"!");
			return;
		}

		//replace script
		if (auto findIt = mapper.find(info.identifier.getIdentifier()); findIt != mapper.end()) {
			infos[findIt->second] = std::move(info);
			return;
		}

		mapper[info.identifier.getIdentifier()] = infos.size();
		infos.push_back(std::move(info));
	};


	if (auto findIt = j.find("scripts"); findIt != j.end()) {
		for (auto& jScripts : *findIt) {
			addScript(jScripts, m_scriptsMapper, m_scripts, "script");
		}

	}

	if (auto findIt = j.find("talkScripts"); findIt != j.end()) {
		for (auto& jScripts : *findIt) {
			addScript(jScripts, m_talkScriptsMapper, m_talkScripts, "tscript");
		}
	}
}

static inline std::string readFromZip(const PG_DisaScriptsModel::ScriptInfo& info) {
	//a script is only read while its mod is in use, so reading around the resource manager is a hint at a bug
	if (!info.modInfo->isInUse())
		OUT_WARNING("Script \"" << info.identifier.getIdentifier() << "\" is read from '" << info.modInfo->zipFile << "' while its mod is not in use!");

	auto handle = info.modInfo->open("scripts/" + info.fileInZip.getPath());
	if (!handle) {
		OUT_ERROR("Failed to read file from zip \"" << info.identifier.getIdentifier() << "\" from zip '" << info.modInfo->zipFile << "!");
		return {};
	}
	const std::vector<char> data = handle.readAll();
	return std::string(data.begin(), data.end());
}

int PG_DisaScriptsModel::readTalkScript(unsigned char*& dataLocation, const PG_DisaIdentifier& identifier) {
	if (!identifier.isValid()) {
		dataLocation = nullptr;
		return 0;
	}

	std::string code;
	{ //try load from file system
		PG_File scriptFile = PG_DisaMainBaseModel::get()->getCustomTalkScripsDir().toFilePath(identifier.getNamespace() + "/" + identifier.getName() + ".tscript");
		if (scriptFile.exists()) {
			if (PG::STREAM::InByteFile streamFile(scriptFile); streamFile.isOpen()) {
				OUT_INFO("Replacement talk script found at: " << scriptFile);
				code.resize(streamFile.size());
				streamFile.read(code.data(), code.size());
			}
		}
	}

	//try load from loaded zip
	if (code.empty()) {
		if (auto findIt = m_talkScriptsMapper.find(identifier.getIdentifier()); findIt != m_talkScriptsMapper.end()) {
			const auto& info = m_talkScripts[findIt->second];
			code = readFromZip(info);
		}
	}

	if (code.empty() || code.size() < 3) {
		if (PG_StringUtil::isEqualCaseless(identifier.getNamespace(), "disa")) {
			const auto parameters = PG_StringUtil::split(identifier.getName(), '_');
			if (parameters.size() == 3) {
				return readDisaTalkScript(dataLocation, PG_StringUtil::toInt(parameters[0]), PG_StringUtil::toInt(parameters[1]), PG_StringUtil::toInt(parameters[2]));
			}
			else {
				return readDisaTalkScript(dataLocation, identifier.getNameAsNumber(), 0, 0);
			}
		}
		OUT_ERROR("Failed to read or find talk script: " << identifier.getIdentifier());
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
		OUT_ERROR("Failed to compile talk script: \"" << identifier.getIdentifier() << "\"! Reason: " << ex.what());
		dataLocation = 0;
		return 0;
	}

	gRollingScriptMemory.next();
	gRollingScriptMemory.current() = std::move(buffer);
	dataLocation = gRollingScriptMemory.current().data();
	return 9001;

}

static inline std::string opCodeToComment(unsigned char op) {
	switch (op)
	{
	case 0x00: return "Error";
	case 0x01: return "Sleep";
	case 0x02: return "Return";
	case 0x03: return "RepeatStart";
	case 0x04: return "RepeatEnd";
	case 0x05: return "LockUser";
	case 0x06: return "UnlockUser";
	case 0x07: return "If";
	case 0x08: return "SetFlag";
	case 0x09: return "Nop";
	case 0x0A: return "SetFade";
	case 0x0B: return "SleepSkippable";
	case 0x0C: return "ShakeScreen";
	case 0x0E: return "SetFadeColor";
	case 0x11: return "LoadTalkScript";
	case 0x12: return "RunScript";
	case 0x15: return "SetDungeon";
	case 0x16: return "EnterDungeon";
	case 0x17: return "LoadDungeonGeometry";
	case 0x18: return "WaitForMapLoad";
	case 0x1B: return "SetScreenColor";
	case 0x1C: return "SetActorRawPos";
	case 0x23: return "ClearActorData";
	case 0x24: return "SetRenderGroup";
	case 0x27: return "SetCursorPos";
	case 0x28: return "SetCameraMode";
	case 0x29: return "SetCameraZoom";
	case 0x2A: return "RotateCamera";
	case 0x2B: return "SetCameraFOV";
	case 0x2C: return "SetCameraPos";
	case 0x2D: return "SetActorGravity";
	case 0x2E: return "SetCameraRotation";
	case 0x32: return "Text";
	case 0x33: return "ClearText";
	case 0x34: return "WaitForUser";
	case 0x37: return "ShowText";
	case 0x38: return "ResetTextScroll";
	case 0x39: return "LoadPortrait";
	case 0x3A: return "ShowPortrait";
	case 0x46: return "SetUnitField";
	case 0x4E: return "SpawnPartyMember";
	case 0x4F: return "AddPartyMember";
	case 0x50: return "AddMoney";
	case 0x51: return "GiveItem";
	case 0x53: return "RequestTalk";
	case 0x54: return "WaitForFlag";
	case 0x55: return "WaitForPortrait";
	case 0x5A: return "LoadData";
	case 0x5C: return "SetTextColor";
	case 0x61: return "RunScriptOnActor";
	case 0x64: return "MoveActor";
	case 0x65: return "SetActorPos";
	case 0x66: return "SetActorAnimation";
	case 0x67: return "SetActorAnimationSet";
	case 0x68: return "ContinueActorAnimation";
	case 0x69: return "SetActorGroup";
	case 0x6A: return "SetActorPoseDir";
	case 0x6B: return "SetActorAnimationMode";
	case 0x6D: return "SetActor";
	case 0x6E: return "SetActorSprite";
	case 0x6F: return "PlayEffect";
	case 0x70: return "SetMapTile";
	case 0x71: return "PlayAimedEffect";
	case 0x72: return "PlayEffectEx";
	case 0x73: return "setActorRotation";
	case 0x74: return "PlayAimedEffectEx";
	case 0x77: return "OrbitActor";
	case 0x78: return "DoAction";
	case 0x79: return "SetActorFlag";
	case 0x7A: return "SetMultRGBA";
	case 0x7B: return "SetActorPalette";
	case 0x82: return "LoadStream";
	case 0x83: return "PlayStream";
	case 0x87: return "PlayVoice";
	case 0x88: return "StopVoice";
	case 0x8C: return "PlaySound";
	case 0x8D: return "FadeSoundVolume";
	case 0x91: return "PlayBGM";
	case 0x92: return "FadeBGM";
	case 0x93: return "PlayBGMDirect";
	case 0x94: return "PlayBGMWait";
	case 0x95: return "PlayBGMAt";
	case 0x98: return "SetUnitScript";
	case 0xC8: return "RunFunction";
	case 0xC9: return "SetSceneMode";
	case 0xCA: return "SetActorFlag2";
	case 0xCB: return "SetActorDepth";
	case 0xCC: return "PlayEffectId";
	case 0xCD: return "PlayAimedEffectId";
	case 0xCE: return "PlayEffectExId";
	case 0xCF: return "PlayAimedEffectExId";
	case 0xD0: return "SetEffectParam";
	case 0xD1: return "SetEffectValue";
	default:
		break;
	}
	return {};
}


static inline void printScript(unsigned char* ptr, std::ostream& out) {
	if (!ptr) return;

	out << "\n";
	out.fill('0');
	int killCount = 10000;
	while (--killCount > 0) {
		const unsigned char opCode = *ptr;
		if (opCode == 0) {
			out << std::hex << std::setw(2) << (int)opCode << "\n";
			break;
		}
		unsigned int argsSize = *(ptr + 1);
		out << std::hex << std::setw(2) << (int)opCode << " " << std::hex << std::setw(2) << (int)argsSize;
		ptr += 2;

		//is if statement
		if (opCode == 7) {
			const unsigned int operations = *(ptr + 1);
			argsSize = 2 + operations * 7;
		}

		for (int i = 0; i < argsSize; ++i) {
			out << " " << std::hex << std::setw(2) << (int)*ptr;
			++ptr;
		}
		if (auto comment = opCodeToComment(opCode); !comment.empty()) {
			out << " #" << comment;
		}
		if (opCode == 0x02)
			break;
		out << "\n";
	}
}

int PG_DisaScriptsModel::readScript(unsigned int*& dataLocation, const PG_DisaIdentifier& identifier) {
	if (!identifier.isValid()) {
		dataLocation = nullptr;
		return -1;
	}

	if (isPrintScript())  OUT_INFO("Script requested with identifier: " << identifier.getIdentifier());

	std::string code;
	int foundIndex = PG_DisaIdentifier::customScriptIdStart;
	{ //try load from file system
		PG_File scriptFile = PG_DisaMainBaseModel::get()->getCustomScripsDir().toFilePath(identifier.getNamespace() + "/" + identifier.getName() + ".script");
		if (isPrintScript()) OUT_INFO("Searching replacement at: " << scriptFile);
		
		if (scriptFile.exists()) {
			if (PG::STREAM::InByteFile streamFile(scriptFile); streamFile.isOpen()) {
				if (PG_StringUtil::isEqualCaseless(identifier.getNamespace(), "disa")) {
					foundIndex = readDisaScript(dataLocation, identifier.getNameAsNumber());
				}

				OUT_INFO("Replacement talk script found at: " << scriptFile);
				code.resize(streamFile.size());
				streamFile.read(code.data(), code.size());
			}
		}
	}

	//try load from loaded zip
	if (code.empty()) {
		if (auto findIt = m_scriptsMapper.find(identifier.getIdentifier()); findIt != m_scriptsMapper.end()) {
			const auto& info = m_scripts[findIt->second];
			code = readFromZip(info);
		}
	}

	if (code.empty() || code.size() < 3) {
		if (PG_StringUtil::isEqualCaseless(identifier.getNamespace(), "disa")) {
			return readDisaScript(dataLocation, identifier.getNameAsNumber());
		}
		OUT_ERROR("Failed to read or find script: " << identifier.getIdentifier());
		dataLocation = nullptr;
		return -1;
	}

	auto hexData = PG_StringUtil::hexToData(code);
	if (hexData.empty()) {
		OUT_ERROR("Failed to compile script: \"" << identifier.getIdentifier() << "\"!");
		dataLocation = 0;
		return -1;
	}

	if (isPrintScript()) {
		OUT_INFO("Script:");
		printScript(hexData.data(), std::cout);
		std::cout << std::endl;
	}

	gRollingScriptMemory.next();
	gRollingScriptMemory.current() = std::move(hexData);
	dataLocation = (unsigned int*)gRollingScriptMemory.current().data();
	return foundIndex;
}

std::string PG_DisaScriptsModel::scriptToString(unsigned int* dataLocation) const {
	std::stringstream ss;
	printScript((unsigned char*)dataLocation, ss);
	return ss.str();
}

std::string PG_DisaScriptsModel::talkScriptToString(unsigned char* dataLocation) const {
	size_t size = 0;
	auto* at = dataLocation;
	while ((*at != 0xFF || *at != 0xFE) && size < 19000) { ++at; ++size; }
	
	PG::STREAM::InByteArray stream((char*)dataLocation, size);
	try {
		return PG_DisaTalkScript::decompile(&stream);
	}
	catch (...) {
		return {};
	}
}

PG_DisaScriptsModel::ScriptInfo g_emtpy;
const PG_DisaScriptsModel::ScriptInfo& PG_DisaScriptsModel::getScriptInfo(unsigned int index) {
	if (index >= m_scripts.size()) return g_emtpy;
	return m_scripts[index];
}

const PG_DisaScriptsModel::ScriptInfo& PG_DisaScriptsModel::getTalkScriptInfo(unsigned int index) {
	if (index >= m_talkScripts.size()) return g_emtpy;
	return m_talkScripts[index];
}