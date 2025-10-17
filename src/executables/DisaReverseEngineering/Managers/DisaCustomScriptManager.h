#pragma once

#include <Disa/PG_DisaScriptsModel.h>

class DisaCustomScriptManager : public PG_DisaScriptsModel {
public:
	using PG_DisaScriptsModel::PG_DisaScriptsModel;

	using PG_DisaScriptsModel::readTalkScript;
	using PG_DisaScriptsModel::readScript;

	int readTalkScript(unsigned char*& dataLocation, unsigned int scriptId, unsigned int dungeonId, unsigned int stateFlag);
	/**
	* @brief Reads a script found by given identifier
	* Otherwise will search for a replacement script with "Disa" as namespace.
	*/
	void readScript(unsigned int*& dataLocation, int scriptId);


	int readDisaTalkScript(unsigned char*& dataLocation, uint32_t scriptId, uint32_t dungeonId, uint32_t stateFlag) override;
	void readDisaScript(unsigned int*& dataLocation, uint32_t id) override;

	static DisaCustomScriptManager* get();
};