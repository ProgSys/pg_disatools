#include "PG_DisaModCharactersListModel.h"
#include <Disa/models/PG_CustomCharacterModel.h>
#include <Disa/models/PG_DisaMainModel.h>

void PG_DisaModCharactersListModel::loadData() {
	PG_Dir dir = PG_DisaMainModel::getBase()->getCustomCharactersDir();
	if (!dir.exists()) return;

	std::vector<PG_DisaModListModelItem> items;
	addCustomCharacters(items, dir);
	set(std::move(items));
}

static inline void customDelete(PG_ListModelBase* ptr) {
	if (!ptr->getParent() || !PG_EventLoop::get(ptr->getThreadId())) delete ptr;
	else ptr->deleteLater();
}

void PG_DisaModCharactersListModel::addCustomCharacters(std::vector<PG_DisaModListModelItem>& items, const PG_Dir& dir) {

	auto files = dir.findFiles([](const PG_File& f) ->bool {
		return f.getFileExtensionFormated() == "zip";
		}, 1);

	if (files.empty()) return;

	items.reserve(items.size() + files.size());

	for (const auto& file : files) {

		PG_CustomCharacterModel* customharacterModel = PG_new<PG_CustomCharacterModel>("PG_CustomCharacterModel", this);
		auto sPtr = std::shared_ptr<PG_ListModelBase>(customharacterModel, customDelete);
		customharacterModel->PG_DisaFileContainer::open(file);
		if (!customharacterModel->getCharacterJsonContainer() || customharacterModel->getCharacterJsonContainer()->getId().empty()) {
			sPtr.reset();
			continue;
		}
		PG_DisaModListModelItem item{
			[customharacterModel]()->std::string { return customharacterModel->getCharacterJsonContainer()->getName(); },
			[customharacterModel]()->std::string { return customharacterModel->getCharacterJsonContainer()->getDescription(); } ,
			[customharacterModel]()->std::string { return customharacterModel->getCharacterJsonContainer()->getAuthor(); },
			[customharacterModel]()->PG_Texture { return customharacterModel->getPreviewImg(); },
			0,
			std::move(sPtr)
		};
		items.push_back(std::move(item));
	}
}