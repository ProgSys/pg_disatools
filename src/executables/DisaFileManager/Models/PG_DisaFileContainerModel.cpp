#include "PG_DisaFileContainerModel.h"

#include <Scene/Model/PG_ListSelectionWrapperModel.h>
#include <Scene/Model/PG_ListModelFilter.h>
#include <Disa/fileformats/PG_DisaFileContainer.h>
#include <Models/PG_DisaFileFilterModel.h>
#include <Models/PG_DisaFileManagerMainModel.h>
#include <Disa/fileformats/PG_DisaFileTests.h>
#include <Disa/fileformats/PG_OLA.h>
#include <Disa/fileformats/PG_SOLA.h>
#include <Disa/fileformats/PG_PSPFS.h>

PG_ListModelBase* PG_DisaFileContainerModel::getListModel() const {
	return m_selectionModel;
}

const std::string& PG_DisaFileContainerModel::getSearchText() const {
	return m_filterModel? m_filterModel->getSearchText(): getName();
}
void PG_DisaFileContainerModel::setSearchText(const std::string& str) {
	if(m_filterModel) m_filterModel->setSearchText(str);
}

void PG_DisaFileContainerModel::setContainer(PG_DisaFileContainer* fileContainer) {
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

void PG_DisaFileContainerModel::addOpenedFile(std::shared_ptr<PG_DisaFileContainer>&& container, std::string&& name) {
	OpenedFiles* openedFiles = findFirstChild<OpenedFiles>(name);
	if (!openedFiles) openedFiles = PG_new<OpenedFiles>(name, this);
	openedFiles->files.push_back(std::move(container));
}

static inline std::shared_ptr<PG_DisaFileContainer> createFileContainer(PG_DisaFileInfo::Type type) {
	switch (type) {
	case PG_DisaFileInfo::PSPFS_V1: return std::shared_ptr<PG_PSPFS>(PG_new<PG_PSPFS>("PG_PSPFS"));
	case PG_DisaFileInfo::OLA: return std::shared_ptr<PG_OLA>(PG_new<PG_OLA>("PG_OLA"));
	case PG_DisaFileInfo::SOLA: return std::shared_ptr<PG_SOLA>(PG_new<PG_SOLA>("PG_SOLA"));
	}
	return {};
}

static inline std::shared_ptr<PG_DisaFileContainer> createFileContainer(const std::shared_ptr<PG::STREAM::In>& reader) {
	if (!reader) return {};
	return createFileContainer(PG_DisaFileInfo::typeFromFileFormat(reader.get()));
}

PG_DisaFileContainerModel* PG_DisaFileContainerModel::tryOpen(const PG_File& file, std::shared_ptr<PG::STREAM::In> reader) {
	auto container = createFileContainer(reader);
	if (!container) return nullptr;
	container->setFile(file);
	container->open(reader);
	auto containerModel = PG_new<PG_DisaFileContainerModel>("PG_DisaFileContainerModel", container.get());
	containerModel->setContainer(container.get());
	containerModel->addOpenedFile(std::move(container), "RootFile");

	return containerModel;
}

void PG_DisaFileContainerModel::openEntry(const PG_Index& index) {
	if (!index.isValid()) return;

	std::shared_ptr<PG_DisaFileContainer> containerSptr;
	PG_DisaFileEntry fileEntry = m_filterModel->getData(index, Identity_Item).get<PG_DisaFileEntry>();

	//TODO check if filecontaner is allready present
	if (fileEntry.getContainer()) return;

	auto container = createFileContainer(fileEntry.getInfo().getType());
	if (!container) return;
	fileEntry.setContainer(container);
	if (!container->open(fileEntry.createReader())) return;

	auto containerModel = PG_new<PG_DisaFileContainerModel>("PG_DisaFileContainerModel", container.get());
	containerModel->setContainer(container.get());
	PG_DisaFileManagerMainModel::get()->pushStack(containerModel);
	
	addOpenedFile(std::move(container));
}