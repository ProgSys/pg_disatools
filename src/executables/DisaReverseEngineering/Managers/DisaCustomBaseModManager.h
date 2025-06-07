#pragma once

#include <string>
#include <vector>
#include <stack>
#include <variant>
#include <Util/PG_File.h>
#include <glm/glm.hpp>



/**
* @brief Manages basic global stuff like scripts
*/
class DisaCustomBaseModManager {
public:
	bool isPrintScript() const { return m_printScript; }
	void setIsPrintScript(bool v) { m_printScript = v; }

	/**
	* @brief Reads a talkscript found by given identifiers
	* If dungeonId indicates a custom map (dungeonId / 100 >= 655) then use DisaCustomCampaignManager to load its scripts.
	* Otherwise will search for a replacement script with "Disa" as namespace.
	*/
	int readTalkScript(unsigned char*& dataLocation, unsigned int scriptId, unsigned int dungeonId, unsigned int unknownFlag);
	/**
	* @brief Load custom talkscript with given scriptName
	* @param scriptName Name with syntax "namespace::name". If namespace not given then "Base" will be assumed.
	* "Disa" is the namespace for all original scripts.
	* Namespace is equalent to a folder in "CustomTalkScript".
	*/
	int readTalkScript(unsigned char*& dataLocation, const std::string& scriptName);

	/**
	* @brief Reads a script found by given identifier
	* Otherwise will search for a replacement script with "Disa" as namespace.
	*/
	void readScript(unsigned int*& dataLocation, int scriptId);
	/**
	* @brief Load custom script with given scriptName
	* @param scriptName Name with syntax "namespace::name". If namespace not given then "Base" will be assumed.
	* "Disa" is the namespace for all original scripts.
	* Namespace is equalent to a folder in "CustomTalkScript".
	*/
	void readScript(unsigned int*& dataLocation, const std::string& function);

	static std::string getPath();
	/**
	* @returns Singelton
	*/
	static DisaCustomBaseModManager* get();

private:
	bool m_printScript = false;
};