#pragma once

#include <Disa/dataStructs/PG_DisaScriptManager.h>


struct DisaScriptManager: public PG_DisaTScriptManager {

	/// run talk script
	int __thiscall FUN_004bf7e0_RunTalkScript(unsigned int scriptId, unsigned int dungeonId, unsigned int unknownFlag, unsigned int param_4);
	int __thiscall callTalkScript(const std::string& scriptName, unsigned int param_4);

	int readDisaTalkScript(unsigned char*& dataLocation, uint32_t scriptId, uint32_t dungeonId, uint32_t stateFlag);
	int readDisaScript(unsigned int*& dataLocation, uint32_t scriptId);

	void clearCharDisplay();

	static DisaScriptManager& get();
private:
	void setupTalkScript(unsigned int scriptId, unsigned int dungeonId, unsigned int unknownFlag, unsigned int param_4);
};