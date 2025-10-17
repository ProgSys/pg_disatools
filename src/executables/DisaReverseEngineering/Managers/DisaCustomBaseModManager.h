#pragma once

#include <string>
#include <vector>
#include <stack>
#include <variant>
#include <Util/PG_File.h>
#include <glm/glm.hpp>

#include <Disa/models/PG_DisaMainModel.h>

class PG_DisaScriptsModel;

/**
* @brief Manages basic global stuff like scripts
*/
class DisaCustomBaseModManager: public PG_DisaMainBaseModel {
public:
	DisaCustomBaseModManager();

	PG_File getExePath() const override;
	/**
	* @returns Singelton
	*/
	static DisaCustomBaseModManager* get() { return reinterpret_cast<DisaCustomBaseModManager*>(PG_DisaMainBaseModel::get()); }

	PG_DisaScriptsModel* getScriptsModel() const { return m_scriptsModel; }

private:
	PG_DisaScriptsModel* m_scriptsModel = nullptr;
};