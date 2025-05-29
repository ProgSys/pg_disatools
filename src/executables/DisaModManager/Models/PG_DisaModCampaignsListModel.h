#pragma once

#include <Scene/Model/PG_ListModel.h>
#include <Models/PG_DisaModListModelItem.h>
#include <Util/PG_File.h>

class PG_DisaModCampaignsListModel : public PG_ListModel<PG_DisaModListModelItem> {
	PG_OBJECT(PG_DisaModCampaignsListModel);
	PG_PARENT(PG_ListModel<PG_DisaModListModelItem>);

public:
	enum ECustomIdentity : Identity {
		Identity_Description = EIdentity::Identity_Custom,
		Identity_Author,
		Identity_Order,
	};
	using PG_ListModel<PG_DisaModListModelItem>::PG_ListModel;

	void reload();

	PG_Variety getData(PG_Index index, Identity identity) const override;

public:
	void addCustomCampaigns(std::vector<PG_DisaModListModelItem>& items, const PG_File& exePath);
};