#pragma once

#include <Scene/Model/PG_ListModel.h>
#include <Disa/resources/PG_DisaSprite.h>

class PG_SOLA;

struct PG_DisaSpriteListModelItem {
	unsigned short id;
	PG_DisaSprite sprite;
};


class PG_DisaSpriteListModel : public PG_ListModel<PG_DisaSpriteListModelItem> {
	PG_OBJECT(PG_DisaSpriteListModel);
	PG_PARENT(PG_ListModel<PG_DisaSpriteListModelItem>);

public:
	using PG_ListModel<PG_DisaSpriteListModelItem>::PG_ListModel;
	void open(const PG_SOLA* sola);


	PG_Variety getData(PG_Index index, Identity identity) const override;
};