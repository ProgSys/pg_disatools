#pragma once

#include <Util/PG_File.h>
#include <Disa/PG_DisaIdentifier.h>
#include <map>

struct PG_DisaModLoadInfo;
struct PG_DisaModInfo;

class PG_DisaScriptsModel {
	friend class PG_DisaCustomCharactersModel;
public:
	struct ScriptInfo {
		PG_DisaModInfo* modInfo = nullptr;
		PG_DisaIdentifier identifier;
		PG_File fileInZip;
	};

	bool isPrintScript() const { return m_isPrintScript; }
	void setIsPrintScript(bool v) { m_isPrintScript = v; }

	/**
	* @brief Load custom talkscript with given scriptName
	* @param identifier A valid disa identifier.
	* "Disa" is the namespace for all original scripts.
	*/
	int readTalkScript(unsigned char*& dataLocation, const PG_DisaIdentifier& identifier);

	/**
	* @brief Load custom script with given scriptName
	* @param identifier A valid disa identifier.
	* "Disa" is the namespace for all original scripts.
	*/
	int readScript(unsigned int*& dataLocation, const PG_DisaIdentifier& identifier);

	virtual int readDisaTalkScript(unsigned char*& dataLocation, uint32_t scriptId, uint32_t dungeonId, uint32_t stateFlag) {
		dataLocation = nullptr;
		return 0;
	}

	virtual int readDisaScript(unsigned int*& dataLocation, uint32_t id) {
		dataLocation = nullptr;
		return -1;
	}

	std::string scriptToString(unsigned int* dataLocation) const;
protected:
	void loadScripts(PG_DisaModInfo* modInfo, const PG_DisaModLoadInfo& info); //can throw

private:
	std::vector<ScriptInfo> m_scripts;
	std::map<std::string, unsigned int> m_scriptsMapper;
	std::vector<ScriptInfo> m_talkScripts;
	std::map<std::string, unsigned int> m_talkScriptsMapper;

	bool m_isPrintScript = false;
};