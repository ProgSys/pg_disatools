#pragma once

#include <string>
#include <vector>
#include <stack>
#include <variant>
#include <Util/PG_File.h>
#include <glm/glm.hpp>

#include <Disa/models/PG_DisaMainModel.h>



/**
* @brief Manages basic global stuff like scripts
*/
class DisaCustomBaseModManager: public PG_DisaMainBaseModel {
public:
	using PG_DisaMainBaseModel::PG_DisaMainBaseModel;

	PG_File getExePath() const override;
	/**
	* @returns Singelton
	*/
	static DisaCustomBaseModManager* get() { return reinterpret_cast<DisaCustomBaseModManager*>(PG_DisaMainBaseModel::get()); }
};