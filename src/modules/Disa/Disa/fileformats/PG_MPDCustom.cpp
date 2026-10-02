#include <Disa/fileformats/PG_MPDCustom.h>
#include <algorithm>
#include <Util/Debug/PG_DebugOut.h>

void PG_MPDCustom::writeModels(PG_DataSerializer& metaData, const ChunkModels& models) {
	if (std::ranges::all_of(models, [](const auto& pair) { return pair.second.empty(); })) return;

	metaData["customModelsVersion"] = modelsVersion;
	PG_DataSerializer chunks = metaData["chunks"];
	for (const auto& [chunkIndex, chunkModels] : models) {
		if (chunkModels.empty()) continue;

		PG_DataSerializer chunk = chunks.emplaceBack();
		chunk["index"] = chunkIndex;
		PG_DataSerializer list = chunk["models"];
		for (const PG_MPDCustomModel& model : chunkModels) {
			PG_DataSerializer entry = list.emplaceBack();
			entry["name"] = model.name;
			entry["x"] = model.object.x;
			entry["y"] = model.object.y;
			entry["z"] = model.object.z;
			entry["xRot"] = model.object.xRot;
			entry["yRot"] = model.object.yRot;
			entry["zRot"] = model.object.zRot;
			entry["xScale"] = model.object.xScale;
			entry["yScale"] = model.object.yScale;
			entry["zScale"] = model.object.zScale;
		}
	}
}

std::vector<PG_MPDCustomModel> PG_MPDCustom::readModels(const PG_DataSerializer& metaData, int chunkIndex) {
	std::vector<PG_MPDCustomModel> out;
	//only the const lookups, a mutable one adds the key it can't find
	const PG_DataSerializer chunks = metaData["chunks"];
	if (!chunks.isValid()) return out;

	if (const int fileVersion = metaData["customModelsVersion"].asInt(); fileVersion > modelsVersion) {
		OUT_WARNING("The custom models are of version " << fileVersion << ", newer than " << modelsVersion << ", they are read as far as known!");
	}

	for (const PG_DataSerializer& chunk : chunks) {
		int index = -1;
		if (!chunk["index"].getTo(index) || index != chunkIndex) continue;

		for (const PG_DataSerializer& entry : chunk["models"]) {
			PG_MPDCustomModel& model = out.emplace_back();
			model.name = entry["name"].asString();
			model.object = mpdfileChunkObject::createEmpty();
			entry["x"].getTo(model.object.x);
			entry["y"].getTo(model.object.y);
			entry["z"].getTo(model.object.z);
			entry["xRot"].getTo(model.object.xRot);
			entry["yRot"].getTo(model.object.yRot);
			entry["zRot"].getTo(model.object.zRot);
			entry["xScale"].getTo(model.object.xScale);
			entry["yScale"].getTo(model.object.yScale);
			entry["zScale"].getTo(model.object.zScale);
			if (model.name.empty()) {
				OUT_WARNING("A custom model of chunk " << chunkIndex << " has no name, it is skipped!");
				out.pop_back();
			}
		}
	}
	return out;
}
