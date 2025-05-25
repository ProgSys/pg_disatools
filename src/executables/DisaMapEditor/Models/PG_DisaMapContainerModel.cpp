#include "PG_DisaMapContainerModel.h"

std::shared_ptr<PG_DisaMapContainerModel> PG_DisaMapContainerModel::open(PG_DisaFileEntry mpd, PG_DisaFileEntry mppSD, PG_DisaFileEntry mppHD) {
	auto container = mpd.open<PG_DisaMapContainerModel>();
	container->createModel()->openMapAssets(mppSD, mppHD);
	return container;
}

std::shared_ptr<PG_DisaMapContainerModel> PG_DisaMapContainerModel::open(PG_DisaFileEntry mpp, int topIndex, int sideIndex, int decalIndex) {
	auto ptr = std::shared_ptr<PG_DisaMapContainerModel>(PG_new<PG_DisaMapContainerModel>("PG_DisaMapContainerModel"));
	ptr->createModel()->openAsset(mpp, topIndex, sideIndex, decalIndex);
	return ptr;
}