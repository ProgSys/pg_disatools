#pragma once

#include <Base/PG_Object.h>
#include <Util/PG_File.h>
#include <Util/PG_Dir.h>
#include <Disa/utils/PG_DisaConfig.h>
#include <Disa/models/PG_DisaMainModel.h>
#include <Models/PG_UiStateBaseModel.h>

class PG_DisaMapListModel;
class PG_DisaCharactersModel;
class PG_DisaGeoEffectsModel;
class PG_DisaItemsModel;
class PG_ListModelBase;
class PG_PSPFS;
class PG_OLA;
class PG_DisaMapEditorMainModelResources;

class PG_DisaMapEditorMainModel : public PG_DisaMainModel {
	PG_OBJECT(PG_DisaMapEditorMainModel);
	PG_PARENT(PG_DisaMainModel);
signals:
	void uiStateChanged(PG_UiStateBaseModel::UiState state);
	void gridModelChanged(PG_ListModelBase* model);
	void paintSurfacesPressed(unsigned int index); //emmited when paint brush is selected in asset window
	void addModelPressed(unsigned int index); //emmited when paint brush is selected in asset window
	void displayProgress(const std::string& text, float progress); //emmited when a progress disalog is running
public:
	using PG_DisaMainModel::PG_DisaMainModel;

	void init() override;

	static PG_DisaMapEditorMainModel* get();
	//Ui state
	PG_UiStateBaseModel::UiState getUiState() const;
	auto getUiStackSize() const { return m_modelStack.size(); }
	const std::shared_ptr<PG_UiStateBaseModel>& getUiStackEntry(int index) const { return m_modelStack[index]; }
	const std::shared_ptr<PG_UiStateBaseModel>& getCurrent() const { return m_modelStack.back(); }
	std::shared_ptr<PG_Object> getCurrentObject() const { return std::dynamic_pointer_cast<PG_Object>(m_modelStack.back()); }

	PG_ListModelBase* getGridModel() const { return m_gridModel; }

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
	PG_DisaMapEditorMainModelResources* m_disaMainModelResources = nullptr;

	PG_DisaConfig m_config;
	PG_ListModelBase* m_gridModel = nullptr; //buffer pointer set by pushStateStack
	std::vector<std::shared_ptr<PG_UiStateBaseModel>> m_modelStack;
};