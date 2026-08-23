#include "PG_DisaMapListModel.h"

#include <ranges>
#include <Disa/utils/PG_FileDialog.h>
#include <Disa/fileformats/PG_MPD.h>
#include <Base/Thread/PG_Concurrent.h>
#include <Stream/PG_StreamOutByteFile.h>
#include <Stream/PG_StreamInOutByteFile.h>
#include <Stream/PG_StreamInOutByteVector.h>
#include <Scene/Gui/PG_GuiProgressDialog.h>
#include <SDLApp/PG_SDLApplication.h>
#include <Scene/Gui/PG_GuiRectangle.h>

void PG_DisaMapListModel::parseFileSystem() {
	std::vector<PG_DisaMPDPack> filtered;
	for (const PG_DisaFileEntry& entry : *m_fileSystem) {
		auto info = entry.getInfo();
		if (info.getType() == PG_DisaFileInfo::MPD) {
			PG_DisaMPDPack item;
			item.mpd = m_fileSystem->getLocation(entry.getLocation());
			item.mppSD = m_fileSystem->findLocation(info.getName().replaceFileExtension("mpp"), false);
			if (item.mppSD) {
				item.mppHD = m_fileSystem->findLocation("NEW_" + std::string(item.getMPPSD().getInfo().getName()), false);
			}
			filtered.push_back(std::move(item));
		}
	}
	std::ranges::sort(filtered, [](const PG_DisaMPDPack& a, const PG_DisaMPDPack& b) {
		return a.getMPD().getInfo().getName().getName() < b.getMPD().getInfo().getName().getName();
		});

	set(std::move(filtered));
}

void PG_DisaMapListModel::open(const PG_File& file) {
	if (!isEnabled()) return;
	setEnabled(false);

	if (!m_fileSystem) m_fileSystem = std::make_shared<PG_PSPFS>();

	if (!m_fileSystem->PG_DisaFileContainer::open(file)) {
		clear();
	}
	parseFileSystem();
	emit hasChangesChanged();
	setEnabled(true);
}

void PG_DisaMapListModel::save() {
	save(m_fileSystem->getFile());
}

void PG_DisaMapListModel::save(const PG_File& file) {
	/*
	if (!isEnabled() || m_changes.empty()) return; //nothing to save
	setEnabled(false);

	auto guiRoot = findParent<PG_GuiItem>();
	auto progress = PG_GuiProgressDialog::create(guiRoot);
	if (!progress) {
		OUT_ERROR("Progress dialog is missing!");
		setEnabled(true);
		return;
	}
	progress->setTitleText("Saving...");
	progress->getProgressBar()->setMaterial(PG_Material("M_DisaGuiAnimatedRectangle"));

	const auto progressFunc = [progress](int curr, int total) {
		if (curr % 1000 == 0) OUT_INFO("Saving: " << curr << "/" << total);
		if (curr % 100 == 0) {
			progress->postCallEvent(PG_Event_Behaviour_Replace, &PG_GuiProgressDialog::setProgress, float(curr) / float(total));
		}
		};

	const auto convertFunc = [this](PG_DisaFileInfo& info, std::vector<char>& data) {
		auto findIt = m_changes.find(info.getOffset());
		if (findIt == m_changes.end()) {
			return;
		}
		const PG_DisaFileInfo& infoNew = findIt->second;
		auto reader = infoNew.createReader();
		if (!reader) {
			return;
		}

		if (infoNew.getType() == PG_DisaFileInfo::MPD_JSON) {
			PG_MPD mpd;
			mpd.openJson(reader);
			delete reader;
			PG::STREAM::InOutByteVector writer(std::move(data));
			mpd.save(&writer);
			info.setType(PG_DisaFileInfo::MPD);
			info.setDecompressedSize(0);
			data = std::move(writer.getData());
		}
		else {
			data.resize(reader->size());
			reader->readBytes(data, data.size());
			delete reader;
			info.setDecompressedSize(0);
		}
		};

	//special handling for fast saving
	if (m_changes.size() == 1 && file == m_fileSystem.getFile() && file.exists() &&
		m_fileSystem.getSize() > m_changes.begin()->second.getIndex()) {
		//we just replace data in file system as size is the same
		const auto& item = m_changes.begin()->second;
		auto& replaceItem = m_fileSystem[item.getIndex()];
		const auto originalOffset = replaceItem.getOffset();
		const auto originalSize = replaceItem.getSize();
		replaceItem.setOffset(m_changes.begin()->first);
		std::vector<char> data;
		convertFunc(m_fileSystem[item.getIndex()], data);
		replaceItem.setOffset(originalOffset);
		replaceItem.setSize(originalSize);

		if (data.size() == originalSize) {
			PG::STREAM::InOutByteFile writer(file);
			if (writer.isOpen()) {
				writer.seek(m_fileSystem[item.getIndex()].getOffset());
				writer.writeData(data);
				writer.close();
			}
			else {
				OUT_ERROR("Failed to open file " << file << "!");
			}

			m_changes.clear();
			emit hasChangesChanged();
			progress->close();
			setEnabled(true);
			return;
		}
	}


	for (auto& change : m_changes) {
		m_fileSystem[change.second.getIndex()].setOwner(nullptr);
		m_fileSystem[change.second.getIndex()].setOffset(change.first);
	}


	const bool fileExists = file.exists();
	PG_File tempFile = fileExists? file.replaceFileName(file.getName() + "_buffer"): file;

	auto concurrent = PG_Concurrent::runLambda<false>(this, [&]() {
		{
			PG::STREAM::OutByteFile writer(tempFile);
			if (!m_fileSystem.save(&writer, convertFunc, progressFunc)) {
				OUT_ERROR("Save of temp file '" << tempFile << "' failed!");
				m_changes.clear();
				tempFile.remove();
				setEnabled(true);
				return;
			}
			writer.close();
			m_changes.clear();
		}

		if (fileExists) {
			if (!file.remove()) {
				OUT_ERROR("Failed to remove old file: " << file << "!");
				return;
			}

			if (!tempFile.move(file.getPath())) {
				OUT_ERROR("Failed to rename temp file: " << tempFile << " to " << file << "! I guess you will need to do it by hand.");
			}
		}
	});

	while (concurrent->isRunning()) {
		static_cast<PG_SDLApplication*>(PG_Application::get())->step(); //@TODO replace with event loop!!!
	}
	delete concurrent;

	OUT_INFO("Saving Done");
	parseFileSystem();
	emit hasChangesChanged();
	progress->close();
	setEnabled(true);
	*/
}

PG_Variety PG_DisaMapListModel::getData(PG_Index index, Identity identity) const {
	switch (identity) 	{
		case PG_Identity_Text: {
			std::string name = m_items[index].getMPD().getInfo().getName().getName();
			return name;
		}
		case PG_Identity_Item: {
			auto item = m_items[index];
			return item;
		}
		case Identity_CanJSONExport:
		case Identity_CanGLTFExport: {
			return m_items[index].getMPD().getInfo().isValid();
		}
		case Identity_HasChanges: {
			return false;  
		}
		case Identity_File: return m_fileSystem->getFile();
	}
	return PG_ListModel<PG_DisaMPDPack>::getData(index, identity);
}



void PG_DisaMapListModel::exportData(PG_ListModelLocation location) {
	if (!location.isValid() || location.getModelRoot() != this) return;
	auto data = location.getDataT<PG_DisaMPDPack>(PG_Identity_Item);
	PG_File file = PG_FileDialog::save(this, "MPD", data.getMPD().getInfo().getName().getFile());
	if (file.isEmpty()) return;
	auto reader = m_items[location.getIndex()].getMPD().createReader();
	std::vector<char> fileData;
	reader->read(fileData, reader->size());
	reader.reset();

	file.write(fileData);
}

void PG_DisaMapListModel::importData(PG_ListModelLocation location) {
	if (!location.isValid() || location.getModelRoot() != this) return;
	/*
	PG_File file = openDialog(this, "MPD");
	if (file.isEmpty()) return;
	auto fileInfo = PG_DisaFileInfo(file);
	fileInfo.setIndex(get(location.getIndex()).mpd.getIndex());
	fileInfo.setType(PG_DisaFileInfo::MPD);
	m_changes[location.getIndex()] = std::move(fileInfo);
	emit itemsChanged(location.getIndex(), location.getIndex(), { PG_Identity_Item });
	emit hasChangesChanged();
	*/
}

void PG_DisaMapListModel::exportJSON(PG_ListModelLocation location) const{
	/*
	if (!location.isValid() || location.getModelRoot() != this) return;
	auto data = location.getDataT<PG_DisaMPDPack>(PG_Identity_Item);
	std::string fileName = data.mpd.getName().getName() + ".json";
	PG_File file = saveDialog(this, "json", fileName);
	if (file.isEmpty()) return;
	if (data.mpd.getType() == PG_DisaFileInfo::MPD_JSON) {
		OUT_WARNING("File is already a Json, so we just copy it!");
		data.mpd.getName().copy(fileName);
	}else{
		PG_MPD mpd;
		auto ss = m_items[location.getIndex()].mpd.createReader();
		const bool success = mpd.open(ss);
		delete ss;
		if (!success) {
			OUT_ERROR("Failed to open MPD file!");
			return;
		}
		mpd.saveJson(file);
	}
	*/
}

void PG_DisaMapListModel::importJSON(PG_ListModelLocation location) {
	/*
	if (!location.isValid() || location.getModelRoot() != this) return;
	PG_File file = openDialog(this, "json");
	if (file.isEmpty()) return;
	auto fileInfo = PG_DisaFileInfo(file);
	fileInfo.setIndex(get(location.getIndex()).mpd.getIndex());
	fileInfo.setType(PG_DisaFileInfo::MPD_JSON);
	m_changes[location.getIndex()] = std::move(fileInfo);
	emit itemsChanged(location.getIndex(), location.getIndex(), { PG_Identity_Item });
	emit hasChangesChanged();
	*/
}