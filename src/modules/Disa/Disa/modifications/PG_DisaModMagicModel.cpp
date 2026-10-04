#include "PG_DisaModMagicModel.h"
#include <Disa/fileformats/PG_JsonUtil.h>
#include <Disa/utils/PG_DisaModInfo.h>
#include <Disa/utils/PG_DisaModLoadInfo.h>
#include <Util/Debug/PG_DebugOut.h>

using namespace PG_JsonUtil;

void PG_DisaModMagicModel::load(PG_DisaModInfo* modInfo, const PG_DisaModLoadInfo& loadInfo) {
	auto findIt = loadInfo.json.find("magic");
	if (findIt == loadInfo.json.end()) return;

	for (auto& jMagic : *findIt) {
		MagicInfo info{ modInfo };
		if (!readFromJsonMandatory(jMagic, "identifier", info.identifier.value) || !info.identifier.build(modInfo->namespaceMod)) {
			OUT_ERROR("'identifier' missing or is invalid for magic in \"" << loadInfo.file << "\"!");
			continue;
		}

		PG_File file;
		if (!readFromJsonMandatory(jMagic, "file", file)) {
			OUT_ERROR("'file' missing for magic in \"" << loadInfo.file << "\"!");
			continue;
		}

		info.magicData.fromJson(jMagic);
		if (info.magicData.name.empty()) {
			OUT_ERROR("Magic/Skill with identifier \"" << info.identifier.getIdentifier() << "\" has no name in file \"" << loadInfo.file << "\"! Magic must have a name!");
			continue;
		}

		if (info.magicData.description.empty()) {
			OUT_WARNING("Magic/Skill with identifier \"" << info.identifier.getIdentifier() << "\" has no description in file \"" << loadInfo.file << "\"!");
		}

		readFromJson(jMagic, "scriptIdentifier", info.scriptIdentifier.value);
		if (!info.scriptIdentifier.build(modInfo->namespaceMod) && !info.scriptIdentifier.value.empty()) {
			info.scriptIdentifier.clear();
			OUT_WARNING("Invalid 'scriptIdentifier' in magic \"" << info.identifier.getIdentifier() << "\" in file \"" << loadInfo.file << "\"!");
		}

		auto [added, index] = m_magic.add(std::move(info));
		std::next(m_magic.begin(), index)->magicData.id = index;
	}
}

PG_DisaIdentifier PG_DisaModMagicModel::getMagicIdentifier(unsigned int id) const {
	if (PG_DisaIdentifier::isCustomMagicId(id)) return m_magic.getById(id).identifier;
	return PG_DisaIdentifier(std::to_string(id), "disa");
}
