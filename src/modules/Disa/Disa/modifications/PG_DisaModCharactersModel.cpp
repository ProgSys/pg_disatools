#include "PG_DisaModCharactersModel.h"
#include <Disa/fileformats/PG_JsonUtil.h>
#include <Disa/utils/PG_DisaModInfo.h>
#include <Disa/utils/PG_DisaModLoadInfo.h>
#include <Scene/Gui/Validator/PG_GuiTextValidators.h>
#include <Util/PG_StringUtil.h>
#include <Util/Debug/PG_DebugOut.h>

using namespace PG_JsonUtil;

void PG_DisaModCharactersModel::CharacterInfo::fromString(ERecruitmentType& type, std::string str) {
	PG_StringUtil::toUpper(str);
	if (str.empty() || str == "NORMAL") type = RecruitmentType_Normal;
	else if (str == "DEFEATED") type = RecruitmentType_Defeated;
	else if (str == "MONSTER") type = RecruitmentType_Monster;
	else type = RecruitmentType_None;
}

void PG_DisaModCharactersModel::load(PG_DisaModInfo* modInfo, const PG_DisaModLoadInfo& loadInfo) {
	auto findBlockIt = loadInfo.json.find("characters");
	if (findBlockIt == loadInfo.json.end()) return;

	const std::size_t firstOfMod = m_characters.size();

	for (auto& jCharacter : *findBlockIt) {
		CharacterInfo info;
		info.modInfo = modInfo;

		if (!readFromJsonMandatory(jCharacter, "identifier", info.identifier.value) || !info.identifier.build(modInfo->namespaceMod)) {
			OUT_ERROR("'identifier' missing or is invalid for character in \"" << loadInfo.file << "\"!");
			continue;
		}
		if (!readFromJsonMandatory(jCharacter, "spriteIdentifier", info.spriteIdentifier.value) || !info.spriteIdentifier.build(modInfo->namespaceMod)) {
			OUT_ERROR("'spriteIdentifier' missing or is invalid for character in \"" << loadInfo.file << "\"!");
			continue;
		}

		readFromJson(jCharacter, "faceIdentifier", info.faceIdentifier.value);
		if (!info.faceIdentifier.build(modInfo->namespaceMod)) {
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
			auto ofMod = std::ranges::subrange(m_characters.begin() + firstOfMod, m_characters.end());
			if (auto findIt = std::ranges::find(ofMod, deriveFrom, [](const CharacterInfo& c) { return c.identifier.getName(); });
				findIt != ofMod.end()) {
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
				if (!skillEntry.first.build(modInfo->namespaceMod) || !skillEntry.first.isValid()) {
					OUT_WARNING("'identifier' for a magic/skill is invalid for character \"" << info.identifier << "\" in \"" << loadInfo.file << "\"!");
					continue;
				}
				skillEntry.second = 0;
				readFromJson(jSkill, "level", skillEntry.second);
				skillEntry.second = std::min(skillEntry.second, static_cast<uint16_t>(9999));
				info.skills.emplace_back(std::move(skillEntry));
			}
			if (info.skills.size() > 20) info.skills.resize(20);
			std::ranges::stable_sort(info.skills, {}, &std::pair<PG_DisaIdentifier, uint16_t>::second);
			info.charData.numberOfSkills = info.skills.size();
		}

		m_characters.push_back(std::move(info));
	}
}

void PG_DisaModCharactersModel::clear() {
	m_characters.clear();
	m_registered.clear();
}

std::vector<unsigned int> PG_DisaModCharactersModel::getIndicesInOrder(const std::list<PG_DisaModInfo>& modifications) const {
	std::vector<unsigned int> indices;
	indices.reserve(m_characters.size());
	for (const PG_DisaModInfo& mod : modifications) {
		for (unsigned int i = 0; i < m_characters.size(); ++i) {
			if (m_characters[i].modInfo == &mod) indices.push_back(i);
		}
	}
	return indices;
}

std::pair<bool, unsigned int> PG_DisaModCharactersModel::registerCharacter(unsigned int characterIndex) {
	if (characterIndex >= m_characters.size()) return { false, 0 };
	return m_registered.add(RegisteredInfo{ m_characters[characterIndex].identifier, characterIndex });
}

PG_DisaIdentifier PG_DisaModCharactersModel::getCharacterIdentifier(unsigned int id) const {
	if (PG_DisaIdentifier::isCustomId(id)) return m_registered.getById(id).identifier;
	return PG_DisaIdentifier(std::to_string(id), "disa");
}

const PG_DisaModCharactersModel::CharacterInfo* PG_DisaModCharactersModel::findCharacter(unsigned int id) const {
	if (!m_registered.containsId(id)) return nullptr;
	return &m_characters[m_registered.getById(id).characterIndex];
}

const PG_DisaModCharactersModel::CharacterInfo* PG_DisaModCharactersModel::findCharacter(const PG_DisaIdentifier& identifier) const {
	const RegisteredInfo* info = m_registered.find(identifier);
	return info ? &m_characters[info->characterIndex] : nullptr;
}
