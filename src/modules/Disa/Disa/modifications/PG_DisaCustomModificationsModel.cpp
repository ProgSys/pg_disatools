#include "PG_DisaCustomModificationsModel.h"
#include <Disa/modifications/PG_DisaModSpritesModel.h>
#include <Disa/modifications/PG_DisaModFacesModel.h>
#include <Disa/modifications/PG_DisaModMagicModel.h>
#include <Disa/modifications/PG_DisaModCharactersModel.h>
#include <Disa/fileformats/PG_ZIPUtil.h>
#include <Disa/fileformats/PG_JsonUtil.h>
#include <Disa/utils/PG_DisaModLoadInfo.h>
#include <Disa/models/PG_DisaMainModel.h>
#include <Disa/models/PG_DisaModLoadOrderListModel.h>
#include <Disa/PG_DisaScriptsModel.h>
#include <Disa/PG_DisaTranslationModel.h>
#include <Disa/PG_DisaVersion.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Resources/PG_ResourceName.h>

using namespace PG_JsonUtil;

PG_DisaCustomModificationsModel::PG_DisaCustomModificationsModel(PG_DisaScriptsModel* scriptsModel, PG_DisaTranslationModel* translationModel):
	m_spritesModel(std::make_unique<PG_DisaModSpritesModel>()),
	m_facesModel(std::make_unique<PG_DisaModFacesModel>()),
	m_magicModel(std::make_unique<PG_DisaModMagicModel>()),
	m_charactersModel(std::make_unique<PG_DisaModCharactersModel>()),
	m_scriptsModel(scriptsModel), m_translationModel(translationModel) {
	assert(scriptsModel);
}

PG_DisaCustomModificationsModel::~PG_DisaCustomModificationsModel() = default;

/// @throws When id, name or namespace are missing or invalid
static void readModHeader(PG_DisaModInfo& mod, const PG_DisaModLoadInfo& loadInfo) {
	auto& j = loadInfo.json;
	if (j.empty()) throw std::exception("Json is empty!");
	if (!readFromJsonMandatory(j, "id", mod.id)) throw std::exception("'id' is missing in 'mod.json'!");
	if (!readFromJsonMandatory(j, "name", mod.name)) throw std::exception("'name' is missing in 'mod.json'!");

	mod.namespaceMod = PG_DisaModInfo::resolveNameSpace(j, loadInfo.file);
	if (mod.namespaceMod.empty()) {
		throw std::exception("Invalid namespace format! Either filename or namespace dosn't match naming convention!");
	}
	mod.zipFile = loadInfo.file;
}

/// A name of the mod, "::" in front makes it a global one and one with a namespace stays as it is
static std::string resolveModName(const std::string& name, const std::string& nameSpace) {
	if (name.starts_with("::")) return name.substr(2);
	if (name.find("::") != std::string::npos) return name;
	return PG_ResourceName::join(nameSpace, name);
}

/**
* @brief Lays the "betterControls" block of the mod over the settings, a key it leaves out keeps what is there.
* @details The first block turns the settings on with their defaults.
* @returns False when the mod has no block
*/
static bool applyBetterControls(const PG_DisaModInfo& mod, const nlohmann::json& j, std::optional<PG_DisaCustomModificationsModel::BetterControls>& settings) {
	const auto block = j.find("betterControls");
	if (block == j.end()) return false;
	if (!settings) settings.emplace();
	if (!block->is_object()) {
		OUT_WARNING("Mod '" << mod.name << "': \"betterControls\" is no object, it changes nothing");
		return true;
	}

	readFromJson(*block, "radialMenu", settings->isRadialMenu);
	std::string name;
	readFromJson(*block, "radialMenuStyle", name);
	if (!name.empty()) settings->radialMenuStyle = resolveModName(name, mod.namespaceMod);
	name.clear();
	readFromJson(*block, "pathMaterial", name);
	if (!name.empty()) settings->pathMaterial = resolveModName(name, mod.namespaceMod);
	return true;
}

void PG_DisaCustomModificationsModel::loadModifications(const PG_File& exePath) {
	m_betterControls.reset();
	m_spritesModel->clear();
	m_facesModel->clear();
	m_magicModel->clear();
	m_charactersModel->clear();
	m_modifications.clear();

	if (exePath.isEmpty() || !exePath.exists()) {
		return;
	}
	PG_Dir modificationsDir = PG_DisaMainBaseModel::customModificationsDir(exePath);

	std::vector<PG_DisaModLoadInfo> loadInfos;

	for (const auto& zipFile : modificationsDir.findFiles([](const PG_File& file)->bool {  return file.getFileExtensionFormated() == "zip"; }, 1)) {
		auto archive = PG_ZIPUtil::openZip(zipFile);
		if (!archive) continue;
		PG_DisaModLoadInfo info;
		info.file = zipFile;

		try {
			info.json = PG_ZIPUtil::readJson(archive, "mod.json");
			if (info.json.empty()) {
				OUT_ERROR("Failed to parse 'mod.json' inside zip '" << zipFile << "'!");
				continue;
			}

			if (!readFromJsonMandatory(info.json, "id", info.id)) {
				OUT_ERROR("'mod.json' has no 'id' inside zip '" << zipFile << "'!");
				continue;
			}

			std::string strVersion;
			readFromJson(info.json, "min_supported_version", strVersion);
			if (!strVersion.empty()) {
				auto range = PG_StringUtil::splitView(strVersion, '.');
				if (range.size() != 3) {
					OUT_ERROR("'mod.json' has invalid 'version' inside zip '" << zipFile << "'!");
					continue;
				}
				//the tweak of the j.dll version is left out, a bugfix build doesn't change what mods can rely on
				constexpr std::array<int, 3> toolsVersion{ DISA_MOD_VERSION_MAJOR, DISA_MOD_VERSION_MINOR, DISA_MOD_VERSION_PATCH };
				std::array<int, 3> minVersion{};
				std::ranges::transform(range, minVersion.begin(), PG_StringUtil::toIntView);
				if (minVersion > toolsVersion) {
					OUT_ERROR("The minimal supported version of '" << zipFile << "' is bigger then of the tools! The mod is not loaded!");
					continue;
				}
			}

		}
		catch (const std::exception& ex) {
			OUT_ERROR("Failed to parse 'mod.json' inside zip '" << zipFile << "'! Reason: " << ex.what());
			continue;
		}
		catch (...) {
			OUT_ERROR("Failed to parse 'mod.json' inside zip '" << zipFile << "'!");
			continue;
		}
		loadInfos.push_back(std::move(info));
	}

	{ //remove dublicate ids
		std::ranges::sort(loadInfos, {}, &PG_DisaModLoadInfo::id);
		auto range = std::ranges::unique(loadInfos);
		loadInfos.erase(range.begin(), range.end());
	}

	std::unordered_map<std::string, std::size_t> mapping;
	{ //sort by load order
		PG_DisaModLoadOrderListModel orderModel;
		orderModel.open(modificationsDir.toFilePath("loadOrder.json"));
		mapping = orderModel.getOrderMapping();
		//load the items on the bottom first
		std::ranges::sort(loadInfos, std::ranges::greater{}, [&mapping](const PG_DisaModLoadInfo& c) {
			auto findIt = mapping.find(c.id);
			return findIt != mapping.end() ? findIt->second : std::numeric_limits<std::size_t>::max();
			});
		std::erase_if(loadInfos, [&mapping, &orderModel](const PG_DisaModLoadInfo& c) {
			auto findIt = mapping.find(c.id);
			return findIt != mapping.end() ? orderModel.get(findIt->second).disabled : false;
			});
	}

	for (const auto& loadInfo : loadInfos) {
		m_modifications.push_back({});
		PG_DisaModInfo* mod = &m_modifications.back();
		try {
			readModHeader(*mod, loadInfo);
		}
		catch (const std::exception& ex) {
			m_modifications.pop_back();
			OUT_ERROR("Failed to parse 'mod.json' inside zip '" << loadInfo.file << "'! Reason: " << ex.what());
			continue;
		}

		//the sub-models point at the mod from here on, so a failure below keeps it in the list
		try {
			//loaded bottom first, so every mod lays its keys over the ones of the mods below it
			if (applyBetterControls(*mod, loadInfo.json, m_betterControls)) {
				OUT_INFO("Better controls changed by mod '" << mod->name << "', radial menu " << (m_betterControls->isRadialMenu ? "on" : "off"));
			}
			m_magicModel->load(mod, loadInfo);
			m_spritesModel->load(mod, loadInfo);
			m_facesModel->load(mod, loadInfo);
			m_charactersModel->load(mod, loadInfo);
			//the list is sorted bottom first, so the top of the load order registers last and wins
			if (!mod->acquire())
				OUT_WARNING("Mod '" << loadInfo.file << "' is not offered to the resource manager!");
			m_scriptsModel->loadScripts(mod, loadInfo);
			if (m_translationModel) m_translationModel->loadTranslation(mod, loadInfo);
		}
		catch (const std::exception& ex) {
			OUT_ERROR("Failed to load mod '" << loadInfo.file << "'! Reason: " << ex.what());
		}
		catch (...) {
			OUT_ERROR("Failed to load mod '" << loadInfo.file << "'!");
		}
	}

	//sort mods to be the same order, as load order
	if(!mapping.empty()){
		m_modifications.sort([&mapping](const PG_DisaModInfo& a, const PG_DisaModInfo& b) {
			auto findIt = mapping.find(a.id);
			size_t aOrder = findIt != mapping.end() ? findIt->second : std::numeric_limits<std::size_t>::max();
			findIt = mapping.find(b.id);
			size_t bOrder = findIt != mapping.end() ? findIt->second : std::numeric_limits<std::size_t>::max();
			return aOrder < bOrder;
			});
	}
}

PG_DisaCustomModificationsModel::DisplayInfo PG_DisaCustomModificationsModel::getDisplayInfo(const PG_DisaIdentifier& identifier) const {
	const PG_DisaModCharactersModel::CharacterInfo* character = m_charactersModel->findCharacter(identifier);
	if (!character) return {};

	if (character->charData.characterId == 0) {
		OUT_WARNING("Character \"" << character->identifier << "\" (" << character->charData.getName() << ") has missing characterId!");
	}
	return {
		character->charData.characterId,
		m_spritesModel->getSpriteId(character->spriteIdentifier),
		m_facesModel->getFaceId(character->faceIdentifier),
		static_cast<unsigned short>(character->charDataIndex)
	};
}
