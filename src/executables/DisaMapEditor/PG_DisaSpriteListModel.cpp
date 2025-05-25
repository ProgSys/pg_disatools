#include "PG_DisaSpriteListModel.h"
#include <Disa/fileformats/PG_SOLA.h>

void PG_DisaSpriteListModel::open(const PG_SOLA* sola) {
	assert(sola);

	std::vector<PG_DisaSpriteListModelItem> items;
	items.reserve(sola->size());

	for (const PG_DisaFileEntry& entry : *sola) {
		auto& extra = entry.getExtraData();
		if (extra.isEmpty() || !extra.is<unsigned short>()) continue;
		PG_DisaSpriteListModelItem item;
		item.id = extra.get<unsigned short>();
		auto reader = entry.createReader();
		PG_SH sh;
		if (!sh.open(reader.get())) continue;
		item.sprite.load(item.id, std::move(sh));
		items.push_back(std::move(item));
	}

	set(std::move(items));
}

PG_Variety PG_DisaSpriteListModel::getData(PG_Index index, Identity identity) const {
	return PG_Variety();
}