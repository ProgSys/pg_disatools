#pragma once

#include <Models/PG_UiStateBaseModel.h>
#include <Disa/PG_DisaMapModel.h>
#include <Util/PG_Transformation.h>

class PG_DisaMapContainerModel : public PG_DisaMPDContainer, public PG_UiStateBaseModel {
	PG_OBJECT(PG_DisaMapContainerModel);
	PG_PARENT(PG_DisaMPDContainer);
public:
	using PG_DisaMPDContainer::PG_DisaMPDContainer;
	using PG_DisaMPDContainer::open;
	using PG_DisaMPDContainer::save;
	using PG_DisaMPDContainer::createReader;
	using PG_DisaMPDContainer::createReaderOriginal;
	using PG_DisaMPDContainer::getModel;

	UiState getUiState() const final { return UiState::MapView; };

	const PG_Transformation& getCameraStartTransformation() const { return m_cameraStartTransform; }
	void setCameraStartTransformation(const PG_Transformation& t) { m_cameraStartTransform = t; }

	static std::shared_ptr<PG_DisaMapContainerModel> open(PG_DisaFileEntry mpd, PG_DisaFileEntry mppSD, PG_DisaFileEntry mppHD);
	static std::shared_ptr<PG_DisaMapContainerModel> open(PG_DisaFileEntry mpp, int topIndex = 1, int sideIndex = 2, int decalIndex = 0);

private:
	PG_Transformation m_cameraStartTransform;
};
