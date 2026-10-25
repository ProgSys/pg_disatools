#include "PG_DisaMainModel.h"

#include <algorithm>
#include <cassert>
#include <vector>
#include <Disa/fileformats/PG_DisaFileContainer.h>
#include <Disa/resources/PG_DisaModel.h>
#include <Util/Debug/PG_DebugOut.h>

static PG_DisaMainBaseModel* g_baseMainModel = nullptr;

/**
* @brief Holds some globaly cashed resourses
*/
struct PG_DisaModelResources: public PG_Object {
	using PG_Object::PG_Object;

	std::vector<PG_DisaModel> geometryCache;
};

PG_DisaMainBaseModel::PG_DisaMainBaseModel() {
	g_baseMainModel = this;
}

void PG_DisaMainModel::init() {}

PG_DisaMainBaseModel* PG_DisaMainBaseModel::get() {
	return g_baseMainModel;
}


PG_Dir PG_DisaMainBaseModel::getModDir() const {
	auto exePath = getExePath();
	return exePath.getDir() + "/mod";
}
PG_Dir PG_DisaMainBaseModel::getCustomCampaignsDir() const {
	auto exePath = getExePath();
	return exePath.getDir() + "/mod/CustomCampaigns";
}

PG_Dir PG_DisaMainBaseModel::getCustomModificationsDir() const {
	return customModificationsDir(getExePath());
}

PG_Dir PG_DisaMainBaseModel::customModificationsDir(const PG_File& exePath) {
	return exePath.getDir() + "/mod/CustomModifications";
}

PG_Dir PG_DisaMainBaseModel::getCustomScripsDir() const {
	auto exePath = getExePath();
	return exePath.getDir() + "/mod/CustomScript";
}

PG_Dir PG_DisaMainBaseModel::getCustomTalkScripsDir() const {
	auto exePath = getExePath();
	return exePath.getDir() + "/mod/CustomTalkScript";
}

PG_Dir PG_DisaMainBaseModel::getCustomTranslationDir() const {
	auto exePath = getExePath();
	return exePath.getDir() + "/mod/CustomTranslation";
}

PG_File PG_DisaMainModel::getExePath() const {
	return m_config.getDisaExePath();
}

bool PG_DisaMainModel::cachDisaGeometry(PG_DisaFileContainer* container, const std::string& fileName, const std::string& resourceName) {
	assert(container);
	PG_DisaModelResources* cacheObj = findFirstChild<PG_DisaModelResources>("PG_DisaModelResources");
	if (!cacheObj) {
		cacheObj = PG_new<PG_DisaModelResources>("PG_DisaModelResources", this);
	}

	auto& cache = cacheObj->geometryCache;
	const std::string name = resourceName.empty() ? "R_" + fileName : resourceName;
	if (std::ranges::find(cache, name, &PG_Model::getName) != cache.end()) return SUCCESS;

	auto entry = container->find(PG_File(fileName), false);
	if (!entry) {
		OUT_ERROR("Failed to find \"" << fileName << "\"!");
		return FAILURE;
	}

	PG_GEO geo;
	auto reader = entry.createReader();
	if (!reader || !geo.open(reader.get())) {
		OUT_ERROR("Failed to read \"" << fileName << "\" as a GEO!");
		return FAILURE;
	}

	cache.emplace_back().load(name, geo);
	return SUCCESS;
}
