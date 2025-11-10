#include "PG_CustomCharacterModel.h"

#include <nlohmann/json.hpp>

#include <Disa/fileformats/PG_DisaMPDPack.h>
#include <Disa/models/PG_DisaMainModel.h>
#include <Disa/PG_DisaMapModel.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamInOutByteVector.h>
#include <Util/PG_ImageLoad.h>
#include <Util/PG_StringUtil.h>
#include <Disa/fileformats/PG_JsonUtil.h>


/**
* =============== PG_DisaCharacterJsonContainer ===============
*/
void PG_DisaCharacterJsonContainer::init() {

}


static inline void appendImg(std::string& path) {
	if (!path.empty()) path = "img/" + path;
}

bool PG_DisaCharacterJsonContainer::open(std::shared_ptr<PG::STREAM::In> reader)  {
	if (!reader) return SUCCESS;
	std::string content = reader->readString(reader->size());
	reader.reset();
	nlohmann::json j;
	try {
		j = nlohmann::json::parse(content);
	}
	catch (...) {
		OUT_ERROR("Failed to parse json!");
	}

	if (j.empty() || j.find("id") == j.end()) {
		return FAILURE;
	}
	PG_JsonUtil::readFromJson(j, "id", m_id);
	PG_JsonUtil::readFromJson(j, "name", m_name);
	PG_JsonUtil::readFromJson(j, "author", m_author);
	PG_JsonUtil::readFromJson(j, "description", m_description);
	PG_JsonUtil::readFromJson(j, "previewImage", m_previewImgPath);

	if (m_name.empty()) m_name = "Name missing";
	appendImg(m_previewImgPath);

	if (m_isModified) {
		m_isModified = false;
		triggerFileChanged();
	}

	return SUCCESS;
}


bool PG_DisaCharacterJsonContainer::refresh(std::shared_ptr<PG::STREAM::In> reader) {
	return open(reader);
}

static inline std::string getCurrentTimepoint() {
	// Get the current time
	auto now = std::chrono::system_clock::now();

	// Convert to time_t to work with C-style time functions
	std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

	// Convert time_t to tm struct for formatting
	std::tm localTime = *std::localtime(&currentTime);

	// Format the time into a string
	std::ostringstream oss;
	oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S"); // Example format: "2024-12-05 15:30:45"
	return oss.str();
}

bool PG_DisaCharacterJsonContainer::save(std::shared_ptr<PG::STREAM::Out> writer) const {

	nlohmann::json j;
	if (m_id.empty()) return FAILURE;

	//TODO

	std::string data = j.dump(4);
	writer->writeString(data);
	return FAILURE;
}

void PG_DisaCharacterJsonContainer::markModified() {
	if (m_isModified) return;
	m_isModified = true;
	triggerFileChanged();
}



PG_CustomCharacterModel* PG_DisaCharacterJsonContainer::getRootModel() const {
	return dynamic_cast<PG_CustomCharacterModel*>(m_disaFileParent.lock().get());
}

/**
* =============== PG_CustomCharacterModel ===============
*/
#pragma comment(lib, "rpcrt4.lib")  // UuidCreate - Minimum supported OS Win 2000
#include <windows.h>
#include <iostream>
static inline std::string createUUID() {
	UUID uuid;
	UuidCreate(&uuid);
	char* str;
	UuidToStringA(&uuid, (RPC_CSTR*)&str);
	auto out = std::string(str);
	RpcStringFreeA((RPC_CSTR*)&str);
	return out;
}

void PG_CustomCharacterModel::createNew(std::string name) {
	PG_StringUtil::trim(name);
	if (name.empty()) return;

	auto characterFile = PG_File("character.json");
	PG_DisaFileEntry characterInfo = find(characterFile);
	if (characterInfo) {
		OUT_ERROR("'character.json' already exists!");
		return;
	}

	characterInfo = PG_DisaFileEntry();
	characterInfo.setParent(this);
	characterInfo.getInfo().setName(characterFile);
	characterInfo.getInfo().setType(PG_DisaFileInfo::JSON);
	characterInfo.getInfo().setSize(1);
	push_back(characterInfo);
	characterInfo = back();
	m_characterJsonContainer = characterInfo.open<PG_DisaCharacterJsonContainer>();
	m_characterJsonContainer->setId(createUUID());
	m_characterJsonContainer->setName(name);
	m_characterJsonContainer->setDescription("New custom character");
	m_characterJsonContainer->markModified();
}



void PG_CustomCharacterModel::afterOpen(zipUPtr& zip) {
	auto campaignInfo = find(PG_File("character.json"));
	if (!campaignInfo) {
		campaignInfo = find(PG_File("mod.json"));
	}
	if (!campaignInfo) {
		clear();
		return;
	}

	m_characterJsonContainer = campaignInfo.open<PG_DisaCharacterJsonContainer>();

}



PG_Texture PG_CustomCharacterModel::getPreviewImg() const {
	PG_Texture texture;
	if (!m_characterJsonContainer || m_characterJsonContainer->getPreviewImgPath().empty()) return texture;
	if (auto previewInfo = find(PG_File(m_characterJsonContainer->getPreviewImgPath()))) {
		texture.load(m_characterJsonContainer->getId() + m_characterJsonContainer->getPreviewImgPath(), [reader = previewInfo.createReader()]()->PG_Texture::ImageFormats {
			std::vector<unsigned char> data;
			reader->read(data, reader->size());
			PG_RGBAImage img;
			PG_ImageLoad::load(data, img);
			return PG_Texture::ImageFormats{ std::move(img) };
			});
	}
	return texture;
}


static inline void addFile(PG_ZIP* self, std::vector<std::shared_ptr<PG_DisaChangedDataContainer>>& keepers, const PG_File& file, std::string name,
	const std::function<bool(std::vector<char>& data)>& dataCheck = {}) {
	std::vector<char> data;
	{
		PG::STREAM::InByteFile reader(file);
		if (!reader.isOpen()) return;
		auto size = reader.size();
		if (size > 9900000) {
			OUT_ERROR("File too big!");
			return;
		}
		reader.read(data, reader.size());
	}

	if (dataCheck && !dataCheck(data)) {
		OUT_ERROR("Data is invalid!");
		return;
	}

	PG_DisaFileInfo info;
	info.setName(name);
	info.setSize(data.size());
	info.determinTypeFromFileExtention();
	std::shared_ptr<PG_DisaChangedDataContainer> keep;
	auto findIt = std::ranges::find_if(*self, [&info](const PG_DisaFileEntry& entry) {
		return entry.getInfo().getName() == info.getName();
		});
	if (findIt == self->end()) {
		PG_DisaFileEntry entry = PG_DisaFileEntry(self);
		entry.setInfo(std::move(info));
		self->push_back(entry);
		keep = self->getItems().back().replace<PG_DisaChangedDataContainer>(std::move(data));
	}
	else {
		keep = findIt->replace<PG_DisaChangedDataContainer>(std::move(data));
	}
	keepers.push_back(std::move(keep));
}

constexpr auto spritesFolder = "sprites/";

void PG_CustomCharacterModel::importFile(const PG_File& file) {
	if (file.isEmpty() || !file.exists()) return;
	return; //TODO
	/*
	const std::string fileExtension = file.getFileExtensionFormated();
	if (fileExtension == "png") {

		std::vector<char> data;
		{
			PG::STREAM::InByteFile reader(file);
			if (!reader.isOpen()) return;
			auto size = reader.size();
			if (size > 9900000) {
				OUT_ERROR("File too big!");
				return;
			}
			reader.read(data, reader.size());
		}
		if (!PG_ImageLoad::isPNG(*(std::vector<unsigned char>*)(&data))) {
			OUT_ERROR("Not a PNG image!");
			return;
		}

		std::vector<std::shared_ptr<PG_DisaChangedDataContainer>> keepers;
		addFile(this, keepers, file, "img/" + file.getFile(), [](std::vector<char>& data) {
			if (!PG_ImageLoad::isPNG(*(std::vector<unsigned char>*)(&data))) {
				OUT_ERROR("Not a PNG image!");
				return false;
			}
			return true;
		});
		save();
	}
	else if (fileExtension == "mpd") {
		const PG_File mppSD = file.replaceFileExtension("mpp");
		const PG_File mppHD = mppSD.replaceFileName("NEW_" + mppSD.getName());

		if (!mppSD.exists() || !mppHD.exists()) {
			OUT_ERROR("Asset files are missing: " << mppSD.getFile() << " and " << mppHD.getFile());
			return;
		}

		std::vector<std::shared_ptr<PG_DisaChangedDataContainer>> keepers;
		addFile(this, keepers, file, dungeonsFolder + file.getFile());
		addFile(this, keepers, mppSD, dungeonsFolder + mppSD.getFile());
		addFile(this, keepers, mppHD, dungeonsFolder + mppHD.getFile());
		save();
	}
	else if (fileExtension == "tscript" || fileExtension == "script") {
		std::vector<std::shared_ptr<PG_DisaChangedDataContainer>> keepers;
		addFile(this, keepers, file, "scripts/" + file.getFile());
		save();
	} else {
		OUT_ERROR("File extension '"<< fileExtension<< "' is not supported!");
		return;
	}
	*/
}

void PG_CustomCharacterModel::customRemove(std::size_t index) {
	if (index > size()) return;

	if (m_items[index].getInfo().getType() == PG_DisaFileInfo::MPD) {
		//special case: also remove both MPPs
		auto name = m_items[index].getInfo().getName();
	
		remove(index);

		PG_File::replaceFileExtension(name, "mpp");
		if (auto loc = findLocation(name)) {
			remove(loc.getIndex());
		}

		PG_File::prependToFileName(name, "new_");
		if (auto loc = findLocation(name)) {
			remove(loc.getIndex());
		}

		return;
	}

	remove(index);
}
