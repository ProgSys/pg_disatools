#include "PG_DisaCustomModificationsModel.h"
#include <Disa/fileformats/PG_ZIPUtil.h>
#include <Disa/fileformats/PG_JsonUtil.h>
#include <Disa/utils/PG_DisaModLoadInfo.h>
#include <Disa/models/PG_DisaMainModel.h>
#include <Disa/models/PG_DisaModLoadOrderListModel.h>
#include <Disa/PG_DisaScriptsModel.h>
#include <Disa/PG_DisaTranslationModel.h>
#include <Util/PG_StringUtil.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Scene/Gui/Validator/PG_GuiTextValidators.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamInOutByteVector.h>

using namespace PG_JsonUtil;

static PG_DisaCustomModificationsModel::Info g_emptyInfo;
static PG_DisaCustomModificationsModel::Info g_tempInfo;

PG_DisaCustomModificationsModel::PG_DisaCustomModificationsModel():
	m_scriptsModel(new PG_DisaScriptsModel()) {}

PG_DisaCustomModificationsModel::PG_DisaCustomModificationsModel(PG_DisaScriptsModel* scriptsModel, PG_DisaTranslationModel* translationModel):
	m_scriptsModel(scriptsModel), m_translationModel(translationModel) {
	assert(scriptsModel);
}

void PG_DisaCustomModificationsModel::loadModifications(const PG_File& exePath) {

	m_charactersMapper.clear();
	m_spritesMapper.clear();
	m_facesMapper.clear();
	m_characters.clear();
	m_sprites.clear();
	m_faces.clear(); //portraits
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
		try {
			loadModification(m_modifications.back(), loadInfo);
			//the list is sorted bottom first, so the top of the load order registers last and wins
			if (!m_modifications.back().acquire())
				OUT_WARNING("Mod '" << loadInfo.file << "' is not offered to the resource manager!");
			if (m_scriptsModel) m_scriptsModel->loadScripts(&m_modifications.back(), loadInfo);
			if (m_translationModel) m_translationModel->loadTranslation(&m_modifications.back(), loadInfo);
		}
		catch (const std::exception& ex) {
			m_modifications.pop_back();
			OUT_ERROR("Failed to parse 'mod.json' inside zip '" << loadInfo.file << "'! Reason: " << ex.what());
		}
		catch (...) {
			m_modifications.pop_back();
			OUT_ERROR("Failed to parse 'mod.json' inside zip '" << loadInfo.file << "'!");
		}
	}

	//sort mods to be the same order, as load order
	if(!mapping.empty()){
		m_modifications.sort([&mapping](const Modification& a, const Modification& b) {
			auto findIt = mapping.find(a.id);
			size_t aOrder = findIt != mapping.end() ? findIt->second : std::numeric_limits<std::size_t>::max();
			findIt = mapping.find(b.id);
			size_t bOrder = findIt != mapping.end() ? findIt->second : std::numeric_limits<std::size_t>::max();
			return aOrder < bOrder;
			});
	}
}

void PG_DisaCustomModificationsModel::loadModification(Modification& mod, const PG_DisaModLoadInfo& metaInfo) {
	auto& j = metaInfo.json;

	if (j.empty()) {
		throw std::exception("Json is empty!");
	}

	if (!readFromJsonMandatory(j, "id", mod.id)) {
		throw std::exception("'id' is missing in 'mod.json'!");
	}

	if (!readFromJsonMandatory(j, "name", mod.name)) {
		throw std::exception("'name' is missing in 'mod.json'!");
	}

	mod.namespaceMod = PG_DisaModInfo::resolveNameSpace(j, metaInfo.file);
	if (mod.namespaceMod.empty()) {
		throw std::exception("Invalid namespace format! Either filename or namespace dosn't match naming convention!");
	}

	mod.zipFile = metaInfo.file;

	if (auto findIt = j.find("magic"); findIt != j.end()) {
		for (auto& jMagic : *findIt) {
			Info info{ &mod };
			if (!readFromJsonMandatory(jMagic, "identifier", info.identifier.value) || !info.identifier.build(mod.namespaceMod)) {
				OUT_ERROR("'identifier' missing or is invalid for magic in \"" << metaInfo.file << "\"!");
				continue;
			}

			if (!readFromJsonMandatory(jMagic, "file", info.fileInZip)) {
				OUT_ERROR("'file' missing for magic in \"" << metaInfo.file << "\"!");
				continue;
			}
			MagicInfo magicInfo;
			magicInfo.identifier = info.identifier;
			magicInfo.magicData.fromJson(jMagic);

			if (magicInfo.magicData.name.empty()) {
				OUT_ERROR("Magic/Skill with identifier \"" << info.identifier.getIdentifier() << "\" has no name in file \"" << metaInfo.file << "\"! Magic must have a name!");
				continue;
			}

			if (magicInfo.magicData.description.empty()) {
				OUT_WARNING("Magic/Skill with identifier \"" << info.identifier.getIdentifier() << "\" has no description in file \"" << metaInfo.file << "\"!");
			}

			readFromJson(jMagic, "scriptIdentifier", magicInfo.scriptIdentifier.value);
			if (!magicInfo.scriptIdentifier.build(mod.namespaceMod) && !magicInfo.scriptIdentifier.value.empty()) {
				magicInfo.scriptIdentifier.clear();
				OUT_WARNING("Invalid 'scriptIdentifier' in magic \"" << info.identifier.getIdentifier() << "\" in file \"" << metaInfo.file << "\"!");
			}
			

			auto [added, index] = addInfo(std::move(info), m_magic, m_magicMapper);
			if (!added) {
				OUT_ERROR("Magic/Skill with identifier \"" << info.identifier.getIdentifier() << "\" is already present \"" << metaInfo.file << "\"!");
				continue;
			}
			magicInfo.magicData.id = index;

			mod.magics.push_back(std::move(magicInfo));
		}
	}

	if (auto findIt = j.find("sprites"); findIt != j.end()) {
		for (auto& jSprite : *findIt) {
			Info info{ &mod };

			if (!readFromJsonMandatory(jSprite, "identifier", info.identifier.value) || !info.identifier.build(mod.namespaceMod)) {
				OUT_ERROR("'identifier' missing or is invalid for sprite in \"" << metaInfo.file << "\"!");
				continue;
			}

			if (!readFromJsonMandatory(jSprite, "file", info.fileInZip)) {
				OUT_ERROR("'file' missing for sprite in \"" << metaInfo.file << "\"!");
				continue;
			}
			info.fileInZip = std::string("sprites/") + info.fileInZip.getPath();

			addInfo(std::move(info), m_sprites, m_spritesMapper);
		}
	}

	if (auto findIt = j.find("faces"); findIt != j.end()) {
		for (auto& jFace : *findIt) {
			Info info{ &mod };

			if (!readFromJsonMandatory(jFace, "identifier", info.identifier.value) || !info.identifier.build(mod.namespaceMod)) {
				OUT_ERROR("'identifier' missing or is invalid for sprite in \"" << metaInfo.file << "\"!");
				continue;
			}

			readFromJson(jFace, "fileHD", info.fileInZip);
			readFromJson(jFace, "fileSD", info.secondfileInZip);

			if (info.fileInZip.isEmpty()) {
				OUT_ERROR("'fileHD' missing for face in \"" << metaInfo.file << "\"! An HD image must be provided!");
				continue;
			}

			info.fileInZip = std::string("img/") + info.fileInZip.getPath();

			if (!info.secondfileInZip.isEmpty()) {
				info.secondfileInZip = std::string("img/") + info.secondfileInZip.getPath();
			}

			addInfo(std::move(info), m_faces, m_facesMapper);
		}
	}

	if (auto findIt = j.find("characters"); findIt != j.end()) {
		for (auto& jCharacter : *findIt) {
			CharacterInfo info;

			if (!readFromJsonMandatory(jCharacter, "identifier", info.identifier.value) || !info.identifier.build(mod.namespaceMod)) {
				OUT_ERROR("'identifier' missing or is invalid for character in \"" << metaInfo.file << "\"!");
				continue;
			}
			if (!readFromJsonMandatory(jCharacter, "spriteIdentifier", info.spriteIdentifier.value) || !info.spriteIdentifier.build(mod.namespaceMod)) {
				OUT_ERROR("'spriteIdentifier' missing or is invalid for character in \"" << metaInfo.file << "\"!");
				continue;
			}

			readFromJson(jCharacter, "faceIdentifier", info.faceIdentifier.value);
			if (!info.faceIdentifier.build(mod.namespaceMod)) {
				info.faceIdentifier = "Disa::0";
			}

			info.charData.clear();
			//some default values
			info.charData.type = 3;
			info.charData.manaCostScale = 10;
			info.charData.gender = 13;
			info.charData.experienceScale = 1;

			std::string deriveFrom;
			readFromJson(jCharacter, "deriveFrom", deriveFrom);
			PG_SimpleWordValidator()(deriveFrom);
			if (!deriveFrom.empty()) {
				if (auto findIt = std::ranges::find(mod.characters, deriveFrom, [](const CharacterInfo& c) { return c.identifier.getName(); });
					findIt != mod.characters.end()) {
					info.charData = findIt->charData;
					info.recruitmentType = findIt->recruitmentType;
					info.recruitGroupName = findIt->recruitGroupName;
					info.skills = findIt->skills;
				}
				else {
					OUT_ERROR("Failed to find deriveFrom \"" << deriveFrom << "\" in json!");
				}
			}

			{
				std::string recruitmentType;
				readFromJson(jCharacter, "recruitmentType", recruitmentType, 32);
				CharacterInfo::fromString(info.recruitmentType, std::move(recruitmentType));
			}

			readFromJson(jCharacter, "recruitGroupName", info.recruitGroupName);
			PG_SimpleWordValidator()(info.recruitGroupName);

			info.charData.fromJson(jCharacter);
			info.charData.characterId = 0;
			info.charData.spriteId = 0;

			if (auto findSkillIt = jCharacter.find("skills"); findSkillIt != jCharacter.end()) {
				std::pair<PG_DisaIdentifier, uint16_t> skillEntry;
				for (auto& jSkill : *findSkillIt) {
					readFromJson(jSkill, "identifier", skillEntry.first.value);
					if (!skillEntry.first.build(mod.namespaceMod) || !skillEntry.first.isValid()) {
						OUT_WARNING("'identifier' for a magic/skill is invalid for character \"" << info.identifier << "\" in \"" << metaInfo.file << "\"!");
						continue;
					}
					skillEntry.second = 0;
					readFromJson(jSkill, "level", skillEntry.second);
					skillEntry.second = std::min(skillEntry.second, static_cast<uint16_t>(9999));
					info.skills.emplace_back(std::move(skillEntry));
				}
				if (info.skills.size() > 20) info.skills.resize(20);
				std::ranges::stable_sort(info.skills, {}, & std::pair<PG_DisaIdentifier, uint16_t>::second);
				info.charData.numberOfSkills = info.skills.size();
			}

			mod.characters.push_back(std::move(info));
		}
	}
}

static inline const PG_DisaCustomModificationsModel::Info& getInfoFallback(unsigned int spriteId,
	const std::vector<PG_DisaCustomModificationsModel::Info>& list, const std::map<std::string, unsigned int>& mapper) {

	const PG_DisaCustomModificationsModel::Info* info = nullptr;

	//loading custom sprite?
	if (PG_DisaIdentifier::isCustomId(spriteId) && (spriteId - PG_DisaIdentifier::customIdStart) < list.size()) {
		return list[spriteId - PG_DisaIdentifier::customIdStart];
	}

	//replacing a default sprite?

	g_tempInfo.identifier = "disa::" + std::to_string(spriteId);
	if (auto findIt = mapper.find(g_tempInfo.identifier.getIdentifier()); findIt != mapper.end()) {
		return list[findIt->second];
	}

	return g_tempInfo;
}

std::pair<PG::STREAM::In*, const PG_DisaCustomModificationsModel::Info&>  PG_DisaCustomModificationsModel::createSpriteReader(const std::string& baseDir, unsigned int spriteId) {
	const Info& info = getInfoFallback(spriteId, m_sprites, m_spritesMapper);

	// try to load from file
	if (info.identifier) {
		PG_File file = baseDir + "mod/CustomSprite/" + info.identifier.getNamespace() + "/" + info.identifier.getName() + ".sh";
		auto* reader = new PG::STREAM::InByteFile(file);
		if (reader->isOpen()) {
			OUT_INFO("Found replacement sprite for \"" << info.identifier.getIdentifier() << "\" at \"" << file << "\".");
			return { reader , info };
		}
		delete reader;
	}

	if (info.mod) {
		if (auto handle = info.mod->open(info.fileInZip.getPath())) {
			return { new PG::STREAM::InOutByteVector(handle.readAll()) , info };
		}
		OUT_ERROR("Failed to read '" << info.fileInZip << "' from zip '" << info.mod->zipFile << "'!");
	}

	return { nullptr, info };
}

std::pair<PG::STREAM::In*, const PG_DisaCustomModificationsModel::Info&>  PG_DisaCustomModificationsModel::createFaceReader(const std::string& baseDir, unsigned int faceId, bool isHD) {
	const Info& info = getInfoFallback(faceId, m_faces, m_facesMapper);

	// try to load from file
	if (info.identifier) {
		PG_File file = baseDir + "mod/CustomImg/" + info.identifier.getNamespace() + "/" + info.identifier.getName() + (isHD ? "_HD" : "_SD") + ".tx2";
		auto* reader = new PG::STREAM::InByteFile(file);
		if (reader->isOpen()) {
			OUT_INFO("Found replacement face for \"" << info.identifier.getIdentifier() << "\" at \"" << file << "\".");
			return { reader , info };
		}
		delete reader;
	}

	if (info.mod) {
		const auto& target = isHD && !info.fileInZip.isEmpty() ? info.fileInZip : info.secondfileInZip;
		if (!target.isEmpty()) {
			if (auto handle = info.mod->open(target.getPath())) {
				return { new PG::STREAM::InOutByteVector(handle.readAll()), info };
			}
			OUT_ERROR("Failed to read '" << target << "' from zip '" << info.mod->zipFile << "'!");
		}
	}

	return { nullptr, info };
}

const PG_DisaCustomModificationsModel::Info& PG_DisaCustomModificationsModel::getCharacterInfoIndex(unsigned int index) const {
	if (index >= m_characters.size()) return g_emptyInfo;
	return m_characters[index];
}

static unsigned short getId(const std::map<std::string, unsigned int>& mapper, const PG_DisaIdentifier& identifier) {
	if (auto findIt = mapper.find(identifier); findIt != mapper.end()) return findIt->second + PG_DisaIdentifier::customIdStart;
	return 0;
}

PG_DisaCustomModificationsModel::DisplayInfo PG_DisaCustomModificationsModel::getDisplayInfo(const PG_DisaIdentifier& identifier) const {
	if (!identifier) return {0,0,0,0};
	auto findIt = m_charactersMapper.find(identifier.getIdentifier());
	if (findIt == m_charactersMapper.end()) return { 0,0,0,0 };
	
	for (const auto& c : m_characters[findIt->second].mod->characters) {
		if (c.identifier != identifier) continue;
		return getDisplayInfo(c);
	}

	return { 
		static_cast<unsigned short>(findIt->second + PG_DisaIdentifier::customIdStart),
		0,
		0,
		0
	};

}

PG_DisaCustomModificationsModel::DisplayInfo PG_DisaCustomModificationsModel::getDisplayInfo(const PG_DisaCustomModificationsModel::CharacterInfo& c) const {
	if (c.charData.characterId == 0) {
		OUT_WARNING("Character \"" << c.identifier << "\" (" << c.charData.getName() << ") has missing characterId!");
	}
	return {
		c.charData.characterId,
		getSpriteId(c.spriteIdentifier),
		getId(m_facesMapper, c.faceIdentifier),
		static_cast<unsigned short>(c.charDataIndex)
	};
}

void PG_DisaCustomModificationsModel::CharacterInfo::fromString(ERecruitmentType& type, std::string str) {
	PG_StringUtil::toUpper(str);
	if (str.empty() || str == "NORMAL") type = PG_DisaCustomModificationsModel::CharacterInfo::RecruitmentType_Normal;
	else if (str == "DEFEATED") type = PG_DisaCustomModificationsModel::CharacterInfo::RecruitmentType_Defeated ;
	else if (str == "MONSTER") type = PG_DisaCustomModificationsModel::CharacterInfo::RecruitmentType_Monster;
	else type = RecruitmentType_None;
}

const PG_DisaCustomModificationsModel::Info& PG_DisaCustomModificationsModel::getInfo(unsigned int id, const std::vector<PG_DisaCustomModificationsModel::Info>& list, bool isMagic) {
	if (isMagic) {
		if (!PG_DisaIdentifier::isCustomMagicId(id)) return g_emptyInfo;
		id -= PG_DisaIdentifier::customMagicIdStart;
	}
	else {
		if (!PG_DisaIdentifier::isCustomId(id)) return g_emptyInfo;
		id -= PG_DisaIdentifier::customIdStart;
	}

	if (id >= list.size()) return g_emptyInfo;
	return list[id];
}

const PG_DisaCustomModificationsModel::Info& PG_DisaCustomModificationsModel::getInfo(const std::string& id, const std::vector<Info>& list, const std::map<std::string, unsigned int>& mapper) {
	auto findIt = mapper.find(id);
	if (findIt == mapper.end()) return g_emptyInfo;
	return list[findIt->second];
}

std::pair<bool, unsigned int> PG_DisaCustomModificationsModel::addInfo(PG_DisaCustomModificationsModel::Info&& info, std::vector<Info>& list, std::map<std::string, unsigned int>& mapper) {
	if (!info.identifier) return { false, 0 };

	if (auto findIt = mapper.find(info.identifier.getIdentifier()); findIt != mapper.end()) {
		list[findIt->second] = std::move(info);
		return { false, findIt->second };
	}
	else {
		mapper[info.identifier.getIdentifier()] = list.size();
		list.push_back(std::move(info));
		return { true, static_cast<unsigned int>(list.size() - 1) };
	}
}

