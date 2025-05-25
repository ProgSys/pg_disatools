#include "PG_OverviewListModel.h"
#include <PG_DisaMainModel.h>
#include <Util/PG_Dir.h>
#include <PG_DisaMapListModel.h>
#include <Disa/fileformats/PG_TX2.h>
#include <Models/PG_CustomCampaignsModel.h>
#include <Models/PG_DisaDataDatModel.h>

PG_OverviewListModel::~PG_OverviewListModel() {
	clear();
}

void PG_OverviewListModel::reload() {
	{ //clean up
		auto oldItems = std::move(m_items);
		emit itemsRemoved(0, oldItems.size() - 1);
		emit sizeChanged();
	}
	
	PG_DisaMainModel* mainModel = PG_DisaMainModel::get();
	if (!mainModel) {
		return;
	}

	auto path = mainModel->getExePath();
	if (path.isEmpty()) return;

	std::vector<PG_OverviewListModelListModelItem> items;
	addDisa(items, path);
	addCustomCampaigns(items, path);
	set(std::move(items));
}

static inline void customDelete(PG_ListModelBase* ptr) {
	if (!ptr->getParent() || !PG_EventLoop::get(ptr->getThreadId())) delete ptr;
	else ptr->deleteLater();
}

void PG_OverviewListModel::addDisa(std::vector<PG_OverviewListModelListModelItem>& items, PG_File origin) {
	origin.replaceFileNameAndExtension("DATA.DAT");

	if (!origin.exists()) return;

	auto disaModel = PG_new<PG_DisaDataDatModel>("PG_DisaDataDatModel", this);
	auto sPtr = std::shared_ptr<PG_ListModelBase>(disaModel, customDelete);
	disaModel->PG_DisaFileContainer::open(origin);

	if (disaModel->empty()) return;

	auto titleEntry = disaModel->find(PG_File("TITLEBG.TX2"));
	if (!titleEntry) return;

	PG_Texture preview;
	{
		PG_TX2 tx2;
		auto reader = titleEntry.createReader();
		if (tx2.open(reader.get())) {
			preview.load("DisaGridTilePreview", [tex = std::move(tx2)]() -> PG_Texture::ImageFormats {
				PG_RGBAImage out;
				decompressTX2(tex, out);
				return out;
			});

		}
	}

	PG_OverviewListModelListModelItem item{
			[]()->std::string { return "Disa"; },
			[]()->std::string { return "Game files"; } ,
			[p = std::move(preview)]()->PG_Texture { return p; },
			std::move(sPtr)
	};
	
	items.push_back(item);
}

void PG_OverviewListModel::addCustomCampaigns(std::vector<PG_OverviewListModelListModelItem>& items, PG_File origin) {
	PG_Dir dir(origin.getDir());
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
		if (!customCampaignsModel->getCampaignJsonContainer()  || customCampaignsModel->getCampaignJsonContainer()->getId().empty()) {
			sPtr.reset();
			continue;
		}
		PG_OverviewListModelListModelItem item{
			[customCampaignsModel]()->std::string { return customCampaignsModel->getCampaignJsonContainer()->getName(); },
			[customCampaignsModel]()->std::string { return customCampaignsModel->getCampaignJsonContainer()->getDescription(); } ,
			[customCampaignsModel]()->PG_Texture { return customCampaignsModel->getPreviewImg(); },
			std::move(sPtr)
		};
		items.push_back(std::move(item));
	}
		
}

PG_Variety PG_OverviewListModel::getData(PG_Index index, Identity identity) const {
	switch (identity) {
	case Identity_PreviewImage: return m_items[index].previewImage();
	case Identity_Description: return m_items[index].description();
	case Identity_OpenModel: return std::dynamic_pointer_cast<PG_UiStateBaseModel>(m_items[index].model);
	}
	return PG_ListModel<PG_OverviewListModelListModelItem>::getData(index, identity);
}

void PG_OverviewListModel::tileClicked(PG_ListModelLocation location) {
	auto model = location.getDataT<std::shared_ptr<PG_UiStateBaseModel>>(Identity_OpenModel);
	if (!model) return;
	PG_DisaMainModel* mainModel = PG_DisaMainModel::get();
	mainModel->pushStateStack(std::move(model));
}

void PG_OverviewListModel::addNewCampaign(const std::string& campainName, bool* successFlag) {
	if (campainName.empty()) return;
	PG_DisaMainModel* mainModel = PG_DisaMainModel::get();

	PG_CustomCampaignsModel* customCampaignsModel = PG_new<PG_CustomCampaignsModel>("PG_CustomCampaignsModel", this);
	auto sPtr = std::shared_ptr<PG_ListModelBase>(customCampaignsModel, customDelete);
	if (!customCampaignsModel->open(mainModel->getCustomCampaignsDir().toFilePath(campainName + ".zip"))) {
		if (successFlag) *successFlag = false;
		return;
	}
	customCampaignsModel->PG_CustomCampaignsModel::createNew(campainName);
	if (!customCampaignsModel->save()) {
		if (successFlag) *successFlag = false;
		return;
	}

	if (successFlag) *successFlag = true;
	PG_OverviewListModelListModelItem item{
	[customCampaignsModel]()->std::string { return customCampaignsModel->getCampaignJsonContainer() ? customCampaignsModel->getCampaignJsonContainer()->getName(): "ERROR"; },
	[customCampaignsModel]()->std::string { return customCampaignsModel->getCampaignJsonContainer() ? customCampaignsModel->getCampaignJsonContainer()->getDescription() : "ERROR"; } ,
	[customCampaignsModel]()->PG_Texture { return customCampaignsModel->getPreviewImg(); },
	std::move(sPtr)
	};
	customCampaignsModel->setSubUiState(PG_CustomCampaignsModel::SubUiState_Settings);
	push_back(std::move(item));

	tileClicked(getLocation(size() - 1));
}
