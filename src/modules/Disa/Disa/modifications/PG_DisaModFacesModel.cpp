#include "PG_DisaModFacesModel.h"
#include <Disa/fileformats/PG_JsonUtil.h>
#include <Disa/utils/PG_DisaModInfo.h>
#include <Disa/utils/PG_DisaModLoadInfo.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamInOutByteVector.h>
#include <Util/Debug/PG_DebugOut.h>

using namespace PG_JsonUtil;

void PG_DisaModFacesModel::load(PG_DisaModInfo* modInfo, const PG_DisaModLoadInfo& loadInfo) {
	auto findIt = loadInfo.json.find("faces");
	if (findIt == loadInfo.json.end()) return;

	for (auto& jFace : *findIt) {
		FaceInfo info{ modInfo };

		if (!readFromJsonMandatory(jFace, "identifier", info.identifier.value) || !info.identifier.build(modInfo->namespaceMod)) {
			OUT_ERROR("'identifier' missing or is invalid for face in \"" << loadInfo.file << "\"!");
			continue;
		}

		readFromJson(jFace, "fileHD", info.fileHD);
		readFromJson(jFace, "fileSD", info.fileSD);

		if (info.fileHD.isEmpty()) {
			OUT_ERROR("'fileHD' missing for face in \"" << loadInfo.file << "\"! An HD image must be provided!");
			continue;
		}

		info.fileHD = std::string("img/") + info.fileHD.getPath();
		if (!info.fileSD.isEmpty()) info.fileSD = std::string("img/") + info.fileSD.getPath();

		m_faces.add(std::move(info));
	}
}

const PG_DisaModFacesModel::FaceInfo& PG_DisaModFacesModel::getInfoOrReplacement(unsigned int faceId) const {
	if (PG_DisaIdentifier::isCustomId(faceId) && m_faces.containsId(faceId)) return m_faces.getById(faceId);

	//a mod may replace an original face, without one the identifier still allows a loose file
	static FaceInfo tempInfo;
	tempInfo.identifier = "disa::" + std::to_string(faceId);
	if (const FaceInfo* info = m_faces.find(tempInfo.identifier)) return *info;
	return tempInfo;
}

std::pair<PG::STREAM::In*, const PG_DisaModFacesModel::FaceInfo&> PG_DisaModFacesModel::createFaceReader(const std::string& baseDir, unsigned int faceId, bool isHD) const {
	const FaceInfo& info = getInfoOrReplacement(faceId);

	if (info.identifier) {
		PG_File file = baseDir + "mod/CustomImg/" + info.identifier.getNamespace() + "/" + info.identifier.getName() + (isHD ? "_HD" : "_SD") + ".tx2";
		auto* reader = new PG::STREAM::InByteFile(file);
		if (reader->isOpen()) {
			OUT_INFO("Found replacement face for \"" << info.identifier.getIdentifier() << "\" at \"" << file << "\".");
			return { reader, info };
		}
		delete reader;
	}

	if (info.modInfo) {
		const PG_File& target = isHD && !info.fileHD.isEmpty() ? info.fileHD : info.fileSD;
		if (!target.isEmpty()) {
			if (auto handle = info.modInfo->open(target.getPath())) {
				return { new PG::STREAM::InOutByteVector(handle.readAll()), info };
			}
			OUT_ERROR("Failed to read '" << target << "' from zip '" << info.modInfo->zipFile << "'!");
		}
	}

	return { nullptr, info };
}
