#include "PG_DisaMainModel.h"

static PG_DisaMainModel* g_baseMainModel = nullptr;

void PG_DisaMainModel::init() {
	m_config.open();
	g_baseMainModel = this;
}

PG_DisaMainModel* PG_DisaMainModel::getBase() {
	return g_baseMainModel;
}

PG_File PG_DisaMainModel::getExePath() const {
	return m_config.getDisaExePath();
}

PG_Dir PG_DisaMainModel::getModDir() const {
	auto exePath = getExePath();
	return exePath.getDir() + "/mod";
}
PG_Dir PG_DisaMainModel::getCustomCampaignsDir() const {
	auto exePath = getExePath();
	return exePath.getDir() + "/mod/CustomCampaigns";
}

PG_Dir PG_DisaMainModel::getCustomCharactersDir() const {
	auto exePath = getExePath();
	return exePath.getDir() + "/mod/CustomCharacters";
}