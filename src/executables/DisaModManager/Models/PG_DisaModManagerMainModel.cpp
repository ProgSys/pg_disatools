#include "PG_DisaModManagerMainModel.h"
#include <Models/PG_DisaModCampaignsListModel.h>
#include <Models/PG_DisaModCharactersListModel.h>

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
	case CustomCharacters: {
		PG_DisaModCharactersListModel* model = PG_new<PG_DisaModCharactersListModel>("PG_DisaModCharactersListModel", this);
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