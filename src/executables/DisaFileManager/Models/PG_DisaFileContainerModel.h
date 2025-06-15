#pragma once

#include <Base/PG_Object.h>
#include <Models/PG_UiStateBaseModel.h>
#include <Disa/fileformats/PG_DisaFileContainer.h>
#include <Util/PG_File.h>
#include <Base/PG_Index.h>

class PG_DisaFileContainer;
class PG_ListSelectionWrapperModel;
class PG_DisaFileFilterModel;

class PG_DisaFileContainerModel : public PG_Object, public PG_UiStateBaseModel {
	PG_OBJECT(PG_DisaFileContainerModel);
	PG_PARENT(PG_Object);

	PG_PROPERTY(std::string searchText GET getSearchText SET setSearchText CHANGED searchStringChanged)
signals:
	void searchStringChanged();
public:
	using PG_Object::PG_Object;

	UiState getUiState() const override { return FileArchive; }
	PG_ListModelBase* getListModel() const;

	const std::string& getSearchText() const;
	void setSearchText(const std::string& str);

	void open(PG_DisaFileContainer* fileContainer);
	
	void addContainer(std::shared_ptr<PG_DisaFileContainer>&& container);
receivers:
	/**
	* @brief Will open given entry
	*/
	void openEntry(const PG_Index& index);
private:
	PG_DisaFileContainer* m_fileContainer = nullptr;
	PG_ListSelectionWrapperModel* m_selectionModel = nullptr;
	PG_DisaFileFilterModel* m_filterModel = nullptr;
};