#include "PG_DisaModCharactersListModel.h"
#include <Disa/models/PG_CustomCharacterModel.h>
#include <Disa/models/PG_DisaMainModel.h>
#include <Disa/Models/PG_DisaModLoadOrderListModel.h>

void PG_DisaModCharactersListModel::loadData() {
	PG_Dir dir = PG_DisaMainModel::getBase()->getCustomCharactersDir();
	if (!dir.exists()) return;

	std::vector<PG_DisaModListModelItem> items;
	addCustomCharacters(items, dir);
	m_orderFile = dir.toFilePath("loadOrder.json");
	loadAndApplyOrder(items);
	set(std::move(items));
}

static inline void customDelete(PG_ListModelBase* ptr) {
	if (!ptr->getParent() || !PG_EventLoop::get(ptr->getThreadId())) delete ptr;
	else ptr->deleteLater();
}

static inline PG_DisaModListModelItem toItem(PG_CustomCharacterModel* customCharacterModel, std::shared_ptr<PG_DisaFileContainer>&& sptr) {
	return PG_DisaModListModelItem{
		[customCharacterModel]()->std::string { return customCharacterModel->getCharacterJsonContainer()->getId(); },
		[customCharacterModel]()->std::string { return customCharacterModel->getCharacterJsonContainer()->getName(); },
		[customCharacterModel]()->std::string { return customCharacterModel->getCharacterJsonContainer()->getDescription(); } ,
		[customCharacterModel]()->std::string { return customCharacterModel->getCharacterJsonContainer()->getAuthor(); },
		[customCharacterModel]()->PG_Texture { return customCharacterModel->getPreviewImg(); },
		0,
		std::move(sptr)
	};
}

void PG_DisaModCharactersListModel::addZip(const PG_File& file) {
	PG_Dir dir = PG_DisaMainModel::getBase()->getCustomCharactersDir();
	if (!dir.exists()) {
		if (!dir.create()) throw_Exception("Failed to create output dir!");
	}
	PG_File targetFile = dir.toFilePath(file.getFile());
	if (file == targetFile) throw_Exception("Mod already installed!");
	if (!file.copy(targetFile, true)) throw_Exception("Failed to copy file!");


	PG_CustomCharacterModel* customCharacterModel = PG_new<PG_CustomCharacterModel>("PG_CustomCharacterModel", this);
	auto sPtr = std::shared_ptr<PG_DisaFileContainer>(customCharacterModel, customDelete);
	customCharacterModel->PG_DisaFileContainer::open(targetFile);
	if (!customCharacterModel->getCharacterJsonContainer()) {
		throw_Exception("Invalid json!");
	}

	if (customCharacterModel->getCharacterJsonContainer()->getId().empty()) {
		throw_Exception("Missing 'id'!");
	}

	if (customCharacterModel->getCharacterJsonContainer()->getName().empty()) {
		throw_Exception("Missing 'name'!");
	}

	auto findIt = std::ranges::find(getItems(), customCharacterModel->getCharacterJsonContainer()->getId(), [](const PG_DisaModListModelItem& item) { return item.id(); });

	if (findIt != end()) {
		set(std::distance(begin(), findIt), toItem(customCharacterModel, std::move(sPtr)));
		return;
	}

	push_back(toItem(customCharacterModel, std::move(sPtr)));
}

void PG_DisaModCharactersListModel::addCustomCharacters(std::vector<PG_DisaModListModelItem>& items, const PG_Dir& dir) {

	auto files = dir.findFiles([](const PG_File& f) ->bool {
		return f.getFileExtensionFormated() == "zip";
		}, 1);

	if (files.empty()) return;

	items.reserve(items.size() + files.size());

	for (const auto& file : files) {

		PG_CustomCharacterModel* customCharacterModel = PG_new<PG_CustomCharacterModel>("PG_CustomCharacterModel", this);
		auto sPtr = std::shared_ptr<PG_DisaFileContainer>(customCharacterModel, customDelete);
		customCharacterModel->PG_DisaFileContainer::open(file);
		if (!customCharacterModel->getCharacterJsonContainer() || customCharacterModel->getCharacterJsonContainer()->getId().empty()) {
			sPtr.reset();
			continue;
		}
		PG_DisaModListModelItem item{
			[customCharacterModel]()->std::string { return customCharacterModel->getCharacterJsonContainer()->getId(); },
			[customCharacterModel]()->std::string { return customCharacterModel->getCharacterJsonContainer()->getName(); },
			[customCharacterModel]()->std::string { return customCharacterModel->getCharacterJsonContainer()->getDescription(); } ,
			[customCharacterModel]()->std::string { return customCharacterModel->getCharacterJsonContainer()->getAuthor(); },
			[customCharacterModel]()->PG_Texture { return customCharacterModel->getPreviewImg(); },
			0,
			std::move(sPtr)
		};
		items.push_back(std::move(item));
	}
}