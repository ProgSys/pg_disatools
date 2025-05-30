#pragma once

#include <functional>
#include <string>
#include <Resources/PG_Texture.h>
#include <Scene/Model/PG_ListModel.h>
#include <Util/PG_File.h>

class PG_DisaFileContainer;

struct PG_DisaModListModelItem {
	std::function<std::string()> id;
	std::function<std::string()> title;
	std::function<std::string()> description;
	std::function<std::string()> author;
	std::function<PG_Texture()> previewImage;
	bool disabled = false;
	std::shared_ptr<PG_DisaFileContainer> model;

	std::string toString() const { return title(); }
};

class PG_DisaModListModel : public PG_ListModel<PG_DisaModListModelItem> {
	PG_OBJECT(PG_DisaModListModel);
	PG_PARENT(PG_ListModel<PG_DisaModListModelItem>);

public:
	enum ECustomIdentity : Identity {
		Identity_Description = EIdentity::Identity_Custom,
		Identity_Author,
		Identity_Disabled,
	};
	using PG_ListModel<PG_DisaModListModelItem>::PG_ListModel;
	~PG_DisaModListModel() override;
	/**
	* @brief Reload data in model
	* Will clear model and then call loadData
	*/
	void reload();
	/**
	* @brief Will try add a new mod zip file
	*/
	virtual void addZip(const PG_File& file) = 0;

	PG_Variety getData(PG_Index index, Identity identity) const override;
receivers:
	void moveUp(PG_ListModelLocation loc);
	void moveDown(PG_ListModelLocation loc);
	void disableMod(PG_ListModelLocation loc);
	void deleteMod(PG_ListModelLocation loc);

protected:
	virtual void loadData() = 0;
	void loadAndApplyOrder(std::vector<PG_DisaModListModelItem>& items);
	void saveOrder(const std::vector<PG_DisaModListModelItem>& items) const;
	void startSaveOrder(); //start timer to save order

	PG_File m_orderFile;
};