#include "PG_DisaModModificationsListModel.h"
#include <Disa/models/PG_CustomModificationModel.h>
#include <Disa/models/PG_DisaMainModel.h>
#include <Disa/Models/PG_DisaModLoadOrderListModel.h>

void PG_DisaModModificationsListModel::loadData() {
	PG_Dir dir = PG_DisaMainModel::getBase()->getCustomModificationsDir();
	if (!dir.exists()) return;

	std::vector<PG_DisaModListModelItem> items;
	addCustomModifications(items, dir);
	m_orderFile = dir.toFilePath("loadOrder.json");
	loadAndApplyOrder(items);
	set(std::move(items));
}

static inline void customDelete(PG_ListModelBase* ptr) {
	if (!ptr->getParent() || !PG_EventLoop::get(ptr->getThreadId())) delete ptr;
	else ptr->deleteLater();
}

static inline PG_DisaModListModelItem toItem(PG_CustomModificationModel* modificationModel, std::shared_ptr<PG_DisaFileContainer>&& sptr) {
	return PG_DisaModListModelItem{
		[modificationModel]()->std::string { return modificationModel->getModificationJsonContainer()->getId(); },
		[modificationModel]()->std::string { return modificationModel->getModificationJsonContainer()->getName(); },
		[modificationModel]()->std::string { return modificationModel->getModificationJsonContainer()->getDescription(); } ,
		[modificationModel]()->std::string { return modificationModel->getModificationJsonContainer()->getAuthor(); },
		[modificationModel]()->PG_Texture { return modificationModel->getPreviewImg(); },
		0,
		std::move(sptr)
	};
}

void PG_DisaModModificationsListModel::addZip(const PG_File& file) {
	PG_Dir dir = PG_DisaMainModel::getBase()->getCustomModificationsDir();
	if (!dir.exists()) {
		if (!dir.create()) throw_Exception("Failed to create output dir!");
	}
	PG_File targetFile = dir.toFilePath(file.getFile());
	if (file == targetFile) throw_Exception("Mod already installed!");
	if (!file.copy(targetFile, true)) throw_Exception("Failed to copy file!");


	PG_CustomModificationModel* modificationModel = PG_new<PG_CustomModificationModel>("PG_CustomModificationModel", this);
	auto sPtr = std::shared_ptr<PG_DisaFileContainer>(modificationModel, customDelete);
	modificationModel->PG_DisaFileContainer::open(targetFile);
	if (!modificationModel->getModificationJsonContainer()) {
		throw_Exception("Invalid json!");
	}

	if (modificationModel->getModificationJsonContainer()->getId().empty()) {
		throw_Exception("Missing 'id'!");
	}

	if (modificationModel->getModificationJsonContainer()->getName().empty()) {
		throw_Exception("Missing 'name'!");
	}

	auto findIt = std::ranges::find(getItems(), modificationModel->getModificationJsonContainer()->getId(), [](const PG_DisaModListModelItem& item) { return item.id(); });

	if (findIt != end()) {
		set(std::distance(begin(), findIt), toItem(modificationModel, std::move(sPtr)));
		return;
	}

	push_back(toItem(modificationModel, std::move(sPtr)));
}

void PG_DisaModModificationsListModel::addCustomModifications(std::vector<PG_DisaModListModelItem>& items, const PG_Dir& dir) {

	auto files = dir.findFiles([](const PG_File& f) ->bool {
		return f.getFileExtensionFormated() == "zip";
		}, 1);

	if (files.empty()) return;

	items.reserve(items.size() + files.size());

	for (const auto& file : files) {

		PG_CustomModificationModel* modificationModel = PG_new<PG_CustomModificationModel>("PG_CustomModificationModel", this);
		auto sPtr = std::shared_ptr<PG_DisaFileContainer>(modificationModel, customDelete);
		modificationModel->PG_DisaFileContainer::open(file);
		if (!modificationModel->getModificationJsonContainer() || modificationModel->getModificationJsonContainer()->getId().empty()) {
			sPtr.reset();
			continue;
		}
		PG_DisaModListModelItem item{
			[modificationModel]()->std::string { return modificationModel->getModificationJsonContainer()->getId(); },
			[modificationModel]()->std::string { return modificationModel->getModificationJsonContainer()->getName(); },
			[modificationModel]()->std::string { return modificationModel->getModificationJsonContainer()->getDescription(); } ,
			[modificationModel]()->std::string { return modificationModel->getModificationJsonContainer()->getAuthor(); },
			[modificationModel]()->PG_Texture { return modificationModel->getPreviewImg(); },
			0,
			std::move(sPtr)
		};
		items.push_back(std::move(item));
	}
}