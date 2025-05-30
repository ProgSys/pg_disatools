#include "PG_DisaModCampaignsListModel.h"
#include <Disa/models/PG_CustomCampaignsModel.h>
#include <Disa/models/PG_DisaMainModel.h>
#include <Util/PG_Exception.h>

void PG_DisaModCampaignsListModel::loadData() {
	PG_Dir dir = PG_DisaMainModel::getBase()->getCustomCampaignsDir();
	if (!dir.exists()) return;

	std::vector<PG_DisaModListModelItem> items;
	addCustomCampaigns(items, dir);
	m_orderFile = dir.toFilePath("loadOrder.json");
	loadAndApplyOrder(items);
	set(std::move(items));

}

static inline void customDelete(PG_ListModelBase* ptr) {
	if (!ptr->getParent() || !PG_EventLoop::get(ptr->getThreadId())) delete ptr;
	else ptr->deleteLater();
}

static inline PG_DisaModListModelItem toItem(PG_CustomCampaignsModel* customCampaignsModel, std::shared_ptr<PG_DisaFileContainer>&& sptr) {
	return PG_DisaModListModelItem{
		[customCampaignsModel]()->std::string { return customCampaignsModel->getCampaignJsonContainer()->getId(); },
		[customCampaignsModel]()->std::string { return customCampaignsModel->getCampaignJsonContainer()->getName(); },
		[customCampaignsModel]()->std::string { return customCampaignsModel->getCampaignJsonContainer()->getDescription(); } ,
		[customCampaignsModel]()->std::string { return customCampaignsModel->getCampaignJsonContainer()->getAuthor(); },
		[customCampaignsModel]()->PG_Texture { return customCampaignsModel->getPreviewImg(); },
		false,
		std::move(sptr)
	};
}

void PG_DisaModCampaignsListModel::addZip(const PG_File& file) {
	PG_Dir dir = PG_DisaMainModel::getBase()->getCustomCampaignsDir();
	if (!dir.exists()) {
		if(!dir.create()) throw_Exception("Failed to create output dir!");
	}
	PG_File targetFile = dir.toFilePath(file.getFile());
	if (file == targetFile) throw_Exception("Mod already installed!");
	if (!file.copy(targetFile)) throw_Exception("Failed to copy file!");

	PG_CustomCampaignsModel* customCampaignsModel = PG_new<PG_CustomCampaignsModel>("PG_CustomCampaignsModel", this);
	auto sPtr = std::shared_ptr<PG_DisaFileContainer>(customCampaignsModel, customDelete);
	customCampaignsModel->PG_DisaFileContainer::open(targetFile);
	if (!customCampaignsModel->getCampaignJsonContainer()) {
		throw_Exception("Invalid json!");
	}

	if (customCampaignsModel->getCampaignJsonContainer()->getId().empty()) {
		throw_Exception("Missing 'id'!");
	}

	if (customCampaignsModel->getCampaignJsonContainer()->getName().empty()) {
		throw_Exception("Missing 'name'!");
	}

	auto findIt = std::ranges::find(getItems(), customCampaignsModel->getCampaignJsonContainer()->getId(), [](const PG_DisaModListModelItem& item) { return item.id(); });

	if (findIt != end()) {
		set(std::distance(begin(), findIt), toItem(customCampaignsModel, std::move(sPtr)));
		return;
	}

	push_back(toItem(customCampaignsModel, std::move(sPtr)));
}

void PG_DisaModCampaignsListModel::addCustomCampaigns(std::vector<PG_DisaModListModelItem>& items, const PG_Dir& dir) {

	auto files = dir.findFiles([](const PG_File& f) ->bool {
		return f.getFileExtensionFormated() == "zip";
		}, 1);

	if (files.empty()) return;

	items.reserve(items.size() + files.size());

	for (const auto& file : files) {

		PG_CustomCampaignsModel* customCampaignsModel = PG_new<PG_CustomCampaignsModel>("PG_CustomCampaignsModel", this);
		auto sPtr = std::shared_ptr<PG_DisaFileContainer>(customCampaignsModel, customDelete);
		customCampaignsModel->PG_DisaFileContainer::open(file);
		if (!customCampaignsModel->getCampaignJsonContainer() || customCampaignsModel->getCampaignJsonContainer()->getId().empty() ||
			customCampaignsModel->getCampaignJsonContainer()->getName().empty()) {
			sPtr.reset();
			continue;
		}

		items.push_back(toItem(customCampaignsModel, std::move(sPtr)));
	}
}