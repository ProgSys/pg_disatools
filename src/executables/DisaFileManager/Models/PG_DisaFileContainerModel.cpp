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
#include <Stream/PG_StreamOutByteFile.h>

std::string PG_DisaFileContainerModel::getFileName() const {
	if (!m_fileContainer) return {};
	if (!m_fileContainer->getFile().isEmpty()) return m_fileContainer->getFile().getFile();
	return m_fileContainer->getParentEntry().getName();
}

PG_ListModelBase* PG_DisaFileContainerModel::getListModel() const {
	return m_selectionModel;
}

const std::string& PG_DisaFileContainerModel::getSearchText() const {
	return m_filterModel? m_filterModel->getSearchText(): getName();
}
void PG_DisaFileContainerModel::setSearchText(const std::string& str) {
	if(m_filterModel) m_filterModel->setSearchText(str);
}

bool PG_DisaFileContainerModel::hasSelection() const {
	return m_selectionModel && m_selectionModel->hasSelection();
}

int PG_DisaFileContainerModel::getSelectionSize() const {
	if (!m_selectionModel || !m_selectionModel->hasSelection()) return 0;
	return m_selectionModel->getSelection().size();
}

PG_ListModelLocation PG_DisaFileContainerModel::getLastSelected() const {
	if (!m_selectionModel || !m_selectionModel->hasSelection()) return {};
	return m_selectionModel->getLocation(*std::prev(m_selectionModel->getSelection().end()));
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
	if (!m_selectionModel) {
		m_selectionModel = PG_new<PG_ListSelectionWrapperModel>("PG_ListSelectionWrapperModel", this);
		connect(m_selectionModel, &PG_ListSelectionWrapperModel::selectionChanged, this, &PG_DisaFileContainerModel::lastSelectedChanged);
	}

	m_selectionModel->setModel(m_filterModel);
	m_filterModel->setModel(fileContainer);
}

static inline std::shared_ptr<PG_DisaFileContainer> createFileContainer(PG_DisaFileInfo::Type type) {
	switch (type) {
	case PG_DisaFileInfo::PSPFS_V1: return std::shared_ptr<PG_PSPFS>(PG_new<PG_PSPFS>("PG_PSPFS"));
	case PG_DisaFileInfo::OLA: return std::shared_ptr<PG_OLA>(PG_new<PG_OLA>("PG_OLA"));
	case PG_DisaFileInfo::SOLA: return std::shared_ptr<PG_SOLA>(PG_new<PG_SOLA>("PG_SOLA"));
	}
	return {};
}

static std::list<std::shared_ptr<PG_DisaFileContainer>> g_openFiles;
using ListIt = std::list<std::shared_ptr<PG_DisaFileContainer>>::iterator;
struct OpenedFiles : public PG_Object {
	using PG_Object::PG_Object;
	ListIt file;
};

void PG_DisaFileContainerModel::addOpenedFile(std::shared_ptr<PG_DisaFileContainer>&& container, std::string&& name) {
	OpenedFiles* openedFiles = findFirstChild<OpenedFiles>(name);
	if (!openedFiles) openedFiles = PG_new<OpenedFiles>(name, this);
	auto findIt = std::ranges::find(g_openFiles, container);
	if (findIt != g_openFiles.end()) {
		openedFiles->file = findIt;
		return;
	}
	g_openFiles.push_back(std::move(container));
	openedFiles->file = std::prev(g_openFiles.end());
}

void PG_DisaFileContainerModel::closeUi() {
	OpenedFiles* openedFiles = findFirstChild<OpenedFiles>("OpenedFiles");
	if (!openedFiles) return; //nothing to do

	if ((*openedFiles->file)->isModified()) return;
	g_openFiles.erase(openedFiles->file);
	delete openedFiles;
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
	auto containerModel = PG_new<PG_DisaFileContainerModel>("PG_DisaFileContainerModel");
	containerModel->setContainer(container.get());
	containerModel->addOpenedFile(std::move(container), "RootFile");

	return containerModel;
}

void PG_DisaFileContainerModel::openEntry(const PG_Index& index) {
	if (!index.isValid()) return;

	std::shared_ptr<PG_DisaFileContainer> containerSptr;
	PG_DisaFileEntry fileEntry = m_filterModel->getData(index, Identity_Item).get<PG_DisaFileEntry>();

	//check if filecontaner is allready present
	std::shared_ptr<PG_DisaFileContainer> container = fileEntry.getContainer<PG_DisaFileContainer>();
	if (!container) {
		container = createFileContainer(fileEntry.getInfo().getType());
		if (!container) return;
		fileEntry.setContainer(container);
		if (!container->open(fileEntry.createReader())) return;
	}

	auto containerModel = PG_new<PG_DisaFileContainerModel>("PG_DisaFileContainerModel");
	containerModel->setContainer(container.get());

	PG_DisaFileManagerMainModel::get()->pushStack(containerModel);
	containerModel->addOpenedFile(std::move(container));
}

static inline void copyFile(const PG_DisaFileEntry& entry, const PG_File& tagetFile, std::vector<char>& buffer) {
	auto reader = entry.createReader();
	if (!reader) {
		OUT_ERROR("Failed to create reader for: " << entry.getName() << "!");
		return;
	}

	PG::STREAM::OutByteFile writer(tagetFile);
	if (!writer.isOpen()) return;

	size_t remainingSize = reader->size();
	size_t toCopy;
	while (remainingSize > 0) {
		toCopy = std::min(remainingSize, buffer.size());
		reader->read(buffer.data(), toCopy);
		remainingSize -= toCopy;
		writer.write(buffer.data(), toCopy);
	}
	writer.close();
}

void PG_DisaFileContainerModel::exportSelected(const PG_Dir& targetDir) {
	if (targetDir.isEmpty() || !targetDir.exists() || !m_selectionModel->hasSelection()) return;

	std::vector<char> data(1024 * 1024);
	for (const auto& index : m_selectionModel->getSelection()) {
		auto var = m_selectionModel->getData(index, EIdentity::Identity_Item);
		if (!var) continue;
		auto entry = var.cast<PG_DisaFileEntry>();
		copyFile(*entry, targetDir.toFilePath(entry->getName()), data);
	}
}

void PG_DisaFileContainerModel::exportLastSelected(const PG_File& targetFile) {
	if (targetFile.isEmpty() || !m_selectionModel->hasSelection()) return;
	auto var = m_selectionModel->getData(*std::prev(m_selectionModel->getSelection().end()), EIdentity::Identity_Item);
	if (!var) return;
	auto entry = var.cast<PG_DisaFileEntry>();
	std::vector<char> data(1024 * 1024);
	copyFile(*entry, targetFile, data);
}