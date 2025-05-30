#pragma once

#include <functional>
#include <string>
#include <Resources/PG_Texture.h>
#include <Scene/Model/PG_ListModel.h>
#include <Util/PG_File.h>

struct PG_DisaModLoadOrderListModelItem {
	std::string id;
	std::string name;
	bool disabled;

	const std::string& toString() const { return name; }
};

class PG_DisaModLoadOrderListModel : public PG_ListModel<PG_DisaModLoadOrderListModelItem> {
	PG_OBJECT(PG_DisaModLoadOrderListModel);
	PG_PARENT(PG_ListModel<PG_DisaModLoadOrderListModelItem>);

public:
	enum ECustomIdentity : Identity {
		Identity_Id = EIdentity::Identity_Custom,
		Identity_Disabled,
	};
	using PG_ListModel<PG_DisaModLoadOrderListModelItem>::PG_ListModel;

	void open(const PG_File& file);
	bool save(const PG_File& file) const;

	PG_Variety getData(PG_Index index, Identity identity) const override;
private:
	void readJson(const std::string& json);
	std::string writeJson() const;
};