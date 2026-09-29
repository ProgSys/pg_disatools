#pragma once

#include <string>
#include <nlohmann/json_fwd.hpp>
#include <Util/PG_File.h>
#include <Resources/PG_ResourceHandle.h>
#include <Resources/PG_ResourceSourceScope.h>
#include <Disa/resources/PG_ZIPResourceSource.h>

/**
* @brief One mod zip, a campaign or a character mod, and its place in the resource manager.
* @details The registration is the only thing holding the zip, so letting the mod go closes it.
*/
struct PG_DisaModInfo {
	std::string id;
	std::string name;
	std::string namespaceMod;

	PG_File zipFile;
	/// Registered while the mod is in use, under namespaceMod
	PG_ResourceSourceScope<PG_ZIPResourceSource> resource;

	inline bool isInUse() const { return static_cast<bool>(resource); }
	/**
	* @brief Puts the zip on top of its namespace. Doing it twice does nothing.
	* @returns false when the zip can't be read, there is no namespace or no resource manager.
	*/
	bool acquire();
	/// Takes the zip out of the resource manager. Reads already running finish.
	inline void release() { resource.expire(); }
	/**
	* @brief Opens "<namespaceMod>::<pathInZip>" through the resource manager, so a mod stacked on
	* top of this one can replace the file.
	* @details A mod that is not in use reads its own zip directly, without any stacking.
	* @returns A closed handle when nobody carries the file.
	*/
	PG_ResourceHandle open(const std::string& pathInZip) const;

	/**
	* @brief The namespace a mod declares: "namespace" (or the older misspelled "namesspace"),
	* else the name of its zip, lowercased.
	* @returns Empty when that is no valid namespace.
	*/
	static std::string resolveNameSpace(const nlohmann::json& j, const PG_File& zipFile);
	/// @see resolveNameSpace, for a namespace that was already read. Empty falls back to the name of the zip.
	static std::string resolveNameSpace(std::string declared, const PG_File& zipFile);

	bool operator==(const PG_DisaModInfo& other) const { return id == other.id; }
};
