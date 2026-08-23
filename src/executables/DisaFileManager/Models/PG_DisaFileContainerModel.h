#pragma once

#include <Base/PG_Object.h>
#include <Models/PG_UiStateBaseModel.h>
#include <Disa/fileformats/PG_DisaFileContainer.h>
#include <Util/PG_File.h>
#include <Util/PG_Dir.h>
#include <Base/PG_Index.h>
#include <Stream/PG_StreamIn.h>
#include <set>

class PG_DisaFileContainer;
class PG_ListSelectionWrapperModel;
class PG_DisaFileFilterModel;

class PG_DisaFileContainerModel : public PG_Object, public PG_UiStateBaseModel {
	PG_OBJECT(PG_DisaFileContainerModel);
	PG_PARENT(PG_Object);

	PG_PROPERTY(bool isModified GET isModified CHANGED isModifiedChanged) //is any file modfied
	PG_PROPERTY(std::string searchText GET getSearchText SET setSearchText CHANGED searchStringChanged)
	PG_PROPERTY(bool hasSelection GET hasSelection CHANGED lastSelectedChanged)
	PG_PROPERTY(bool hasSingleSelection GET hasSingleSelection CHANGED lastSelectedChanged)
	PG_PROPERTY(int selectionSize GET getSelectionSize CHANGED lastSelectedChanged)
	PG_PROPERTY(PG_ListModelLocation lastSelected GET getLastSelected CHANGED lastSelectedChanged)

signals:
	void isModifiedChanged();
	void searchStringChanged();
	void lastSelectedChanged();
public:
	using PG_Object::PG_Object;

	bool isModified() const;

	UiState getUiState() const override { return FileArchive; }
	PG_ListModelBase* getListModel() const 	{ return (PG_ListModelBase*)m_selectionModel; }
	PG_File getOriginFile() const;
	std::string getFileName() const override;

	const std::string& getSearchText() const;
	void setSearchText(const std::string& str);

	bool hasSelection() const;
	bool hasSingleSelection() const;
	int getSelectionSize() const;
	PG_ListModelLocation getLastSelected() const;
	PG_DisaFileContainer* getDisaFileContainer() const { return m_fileContainer; }

	void openUi() override;
	void closeUi() override;
	bool isHoldingContainer(PG_DisaFileContainer* fileContainer) const override { return m_fileContainer == fileContainer; }

	static PG_DisaFileContainerModel* tryOpen(const PG_File& file, std::shared_ptr<PG::STREAM::In> reader);
receivers:
	/**
	* @brief Save data to given file
	*/
	void exportToFile(const PG_File& file);
	/**
	* @brief Will open given entry
	*/
	void openEntry(const PG_Index& index);
	/**
	* @brief Will extract selected items into given target dir
	*/
	void exportSelected(const PG_Dir& targetDir);
	/**
	* @brief Will extract last selected items into given target file
	*/
	void exportLastSelected(const PG_File& targetFile);
	/**
	* @brief Will replace the last selected items with the given target file
	* @param replaceName If true then the name will be changed to the target file name
	*/
	void replaceLastSelected(const PG_File& targetFile, bool replaceName);
	/**
	* @brief Will insert the given files below the last selected item
	* @details If there is no selection then the files are added at the very end
	*/
	void importFiles(const std::vector<PG_File>& files);
private:
	void setContainer(PG_DisaFileContainer* fileContainer);
	void setOwningContainer(std::shared_ptr<PG_DisaFileContainer>&& container);

	PG_DisaFileContainer* m_fileContainer = nullptr;
	PG_ListSelectionWrapperModel* m_selectionModel = nullptr;
	PG_DisaFileFilterModel* m_filterModel = nullptr;
	std::shared_ptr<PG_DisaFileContainer> m_ownershipContainer;
};