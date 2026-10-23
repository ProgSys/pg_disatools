#include "PG_DisaFileContainerModel.h"

#include <Scene/Model/PG_ListSelectionWrapperModel.h>
#include <Scene/Model/PG_ListModelFilter.h>
#include <Disa/fileformats/PG_DisaFileContainer.h>
#include <Disa/fileformats/PG_DisaFileContainerFactory.h>
#include <Models/PG_DisaFileFilterModel.h>
#include <Models/PG_DisaFileManagerMainModel.h>
#include <Disa/fileformats/PG_DisaFileTests.h>
#include <Disa/fileformats/PG_OLA.h>
#include <Disa/fileformats/PG_SOLA.h>
#include <Disa/fileformats/PG_KM3.h>
#include <Disa/fileformats/PG_PSPFS.h>
#include <Disa/fileformats/PG_Script.h>
#include <Disa/fileformats/PG_TalkScript.h>
#include <Disa/fileformats/PG_IMY.h>
#include <Stream/PG_StreamOutByteFile.h>
#include <Stream/PG_StreamInOutByteVector.h>
#include <algorithm>


bool PG_DisaFileContainerModel::isModified() const {
	if (!m_fileContainer) return false;
	return m_fileContainer->isModified();
}

PG_File PG_DisaFileContainerModel::getOriginFile() const {
	if (!m_fileContainer) return {};
	if (!m_fileContainer->getFile().isEmpty()) return m_fileContainer->getFile();
	auto parent = m_fileContainer->getParentEntry().getParent();
	while (parent) {
		if (parent->getFile()) return parent->getFile();
		parent = parent->getParentEntry().getParent();
	}
	return {};
}

std::string PG_DisaFileContainerModel::getFileName() const {
	if (!m_fileContainer) return {};
	if (!m_fileContainer->getFile().isEmpty()) return m_fileContainer->getFile().getFile();
	return m_fileContainer->getParentEntry().getName();
}

const std::string& PG_DisaFileContainerModel::getSearchText() const {
	return m_filterModel? m_filterModel->getSearchText(): getName();
}
void PG_DisaFileContainerModel::setSearchText(const std::string& str) {
	if(m_filterModel) m_filterModel->setSearchText(str);
}

const std::vector<PG_DisaFileInfo::Type>& PG_DisaFileContainerModel::getFileTypes() const {
	static const std::vector<PG_DisaFileInfo::Type> empty;
	return m_filterModel ? m_filterModel->getFileTypes() : empty;
}
void PG_DisaFileContainerModel::setFileTypes(std::vector<PG_DisaFileInfo::Type> types) {
	if (m_filterModel) m_filterModel->setFileTypes(std::move(types));
}

bool PG_DisaFileContainerModel::hasSelection() const {
	return m_selectionModel && m_selectionModel->hasSelection();
}

bool PG_DisaFileContainerModel::hasSingleSelection() const {
	return m_selectionModel && m_selectionModel->getSelection().size() == 1;
}

int PG_DisaFileContainerModel::getSelectionSize() const {
	if (!m_selectionModel || !m_selectionModel->hasSelection()) return 0;
	return m_selectionModel->getSelection().size();
}

int PG_DisaFileContainerModel::getSelectionByteSize() const {
	if (!m_selectionModel || !m_selectionModel->hasSelection()) return 0;

	int total = 0;
	for (const auto& index : m_selectionModel->getSelection()) {
		auto var = m_selectionModel->getData(index, EIdentity::PG_Identity_Item);
		if (!var) continue;
		auto entry = var.cast<PG_DisaFileEntry>();
		if (!entry) continue;
		total += (int)entry->getInfo().getSize();
	}
	return total;
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
		connect(m_filterModel, &PG_DisaFileFilterModel::fileTypesChanged, this, &PG_DisaFileContainerModel::fileTypesChanged);
	}
	if (!m_selectionModel) {
		m_selectionModel = PG_new<PG_ListSelectionWrapperModel>("PG_ListSelectionWrapperModel", this);
		connect(m_selectionModel, &PG_ListSelectionWrapperModel::selectionChanged, this, &PG_DisaFileContainerModel::lastSelectedChanged);
	}

	connect(m_fileContainer, &PG_DisaFileContainer::isModifiedChanged, this, &PG_DisaFileContainerModel::isModifiedChanged);

	m_selectionModel->setModel(m_filterModel);
	m_filterModel->setModel(m_fileContainer);
}

struct Ownership: public PG_Object {
	std::shared_ptr<PG_DisaFileContainer> container;
};

void PG_DisaFileContainerModel::setOwningContainer(std::shared_ptr<PG_DisaFileContainer>&& container) {
	if (!container) return;
	
	Ownership* ownership = container->findFirstChild<Ownership>("Ownership");
	if (!ownership) {
		ownership = new Ownership();
		ownership->setName("Ownership");
		ownership->setParent(container.get());
		ownership->container = container;
	}
	m_ownershipContainer = std::move(container);
}

void PG_DisaFileContainerModel::openUi() {
	assert(m_ownershipContainer);
	if (!m_ownershipContainer) return;
	Ownership* ownership = m_ownershipContainer->findFirstChild<Ownership>("Ownership");
	if (!ownership) return;

	disconnect(ownership->container.get(), &PG_DisaFileContainer::isModifiedChanged, ownership, nullptr);
}

void PG_DisaFileContainerModel::closeUi() {
	assert(m_ownershipContainer);
	Ownership* ownership = m_ownershipContainer->findFirstChild<Ownership>("Ownership");
	if (!ownership) return;

	if (!ownership->container->isModified()) {
		delete ownership;
		return;
	}
	connect(ownership->container.get(), &PG_DisaFileContainer::isModifiedChanged, ownership, [ownership] {
		if (ownership->container->isModified()) return;
		ownership->setParent(nullptr);
		ownership->deleteLater();
		ownership->container.reset();
	});
}

PG_DisaFileContainerModel* PG_DisaFileContainerModel::tryOpen(const PG_File& file, std::shared_ptr<PG::STREAM::In> reader) {
	auto container = PG_Disa::createFileContainer(reader.get());
	if (!container) return nullptr;
	container->setFile(file);
	container->open(reader);
	auto containerModel = PG_new<PG_DisaFileContainerModel>("PG_DisaFileContainerModel");
	containerModel->setContainer(container.get());
	containerModel->setOwningContainer(std::move(container));

	return containerModel;
}

void PG_DisaFileContainerModel::exportToFile(const PG_File& file) {
	if (!m_fileContainer || file.isEmpty()) return;

	auto saveFile = file.replaceFileName(file.getName() + "_tempSave");
	auto writer = std::make_shared<PG::STREAM::OutByteFile>(saveFile);
	if (!writer->isOpen()) {
		OUT_ERROR("Failed to open file " << file << "!");
		return;
	}
	if (!m_fileContainer->save(writer)) {
		OUT_ERROR("Failed to save file " << file << "!");
		saveFile.remove();
		return;
	}
	writer.reset();

	if (file.exists()) {
		if (!file.remove()) {
			OUT_ERROR("Failed to remove original file " << file << "!");
			saveFile.remove();
			return;
		}
	}

	if (!saveFile.move(file)) {
		saveFile.remove();
		return;
	}

	if (file == m_fileContainer->getFile()) {
		if (!m_fileContainer->refresh()) {
			OUT_ERROR("Failed to refresh loaded data from file: " << file << "!");
			return;
		}
	}
}

void PG_DisaFileContainerModel::openEntry(PG_DisaFileEntry& fileEntry) {
	if (!fileEntry) return;
	//check if filecontaner is allready present
	std::shared_ptr<PG_DisaFileContainer> container = fileEntry.getContainer<PG_DisaFileContainer>();
	if (!container) {
		container = PG_Disa::createFileContainer(fileEntry.getInfo().getType());
		if (!container) return;
		fileEntry.setContainer(container);
		if (!container->open(fileEntry.createReader())) return;
	}

	auto containerModel = PG_new<PG_DisaFileContainerModel>("PG_DisaFileContainerModel");
	containerModel->setContainer(container.get());

	containerModel->setOwningContainer(std::move(container));
	PG_DisaFileManagerMainModel::get()->pushStack(containerModel);
}

void PG_DisaFileContainerModel::openEntry(const PG_Index& index) {
	if (!index.isValid()) return;

	PG_DisaFileEntry fileEntry = m_filterModel->getData(index, PG_Identity_Item).get<PG_DisaFileEntry>();
	openEntry(fileEntry);
}

void PG_DisaFileContainerModel::openEntryLastSelected() {
	if (!m_selectionModel->hasSelection()) return;
	auto var = m_selectionModel->getData(*std::prev(m_selectionModel->getSelection().end()), EIdentity::PG_Identity_Item);
	if (!var) return;
	PG_DisaFileEntry& entry = var.get<PG_DisaFileEntry>();
	openEntry(entry);
}

static inline void copyFile(const PG_DisaFileEntry& entry, const PG_File& tagetFile, std::vector<char>& buffer) {
	auto reader = entry.createReader();
	if (!reader) {
		OUT_ERROR("Failed to create reader for: " << entry.getName() << "!");
		return;
	}

	PG::STREAM::OutByteFile writer(tagetFile);
	if (!writer.isOpen()) return;

	PG_DisaChangedReaderContainer::copyData(*reader, writer, buffer);
	writer.close();
}

void PG_DisaFileContainerModel::exportSelected(const PG_Dir& targetDir) {
	if (targetDir.isEmpty() || !targetDir.exists() || !m_selectionModel->hasSelection()) return;

	std::vector<char> data(1024 * 1024);
	for (const auto& index : m_selectionModel->getSelection()) {
		auto var = m_selectionModel->getData(index, EIdentity::PG_Identity_Item);
		if (!var) continue;
		auto entry = var.cast<PG_DisaFileEntry>();
		copyFile(*entry, targetDir.toFilePath(entry->getName()), data);
	}
}

void PG_DisaFileContainerModel::exportLastSelected(const PG_File& targetFile) {
	if (targetFile.isEmpty() || !m_selectionModel->hasSelection()) return;
	auto var = m_selectionModel->getData(*std::prev(m_selectionModel->getSelection().end()), EIdentity::PG_Identity_Item);
	if (!var) return;
	auto entry = var.cast<PG_DisaFileEntry>();
	std::vector<char> data(1024 * 1024);
	copyFile(*entry, targetFile, data);
}

void PG_DisaFileContainerModel::replaceLastSelected(const PG_File& targetFile, bool replaceName) {
	if (targetFile.isEmpty() || !targetFile.exists()) {
		OUT_ERROR("The file " << targetFile << " dosn't exist!");
		return;
	}
	if (!m_selectionModel->hasSelection()) return;

	auto var = m_selectionModel->getData(*std::prev(m_selectionModel->getSelection().end()), EIdentity::PG_Identity_Item);
	if (!var) return;
	PG_DisaFileEntry& entry = var.get<PG_DisaFileEntry>();

	auto container = std::make_shared<PG_DisaChangedFileContainer>(targetFile);
	if (auto reader = container->createReader(); reader && reader->isOpen()) {
		entry.getInfo().setSize(reader->size());
		entry.getInfo().determinTypeFromFileFormat(reader.get());
		if (replaceName) {
			entry.getInfo().setName(targetFile.getFile());
		}
	}
	else {
		OUT_ERROR("Failed to open file " << targetFile << "!");
		return;
	}

	if (auto oldContainer = entry.getContainer()) {
		PG_DisaFileManagerMainModel::get()->eraseChangedFile(oldContainer);
	}
	entry.setContainer(container);
	m_fileContainer->setData(entry.getLocation(), entry, EIdentity::PG_Identity_Item);
	PG_DisaFileManagerMainModel::get()->addChangedFile(std::move(container));
}

void PG_DisaFileContainerModel::importFiles(const std::vector<PG_File>& files) {
	if (files.empty() || !m_fileContainer) return;

	//insert below the last selected file, if nothing is selected then at the very end
	int insertAt = (int)m_fileContainer->size();
	if (m_selectionModel && m_selectionModel->hasSelection()) {
		auto var = m_selectionModel->getData(*std::prev(m_selectionModel->getSelection().end()), EIdentity::PG_Identity_Item);
		if (var) {
			const PG_DisaFileEntry& selected = var.get<PG_DisaFileEntry>();
			if (selected.getLocation() >= 0) insertAt = selected.getLocation() + 1;
		}
	}

	for (const PG_File& file : files) {
		if (file.isEmpty() || !file.exists()) {
			OUT_ERROR("The file " << file << " dosn't exist!");
			continue;
		}

		auto container = std::make_shared<PG_DisaChangedFileContainer>(file);
		PG_DisaFileInfo info(file);
		if (auto reader = container->createReader(); reader && reader->isOpen()) {
			info.setName(file.getFile());
			info.setSize(reader->size());
			info.determinTypeFromFileFormat(reader.get());
		}
		else {
			OUT_ERROR("Failed to open file " << file << "!");
			continue;
		}

		auto& items = m_fileContainer->getItems();
		PG_DisaFileEntry entry(std::move(info), m_fileContainer);
		//containers like SCRIPT.DAT keep data per entry that the file itself doesn't hold
		entry.setExtraData(m_fileContainer->createDefaultExtraData(entry.getName()));
		m_fileContainer->insert(std::next(items.begin(), insertAt), std::move(entry));
		items[insertAt].setContainer(container);
		PG_DisaFileManagerMainModel::get()->addChangedFile(std::move(container));
		++insertAt;
	}
}

void PG_DisaFileContainerModel::decompressAndReplaceLastSelected() {
	if (!m_selectionModel->hasSelection()) return;

	auto var = m_selectionModel->getData(*std::prev(m_selectionModel->getSelection().end()), EIdentity::PG_Identity_Item);
	if (!var) return;
	PG_DisaFileEntry& entry = var.get<PG_DisaFileEntry>();

	if (entry.getInfo().getType() != PG_DisaFileInfo::COLA) {
		OUT_ERROR("Only `Compressed offset list archive` (COLA) can be decompressed!");
		return;
	}

	if (entry.isModified()) {
		OUT_ERROR("File \""<< entry.getName() <<"\" already modified please save or reset!");
		return;
	}
	PG::STREAM::InOutByteVector writer;
	{
		auto reader = entry.createReader();
		if (!PG_Disa::decompressIMYPackage(reader.get(), &writer) || writer.size() == 0) {
			OUT_ERROR("File \"" << entry.getName() << "\" failed to decompress!");
			return;
		}
	}

	auto container = std::make_shared<PG_DisaChangedDataContainer>(std::move(writer.getData()));
	writer.close();
	entry.setContainer(container);
	entry.getInfo().setType(PG_DisaFileInfo::OLA);
	m_fileContainer->setData(entry.getLocation(), entry, EIdentity::PG_Identity_Item);
	PG_DisaFileManagerMainModel::get()->addChangedFile(std::move(container));
}


void PG_DisaFileContainerModel::deleteSelected() {
	if (!m_fileContainer || !m_selectionModel || !m_selectionModel->hasSelection()) return;

	m_selectionModel->removeSelection();
}
