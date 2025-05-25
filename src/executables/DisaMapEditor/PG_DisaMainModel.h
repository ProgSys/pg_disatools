#pragma once

#include <Base/PG_Object.h>
#include <Util/PG_File.h>
#include <Util/PG_Dir.h>
#include <Script/PG_ConfigFile.h>
#include <Models/PG_UiStateBaseModel.h>

class PG_DisaMapListModel;
class PG_DisaCharactersModel;
class PG_DisaGeoEffectsModel;
class PG_DisaItemsModel;
class PG_ListModelBase;
class PG_PSPFS;
class PG_OLA;
class PG_DisaMainModelResources;

class PG_DisaMainModel : public PG_Object {
	PG_OBJECT(PG_DisaMainModel);
signals:
	void uiStateChanged(PG_UiStateBaseModel::UiState state);
	void gridModelChanged(PG_ListModelBase* model);
	void paintSurfacesPressed(unsigned int index); //emmited when paint brush is selected in asset window
	void addModelPressed(unsigned int index); //emmited when paint brush is selected in asset window
	void displayProgress(const std::string& text, float progress); //emmited when a progress disalog is running
public:
	using PG_Object::PG_Object;

	void init();

	static PG_DisaMainModel* get();
	//Ui state
	PG_UiStateBaseModel::UiState getUiState() const;
	auto getUiStackSize() const { return m_modelStack.size(); }
	const std::shared_ptr<PG_UiStateBaseModel>& getUiStackEntry(int index) const { return m_modelStack[index]; }
	const std::shared_ptr<PG_UiStateBaseModel>& getCurrent() const { return m_modelStack.back(); }
	std::shared_ptr<PG_Object> getCurrentObject() const { return std::dynamic_pointer_cast<PG_Object>(m_modelStack.back()); }

	PG_ListModelBase* getGridModel() const { return m_gridModel; }

	//Config
	PG_ConfigFile& getConfig() { return m_config; }
	const PG_ConfigFile& getConfig() const { return m_config; }

	PG_File getExePath() const;
	void setExePath(const PG_File& path);

	PG_Dir getModDir() const;
	PG_Dir getCustomCampaignsDir() const;

	//Functions
	void back();
	void openExe();

	PG_DisaCharactersModel* getCharactersModel() const { return m_charactersModel; }
	PG_DisaItemsModel* getItemsModel() const { return m_itemsModel; }
	PG_DisaGeoEffectsModel* getGeoEffectsModel() const { return m_geoEffectsModel; }

	void pushStateStack(std::shared_ptr<PG_UiStateBaseModel> model);
	void popStateStack();

private:
	void setGridModel(PG_ListModelBase* model);
	void loadDisaMainModelResources(PG_OLA* startDat);

	std::shared_ptr<PG_PSPFS> m_PSPFS; //subdata
	PG_DisaCharactersModel* m_charactersModel = nullptr;
	PG_DisaItemsModel* m_itemsModel = nullptr;
	PG_DisaGeoEffectsModel* m_geoEffectsModel = nullptr;
	PG_DisaMainModelResources* m_disaMainModelResources = nullptr;

	PG_ConfigFile m_config;
	PG_ListModelBase* m_gridModel = nullptr; //buffer pointer set by pushStateStack
	std::vector<std::shared_ptr<PG_UiStateBaseModel>> m_modelStack;
};