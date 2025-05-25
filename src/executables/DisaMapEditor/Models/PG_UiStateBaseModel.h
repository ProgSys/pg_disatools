#pragma once
#include <string>
#include <vector>

class PG_ListModelBase;

class PG_UiStateBaseModel {
public:
	enum UiState {
		Empty,
		Overview,
		GridFileView,
		MapView,
		ModelView,
		CustomCampaignSetting,
		CustomCampaignPlan,
	};

	struct ModeIcon{
		int subMode;
		std::string name;
		std::string icon;
	};

	PG_UiStateBaseModel() = default;

	virtual UiState getUiState() const = 0;
	virtual PG_ListModelBase* getGridModel() const { return nullptr; }
	virtual std::vector<ModeIcon> getSubModeIcons() const { return {}; }
	virtual void openUi() {};
	virtual void closeUi() {};
};