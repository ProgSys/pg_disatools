#include "PG_DisaModSpritesModel.h"
#include <Disa/fileformats/PG_JsonUtil.h>
#include <Disa/utils/PG_DisaModInfo.h>
#include <Disa/utils/PG_DisaModLoadInfo.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamInOutByteVector.h>
#include <Util/Debug/PG_DebugOut.h>

using namespace PG_JsonUtil;

void PG_DisaModSpritesModel::load(PG_DisaModInfo* modInfo, const PG_DisaModLoadInfo& loadInfo) {
	auto findIt = loadInfo.json.find("sprites");
	if (findIt == loadInfo.json.end()) return;

	for (auto& jSprite : *findIt) {
		SpriteInfo info{ modInfo };

		if (!readFromJsonMandatory(jSprite, "identifier", info.identifier.value) || !info.identifier.build(modInfo->namespaceMod)) {
			OUT_ERROR("'identifier' missing or is invalid for sprite in \"" << loadInfo.file << "\"!");
			continue;
		}

		if (!readFromJsonMandatory(jSprite, "file", info.file)) {
			OUT_ERROR("'file' missing for sprite in \"" << loadInfo.file << "\"!");
			continue;
		}
		info.file = std::string("sprites/") + info.file.getPath();

		m_sprites.add(std::move(info));
	}
}

unsigned short PG_DisaModSpritesModel::getSpriteId(const PG_DisaIdentifier& identifier) const {
	constexpr unsigned short prinnySpriteId = 2210;
	if (!identifier) return 0;
	if (unsigned short id = findSpriteId(identifier)) return id;
	return prinnySpriteId;
}

const PG_DisaModSpritesModel::SpriteInfo& PG_DisaModSpritesModel::getInfoOrReplacement(unsigned int spriteId) const {
	if (PG_DisaIdentifier::isCustomId(spriteId) && m_sprites.containsId(spriteId)) return m_sprites.getById(spriteId);

	//a mod may replace an original sprite, without one the identifier still allows a loose file
	static SpriteInfo tempInfo;
	tempInfo.identifier = "disa::" + std::to_string(spriteId);
	if (const SpriteInfo* info = m_sprites.find(tempInfo.identifier)) return *info;
	return tempInfo;
}

std::pair<PG::STREAM::In*, const PG_DisaModSpritesModel::SpriteInfo&> PG_DisaModSpritesModel::createSpriteReader(const std::string& baseDir, unsigned int spriteId) const {
	const SpriteInfo& info = getInfoOrReplacement(spriteId);

	if (info.identifier) {
		PG_File file = baseDir + "mod/CustomSprite/" + info.identifier.getNamespace() + "/" + info.identifier.getName() + ".sh";
		auto* reader = new PG::STREAM::InByteFile(file);
		if (reader->isOpen()) {
			OUT_INFO("Found replacement sprite for \"" << info.identifier.getIdentifier() << "\" at \"" << file << "\".");
			return { reader, info };
		}
		delete reader;
	}

	if (info.modInfo) {
		if (auto handle = info.modInfo->open(info.file.getPath())) {
			return { new PG::STREAM::InOutByteVector(handle.readAll()), info };
		}
		OUT_ERROR("Failed to read '" << info.file << "' from zip '" << info.modInfo->zipFile << "'!");
	}

	return { nullptr, info };
}
