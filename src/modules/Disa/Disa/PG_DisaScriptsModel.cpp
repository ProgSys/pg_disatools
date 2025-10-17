#include "PG_DisaScriptsModel.h"

#include <Disa/fileformats/PG_JsonUtil.h>
#include <Disa/fileformats/PG_ZIPUtil.h>
#include <Disa/utils/PG_DisaModInfo.h>
#include <Disa/utils/PG_DisaModLoadInfo.h>
#include <Disa/models/PG_DisaMainModel.h>
#include <Disa/PG_DisaTalkCompiler.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Util/Debug/PG_DebugOut.h>

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

	auto addScript = [modInfo, &metaInfo](auto& jScripts, std::map<std::string, unsigned int>& mapper, std::vector<ScriptInfo>& infos) {
		ScriptInfo info{ modInfo };
		if (!readFromJsonMandatory(jScripts, "identifier", info.identifier.value) || !info.identifier.build(modInfo->namespaceMod)) {
			OUT_ERROR("'identifier' missing or is invalid for script in \"" << metaInfo.file << "\"!");
			return;
		}

		if (!readFromJsonMandatory(jScripts, "file", info.fileInZip) || info.fileInZip.isEmpty()) {
			OUT_ERROR("'file' missing for script in \"" << metaInfo.file << "\"!");
			return;
		}

		if (info.fileInZip.getFileExtensionFormated() != "script") {
			OUT_ERROR("'file' has invalid extension for script \"" << metaInfo.file << "\"! Should be .script!");
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
			addScript(jScripts, m_scriptsMapper, m_scripts);
		}

	}

	if (auto findIt = j.find("talkscripts"); findIt != j.end()) {
		for (auto& jScripts : *findIt) {
			addScript(jScripts, m_talkScriptsMapper, m_talkScripts);
		}
	}
}

static inline std::string readFromZip(const PG_DisaScriptsModel::ScriptInfo& info) {
	std::string code;
	bool closeAfterwards = false;
	if (!info.modInfo->archive) {
		info.modInfo->archive = PG_ZIPUtil::openZip(info.modInfo->zipFile);
		closeAfterwards = true;
	}
	if (info.modInfo->archive) {
		code = PG_ZIPUtil::readString(info.modInfo->archive.get(), info.fileInZip);
	}
	else {
		OUT_ERROR("Failed to read file from zip \"" << info.identifier.getIdentifier() << "\" from zip '" << info.modInfo->zipFile << "!");
	}
	if (closeAfterwards) info.modInfo->archive.reset();

	return code;
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
	case 0x01: return "Sleep";
	case 0x02: return "Return";
	case 0x03: return "RepeatStart";
	case 0x04: return "RepeatEnd";
	case 0x05: return "LockUser";
	case 0x06: return "UnlockUser";
	case 0x07: return "If";
	case 0x09: return "Nop";
	case 0x0A: return "FadeOut";
	case 0x11: return "LoadTalkScript";
	case 0x12: return "RunScript";
	case 0x27: return "SetCursorPos";
	case 0x29: return "SetCameraZoom";
	case 0x28: return "SetCameraMode";
	case 0x2C: return "SetCameraPos";
	case 0x2E: return "SetCameraRotation";
	case 0x65: return "SetActorPos";
	case 0x67: return "SetActorAnimation";
	case 0x6D: return "SetActor";
	case 0x73: return "RotateActor";
	case 0x7A: return "SetMultRGBA";
	case 0x87: return "PlayVoice";
	case 0x8C: return "PlaySound";
	case 0x91: return "PlayBGM";
	case 0xC8: return "RunFunction";
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

PG_DisaScriptsModel::ScriptInfo g_emtpy;
const PG_DisaScriptsModel::ScriptInfo& PG_DisaScriptsModel::getScriptInfo(unsigned int index) {
	if (index >= m_scripts.size()) return g_emtpy;
	return m_scripts[index];
}

const PG_DisaScriptsModel::ScriptInfo& PG_DisaScriptsModel::getTalkScriptInfo(unsigned int index) {
	if (index >= m_talkScripts.size()) return g_emtpy;
	return m_talkScripts[index];
}