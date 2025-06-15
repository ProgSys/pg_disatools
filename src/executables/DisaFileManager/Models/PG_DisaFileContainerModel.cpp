#include "PG_DisaFileContainerModel.h"

#include <Scene/Model/PG_ListSelectionWrapperModel.h>
#include <Scene/Model/PG_ListModelFilter.h>
#include <Disa/fileformats/PG_DisaFileContainer.h>
#include <Models/PG_DisaFileFilterModel.h>
#include <Models/PG_DisaFileManagerMainModel.h>
#include <Disa/fileformats/PG_OLA.h>

PG_ListModelBase* PG_DisaFileContainerModel::getListModel() const {
	return m_selectionModel;
}

const std::string& PG_DisaFileContainerModel::getSearchText() const {
	return m_filterModel? m_filterModel->getSearchText(): getName();
}
void PG_DisaFileContainerModel::setSearchText(const std::string& str) {
	if(m_filterModel) m_filterModel->setSearchText(str);
}

void PG_DisaFileContainerModel::open(PG_DisaFileContainer* fileContainer) {
	if (m_fileContainer == fileContainer) return;
	if (m_fileContainer) delete m_fileContainer;

	m_fileContainer = fileContainer;
	if (!m_fileContainer) return;

	if (!m_filterModel) {
		m_filterModel = PG_new<PG_DisaFileFilterModel>("PG_DisaFileFilterModel", this);
		connect(m_filterModel, &PG_DisaFileFilterModel::searchStringChanged, this, &PG_DisaFileContainerModel::searchStringChanged);
	}
	if(!m_selectionModel) m_selectionModel = PG_new<PG_ListSelectionWrapperModel>("PG_ListSelectionWrapperModel", this);

	m_selectionModel->setModel(m_filterModel);
	m_filterModel->setModel(fileContainer);
}

struct OpenedFiles : public PG_Object {
	using PG_Object::PG_Object;
	std::vector<std::shared_ptr<PG_DisaFileContainer>> files;
};

void PG_DisaFileContainerModel::addContainer(std::shared_ptr<PG_DisaFileContainer>&& container) {
	OpenedFiles* openedFiles = findFirstChild<OpenedFiles>("OpenedFiles");
	if (!openedFiles) openedFiles = PG_new<OpenedFiles>("OpenedFiles", this);
	openedFiles->files.push_back(std::move(container));
}

void PG_DisaFileContainerModel::openEntry(const PG_Index& index) {
	if (!index.isValid()) return;

	std::shared_ptr<PG_DisaFileContainer> containerSptr;
	PG_DisaFileEntry fileEntry = m_filterModel->getData(index, Identity_Item).get<PG_DisaFileEntry>();
	//TODO check if filecontaner is allready present
	switch (fileEntry.getInfo().getType()) {
	case PG_DisaFileInfo::OLA: {
		auto model = fileEntry.open<PG_OLA>();
		auto container = PG_new<PG_DisaFileContainerModel>("PG_DisaFileContainerModel", model.get());
		container->open(model.get());
		containerSptr = std::move(model);
		PG_DisaFileManagerMainModel::get()->pushStack(container);
	} break;
	}

	if (!containerSptr) return;

	addContainer(std::move(containerSptr));
}