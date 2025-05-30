#pragma once

#include <Scene/Model/PG_ListModel.h>
#include <Models/PG_DisaModListModel.h>
#include <Util/PG_Dir.h>

class PG_DisaModCampaignsListModel : public PG_DisaModListModel {
	PG_OBJECT(PG_DisaModCampaignsListModel);
	PG_PARENT(PG_DisaModListModel);

public:
	using PG_DisaModListModel::PG_DisaModListModel;
	using PG_DisaModListModel::reload;
public:
	void loadData() override;
	void addCustomCampaigns(std::vector<PG_DisaModListModelItem>& items, const PG_Dir& dir);
};