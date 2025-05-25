#include "PG_DisaDataDatModel.h"
#include <Models/PG_CategoryFilterModel.h>
#include <Models/PG_DisaMapContainerModel.h>
#include <PG_DisaMainModel.h>
#include <Disa/fileformats/PG_DisaMPDPack.h>
#include <Scene/PG_CameraNode.h>

void PG_DisaDataDatModel::setSubUiState(SubUiState state) {
	if (m_subUiState == state) return;
	m_subUiState = state;

	updateFilter();
	emit subUiStateChanged();
}


std::vector<PG_DisaDataDatModel::ModeIcon> PG_DisaDataDatModel::getSubModeIcons() const {
	return {
		{SubUiState_Dungeons, "Dungeons", "gui/icon_actor.png"},
		{SubUiState_Images, "Images", "gui/icon_texture.png"},
	};
}

void PG_DisaDataDatModel::openUi() {
	if (!m_filterModel) {
		m_filterModel = PG_new<PG_CategoryFilterModel>("PG_CategoryFilterModel", this);
		m_filterModel->setModel(this);
	}
	updateFilter();
}

void PG_DisaDataDatModel::closeUi() {
	if (!m_filterModel) return;
	delete m_filterModel;
	m_filterModel = nullptr;
}

void PG_DisaDataDatModel::tileClicked(PG_ListModelLocation location) {
	auto var = location.getData(Identity_PreviewImage);
	if (var.is<PG_DisaMPDPack>()) {
		auto pack = var.get<PG_DisaMPDPack>();
		PG_DisaMainModel* mainModel = PG_DisaMainModel::get();
		mainModel->pushStateStack(PG_DisaMapContainerModel::open(pack.getMPD(), pack.getMPPSD(), pack.getMPPHD()));
		return;
	}
}

void PG_DisaDataDatModel::tileClicked(PG_ListModelLocation location, PG_CameraNode* camera) {
	auto var = location.getData(Identity_PreviewImage);
	if (var.is<PG_DisaMPDPack>()) {
		auto pack = var.get<PG_DisaMPDPack>();
		PG_DisaMainModel* mainModel = PG_DisaMainModel::get();
		auto model = PG_DisaMapContainerModel::open(pack.getMPD(), pack.getMPPSD(), pack.getMPPHD());
		if(camera) model->setCameraStartTransformation(camera->getTransformation());
		mainModel->pushStateStack(model);
		return;
	}
}

void PG_DisaDataDatModel::updateFilter() {
	if (!m_filterModel) return;
	switch (m_subUiState)
	{
	default:
	case PG_DisaDataDatModel::SubUiState_Dungeons:
		m_filterModel->setFilterCategory(PG_CategoryFilterModel::Dungeons);
		break;
	case PG_DisaDataDatModel::SubUiState_Images:
		m_filterModel->setFilterCategory(PG_CategoryFilterModel::Image);
		break;
	}
}