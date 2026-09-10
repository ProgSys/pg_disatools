#pragma once

#include <Models/PG_UiStateBaseModel.h>
#include <Disa/PG_DisaMapModel.h>

/**
* @brief A map container that the editors ui state stack can hold
* @details Everything a map container does lives in PG_DisaMPDContainer. This only names the ui state
* the editor should be in while this map is the current one.
*/
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
};
