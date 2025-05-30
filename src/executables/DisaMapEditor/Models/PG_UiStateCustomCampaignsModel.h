#pragma once

#include <Disa/fileformats/PG_ZIP.h>
#include <Disa/models/PG_CustomCampaignsModel.h>
#include <Models/PG_UiStateBaseModel.h>
#include <Resources/PG_Texture.h>
#include <Util/PG_ValueSetter.h>
#include <Scene/Model/PG_ListModel.h>
#include <nlohmann/json_fwd.hpp>

class PG_CategoryFilterModel;

class PG_UiStateCustomCampaignsModel : public PG_CustomCampaignsModel, public PG_UiStateBaseModel {
	PG_OBJECT(PG_UiStateCustomCampaignsModel);
	PG_PARENT(PG_CustomCampaignsModel);
	PG_PROPERTY(int subUiState GET getSubUiStateInt SET setSubUiStateInt CHANGED subUiStateChanged);
signals:
	void uiStateChanged();
	void subUiStateChanged();
public:
	enum SubUiState: int {
		SubUiState_Settings,
		SubUiState_DungeonsPlan,
		SubUiState_Dungeons,
		SubUiState_Images,
		SubUiState_Scripts,
		SubUiState_SIZE,
	};

	using PG_CustomCampaignsModel::PG_CustomCampaignsModel;


	UiState getUiState() const final;
	PG_ListModelBase* getGridModel() const final { return (PG_ListModelBase*)m_filterModel; }

	SubUiState getSubUiState() const { return m_subUiState; }
	void setSubUiState(SubUiState state);

	int getSubUiStateInt() const { return static_cast<int>(getSubUiState()); }
	void setSubUiStateInt(int state) { setSubUiState(static_cast<SubUiState>(glm::clamp(state, 0, (int)SubUiState_SIZE - 1))); }

	std::vector<ModeIcon> getSubModeIcons() const final;

receivers:
	void tileClicked(PG_ListModelLocation location);
	void tileClicked(PG_ListModelLocation location, PG_CameraNode* camera);

	void openUi();
	void closeUi();

private:
	void updateFilter();

	SubUiState m_subUiState = SubUiState_Dungeons;
	PG_CategoryFilterModel* m_filterModel = nullptr;
};