#pragma once

#include <functional>
#include <string>
#include <Resources/PG_Texture.h>
#include <Scene/Model/PG_ListModel.h>

struct PG_DisaModListModelItem {
	std::function<std::string()> title;
	std::function<std::string()> description;
	std::function<std::string()> author;
	std::function<PG_Texture()> previewImage;
	std::size_t order;
	std::shared_ptr<PG_ListModelBase> model;

	std::string toString() const { return title(); }
};

class PG_DisaModListModel : public PG_ListModel<PG_DisaModListModelItem> {
	PG_OBJECT(PG_DisaModListModel);
	PG_PARENT(PG_ListModel<PG_DisaModListModelItem>);

public:
	enum ECustomIdentity : Identity {
		Identity_Description = EIdentity::Identity_Custom,
		Identity_Author,
		Identity_Order,
	};
	using PG_ListModel<PG_DisaModListModelItem>::PG_ListModel;
	/**
	* @brief Reload data in model
	* Will clear model and then call loadData
	*/
	void reload();

	PG_Variety getData(PG_Index index, Identity identity) const override;
protected:
	virtual void loadData() = 0;
};