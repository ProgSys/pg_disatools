#include "DisaCustomModificationsManager.h"
#include <Managers/DisaCustomBaseModManager.h>
#include <Disa/modifications/PG_DisaModSpritesModel.h>
#include <Disa/modifications/PG_DisaModFacesModel.h>
#include <Disa/modifications/PG_DisaModMagicModel.h>
#include <Disa/modifications/PG_DisaModCharactersModel.h>
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

char* DisaCustomModificationsManager::getSpriteData(unsigned int spriteId){
	//do we have data loaded?
	if (auto findSpriteIt = m_spriteData.find(spriteId); findSpriteIt != m_spriteData.end()) {
		if (findSpriteIt->second.size() < sizeof(DisaSpriteLoaderInfo)) return nullptr;
		DisaSpriteLoaderInfo& loader = *(DisaSpriteLoaderInfo*)findSpriteIt->second.data();
		return loader.data;
	}

	std::vector<char> data;
	auto [reader, info] = getSpritesModel()->createSpriteReader(DisaCustomBaseModManager::exePath(), spriteId);

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

static inline std::vector<char> buildFaceData(PG::STREAM::In* reader, unsigned int faceId, const PG_DisaModFacesModel::FaceInfo& info, bool isHD) {
	constexpr unsigned int rawDataHeaderSize = sizeof(unsigned int) * 2 + sizeof(short);
	std::vector<char> data;
	unsigned int fileSize = reader->size();
	data.resize(sizeof(DisaCustomModificationsManager::FaceHeader) + fileSize);

	DisaCustomModificationsManager::FaceHeader& loader = *(DisaCustomModificationsManager::FaceHeader*)data.data();
	memset(&loader, 0, sizeof(DisaCustomModificationsManager::FaceHeader));
	if (info.fileHD.isEmpty()) loader.isHD = false;
	if (info.fileSD.isEmpty()) loader.isHD = true;
	else loader.isHD = isHD;
	loader.isOnlyOneImage = info.fileHD.isEmpty() || info.fileSD.isEmpty();

	char*& target = *((char**)&loader.header[12 * 4]);
	target = data.data() + sizeof(DisaCustomModificationsManager::FaceHeader);
	reader->read(target, fileSize);


	FP::callFastFarA<void>(0x004e2810, loader.header.data());

	return data;
}


DisaCustomModificationsManager::FaceHeader* DisaCustomModificationsManager::getFaceData(unsigned int faceId, bool isHD) {
	//do we have data loaded?
	if (auto findIt = m_faceData.find(faceId); findIt != m_faceData.end()) {
		if (findIt->second.size() < sizeof(FaceHeader)) return nullptr;
		auto* loader = (FaceHeader*)findIt->second.data();
		if (loader->isOnlyOneImage || loader->isHD == isHD) return loader;
	}

	std::vector<char> data;
	auto [reader, info] = getFacesModel()->createFaceReader(DisaCustomBaseModManager::exePath(), faceId, isHD);

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

void DisaCustomModificationsManager::addCharacterToParty(const PG_DisaIdentifier& identifier, unsigned short level, int8_t mentorIndex) const{
	if (!identifier.isValid()) return;
	level = glm::clamp<unsigned short>(level, 1, 9999);

	const PG_DisaDataCharacter* newCharData = nullptr;
	PG_DisaCustomModificationsModel::DisplayInfo displayInfo;

	if (identifier.isCustom()) {
		const auto* character = getCharactersModel()->findCharacter(identifier);
		if (!character) {
			OUT_ERROR("Failed to find custom character with identifier \"" << identifier.getIdentifier() << "\"!");
			return;
		}

		newCharData = &character->charData;
		displayInfo = getDisplayInfo(identifier);
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
		displayInfo.charDataIndex = std::distance(DisaDataCharacterInfo::begin(), findIt);
	}



	if (!newCharData || displayInfo.characterId == 0 || displayInfo.charDataIndex == 0) {
		OUT_ERROR("The character \"" << identifier.getIdentifier() << "\" has invalid data! (characterId or charDataIndex is 0)");
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
	newChar.charDatIndex = displayInfo.charDataIndex;

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

void DisaCustomModificationsManager::recordEnemyKilled(DisaPartyCharacterInfo* characterInfo) {
	if (!characterInfo || !PG_DisaIdentifier::isCustomId(characterInfo->characterId)) return;

	const auto* character = getCharactersModel()->findCharacter(characterInfo->characterId);
	if (!character) {
		assert(false);
		return;
	}

	//can unit even be recruited
	using CharacterInfo = PG_DisaModCharactersModel::CharacterInfo;
	if (character->recruitmentType == CharacterInfo::RecruitmentType_None ||
		character->recruitmentType == CharacterInfo::RecruitmentType_Normal) return;

	//add to history
	auto [it, idAdded] = m_assemblyHistory.try_emplace(character->identifier);
	if (idAdded) it->second.clear(); //needed as we cant use construct
	it->second.characterId = characterInfo->characterId;
	if (it->second.killCount != std::numeric_limits<unsigned short>::max()) {
		++it->second.killCount;
	}
	it->second.killCountLevelSum = std::min(it->second.killCountLevelSum + characterInfo->level, DisaCustomModificationsManager::maxKillLevelSum);
	m_isHistoryChanged = true;
}

void DisaCustomModificationsManager::loadModifications() {
	PG_DisaCustomModificationsModel::loadModifications(DisaCustomBaseModManager::exePath());
	loadMagicIntoMemory();
	loadCharactersIntoMemory();
	m_isHistoryChanged = true;
}

void DisaCustomModificationsManager::waitForModThread() {
	std::unique_lock<std::mutex> lock(m_mtx);
	m_cv.wait(lock, [this] { return !m_paused; });
}

void DisaCustomModificationsManager::wakeUp() {
	std::unique_lock<std::mutex> lock(m_mtx);
	m_paused = false;
	m_cv.notify_all();
}

void DisaCustomModificationsManager::loadMagicIntoMemory() {
	DisaDataMagicInfo::steal();
	PG_DisaModMagicModel& magicModel = *getMagicModel();
	for (auto& m : magicModel) {
		if (!m.identifier) continue;

		if (unsigned int id = magicModel.getMagicId(m.identifier)) {
			m.magicData.id = id;
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
			m.scriptIdentifier.clear();
		}
	}
}

void DisaCustomModificationsManager::loadCharactersIntoMemory() {
	PG_DisaModCharactersModel& charactersModel = *getCharactersModel();
	const auto registrations = charactersModel.resolve(getModifications(), *getSpritesModel(), *getMagicModel());

	DisaDataCharacterInfo::steal();
	const unsigned int originalCharactersSize = DisaDataCharacterInfo::getSize();

	for (const auto& registration : registrations) {
		auto& c = charactersModel.getCharacters()[registration.characterIndex];

		if (c.identifier.isCustom()) {
			const unsigned int entryIndex = registration.registeredIndex + originalCharactersSize;
			c.charDataIndex = entryIndex;
			if (registration.added) {
				reinterpret_cast<PG_DisaDataCharacter&>(DisaDataCharacterInfo::addNewEntry()) = c.charData;
			}
			else {
				reinterpret_cast<PG_DisaDataCharacter&>(DisaDataCharacterInfo::getEntry(entryIndex)) = c.charData;
			}
		}
		else if (auto findIt = std::ranges::find(DisaDataCharacterInfo::begin(), DisaDataCharacterInfo::end(), c.charData.characterId, &PG_DisaDataCharacter::characterId);
			findIt != DisaDataCharacterInfo::end()) {
			c.charDataIndex = std::distance(DisaDataCharacterInfo::begin(), findIt);
			reinterpret_cast<PG_DisaDataCharacter&>(*findIt) = c.charData;
		}
		else {
			OUT_ERROR("Failed to replace character with identifier \"" << c.identifier << "\"! Entry not found!");
		}
	}
}



DisaCustomModificationsManager* DisaCustomModificationsManager::get() {
	static DisaCustomModificationsManager manager(DisaCustomBaseModManager::get()->getScriptsModel(), DisaCustomBaseModManager::get()->getTranslationModel());
	return &manager;
}
