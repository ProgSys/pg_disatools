#include "PG_DisaCharactersModel.h"
#include <Disa/fileformats/PG_OLA.h>
#include <Disa/fileformats/PG_SOLA.h>
#include <Disa/fileformats/PG_TableDAT.h>
#include <Disa/fileformats/PG_PSPFS.h>
#include <Resources/PG_ResourceManager.h>
#include <Util/PG_StringUtil.h>
#include <Stream/PG_StreamInOutByteVector.h>
#include <Stream/PG_StreamInOutByteArray.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamInOutByteVector.h>


static PG_DisaCharactersModel* g_charactersModel = nullptr;
static PG_DisaCharacterModel g_emptyCharacterModel = {
	static_cast<unsigned short>(-1),
	"Missingno",
	"Missingno",
};

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
	assert(!g_charactersModel);
	g_charactersModel = this;
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
		loadModifications(exePath);
		for (auto& mod : m_modifications) {
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
	return g_charactersModel;
}