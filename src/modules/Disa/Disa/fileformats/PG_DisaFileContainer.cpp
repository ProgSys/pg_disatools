/*
 * The MIT License (MIT)
 *
 *	Copyright (c) 2020 ProgSys
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */

#include "PG_DisaFileContainer.h"
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamOutByteFile.h>
#include <Stream/PG_StreamInByteWindow.h>
#include <Stream/PG_StreamInOutByteVector.h>
#include <Stream/PG_StreamInByteArray.h>
#include <Resources/PG_Audio.h>
#include <Resources/PG_Texture.h>

#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_Exception.h>
#include <Util/PG_ImageLoad.h>
#include <Disa/fileformats/PG_DisaMPDPack.h>
#include <Disa/fileformats/PG_TX2.h>
#include <ranges>



PG_DisaFileEntry PG_DisaContainerInterface::getParentEntry() const {
	if (auto parent = m_disaFileParent.lock(); parent && m_parentFileEntryLocation >= 0) {
		return dynamic_cast<PG_ListModel<PG_DisaFileEntry>*>(parent.get())->get(m_parentFileEntryLocation);
	}
	return PG_DisaFileEntry();
}

std::shared_ptr<PG::STREAM::In> PG_DisaContainerInterface::createEntryReaderOriginal(const PG_DisaFileEntry& entry) const {
	auto containerReader = createReaderOriginal();
	if (!containerReader) return nullptr;
	return std::make_shared<PG::STREAM::PG_StreamInByteWindow>(containerReader, entry.getInfo().getOffset(), entry.getInfo().getSize());
}

bool PG_DisaContainerInterface::save() {
	if (auto entry = getParentEntry()) {
		return entry.getParent()->save();
	}
	return false;
}

PG_Variety PG_DisaFileEntry::getPreviewImage() const {
	switch (m_info.getType())
	{
	case PG_DisaFileInfo::PNG: {
		PG_Texture texture;
		auto reader = createReader();
		if (!reader) return texture;
		texture.load(m_info.getName().getPath() + "_" + std::to_string((size_t)m_parent), [reader = std::move(reader)]()->PG_Texture::ImageFormats {
			std::vector<unsigned char> data;
			reader->read(data, reader->size());
			PG_RGBAImage image;
			if (!PG_ImageLoad::load(data, image)) {
				OUT_ERROR("Failed to load png!");
			}
			return PG_Texture::ImageFormats{ std::move(image) };
			});
		return texture;
	}break;
	case PG_DisaFileInfo::MPD: {
		auto* fileContainer = dynamic_cast<PG_DisaFileContainer*>(m_parent);
		if (!fileContainer) return PG_Variety();
		PG_DisaMPDPack pack;
		pack.mpd = fileContainer->getLocation(getLocation());

		PG_File mppName = getInfo().getName().replaceFileExtension("mpp");
		pack.mppSD = fileContainer->findLocation(mppName, false);
		if (pack.mppSD) {
			mppName.replaceFileName("NEW_" + mppName.getName());
			pack.mppHD = fileContainer->findLocation(mppName, false);
		}
		else {
			OUT_ERROR("Failed to find mpp file: \""<< mppName<< "\"!");
		}
		return PG_Variety(std::move(pack));
	}break;
	case PG_DisaFileInfo::MPP: {
		auto* fileContainer = dynamic_cast<PG_DisaFileContainer*>(m_parent);
		if (!fileContainer) return PG_Variety();
		PG_DisaMPPPack pack;
		pack.mpp = fileContainer->getLocation(getLocation());
		return PG_Variety(std::move(pack));
	}break;
	case PG_DisaFileInfo::TX2: {
		PG_Texture texture;
		auto reader = createReader();
		if (!reader) return texture;
		texture.load(m_info.getName().getPath() + "_" + std::to_string((size_t)m_parent), [reader = std::move(reader)]() mutable ->PG_Texture::ImageFormats {
			PG_TX2 tx2;
			const bool result = tx2.open(reader.get());
			reader.reset();
			PG_RGBAImage image;
			if (decompressTX2(tx2, image)) {
				OUT_ERROR("Failed to decompress 'tx2'!");
			}
			return PG_Texture::ImageFormats{ std::move(image) };
			});
		return texture;

	} break;
	case PG_DisaFileInfo::OGG: {
		auto reader = createReader();
		if (!reader) return PG_Variety();
		PG_Audio audio;
		audio.load(m_info.getName().getPath() + "_" + std::to_string((size_t)m_parent), reader, PG_Audio::OGG);
		return PG_Variety(std::move(audio));
	} break;
	case PG_DisaFileInfo::TALK_SCRIPT: {
		return std::string("gui/disa/icon_talk_script.png");
	} break;
	case PG_DisaFileInfo::SCRIPT: {
		return std::string("gui/disa/icon_script.png");
	} break;
	default:
		break;

	}
	return PG_Variety();
}

void PG_DisaFileEntry::setLocation(int index) {
	m_index = index;
	if (auto container = getContainer()) {
		container->m_parentFileEntryLocation = index;
	}
}

std::shared_ptr<PG::STREAM::In> PG_DisaFileEntry::createReader() const {
	if (auto c = m_container.lock()) 
		return c->createReader();

	//file is not modified
	return createReaderOriginal();
}

std::shared_ptr<PG::STREAM::In> PG_DisaFileEntry::createReaderOriginal() const {
	//file is not modified
	if (!getInfo().isValid()) return nullptr;

	if (m_parent) {
		return m_parent->createEntryReaderOriginal(*this);
	}

	return nullptr;
}

bool PG_DisaFileEntry::close() {
	if (auto container = getContainer()) {
		dynamic_cast<PG_Object*>(container.get())->setParent(nullptr);
		container.reset();
		setContainerIntern(nullptr);
	}
	return SUCCESS;
}

bool PG_DisaFileEntry::refresh(std::shared_ptr<PG::STREAM::In> reader) {
	if (auto container = getContainer()) {
		return container->refresh(reader);
	}
	return SUCCESS;
}

bool PG_DisaFileEntry::saveTo(const PG_File& file) const {
	PG::STREAM::OutByteFile writer(file);
	if (!writer.isOpen()) return FAILURE;
	auto reader = createReader();
	if (!reader) return FAILURE;
	long long size = reader->size();
	constexpr long long chunkRead = 10000;
	std::vector<char> data;
	while (size > 0){
		reader->read(data, std::min(size, chunkRead));
		size -= chunkRead;
		writer.write(data.data(), data.size());
	}
	return SUCCESS;
}

void PG_DisaFileEntry::setContainerInParent(std::shared_ptr<PG_DisaContainerInterface> container) {
	setContainerIntern(container);
	assert(m_parent);
	auto* listModel = dynamic_cast<PG_ListModel<PG_DisaFileEntry>*>(m_parent);
	listModel->setData(getLocation(), std::move(container), Identity_Container);
}

void PG_DisaFileEntry::setContainerIntern(std::shared_ptr<PG_DisaContainerInterface> container) {
	if (getLocation() < 0 || !m_parent || container == m_container.lock()) return;

	auto* fileContainer = dynamic_cast<PG_DisaFileContainer*>(m_parent);
	if (!fileContainer) return;

	auto& item = fileContainer->getItems()[getLocation()];

	bool isModefiedOrg = false;
	if (auto oldContainer = item.getContainer()) {
		if (oldContainer == container) return;
		isModefiedOrg = oldContainer->isModified();
	}
	
	m_container = container;
	item.m_container = container;

	if (container) {
		container->m_parentFileEntryLocation = getLocation();
		container->m_disaFileParent = m_parent->shared_from_this();

		if (isModefiedOrg != container->isModified()) {
			m_parent->fileEntryModefiedChanged(*this);
		}
	}
}

bool PG_DisaFileEntry::isModified() const {
	if (auto c = m_container.lock()) return c->isModified();
	return false;
}

bool PG_DisaFileEntry::open(std::shared_ptr<PG::STREAM::In> reader) {
	if (getContainer()) return getContainer()->open(reader);
	return FAILURE;
};
bool PG_DisaFileEntry::save(std::shared_ptr<PG::STREAM::Out> writer) const {
	if (getContainer()) return getContainer()->save(writer);
	return FAILURE;
};

PG_DisaFileContainer* PG_DisaFileEntry::getParent() const {
	return dynamic_cast<PG_DisaFileContainer*>(m_parent);
}

void PG_DisaFileEntry::triggerFileChanged() {
	if (m_index < 0) return;

	if (auto* container = dynamic_cast<PG_ListModelBase*>(m_parent)) {
		emit container->itemsChanged(m_index, m_index, { Identity_Modified });
	}
	if (auto* container = dynamic_cast<PG_DisaFileContainer*>(m_parent)) {
		emit container->isModifiedChanged();
	}
}


PG_Variety PG_DisaFileEntry::getData(std::size_t column, Identity identity) const {
	switch (identity)
	{
	case PG_Identity_Text: {
		switch (column) {
			default:
			case 0: return getName();
			case 1: return PG_DisaFileInfo::toString(m_info.getType());
			case 2: return std::to_string(m_info.getSize());
			case 3: return std::to_string(getLocation());
		}
	}
	case PG_Identity_Edit:
		return m_info.getName().getPath();
	case Identity_Modified:
		return isModified();
	case Identity_Container:
		return m_container;
	case PG_Identity_PreviewImage:
		return getPreviewImage();
	}
	return PG_Variety();
}

bool PG_DisaFileEntry::setData(const PG_Variety& data, Identity identity) {
	switch (identity)
	{
	case Identity_Container: {
		if (data.is<std::shared_ptr<PG_DisaContainerInterface>>()) {
			setContainerIntern(data.get<std::shared_ptr<PG_DisaContainerInterface>>());
		}
		else if (data.is<std::weak_ptr<PG_DisaContainerInterface>>()) {
			setContainerIntern(data.get<std::weak_ptr<PG_DisaContainerInterface>>().lock());
		}
		else break;
		return true;
	} break;
	}
	return false;
}

void PG_DisaFileContainer::init() {
	connect(this, &PG_DisaFileContainer::itemsRemoved, this, [this]() {
		if (m_filesRemoved) return;
		m_filesRemoved = true;
		emit isModifiedChanged();
		});
}

PG_DisaFileEntry PG_DisaFileContainer::find(const PG_File& file, bool caseSensitive) const {
	std::vector<PG_DisaFileEntry>::const_iterator findIt;
	if (caseSensitive) {
		findIt = std::ranges::find(m_items, file.getFile(), [](const PG_DisaFileEntry& entry) { return entry.getName(); });
	}
	else {
		findIt = std::ranges::find_if(m_items, [cStr = file.c_str()](const PG_DisaFileEntry& entry) {
			return strcmpi(cStr, entry.getInfo().getName().c_str()) == 0;
		});
	}

	return findIt == m_items.end() ? PG_DisaFileEntry() : *findIt;
}

std::vector<PG_DisaFileEntry> PG_DisaFileContainer::findAll(const std::regex& regex) const {
	std::vector<PG_DisaFileEntry> out;
	for(const auto& item: *this){
		if (std::regex_search(item.getName(), regex))
			out.push_back(item);
	}
	return out;
}

PG_ListModelLocation PG_DisaFileContainer::findLocation(const PG_File& file, bool caseSensitive) const {
	std::vector<PG_DisaFileEntry>::const_iterator findIt;
	if (caseSensitive) {
		findIt = std::ranges::find(m_items, file.getFile(), [](const PG_DisaFileEntry& entry) { return entry.getName(); });
	}
	else {
		findIt = std::ranges::find_if(m_items, [cStr = file.c_str()](const PG_DisaFileEntry& entry) {
			return strcmpi(cStr, entry.getInfo().getName().c_str()) == 0;
			});
	}

	return findIt == m_items.end() ? PG_ListModelLocation() : getLocation(std::distance(m_items.begin(), findIt));
}

bool PG_DisaFileContainer::open(const PG_File& file) {
	clear();
	if (!file.exists()) {
		OUT_ERROR("Target file \"" << file << "\" doesn't exist!");
		return FAILURE;
	}
	m_file = file;

	try {
		auto reader = std::make_shared<PG::STREAM::InByteFile>(file);
		if (reader->isOpen()) {
			const bool result = open(reader);
			return result;
		}
		else {
			OUT_ERROR("Failed to open file: " << file);
			return FAILURE;
		}
	}
	catch (PG_Exception& e) {
		OUT_ERROR("Failed to open file:" << file << "\n" << e.what());
		clear();
		return FAILURE;
	}
	catch (...) {
		OUT_ERROR("Failed to open file: " << file);
		clear();
		return FAILURE;
	}
}

bool PG_DisaFileContainer::refresh(std::shared_ptr<PG::STREAM::In> reader) {
	const auto lookUp = getEntiesWithContainer();
	return open(reader, [&lookUp](PG_DisaFileEntry& entry) {
		if (auto findIt = lookUp.find(entry.getName()); findIt != lookUp.end()) {
			entry.setContainerRestore(findIt->second->getContainer());
			entry.refresh(entry.createReader());
		}
		});

}


bool PG_DisaFileContainer::save() {
	if (getFile().isEmpty()) return FAILURE;

	auto saveFile = getFile().replaceFileName(getFile().getName() + "_tempSave");
	auto writer = std::make_shared<PG::STREAM::OutByteFile>(saveFile);
	if (!writer->isOpen()) return FAILURE;
	if (!save(writer)) {
		saveFile.remove();
		return FAILURE;
	}
	writer.reset();
	doneSaving();


	if (!getFile().remove()) {
		OUT_ERROR("Failed to remove old file: " << getFile() << "!");
		saveFile.remove();
		return FAILURE;
	}
	if (!saveFile.move(getFile())) {
		return FAILURE;
	}
	if (!refresh()) {
		OUT_ERROR("Failed to refresh loaded data from file: " << getFile() << "!");
		return FAILURE;
	}

	return SUCCESS;
}

bool PG_DisaFileContainer::refresh() {
	if (getFile().isEmpty()) return FAILURE;

	auto reader = std::make_shared<PG::STREAM::InByteFile>(getFile());
	if (!reader->isOpen()) return FAILURE;

	m_originalReader = reader;
	if(!refresh(reader)) return FAILURE;

	for (auto& item : *this) {
		if (!item.getContainer()) continue;
		item.refresh(item.createReaderOriginal());
	}

	m_originalReader.reset();
	m_filesRemoved = false;
	emit isModifiedChanged();

	return SUCCESS;
}

std::shared_ptr<PG::STREAM::In> PG_DisaFileContainer::createReader() const {
	//is modified?
	if (isModified()) {
		auto writer = std::make_shared<PG::STREAM::InOutByteVector>();
		if (!save(writer) || writer->size() == 0) {
			writer.reset();
			return nullptr;
		}
		writer->seek(0);
		return writer;
	}

	if (m_file) {
		return std::make_shared<PG::STREAM::InByteFile>(m_file);
	}
	else if (auto entry = getParentEntry()) {
		return entry.createReaderOriginal();
	}
	return m_originalReader;
}

std::shared_ptr<PG::STREAM::In> PG_DisaFileContainer::createReaderOriginal() const {
	if (m_originalReader) return m_originalReader;

	if (m_file) {
		return std::make_shared<PG::STREAM::InByteFile>(m_file);
	}
	else if (auto parent = m_disaFileParent.lock(); parent && m_parentFileEntryLocation >= 0) {
		if (auto* model = dynamic_cast<PG_DisaContainerInterface*>(parent.get())) {
			auto fileEntry = dynamic_cast<PG_ListModel<PG_DisaFileEntry>*>(parent.get())->get(m_parentFileEntryLocation);
			return model->createEntryReaderOriginal(fileEntry);
		}
	}
	return nullptr;
}



bool PG_DisaFileContainer::isModified() const {
	return m_filesRemoved || std::ranges::any_of(*this, [](const PG_DisaFileEntry& entry) { return entry.isModified(); });
}

void PG_DisaFileContainer::fileEntryModefiedChanged(const PG_DisaFileEntry& entry) {
	emit itemsChanged(entry.getLocation(), entry.getLocation(), { PG_Identity_Item, PG_DisaFileEntry::Identity_Modified });
	emit isModifiedChanged();
	auto parent = m_disaFileParent.lock();
	while (parent) {
		if (auto fileContainer = std::dynamic_pointer_cast<PG_DisaFileContainer>(parent)) {
			emit fileContainer->isModifiedChanged();
			parent = fileContainer->m_disaFileParent.lock();
		}
		else {
			break;
		}
	}
}

std::unordered_map<std::string, const PG_DisaFileEntry*> PG_DisaFileContainer::getEntiesWithContainer() const {
	std::unordered_map<std::string, const PG_DisaFileEntry*> lookUp;
	for (auto& entry : *this) {
		if (!entry.getContainer()) continue;
		lookUp[entry.getName()] = &entry;
	}
	return lookUp;
}

/**
* =============== PG_DisaChangedContainer ===============
*/

std::shared_ptr<PG::STREAM::In> PG_DisaChangedContainer::createReaderOriginal() const {
	if (auto entry = getParentEntry()) {
		return entry.createReaderOriginal();
	}
	return nullptr;
}

std::shared_ptr<PG::STREAM::In> PG_DisaChangedContainer::createReader() const {
	if (!isModified()) return createReaderOriginal();
	auto writer = std::make_shared<PG::STREAM::InOutByteVector>();
	save(writer);
	writer->seek(0);
	return writer;
}

void PG_DisaChangedContainer::triggerFileChanged() {
	if (auto entry = getParentEntry()) {
		entry.triggerFileChanged();
	}
	emit isModifiedChanged();
}


/**
* =============== PG_DisaChangedDataContainer ===============
*/

bool PG_DisaChangedDataContainer::open(std::shared_ptr<PG::STREAM::In> reader) {
	if (auto byte = std::dynamic_pointer_cast<PG::STREAM::InByteArray>(reader); byte && byte->data() == m_data.data()) {
		m_data.resize(reader->size());
		return SUCCESS;
	}
	m_data.resize(reader->size());
	reader->read(m_data.data(), m_data.size());
	return SUCCESS;
};

bool PG_DisaChangedDataContainer::save(std::shared_ptr<PG::STREAM::Out> writer) const {
	writer->write((char*)m_data.data(), m_data.size());
	return SUCCESS;
};

std::shared_ptr<PG::STREAM::In> PG_DisaChangedDataContainer::createReader() const {
	auto writer = std::make_shared<PG::STREAM::InByteArray>(m_data.data(), static_cast<unsigned int>(m_data.size()));
	return writer;
}

/**
* =============== PG_DisaChangedReaderContainer ===============
*/
void PG_DisaChangedReaderContainer::copyData(PG::STREAM::In& reader, PG::STREAM::Out& writer) {
	std::vector<char> buffer(1024 * 1024);
	copyData(reader, writer, buffer);
}

void PG_DisaChangedReaderContainer::copyData(PG::STREAM::In& reader, PG::STREAM::Out& writer, std::vector<char>& targetBuffer) {
	size_t remainingSize = reader.size();
	size_t toCopy;
	while (remainingSize > 0) {
		toCopy = std::min(remainingSize, targetBuffer.size());
		reader.read(targetBuffer.data(), toCopy);
		remainingSize -= toCopy;
		writer.write((char*)targetBuffer.data(), toCopy);
	}
}

bool PG_DisaChangedReaderContainer::save(std::shared_ptr<PG::STREAM::Out> writer) const {
	m_reader->rewind();
	if (!m_reader->isOpen()) return FAILURE;
	copyData(*m_reader, *writer);
	return SUCCESS;
}

std::shared_ptr<PG::STREAM::In> PG_DisaChangedReaderContainer::createReader() const {
	return m_reader;
}

/**
* =============== PG_DisaChangedFileContainer ===============
*/
bool PG_DisaChangedFileContainer::save(std::shared_ptr<PG::STREAM::Out> writer) const {
	PG::STREAM::InByteFile reader(m_file);
	if (!reader.isOpen()) return FAILURE;
	PG_DisaChangedReaderContainer::copyData(reader, *writer);
	reader.close();
	return SUCCESS;
}

std::shared_ptr<PG::STREAM::In> PG_DisaChangedFileContainer::createReader() const {
	return std::make_shared<PG::STREAM::InByteFile>(m_file);
}