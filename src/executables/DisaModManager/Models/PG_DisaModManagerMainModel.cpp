#include "PG_DisaModManagerMainModel.h"
#include <Models/PG_DisaModCampaignsListModel.h>
#include <Models/PG_DisaModModificationsListModel.h>
#include <Disa/fileformats/PG_ZIP.h>

static PG_DisaModManagerMainModel* g_mainModel = nullptr;

void PG_DisaModManagerMainModel::init() {
	PG_DisaMainModel::init();
	g_mainModel = this;
}

PG_DisaModManagerMainModel* PG_DisaModManagerMainModel::get() {
	return g_mainModel;
}

void PG_DisaModManagerMainModel::setUiState(EUiState state) {
	if (m_state == state) return;
	if (m_centerModel) m_centerModel->deleteLater();
	switch (state)
	{
	case CustomCampaigns: {
		PG_DisaModCampaignsListModel* model = PG_new<PG_DisaModCampaignsListModel>("PG_DisaModCampaignsListModel", this);
		model->reload();
		m_centerModel = model;
	} break;
	case CustomModifications: {
		PG_DisaModModificationsListModel* model = PG_new<PG_DisaModModificationsListModel>("PG_DisaModModificationsListModel", this);
		model->reload();
		m_centerModel = model;
	} break;
	default:
		m_centerModel = nullptr;
		break;
	}

	emit uiStateChanged(state);
	emit centerModelChanged(m_centerModel);
}

std::string PG_DisaModManagerMainModel::importZip(const PG_File& zipFile) {
	{
		PG_ZIP zip;
		if (!zip.open(zipFile)) {
			return "Failed to read zip file!";
		}

		auto findModIt = zip.find(PG_File("mod.json"));
		auto findCampIt = zip.find(PG_File("campaign.json"));

		if (!findCampIt && !findModIt) {
			return "No definition json found!";
		}

		if (findCampIt && findModIt) {
			return "There can only be one definition json!";
		}

	
		if (findModIt) {
			setUiState(CustomModifications);
		}else if (findCampIt) {
			setUiState(CustomCampaigns);
		}
	}

	auto center = dynamic_cast<PG_DisaModListModel*>(m_centerModel);
	if (!center) return "Invalid ui state!";
	try
	{
		center->addZip(zipFile);
	}
	catch (const PG_Exception& ex)
	{
		return ex.what();
	}
	return {};
}