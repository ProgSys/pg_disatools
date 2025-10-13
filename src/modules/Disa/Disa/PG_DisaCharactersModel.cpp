#include "PG_DisaCharactersModel.h"
#include <Disa/fileformats/PG_OLA.h>
#include <Disa/fileformats/PG_SOLA.h>
#include <Disa/fileformats/PG_TableDAT.h>
#include <Disa/fileformats/PG_PSPFS.h>
#include <Disa/fileformats/PG_ZIPUtil.h>
#include <Disa/fileformats/PG_JsonUtil.h>
#include <Disa/models/PG_DisaModLoadOrderListModel.h>
#include <Resources/PG_ResourceManager.h>
#include <Util/PG_StringUtil.h>
#include <Stream/PG_StreamInOutByteVector.h>
#include <Scene/Gui/Validator/PG_GuiTextValidators.h>
#include <Stream/PG_StreamInOutByteArray.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamInOutByteVector.h>

using namespace PG_JsonUtil;

static PG_DisaCustomCharactersModel::Info gEmptyInfo;
static PG_DisaCustomCharactersModel::Info gTempInfo;

static PG_DisaCharactersModel* g_global = nullptr;
static PG_DisaCharacterModel g_emptyCharacterModel = {
	static_cast<unsigned short>(-1),
	"Missingno",
	"Missingno",
};

struct MetaCharInfo {
	PG_File file;
	std::string id;
	nlohmann::json json;

	bool operator==(const MetaCharInfo& other) const { return id == other.id; }
};

void PG_DisaCustomCharactersModel::loadCustomCharacters(const PG_File& exePath) {

	m_charactersMapper.clear();
	m_spritesMapper.clear();
	m_facesMapper.clear();
	m_characters.clear();
	m_sprites.clear();
	m_faces.clear(); //portraits
	m_mods.clear();

	if (exePath.isEmpty() || !exePath.exists()) {
		return;
	}
	PG_Dir customCharactersDir(exePath.getDir() + "/mod/CustomCharacters");
	
	std::vector<MetaCharInfo> metaCharInfos;

	for (const auto& zipFile : customCharactersDir.findFiles([](const PG_File& file)->bool {  return file.getFileExtensionFormated() == "zip"; }, 1)) {
		auto archive = PG_ZIPUtil::openZip(zipFile);
		if (!archive) continue;
		MetaCharInfo info;
		info.file = zipFile;
		try {
			info.json = PG_ZIPUtil::readJson(archive, "character.json");
			if (info.json.empty()) {
				OUT_ERROR("Failed to parse 'character.json' inside zip '" << zipFile << "'!");
				continue;
			}

			if (!readFromJsonMandatory(info.json, "id", info.id)) {
				OUT_ERROR("'character.json' has no 'id' inside zip '" << zipFile << "'!");
				continue;
			}
		}
		catch (const std::exception& ex) {
			OUT_ERROR("Failed to parse 'character.json' inside zip '" << zipFile << "'! Reason: " << ex.what());
			continue;
		}
		catch (...) {
			OUT_ERROR("Failed to parse 'character.json' inside zip '" << zipFile << "'!");
			continue;
		}
		metaCharInfos.push_back(std::move(info));
	}

	{ //remove dublicate ids
		std::ranges::sort(metaCharInfos, {}, &MetaCharInfo::id);
		auto range = std::ranges::unique(metaCharInfos);
		metaCharInfos.erase(range.begin(), range.end());
	}

	std::unordered_map<std::string, std::size_t> mapping;
	{ //sort by load order
		PG_DisaModLoadOrderListModel orderModel;
		orderModel.open(customCharactersDir.toFilePath("loadOrder.json"));
		mapping = orderModel.getOrderMapping();
		//load the items on the bottom first
		std::ranges::sort(metaCharInfos, std::ranges::greater{}, [&mapping](const MetaCharInfo& c) {
			auto findIt = mapping.find(c.id);
			return findIt != mapping.end() ? findIt->second : std::numeric_limits<std::size_t>::max();
			});
		std::erase_if(metaCharInfos, [&mapping, &orderModel](const MetaCharInfo& c) {
			auto findIt = mapping.find(c.id);
			return findIt != mapping.end() ? orderModel.get(findIt->second).disabled : false;
			});
	}

	for (const auto& metaCharInfo : metaCharInfos) {
		m_mods.push_back({});
		try {
			loadCharacters(metaCharInfo);
		}
		catch (const std::exception& ex) {
			m_mods.pop_back();
			OUT_ERROR("Failed to parse 'character.json' inside zip '" << metaCharInfo.file << "'! Reason: " << ex.what());
		}
		catch (...) {
			m_mods.pop_back();
			OUT_ERROR("Failed to parse 'character.json' inside zip '" << metaCharInfo.file << "'!");
		}
	}

	//sort mods to be the same order, as load order
	if(!mapping.empty()){
		m_mods.sort([&mapping](const CharacterMod& a, const CharacterMod& b) {
			auto findIt = mapping.find(a.id);
			size_t aOrder = findIt != mapping.end() ? findIt->second : std::numeric_limits<std::size_t>::max();
			findIt = mapping.find(b.id);
			size_t bOrder = findIt != mapping.end() ? findIt->second : std::numeric_limits<std::size_t>::max();
			return aOrder < bOrder;
			});
	}
}

void PG_DisaCustomCharactersModel::loadCharactersFromZip(const PG_File& file) {
	auto archive = PG_ZIPUtil::openZip(file);
	if (!archive) return;

	m_mods.push_back({});


	try {
		MetaCharInfo info;
		info.file = file;
		info.json = PG_ZIPUtil::readJson(archive, "character.json");
		loadCharacters(info);
	}
	catch (const std::exception& ex) {
		m_mods.pop_back();
		OUT_ERROR("Failed to parse 'character.json' inside zip '" << file << "'! Reason: " << ex.what());
	}
	catch (...) {
		m_mods.pop_back();
		OUT_ERROR("Failed to parse 'character.json' inside zip '" << file << "'!");
	}
}

void PG_DisaCustomCharactersModel::loadCharacters(const MetaCharInfo& metaInfo) {
	CharacterMod& mod = m_mods.back();
	auto& j = metaInfo.json;

	if (j.empty()) {
		throw std::exception("Json is empty!");
	}

	if (!readFromJsonMandatory(j, "id", mod.id)) {
		throw std::exception("'id' is missing in 'character.json'!");
	}

	if (!readFromJsonMandatory(j, "name", mod.name)) {
		throw std::exception("'name' is missing in 'character.json'!");
	}

	readFromJson(j, "namesspace", mod.namespaceMod);
	if (mod.namespaceMod.empty()) {
		mod.namespaceMod = metaInfo.file.getName();
	}
	mod.namespaceMod = PG_StringUtil::toLowerCopy(mod.namespaceMod);

	if (!PG_SimpleWordValidator()(mod.namespaceMod) || mod.namespaceMod.empty()) {
		throw std::exception("Invalid namespace format! Either filename or namesspace dosn't match naming convention!");
	}

	mod.zipFile = metaInfo.file;

	if (auto findIt = j.find("magic"); findIt != j.end()) {
		for (auto& jMagic : *findIt) {
			Info info{ &mod };
			if (!readFromJsonMandatory(jMagic, "identifier", info.identifier.value) || !info.identifier.build(mod.namespaceMod)) {
				OUT_ERROR("'identifier' missing or is invalid for spmagicrite in \"" << metaInfo.file << "\"!");
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
				OUT_ERROR("Magic/Skill with identifier \"" << info.identifier << "\" has no name in file \"" << metaInfo.file << "\"! Magic must have a name!");
				continue;
			}

			if (magicInfo.magicData.description.empty()) {
				OUT_WARNING("Magic/Skill with identifier \"" << info.identifier << "\" has no description in file \"" << metaInfo.file << "\"!");
			}

			auto [added, index] = addInfo(std::move(info), m_magics, m_magicsMapper);
			if (!added) {
				OUT_ERROR("Magic/Skill with identifier \"" << info.identifier << "\" is already present \"" << metaInfo.file << "\"!");
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

			readFromJson(jCharacter, "isRecruitable", info.isRecruitable);
			readFromJson(jCharacter, "recruitGroupName", info.recruitGroupName);
			PG_SimpleWordValidator()(info.recruitGroupName);

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
					info.isRecruitable = findIt->isRecruitable;
					info.recruitGroupName = findIt->recruitGroupName;
					info.skills = findIt->skills;
				}
				else {
					OUT_ERROR("Failed to find deriveFrom \"" << deriveFrom << "\" in json!");
				}
			}

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

static inline const PG_DisaCustomCharactersModel::Info& getInfoFallback(unsigned int spriteId,
	const std::vector<PG_DisaCustomCharactersModel::Info>& list, const std::map<std::string, unsigned int>& mapper) {

	const PG_DisaCustomCharactersModel::Info* info = nullptr;

	//loading custom sprite?
	if (PG_DisaIdentifier::isCustomId(spriteId) && (spriteId - PG_DisaIdentifier::customIdStart) < list.size()) {
		return list[spriteId - PG_DisaIdentifier::customIdStart];
	}

	//replacing a default sprite?

	gTempInfo.identifier = "disa::" + std::to_string(spriteId);
	if (auto findIt = mapper.find(gTempInfo.identifier.getIdentifier()); findIt != mapper.end()) {
		return list[findIt->second];
	}

	return gTempInfo;
}

std::pair<PG::STREAM::In*, const PG_DisaCustomCharactersModel::Info&>  PG_DisaCustomCharactersModel::createSpriteReader(const std::string& baseDir, unsigned int spriteId) {
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
		try {
			if (auto archive = PG_ZIPUtil::openZip(info.mod->zipFile)) {
				return { new PG::STREAM::InOutByteVector(PG_ZIPUtil::readData(archive, info.fileInZip)) , info };
			}
		}
		catch (const std::exception& ex) {
			OUT_ERROR("Zip file " << info.mod->zipFile);
			OUT_ERROR(ex.what());
		}

	}

	return { nullptr, info };
}

std::pair<PG::STREAM::In*, const PG_DisaCustomCharactersModel::Info&>  PG_DisaCustomCharactersModel::createFaceReader(const std::string& baseDir, unsigned int faceId, bool isHD) {
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
		try {
			if (auto archive = PG_ZIPUtil::openZip(info.mod->zipFile)) {
				const auto& target = isHD && !info.fileInZip.isEmpty() ? info.fileInZip : info.secondfileInZip;
				if (!target.isEmpty()) {
					return { new PG::STREAM::InOutByteVector(PG_ZIPUtil::readData(archive, target)), info };
				}
			}
		}
		catch (const std::exception& ex) {
			OUT_ERROR("Zip file "<< (isHD && !info.fileInZip.isEmpty() ? info.fileInZip : info.secondfileInZip));
			OUT_ERROR(ex.what());
		}
	}

	return { nullptr, info };
}

const PG_DisaCustomCharactersModel::Info& PG_DisaCustomCharactersModel::getCharacterInfoIndex(unsigned int index) const {
	if (index >= m_characters.size()) return gEmptyInfo;
	return m_characters[index];
}

static unsigned short getId(const std::map<std::string, unsigned int>& mapper, const PG_DisaIdentifier& identifier) {
	if (auto findIt = mapper.find(identifier); findIt != mapper.end()) return findIt->second + PG_DisaIdentifier::customIdStart;
	return 0;
}

PG_DisaCustomCharactersModel::DisplayInfo PG_DisaCustomCharactersModel::getDisplayInfo(const PG_DisaIdentifier& identifier) const {
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

PG_DisaCustomCharactersModel::DisplayInfo PG_DisaCustomCharactersModel::getDisplayInfo(const PG_DisaCustomCharactersModel::CharacterInfo& c) const {
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

const PG_DisaCustomCharactersModel::Info& PG_DisaCustomCharactersModel::getInfo(unsigned int id, const std::vector<PG_DisaCustomCharactersModel::Info>& list) {
	if (!PG_DisaIdentifier::isCustomId(id)) return gEmptyInfo;
	id -= PG_DisaIdentifier::customIdStart;
	if (id >= list.size()) return gEmptyInfo;
	return list[id];
}

const PG_DisaCustomCharactersModel::Info& PG_DisaCustomCharactersModel::getInfo(const std::string& id, const std::vector<Info>& list, const std::map<std::string, unsigned int>& mapper) {
	auto findIt = mapper.find(id);
	if (findIt == mapper.end()) return gEmptyInfo;
	return list[findIt->second];
}

std::pair<bool, unsigned int> PG_DisaCustomCharactersModel::addInfo(PG_DisaCustomCharactersModel::Info&& info, std::vector<Info>& list, std::map<std::string, unsigned int>& mapper) {
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


PG_Variety PG_DisaCharacterModel::getData(Identity identity) const {
	switch (identity)
	{
	case Identity_Id: return id;
	case Identity_Name: return nameNormelized;
	case Identity_Title: return titleNormelized;
	case Identity_Portrait: return portrait;
	}
	return PG_Variety();
}


void PG_DisaCharactersModel::init() {
	assert(!g_global);
	g_global = this;
}

void PG_DisaCharactersModel::open(const PG_File& exePath, std::shared_ptr<PG_PSPFS> subdata, std::shared_ptr<PG_OLA> startDat) {
	m_startDat = startDat;
	m_portraitDat.reset();
	m_spriteMapping.clear();
	m_exePath = exePath;

	if (!startDat) {
		clear();
		return;
	}

	auto entry = startDat->find(PG_File("CHAR_E.DAT"), false);
	if (!entry) {
		OUT_ERROR("Failed to find CHAR_E.DAT!");
		clear();
		return;
	}

	PG_TableDAT tableDat;
	tableDat.setDefFile(PG_File(PG_ResourceManager::resourceLocation() + "/script/disa/defFiles/CHAR.DEF"));
	auto read = entry.createReader();
	if (!tableDat.open(read)) {
		OUT_ERROR("Failed to read CHAR_E.DEF!");
		clear();
		return;
	}

	const int nameIndex = tableDat.getHeaderIndex("Name");
	const int titleIndex = tableDat.getHeaderIndex("Title");
	const int spriteColorOffsetIndex = tableDat.getHeaderIndex("Palette index");
	const int characterIdIndex = tableDat.getHeaderIndex("Character ID");
	const int spriteIdIndex = tableDat.getHeaderIndex("Sprite ID");
	if (nameIndex < 0  || spriteColorOffsetIndex < 0 || characterIdIndex < 0 || spriteIdIndex < 0) {
		OUT_ERROR("Missing needed header data! (Needs: "<< "Name"  <<", " << "Palette index" << ", " << "Character ID" << ", " << "Sprite ID"<< ")");
		clear();
		return;
	}

	auto mapSprites = [this](const std::shared_ptr<PG_DisaFileContainer>& mappingFile, const std::shared_ptr<PG_DisaFileContainer>& targetFile) {
		for (const auto& entry : *mappingFile) {
			auto& extra = entry.getExtraData();
			if (extra.isEmpty() || !extra.is<unsigned short>()) continue;
			if (entry.getName() == "1001_FILE.SH")
				auto i = 0;
			m_spriteMapping[extra.get<unsigned short>()] = targetFile;
		}
	};

	auto spriteDat = std::dynamic_pointer_cast<PG_SOLA>(getReader(startDat, {
		VisitPair{"ANM00.DAT", [](PG_DisaFileEntry& entry) { return entry.open<PG_SOLA>(); } },
		}));
	if (spriteDat) {
		mapSprites(spriteDat, spriteDat);
	}

	{ //to make it extra painfull there are more sprites stored in other packages
		auto anmFiles = subdata->findAll(std::regex("ANM\\d\\d\\.DAT"));
		auto data = std::make_shared<PG::STREAM::InOutByteVector>();
		for (auto& file : anmFiles) {
			if (file.getInfo().isCompressed()) {
				//uncomress and map sprites
				auto ola = file.open<PG_OLA>();
				if (!ola) continue;
				auto sola = std::make_shared<PG_SOLA>();
				{
					data->close();
					if (!ola->uncompress(data.get())) continue;
					data->rewind();
					if (!sola->open(data)) continue;
				}
				mapSprites(sola, ola);
			}
			else {
				auto targetFile = file.open<PG_SOLA>();
				if (!targetFile) continue;
				mapSprites(targetFile, targetFile);
			}
		}

	}

	m_portraitDat = std::dynamic_pointer_cast<PG_SOLA>(getReader(startDat, {
		VisitPair{"NEW_FACE.DAT", [](PG_DisaFileEntry& entry) { return entry.open<PG_SOLA>(); } },
	}));
	if (!m_portraitDat) {
		OUT_ERROR("Failed to load NEW_FACE.DAT!");
		clear();
		m_portraitDat.reset();
		m_spriteMapping.clear();
		return;
	}

	const int max = std::max(std::max(spriteColorOffsetIndex, characterIdIndex), spriteIdIndex);

	std::vector<PG_DisaCharacterModel> models;
	models.reserve(tableDat.size());

	for (const PG_TebleDATRow& row : tableDat) {
		if (max >= row.columns.size()) {
			OUT_ERROR("Invalid column size!");
			break;
		}
		PG_DisaCharacterModel model;
		model.id = row.columns[characterIdIndex].data.convert<decltype(model.id)>();
		model.name = row.columns[nameIndex].data.convert<decltype(model.name)>();
		model.nameNormelized = PG_StringUtil::fullwidthAndHalfwidthUtf8ToNormal(model.name);
		model.title = row.columns[titleIndex].data.convert<decltype(model.title)>();
		model.titleNormelized = PG_StringUtil::fullwidthAndHalfwidthUtf8ToNormal(model.title);
		//OUT_INFO(model.nameNormelized<< " : "<< PG_StringUtil::toHex(model.name));
		model.spriteColorOffset = row.columns[spriteColorOffsetIndex].data.convert<decltype(model.spriteColorOffset)>();
		model.spriteId = row.columns[spriteIdIndex].data.convert<decltype(model.spriteId)>();
		
		models.push_back(std::move(model));
	}

	{//load in custom charcters
		loadCustomCharacters(exePath);
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

				c.charData.characterId = index + PG_DisaIdentifier::customIdStart;

				PG_DisaCharacterModel model;

				model.id = c.charData.characterId;
				model.name = c.charData.name.data();
				model.nameNormelized = c.charData.getName();
				model.title = c.charData.title.data();
				model.titleNormelized = c.charData.getTitle();

				model.spriteColorOffset = c.charData.paletteIndex;
				model.spriteId = c.charData.spriteId;

				models.push_back(std::move(model));
			}
		}
	}

	set(std::move(models));
}


PG_Texture PG_DisaCharactersModel::getPortrait(unsigned short characterId) {
	PG_Texture res;

	auto findIt = std::ranges::find(*this, characterId, &PG_DisaCharacterModel::id);
	if (findIt == end()) return res;
	if (findIt->portrait.isValid()) return findIt->portrait;
	//we need to load it
	res.loadExisting("DisaPortrait" + std::to_string(characterId));
	if (res.isValid()) {
		findIt->portrait = res;
		return res;
	}
	unsigned short faceId = characterId;
	if (PG_DisaIdentifier::isCustomId(faceId)){
		unsigned short charIndex = faceId - PG_DisaIdentifier::customIdStart;
		if (charIndex < m_characters.size()) {
			const auto& modChars = m_characters[charIndex].mod->characters;
			auto findCustomCharIt = std::ranges::find(modChars, characterId, [](const CharacterInfo& i) { return i.charData.characterId; });
			if (findCustomCharIt != modChars.end() && findCustomCharIt->faceIdentifier) {
				faceId = m_facesMapper[findCustomCharIt->faceIdentifier] + PG_DisaIdentifier::customIdStart;
			}
		}
	}
	std::shared_ptr<PG::STREAM::In> reader;
	reader = std::shared_ptr<PG::STREAM::In>(createFaceReader(m_exePath, faceId, true).first);

	//read from game files
	if(!reader && !PG_DisaIdentifier::isCustomId(faceId)){
		auto findPortraitIt = std::ranges::find_if(*m_portraitDat, [id = findIt->spriteId](const PG_DisaFileEntry& entry) {
			auto& extra = entry.getExtraData();
			if (extra.isEmpty() || !extra.is<unsigned short>()) return false;
			return extra.get<unsigned short>() == id;
			});
		if (findPortraitIt == m_portraitDat->end()) return res;
		reader = findPortraitIt->createReader();
	}

	if (!reader) return res;

	PG_TX2 tex;
	if (!tex.open(reader.get())) return res;
	reader.reset();

	res.load("DisaPortrait" + std::to_string(characterId), [tex = std::move(tex)]() -> PG_Texture::ImageFormats {
		PG_RGBAImage out;
		decompressTX2(tex, out);
		return out;
	});
	findIt->portrait = res;
	return res;
}

std::pair<PG_DisaSprite, unsigned short> PG_DisaCharactersModel::getSprite(unsigned short characterId) {
	std::pair<PG_DisaSprite, unsigned short> res;
	res.second = 0;
	unsigned short spriteId = characterId;

	auto findIt = std::ranges::find(*this, characterId, &PG_DisaCharacterModel::id);
	if (findIt != end()) {
		res.second = findIt->spriteColorOffset;
		spriteId = findIt->spriteId;
		if (spriteId > 0 && findIt->sprite.isValid()) {
			res.first = findIt->sprite;
			return res;
		}
		//we need to load it
		res.first.loadExisting("DisaSprite" + std::to_string(characterId));
		if (res.first.isValid()) { //allready present
			findIt->sprite = res.first;
			return res;
		}
		//needs to be loaded from file
	}

	std::shared_ptr<PG::STREAM::In> reader;
	reader = std::shared_ptr<PG::STREAM::In>(createSpriteReader(m_exePath, spriteId).first);

	//read from game files
	if (!reader && !PG_DisaIdentifier::isCustomId(spriteId)) {
		auto findSpritePackIt = m_spriteMapping.find(spriteId);
		if (findSpritePackIt == m_spriteMapping.end()) return res;

		std::shared_ptr<PG_DisaFileContainer> targetContainer;
		if (findSpritePackIt->second->isCompressed()) {
			auto ola = std::dynamic_pointer_cast<PG_OLA>(findSpritePackIt->second);
			assert(ola);

			auto sola = std::make_shared<PG_SOLA>();
			auto data = std::make_shared<PG::STREAM::InOutByteVector>();
			if (!ola->uncompress(data.get())) return res;
			data->rewind();
			if (!sola->open(data)) return res;
			sola->setReader(std::move(data));
			targetContainer = std::move(sola);
		}
		else {
			targetContainer = findSpritePackIt->second;
		}

		auto findSpriteIt = std::ranges::find_if(*targetContainer, [id = spriteId](const PG_DisaFileEntry& entry) {
			auto& extra = entry.getExtraData();
			if (extra.isEmpty() || !extra.is<unsigned short>()) return false;
			return extra.get<unsigned short>() == id;
			});
		if (findSpriteIt == targetContainer->end()) return res;

		reader = findSpriteIt->createReader();
	}

	if (!reader) return res;

	PG_SH sh;
	if (!sh.open(reader.get())) return res;
	reader.reset();

	res.first.load(spriteId, sh);
	if (findIt != end()) findIt->sprite = res.first;
	return res;
}

const PG_DisaCharacterModel& PG_DisaCharactersModel::getCharacterModel(unsigned short characterId) {
	auto findIt = std::ranges::find(*this, characterId, &PG_DisaCharacterModel::id);
	if (findIt == end()) return g_emptyCharacterModel;
	return *findIt;
}


PG_Variety PG_DisaCharactersModel::getData(PG_Index index, Identity identity) const {
	switch (identity)
	{
	case PG_DisaCharacterModel::Identity_Portrait: {
		auto* self = const_cast<PG_DisaCharactersModel*>(this);
		return self->getPortrait(PG_ListModel<PG_DisaCharacterModel>::get(index).id);
	}
	}
	return PG_ListModel<PG_DisaCharacterModel>::getData(index, identity);
}

PG_DisaCharactersModel* PG_DisaCharactersModel::get() {
	return g_global;
}