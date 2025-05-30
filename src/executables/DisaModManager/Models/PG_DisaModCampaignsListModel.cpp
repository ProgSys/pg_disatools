#include "PG_DisaModCampaignsListModel.h"
#include <Disa/models/PG_CustomCampaignsModel.h>
#include <Disa/models/PG_DisaMainModel.h>

void PG_DisaModCampaignsListModel::loadData() {
	PG_Dir dir = PG_DisaMainModel::getBase()->getCustomCampaignsDir();
	if (!dir.exists()) return;

	std::vector<PG_DisaModListModelItem> items;
	addCustomCampaigns(items, dir);
	set(std::move(items));
}

static inline void customDelete(PG_ListModelBase* ptr) {
	if (!ptr->getParent() || !PG_EventLoop::get(ptr->getThreadId())) delete ptr;
	else ptr->deleteLater();
}

void PG_DisaModCampaignsListModel::addCustomCampaigns(std::vector<PG_DisaModListModelItem>& items, const PG_Dir& dir) {

	auto files = dir.findFiles([](const PG_File& f) ->bool {
		return f.getFileExtensionFormated() == "zip";
		}, 1);

	if (files.empty()) return;

	items.reserve(items.size() + files.size());

	for (const auto& file : files) {

		PG_CustomCampaignsModel* customCampaignsModel = PG_new<PG_CustomCampaignsModel>("PG_CustomCampaignsModel", this);
		auto sPtr = std::shared_ptr<PG_ListModelBase>(customCampaignsModel, customDelete);
		customCampaignsModel->PG_DisaFileContainer::open(file);
		if (!customCampaignsModel->getCampaignJsonContainer() || customCampaignsModel->getCampaignJsonContainer()->getId().empty()) {
			sPtr.reset();
			continue;
		}
		PG_DisaModListModelItem item{
			[customCampaignsModel]()->std::string { return customCampaignsModel->getCampaignJsonContainer()->getName(); },
			[customCampaignsModel]()->std::string { return customCampaignsModel->getCampaignJsonContainer()->getDescription(); } ,
			[customCampaignsModel]()->std::string { return customCampaignsModel->getCampaignJsonContainer()->getAuthor(); },
			[customCampaignsModel]()->PG_Texture { return customCampaignsModel->getPreviewImg(); },
			0,
			std::move(sPtr)
		};
		items.push_back(std::move(item));
	}
}