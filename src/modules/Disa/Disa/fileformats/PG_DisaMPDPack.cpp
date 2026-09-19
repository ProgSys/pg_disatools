#include "PG_DisaMPDPack.h"

PG_DisaFileEntry PG_DisaMPDPack::getMPD() const {
	return mpd.isValid()? mpd.getDataT<PG_DisaFileEntry>(PG_Identity_Item) : PG_DisaFileEntry();
}
PG_DisaFileEntry PG_DisaMPDPack::getMPPSD() const {
	return mppSD.isValid()? mppSD.getDataT<PG_DisaFileEntry>(PG_Identity_Item): PG_DisaFileEntry();
}
PG_DisaFileEntry PG_DisaMPDPack::getMPPHD() const {
	return mppHD.isValid()? mppHD.getDataT<PG_DisaFileEntry>(PG_Identity_Item): PG_DisaFileEntry();
}

PG_DisaFileEntry PG_DisaMPPPack::getMPP() const {
	auto data = mpp.getData(PG_Identity_Item);
	if (data && data.is<PG_DisaFileEntry>()) return data.get<PG_DisaFileEntry>();
	data = mpp.getData(PG_Identity_Edit);
	if (data && data.is<PG_DisaFileEntry>()) return data.get<PG_DisaFileEntry>();
	return PG_DisaFileEntry();
}


PG_DisaFileEntry PG_DisaGEOPack::getGEO() const {
	return geo.isValid() ? geo.getDataT<PG_DisaFileEntry>(PG_Identity_Item) : PG_DisaFileEntry();
}

PG_DisaMPPStyle PG_DisaMPPPack::getStyle() const {
	if (auto data = mpp.getData(PG_Identity_Custom); data && data.is<PG_DisaMPPStyle>()) {
		return data.get<PG_DisaMPPStyle>();
	}
	return PG_DisaMPPStyle();
}