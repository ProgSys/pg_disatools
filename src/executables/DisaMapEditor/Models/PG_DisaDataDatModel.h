#pragma once

#include <Disa/fileformats/PG_PSPFS.h>
#include <Disa/fileformats/PG_DisaMPDPack.h>
#include <Models/PG_UiStateBaseModel.h>

class PG_CategoryFilterModel;
class PG_CameraNode;

class PG_DisaDataDatModel : public PG_PSPFS, public PG_UiStateBaseModel {
	PG_OBJECT(PG_DisaDataDatModel);
	PG_PARENT(PG_PSPFS);
	PG_PROPERTY(int subUiState GET getSubUiStateInt SET setSubUiStateInt CHANGED subUiStateChanged);
signals:
	void subUiStateChanged();
public:
	enum SubUiState : int {
		SubUiState_Dungeons,
		SubUiState_Images,
		SubUiState_SIZE,
	};

	using PG_PSPFS::PG_PSPFS;

	UiState getUiState() const final { return UiState::GridFileView; };
	PG_ListModelBase* getGridModel() const final { return (PG_ListModelBase*)m_filterModel; }

	SubUiState getSubUiState() const { return m_subUiState; }
	void setSubUiState(SubUiState state);

	int getSubUiStateInt() const { return static_cast<int>(getSubUiState()); }
	void setSubUiStateInt(int state) { setSubUiState(static_cast<SubUiState>(glm::clamp(state, 0, (int)SubUiState_SIZE - 1))); }

	std::vector<ModeIcon> getSubModeIcons() const final;

receivers:
	void openUi();
	void closeUi();
	void tileClicked(PG_ListModelLocation location);
	void tileClicked(PG_ListModelLocation location, PG_CameraNode* camera);

private:
	void updateFilter();

	SubUiState m_subUiState = SubUiState_Dungeons;

	PG_CategoryFilterModel* m_filterModel = nullptr;
};