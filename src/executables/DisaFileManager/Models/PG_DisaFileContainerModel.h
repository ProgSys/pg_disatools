#pragma once

#include <Base/PG_Object.h>
#include <Models/PG_UiStateBaseModel.h>
#include <Disa/fileformats/PG_DisaFileContainer.h>
#include <Util/PG_File.h>
#include <Base/PG_Index.h>
#include <Stream/PG_StreamIn.h>

class PG_DisaFileContainer;
class PG_ListSelectionWrapperModel;
class PG_DisaFileFilterModel;

class PG_DisaFileContainerModel : public PG_Object, public PG_UiStateBaseModel {
	PG_OBJECT(PG_DisaFileContainerModel);
	PG_PARENT(PG_Object);

	PG_PROPERTY(std::string searchText GET getSearchText SET setSearchText CHANGED searchStringChanged)
	PG_PROPERTY(PG_ListModelLocation lastSelected GET getLastSelected CHANGED lastSelectedChanged)

signals:
	void searchStringChanged();
	void lastSelectedChanged();
public:
	using PG_Object::PG_Object;

	UiState getUiState() const override { return FileArchive; }
	PG_ListModelBase* getListModel() const;
	std::string getFileName() const override;

	const std::string& getSearchText() const;
	void setSearchText(const std::string& str);

	PG_ListModelLocation getLastSelected() const;

	void closeUi() override;
	bool isHoldingContainer(PG_DisaFileContainer* fileContainer) const override { return m_fileContainer == fileContainer; }

	static PG_DisaFileContainerModel* tryOpen(const PG_File& file, std::shared_ptr<PG::STREAM::In> reader);
receivers:
	/**
	* @brief Will open given entry
	*/
	void openEntry(const PG_Index& index);
private:
	void setContainer(PG_DisaFileContainer* fileContainer);
	void addOpenedFile(std::shared_ptr<PG_DisaFileContainer>&& container, std::string&& name = "OpenedFiles");

	PG_DisaFileContainer* m_fileContainer = nullptr;
	PG_ListSelectionWrapperModel* m_selectionModel = nullptr;
	PG_DisaFileFilterModel* m_filterModel = nullptr;
};