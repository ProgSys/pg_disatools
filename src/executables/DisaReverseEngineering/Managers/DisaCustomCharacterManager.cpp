#include "DisaCustomCharacterManager.h"
#include <Managers/DisaCustomBaseModManager.h>
#include <DisaDataFunctions.h>
#include "FunctionPatcher.h"
#include <Util/PG_Dir.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_StringUtil.h>
#include <Scene/Gui/Validator/PG_GuiTextValidators.h>
#include <Stream/PG_StreamInOutByteArray.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamInOutByteVector.h>
#include <Disa/fileformats/PG_ZIPUtil.h>
#include <zip.h>
#include <nlohmann/json.hpp>

static DisaCustomCharacterManager::Info gEmptyInfo;
static DisaCustomCharacterManager::Info gTempInfo;

static inline std::vector<char> buildSpriteData(PG::STREAM::In* reader, unsigned int spriteId) {
	constexpr unsigned int rawDataHeaderSize = sizeof(unsigned int) * 2 + sizeof(short);
	std::vector<char> data;
	unsigned int fileSize = reader->size();
	data.resize(sizeof(DisaSpriteLoaderInfo) + rawDataHeaderSize + fileSize + 0x1200); //0x1200 is size of processed data

	DisaSpriteLoaderInfo& loader = *(DisaSpriteLoaderInfo*)data.data();
	memset(&loader, 0, sizeof(DisaSpriteLoaderInfo));

	PG::STREAM::InOutByteArray anmData(data.data() + sizeof(DisaSpriteLoaderInfo), rawDataHeaderSize);
	anmData.writeInt(1); //size
	anmData.writeInt(sizeof(int) * 2 + sizeof(short)); //address
	anmData.writeShort(spriteId);//id

	reader->read(const_cast<char*>(anmData.getDataAt()), fileSize);

	loader.anmData = (DisaSpriteOffsetListArchiveData*)anmData.getData();
	loader.data = (char*)(anmData.getDataAt() + fileSize);
	loader.FUN_004de780_processANM(1);
	if (!loader.initialized) {
		data.clear();
		data.shrink_to_fit();
	}
	return data;
}

char* DisaCustomCharacterManager::getSpriteData(unsigned int spriteId){
	//do we have data loaded?
	if (auto findSpriteIt = m_spriteData.find(spriteId); findSpriteIt != m_spriteData.end()) {
		if (findSpriteIt->second.size() < sizeof(DisaSpriteLoaderInfo)) return nullptr;
		DisaSpriteLoaderInfo& loader = *(DisaSpriteLoaderInfo*)findSpriteIt->second.data();
		return loader.data;
	}

	std::vector<char> data;
	auto [reader, info] = createSpriteReader(DisaCustomBaseModManager::exePath(), spriteId);

	if (!reader) {
		m_spriteData[spriteId] = std::move(data);
		return nullptr;
	}

	data = buildSpriteData(reader, spriteId);
	delete reader;

	if (data.empty()) {
		m_spriteData[spriteId] = std::move(data);
		return nullptr;
	}

	DisaSpriteLoaderInfo& loader = *(DisaSpriteLoaderInfo*)data.data();
	m_spriteData[spriteId] = std::move(data);
	return loader.data;
}

static inline std::vector<char> buildFaceData(PG::STREAM::In* reader, unsigned int faceId, const DisaCustomCharacterManager::Info& info, bool isHD) {
	constexpr unsigned int rawDataHeaderSize = sizeof(unsigned int) * 2 + sizeof(short);
	std::vector<char> data;
	unsigned int fileSize = reader->size();
	data.resize(sizeof(DisaCustomCharacterManager::FaceHeader) + fileSize);

	DisaCustomCharacterManager::FaceHeader& loader = *(DisaCustomCharacterManager::FaceHeader*)data.data();
	memset(&loader, 0, sizeof(DisaCustomCharacterManager::FaceHeader));
	if (info.fileInZip.isEmpty()) loader.isHD = false;
	if (info.secondfileInZip.isEmpty()) loader.isHD = true;
	else loader.isHD = isHD;
	loader.isOnlyOneImage = info.fileInZip.isEmpty() || info.secondfileInZip.isEmpty();

	char*& target = *((char**)&loader.header[12 * 4]);
	target = data.data() + sizeof(DisaCustomCharacterManager::FaceHeader);
	reader->read(target, fileSize);


	FP::callFastFarA<void>(0x004e2810, loader.header.data());

	return data;
}


DisaCustomCharacterManager::FaceHeader* DisaCustomCharacterManager::getFaceData(unsigned int faceId, bool isHD) {
	//do we have data loaded?
	if (auto findIt = m_faceData.find(faceId); findIt != m_faceData.end()) {
		if (findIt->second.size() < sizeof(FaceHeader)) return nullptr;
		auto* loader = (FaceHeader*)findIt->second.data();
		if (loader->isOnlyOneImage || loader->isHD == isHD) return loader;
	}

	std::vector<char> data;
	auto [reader, info] = createFaceReader(DisaCustomBaseModManager::exePath(), faceId, isHD);

	if (!reader) {
		m_faceData[faceId] = std::move(data);
		return nullptr;
	}

	data = buildFaceData(reader, faceId, info, isHD);
	delete reader;

	if (data.empty()) {
		m_faceData[faceId] = std::move(data);
		return nullptr;
	}

	FaceHeader* loader = (FaceHeader*)data.data();
	m_faceData[faceId] = std::move(data);
	return loader;

}

void DisaCustomCharacterManager::addCharacterToPary(const PG_DisaIdentifier& identifier, unsigned short level, int8_t mentorIndex) const{
	if (!identifier.isValid()) return;
	level = glm::clamp<unsigned short>(level, 1, 9999);

	PG_DisaDataCharacter* newCharData = nullptr;
	PG_DisaCustomCharactersModel::DisplayInfo displayInfo;

	if (identifier.isCustom()) {
		auto findIt = m_charactersMapper.find(identifier);
		if (findIt == m_charactersMapper.end()) {
			OUT_ERROR("Failed to find custom character with identifier \"" << identifier.getIdentifier() << "\"!");
			return;
		}
		auto findCharDataIt = std::ranges::find(m_characters[findIt->second].mod->characters, identifier, [](const CharacterInfo& i) { return i.identifier; });
		if (findCharDataIt == m_characters[findIt->second].mod->characters.end()) {
			OUT_ERROR("Failed to find custom character data with identifier \"" << identifier.getIdentifier() << "\"!");
			return;
		}

		newCharData = &findCharDataIt->charData;
		displayInfo = getDisplayInfo(*findCharDataIt);
	}
	else {
		auto findIt = std::ranges::find(DisaDataCharacterInfo::begin(), DisaDataCharacterInfo::end(), identifier.getNameAsNumber(), &DisaDataCharacterInfo::characterId);
		if (findIt == DisaDataCharacterInfo::end()) {
			OUT_ERROR("Failed to find character with identifier \"" << identifier.getIdentifier() << "\"!");
			return;
		}

		newCharData = &*findIt;
		displayInfo.characterId = newCharData->characterId;
		displayInfo.spriteId = newCharData->spriteId;
		displayInfo.faceId = newCharData->characterId;
		displayInfo.charDatIndex = std::distance(DisaDataCharacterInfo::begin(), findIt);
	}



	if (!newCharData || displayInfo.characterId == 0 || displayInfo.charDatIndex == 0) {
		OUT_ERROR("The character \"" << identifier.getIdentifier() << "\" has invalid data! (characterId or charDatIndex is 0)");
		return;
	}

	
	if (mentorIndex > -1) {
		if (mentorIndex >= DisaPartyCharacterInfo::getSize()) {
			OUT_ERROR("Mentor index " << mentorIndex << " is out of bounds! Player party size is: " << DisaPartyCharacterInfo::getSize());
			return;
		}
		OUT_INFO("Adding character \"" << newCharData->getName() << "\" with identifier \"" << identifier.getIdentifier() << "\" at level " << level << " and \"" << DisaPartyCharacterInfo::getEntry(mentorIndex).getName() << "\" as mentor.");
	}
	else {
		mentorIndex = std::max(mentorIndex, static_cast<int8_t>(-1));
		OUT_INFO("Adding character \"" << newCharData->getName() << "\" with identifier \"" << identifier.getIdentifier() << "\" at level " << level << " and no mentor.");
	}

	auto& newChar = DisaPartyCharacterInfo::addNewEntry();

	//TODO all of this may do nothing as 0x004dafc0 resets everything :p
	newChar.charName = newCharData->name;
	newChar.charTitle = newCharData->title;
	newChar.level = 1;
	if(level != 1) newChar.experience = FP::getData<std::array<unsigned long long, 9999>>(0x005e9b28)[level] * newCharData->experienceScale;
	newChar.characterId = displayInfo.characterId;
	newChar.spriteId = displayInfo.spriteId;
	newChar.charDatIndex = displayInfo.charDatIndex;

	newChar.counterBase = newCharData->counter;
	newChar.counter = newCharData->counter;
	newChar.jumpingPowerBase = newCharData->jumpPower;
	newChar.jumpingPower = newCharData->jumpPower;
	newChar.movementBase = newCharData->movement;
	newChar.movement = newCharData->movement;
	

	newChar.stats = newCharData->baseStats;
	newChar.weaponMasteryBaseStats = newCharData->weaponMasteryBaseStats;
	std::copy(newCharData->skillIds.begin(), newCharData->skillIds.end(), newChar.skillId.begin());
	
	FP::callFastFarA(0x004dafc0, &newChar, newChar.characterId, level, 0, 0); //peform level up
	//level up function changes the face ID and other values
	if (PG_DisaIdentifier::isCustomId(displayInfo.faceId)) {
		newChar.setCustomPortraitId(displayInfo.faceId);
	}
	else {
		newChar.portraitColor = newCharData->paletteIndex;
		newChar.portraitImageId = displayInfo.faceId;
	}

	newChar.mentorIndex = mentorIndex;

}

void DisaCustomCharacterManager::recordEnemyKilled(DisaPartyCharacterInfo* characterInfo) {
	if (!characterInfo || !PG_DisaIdentifier::isCustomId(characterInfo->characterId)) return;

	const auto& charInfo = getCharacterInfo(characterInfo->characterId);
	if (!charInfo.identifier.isValid() || !charInfo.mod) {
		assert(false);
		return;
	}

	//can unit even be recruited
	auto findIt = std::ranges::find(charInfo.mod->characters, charInfo.identifier, &CharacterInfo::identifier);
	if (findIt == charInfo.mod->characters.end() || 
		findIt->recruitmentType == CharacterInfo::RecruitmentType_None ||
		findIt->recruitmentType == CharacterInfo::RecruitmentType_Normal) return;

	//add to history
	auto [it, idAdded] = m_assemblyHistory.try_emplace(charInfo.identifier);
	if (idAdded) it->second.clear(); //needed as we cant use construct
	it->second.characterId = characterInfo->characterId;
	if (it->second.killCount != std::numeric_limits<unsigned short>::max()) {
		++it->second.killCount;
	}
	it->second.killCountLevelSum = std::min(it->second.killCountLevelSum + characterInfo->level, DisaCustomCharacterManager::maxKillLevelSum);
	m_isHistoryChanged = true;
}

void DisaCustomCharacterManager::loadCharacters() {
	loadCustomCharacters(DisaCustomBaseModManager::exePath());
	loadCharactersIntoMememory();
	m_isHistoryChanged = true;
}

void DisaCustomCharacterManager::loadCharactersIntoMememory() {
	//load magic
	{
		DisaDataMagicInfo::steal();
		const unsigned int originalMagicSize = DisaDataMagicInfo::getSize();
		for (auto& mod : m_mods) {
			for (auto& m : mod.magics) {
				if (!m.identifier) continue;

				if (auto findIt = m_magicMapper.find(m.identifier.getIdentifier()); findIt != m_magicMapper.end()) {
					m.magicData.id = findIt->second + PG_DisaIdentifier::customMagicIdStart;
				}
				else if (m.identifier.getNamespace() == "disa") {
					m.magicData.id = m.identifier.getNameAsNumber();
				}

				if (PG_DisaIdentifier::isCustomMagicId(m.magicData.id)) {
					reinterpret_cast<PG_DisaDataMagic&>(DisaDataMagicInfo::addNewEntry()) = m.magicData;
				}
				else if (auto findIt = std::ranges::find(DisaDataMagicInfo::begin(), DisaDataMagicInfo::end(), m.magicData.id, &PG_DisaDataMagic::id);
					findIt != DisaDataMagicInfo::end()) {
					reinterpret_cast<PG_DisaDataMagic&>(*findIt) = m.magicData;
				} else {
					OUT_ERROR("Failed to replace magic/skill with identifier \""<< m.identifier <<"\"! Entry not found!");
					m.identifier.splitAt = 0;
				}
			}
		}
	}

	//load characters
	{
		m_characters.clear();
		m_charactersMapper.clear();
		DisaDataCharacterInfo::steal();
		const unsigned int originalCharactersSize = DisaDataCharacterInfo::getSize();

		for (auto& mod : m_mods) {
			for (auto& c : mod.characters) {
				if (!c.identifier || !c.spriteIdentifier) continue;

				if (auto findIt = m_spritesMapper.find(c.spriteIdentifier.getIdentifier()); findIt != m_spritesMapper.end()) {
					c.charData.spriteId = findIt->second + PG_DisaIdentifier::customIdStart;
				}
				else if (c.spriteIdentifier.getNamespace() == "disa") {
					c.charData.spriteId = c.spriteIdentifier.getNameAsNumber();
				}
				else continue;

				if (c.charData.spriteId == 0) continue;
				auto [added, index] = addInfo(Info{ &mod, c.identifier }, m_characters, m_charactersMapper);

				assert(c.skills.size() <= c.charData.skillIds.size());
				if (!c.skills.empty()) {
					c.charData.skillIds.fill(0);
					c.charData.skillLevels.fill(0);
					int i = 0;
					for (const auto& skill : c.skills) {
						if (!skill.first.isValid()) continue;

						if (skill.first.isCustom()) {
							auto findIt = m_magicMapper.find(skill.first);
							if (findIt == m_magicMapper.end()) continue;
							c.charData.skillIds[i] = findIt->second + PG_DisaIdentifier::customMagicIdStart;
						}
						else {
							c.charData.skillIds[i] = skill.first.getNameAsNumber();
						}
						c.charData.skillLevels[i] = skill.second;
						++i;
					}
				}

				if (c.identifier.isCustom()) {
					c.charData.characterId = index + PG_DisaIdentifier::customIdStart;
					c.charDataIndex = index + originalCharactersSize;
					if (added) {
						reinterpret_cast<PG_DisaDataCharacter&>(DisaDataCharacterInfo::addNewEntry()) = c.charData;
					}
					else {
						reinterpret_cast<PG_DisaDataCharacter&>(DisaDataCharacterInfo::getEntry(index + originalCharactersSize)) = c.charData;
					}
				}
				else {
					c.charData.characterId = c.identifier.getNameAsNumber();
					if (auto findIt = std::ranges::find(DisaDataCharacterInfo::begin(), DisaDataCharacterInfo::end(), c.charData.characterId, &PG_DisaDataCharacter::characterId);
						findIt != DisaDataCharacterInfo::end()) {
						c.charDataIndex = std::distance(DisaDataCharacterInfo::begin(), findIt);
						reinterpret_cast<PG_DisaDataCharacter&>(*findIt) = c.charData;
					}
					else {
						OUT_ERROR("Failed to replace character with identifier \"" << c.identifier << "\"! Entry not found!");
					}
				}
			}
		} //for loop
	}//load characters end
}



DisaCustomCharacterManager* DisaCustomCharacterManager::get() {
	static DisaCustomCharacterManager manager(DisaCustomBaseModManager::get()->getScriptsModel());
	return &manager;
}
