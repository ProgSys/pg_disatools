#include "PG_DisaModCampaignsListModel.h"
#include <Util/PG_Dir.h>
#include <Disa/models/PG_CustomCampaignsModel.h>

void PG_DisaModCampaignsListModel::reload() {
	{ //clean up
		auto oldItems = std::move(m_items);
		emit itemsRemoved(0, oldItems.size() - 1);
		emit sizeChanged();
	}

	PG_File path; // mainModel->getExePath();
	if (path.isEmpty()) return;

	std::vector<PG_DisaModListModelItem> items;
	addCustomCampaigns(items, path);
	set(std::move(items));
}

PG_Variety PG_DisaModCampaignsListModel::getData(PG_Index index, Identity identity) const {
	switch (identity) {
	case Identity_PreviewImage: return m_items[index].previewImage();
	case Identity_Description: return m_items[index].description();
	case Identity_Author: return m_items[index].author();
	case Identity_Order: return m_items[index].order;
	}
	return PG_ListModel<PG_DisaModListModelItem>::getData(index, identity);
}

static inline void customDelete(PG_ListModelBase* ptr) {
	if (!ptr->getParent() || !PG_EventLoop::get(ptr->getThreadId())) delete ptr;
	else ptr->deleteLater();
}

void PG_DisaModCampaignsListModel::addCustomCampaigns(std::vector<PG_DisaModListModelItem>& items, const PG_File& exePath) {
	PG_Dir dir(exePath.getDir());
	dir += "/mod/CustomCampaigns";

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