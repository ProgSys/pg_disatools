#include "PG_DisaMainModel.h"

static PG_DisaMainBaseModel* g_baseMainModel = nullptr;

PG_DisaMainBaseModel::PG_DisaMainBaseModel() {
	g_baseMainModel = this;
}

void PG_DisaMainModel::init() {
	m_config.open();
	
}

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

PG_Dir PG_DisaMainBaseModel::getCustomCharactersDir() const {
	auto exePath = getExePath();
	return exePath.getDir() + "/mod/CustomCharacters";
}

PG_Dir PG_DisaMainBaseModel::getCustomScripsDir() const {
	auto exePath = getExePath();
	return exePath.getDir() + "/mod/CustomScript";
}

PG_Dir PG_DisaMainBaseModel::getCustomTalkScripsDir() const {
	auto exePath = getExePath();
	return exePath.getDir() + "/mod/CustomTalkScript";
}

PG_File PG_DisaMainModel::getExePath() const {
	return m_config.getDisaExePath();
}
