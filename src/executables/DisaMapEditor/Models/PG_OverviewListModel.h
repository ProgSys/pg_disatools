#pragma once

#include <Models/PG_UiStateBaseModel.h>

#include <functional>
#include <Scene/Model/PG_ListModel.h>
#include <Util/PG_File.h>
#include <Resources/PG_Texture.h>

struct PG_OverviewListModelItem {
	std::function<std::string()> title;
	std::function<std::string()> description;
	std::function<PG_Texture()> previewImage;
	std::shared_ptr<PG_ListModelBase> model;

	std::string toString() const { return title(); }
};


class PG_OverviewListModel : public PG_ListModel<PG_OverviewListModelItem>, public PG_UiStateBaseModel {
	PG_OBJECT(PG_OverviewListModel);
	PG_PARENT(PG_ListModel<PG_OverviewListModelItem>);

signals:
	void hasChangesChanged();
	void enabledChanged();
public:
	enum ECustomIdentity : Identity {
		Identity_Description = EIdentity::Identity_Custom,
		Identity_OpenModel,
	};
	using PG_ListModel<PG_OverviewListModelItem>::PG_ListModel;
	~PG_OverviewListModel();
	void reload();

	PG_Variety getData(PG_Index index, Identity identity) const override;

	UiState getUiState() const final{ return UiState::Overview; }
	PG_ListModelBase* getGridModel() const final { return const_cast<PG_ListModelBase*>((const PG_ListModelBase*)this); }

receivers:
	void tileClicked(PG_ListModelLocation location);
	void addNewCampaign(const std::string& campainName, bool* successFlag);
private:
	void addDisa(std::vector<PG_OverviewListModelItem>& items, PG_File origin);
	void addCustomCampaigns(std::vector<PG_OverviewListModelItem>& items, PG_File origin);
};