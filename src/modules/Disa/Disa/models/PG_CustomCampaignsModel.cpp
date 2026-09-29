#include "PG_CustomCampaignsModel.h"

#include <nlohmann/json.hpp>

#include <Disa/fileformats/PG_DisaMPDPack.h>
#include <Disa/models/PG_DisaMainModel.h>
#include <Disa/PG_DisaMapModel.h>
#include <Resources/PG_ResourceManager.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamInOutByteVector.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Scene/PG_CameraNode.h>
#include <Util/PG_ImageLoad.h>
#include <Util/PG_StringUtil.h>
#include <Scene/Gui/Validator/PG_GuiTextValidators.h>

NLOHMANN_JSON_SERIALIZE_ENUM(EDisaUnlockCriteria, {
	{DisaUnlockCriteria_None, "NONE"},
	{DisaUnlockCriteria_PreviousCompleted, "PREVIOUS_COMPLETED"},
	{DisaUnlockCriteria_PreviousPartialCompleted, "PREVIOUS_PARTIAL_COMPLETED"},
	})

static inline void appendImg(std::string& path) {
	if(!path.empty()) path = "img/" + path;
}

static inline void appendDungeons(std::string& path) {
	if (!path.empty()) path = "dungeons/" + path;
}

static inline void getTo(const nlohmann::json& j, const std::string& filed, auto& target) {
	if (auto findIt = j.find(filed); findIt != j.end()) {
		findIt->get_to(target);
	}
	else {
		using T = std::decay_t<decltype(target)>;
		if constexpr (std::is_same_v<T, int>) target = 0;
		else if constexpr (std::is_same_v<T, std::string>) target = std::string();
		else if constexpr (std::is_same_v<T, EDisaUnlockCriteria>) target = DisaUnlockCriteria_PreviousCompleted;
	}
}

static inline void setTo(nlohmann::json& j, const std::string& filed, const auto& target) {
	using T = std::decay_t<decltype(target)>;
	if constexpr (std::is_same_v<T, std::string>) {
		if(!target.empty()) j[filed] = target;
	}
	else {
		j[filed] = target;
	}
}

template<typename T>
static inline void toListModelFromJson(PG_ListModel<T*>* listModel, const nlohmann::json& j, const std::string& filed) {
	constexpr bool hasParseJson = requires(T t, const nlohmann::json & j2) {
		{ t.parseJson(j2) };
	};

	auto findIt = j.find(filed);
	if (findIt == j.end()) return;

	std::vector<T*> newEntries;
	auto it = listModel->begin();
	for (const auto& jsonEntry : *findIt) {
		T* model = nullptr;
		if (it == listModel->end()) {
			model = PG_new<T>("Entry", listModel);
			newEntries.push_back(model);
		}
		else {
			model = *it;
			++it;
		}

		if constexpr (hasParseJson) {
			model->parseJson(jsonEntry);
		}
	}
	listModel->erase(it, listModel->end());
	listModel->append(newEntries.begin(), newEntries.end());
}

template<typename T>
static inline void toJsonFromListModel(const PG_ListModel<T*>* listModel, nlohmann::json& j, const std::string& filed) {
	constexpr bool hasToJson = requires(T t, nlohmann::json & j2) {
		{ t.toJson(j2) };
	};

	auto& target = j[filed];

	if constexpr (hasToJson) {
		for (const T* item : *listModel) {
			nlohmann::json subJ;
			item->toJson(subJ);
			if (subJ.empty()) continue;
			target.push_back(std::move(subJ));
		}

	}
}

/**
* =============== PG_DisaCustomDungeonModel ===============
*/

void PG_DisaCustomDungeonModel::markModified() {
	if (auto* p = findParent<PG_DisaCampaignJsonContainer>(10)) p->markModified();
}

void PG_DisaCustomDungeonModel::parseJson(const nlohmann::json& j) {
	getTo(j, "name", m_name);
	getTo(j, "identifier", m_identifier);
	getTo(j, "description", m_description);
	getTo(j, "mpd", m_dungeon);
	getTo(j, "bonusRank", m_bonusRank);
	getTo(j, "bgm", m_bgm);
	getTo(j, "unlockCriteria", m_unlockCriteria);

	if (auto findIt = j.find("talkScripts"); findIt != j.end()) {
		getTo(*findIt, "intro", m_scripts.intro);
		getTo(*findIt, "introSkipStory", m_scripts.introSkipStory);
		getTo(*findIt, "exit", m_scripts.exit);
		getTo(*findIt, "exitSkipStory", m_scripts.exitSkipStory);
	}

	if (m_name.empty()) m_name = "Name missing";
	if (m_bonusRank <= 0) m_bonusRank = 10;
	if (m_bgm < 0) m_bgm = 0;
	appendDungeons(m_dungeon);


	emit nameChanged();
	emit identifierChanged();
	emit descriptionChanged();
	emit dungeonChanged();
	emit bonusRankChanged();
	emit bgmChanged();
	emit unlockCriteriaChanged();

	emit scriptIntroChanged();
	emit scriptIntroSkipStoryChanged();
	emit scriptExitChanged();
	emit scriptExitSkipStoryChanged();
}

void PG_DisaCustomDungeonModel::toJson(nlohmann::json& j) const {
	if (m_name.empty()) return;
	setTo(j, "name", m_name);
	setTo(j, "identifier", m_identifier);
	setTo(j, "description", m_description);
	setTo(j, "mpd", PG_File(m_dungeon).getFile());
	setTo(j, "bonusRank", m_bonusRank);
	setTo(j, "bgm", m_bgm);
	setTo(j, "unlockCriteria", m_unlockCriteria);

	if (!m_scripts.empty()) {
		auto& target = j["talkScripts"];
		setTo(target, "intro", m_scripts.intro);
		setTo(target, "introSkipStory", m_scripts.introSkipStory);
		setTo(target, "exit", m_scripts.exit);
		setTo(target, "exitSkipStory", m_scripts.exitSkipStory);
	}
}

PG_CustomCampaignsModel* PG_DisaCustomDungeonModel::getRootModel() const {
	return findParent<PG_DisaCampaignJsonContainer>(10)->getRootModel();
}

void PG_DisaCustomDungeonModel::remove() {
	auto* model = getParent<PG_ListModel<PG_DisaCustomDungeonModel*>>();
	auto findIt = std::ranges::find(*model, this);
	if (findIt == model->end()) return;
	model->remove(std::distance(model->begin(), findIt));
	markModified();
}

void PG_DisaCustomDungeonModel::moveUp() {
	auto* model = getParent<PG_ListModel<PG_DisaCustomDungeonModel*>>();
	auto findIt = std::ranges::find(*model, this);
	if (findIt == model->end() || findIt == model->begin()) return;

	auto index = std::distance(model->begin(), findIt);
	model->swap(index - 1, index);
	markModified();
}

void PG_DisaCustomDungeonModel::moveDown() {
	auto* model = getParent<PG_ListModel<PG_DisaCustomDungeonModel*>>();
	auto findIt = std::ranges::find(*model, this);
	if (findIt == model->end() || findIt == std::prev(model->end())) return;

	auto index = std::distance(model->begin(), findIt);
	model->swap(index, index + 1);
	markModified();
}

/**
* =============== PG_DisaCustomAreaModel ===============
*/

void PG_DisaCustomAreaModel::init() {
	m_dungeons = PG_new<PG_ListModel<PG_DisaCustomDungeonModel*>>("Dungeons", this);
}

void PG_DisaCustomAreaModel::markModified() {
	if(auto* p = findParent<PG_DisaCampaignJsonContainer>(10)) p->markModified();
}

PG_CustomCampaignsModel* PG_DisaCustomAreaModel::getRootModel() const {
	return findParent<PG_DisaCampaignJsonContainer>(10)->getRootModel();
}

void PG_DisaCustomAreaModel::add() {
	auto* m = PG_new<PG_DisaCustomDungeonModel>("Dungeon", this);
	m->setName("New Dungeon");
	m_dungeons->push_back(m);
	markModified();
}

void PG_DisaCustomAreaModel::remove() {
	auto* model = getParent<PG_ListModel<PG_DisaCustomAreaModel*>>();
	auto findIt = std::ranges::find(*model, this);
	if (findIt == model->end()) return;
	model->remove(std::distance(model->begin(), findIt));
	markModified();
}

void PG_DisaCustomAreaModel::moveUp() {
	auto* model = getParent<PG_ListModel<PG_DisaCustomAreaModel*>>();
	auto findIt = std::ranges::find(*model, this);
	if (findIt == model->end() || findIt == model->begin()) return;

	auto index = std::distance(model->begin(), findIt);
	model->swap(index - 1, index);
	markModified();
}

void PG_DisaCustomAreaModel::moveDown() {
	auto* model = getParent<PG_ListModel<PG_DisaCustomAreaModel*>>();
	auto findIt = std::ranges::find(*model, this);
	if (findIt == model->end() || findIt == std::prev(model->end())) return;

	auto index = std::distance(model->begin(), findIt);
	model->swap(index, index + 1);
	markModified();
}

void PG_DisaCustomAreaModel::parseJson(const nlohmann::json& j) {
	getTo(j, "name", m_name);
	getTo(j, "description", m_description);
	getTo(j, "backgroundImage", m_backgroundImage);
	getTo(j, "unlockCriteria", m_unlockCriteria);

	if (m_name.empty()) m_name = "Name missing";
	appendImg(m_backgroundImage);

	emit nameChanged();
	emit descriptionChanged();
	emit backgroundImageChanged();
	emit unlockCriteriaChanged();

	toListModelFromJson(m_dungeons, j, "dungeons");
}

void PG_DisaCustomAreaModel::toJson(nlohmann::json& j) const {
	setTo(j, "name", m_name);
	setTo(j, "description", m_description);
	setTo(j, "backgroundImage", PG_File(m_backgroundImage).getFile());
	setTo(j, "unlockCriteria", m_unlockCriteria);

	toJsonFromListModel(m_dungeons, j, "dungeons");
}

/**
* =============== PG_DisaCampaignJsonContainer ===============
*/
void PG_DisaCampaignJsonContainer::init() {
	m_areas = PG_new<PG_ListModel<PG_DisaCustomAreaModel*>>("Areas", this);
}

bool PG_DisaCampaignJsonContainer::open(std::shared_ptr<PG::STREAM::In> reader)  {
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
		m_areas->clear();
		return FAILURE;
	}
	getTo(j, "id", m_id);
	getTo(j, "name", m_name);
	getTo(j, "author", m_author);
	getTo(j, "description", m_description);
	getTo(j, "previewImage", m_previewImgPath);
	getTo(j, "backgroundImage", m_backgroundImgPath);
	setTo(j, "namespace", m_namespaceMod);

	if (m_name.empty()) m_name = "Name missing";
	appendImg(m_previewImgPath);
	appendImg(m_backgroundImgPath);

	toListModelFromJson(m_areas, j, "areas");

	if (m_isModified) {
		m_isModified = false;
		triggerFileChanged();
	}

	return SUCCESS;
}


bool PG_DisaCampaignJsonContainer::refresh(std::shared_ptr<PG::STREAM::In> reader) {
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

bool PG_DisaCampaignJsonContainer::save(std::shared_ptr<PG::STREAM::Out> writer) const {

	nlohmann::json j;
	if (m_id.empty()) return FAILURE;

	setTo(j, "id", m_id);
	setTo(j, "name", m_name);
	setTo(j, "author", m_author);
	setTo(j, "description", m_description);
	setTo(j, "previewImage", PG_File(m_previewImgPath).getFile());
	setTo(j, "backgroundImage", PG_File(m_backgroundImgPath).getFile());
	setTo(j, "toolVersion", PG_DisaMainModel::getBase()->getConfig().getDisaExeVersion());
	setTo(j, "saveTime", getCurrentTimepoint());
	setTo(j, "namespace", m_namespaceMod);

	toJsonFromListModel(m_areas, j, "areas");

	//save scripts
	if (auto* zipModel = getRootModel()) {
		std::unordered_set<std::string> presentTALK_SCRIPT;
		std::unordered_set<std::string> presentSCRIPT;
		nlohmann::json jScript;
		for (const PG_DisaFileEntry& entry : *zipModel) {
			auto type = entry.getInfo().getType();
			if (type != PG_DisaFileInfo::TALK_SCRIPT && type != PG_DisaFileInfo::SCRIPT) continue;
			auto id = entry.getInfo().getName().getName();
			if (!PG_SimpleWordValidator()(id)) {
				OUT_ERROR("File name for a script is invalid to be used as an identifier!");
				continue;
			}
			if (!presentTALK_SCRIPT.insert(id).second) {
				OUT_ERROR("File name for a script is not unique! (" << id << ")");
				continue;
			}

			jScript["identifier"] = id;
			auto file = entry.getInfo().getName().getFile();
			if (PG_StringUtil::startsWidth(file, "scripts/")) {
				file = { file.begin() + 8, file.end() };
			}
			jScript["file"] = std::move(file);

			if (type == PG_DisaFileInfo::TALK_SCRIPT) {
				j["talkScripts"].push_back(std::move(jScript));
			}
			else {
				j["scripts"].push_back(std::move(jScript));
			}
		}
	}

	std::string data = j.dump(4);
	writer->writeString(data);
	return SUCCESS;
}

void PG_DisaCampaignJsonContainer::markModified() {
	if (m_isModified) return;
	m_isModified = true;
	triggerFileChanged();
}

void PG_DisaCampaignJsonContainer::addArea() {
	auto* area = PG_new<PG_DisaCustomAreaModel>("Area", m_areas);
	area->setName("New Area");
	m_areas->push_back(area);
}

PG_CustomCampaignsModel* PG_DisaCampaignJsonContainer::getRootModel() const {
	return dynamic_cast<PG_CustomCampaignsModel*>(m_disaFileParent.lock().get());
}

/**
* =============== PG_CustomCampaignsModel ===============
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

void PG_CustomCampaignsModel::createNew(std::string name) {
	PG_StringUtil::trim(name);
	if (name.empty()) return;

	auto campaignFile = PG_File("campaign.json");
	PG_DisaFileEntry campaignInfo = find(campaignFile);
	if (campaignInfo) {
		OUT_ERROR("'campaign.json' already exists!");
		return;
	}

	campaignInfo = PG_DisaFileEntry();
	campaignInfo.setParent(this);
	campaignInfo.getInfo().setName(campaignFile);
	campaignInfo.getInfo().setType(PG_DisaFileInfo::JSON);
	campaignInfo.getInfo().setSize(1);
	push_back(campaignInfo);
	campaignInfo = back();
	m_campaignJsonContainer = campaignInfo.open<PG_DisaCampaignJsonContainer>();
	m_campaignJsonContainer->setId(createUUID());
	m_campaignJsonContainer->setName(name);
	m_campaignJsonContainer->setDescription("New custom campaign");
	m_campaignJsonContainer->markModified();
}



void PG_CustomCampaignsModel::afterOpen(zipUPtr& zip) {
	auto campaignInfo = find(PG_File("campaign.json"));
	if (!campaignInfo) {
		clear();
		return;
	}

	m_campaignJsonContainer = campaignInfo.open<PG_DisaCampaignJsonContainer>();
	registerResourceSource();
}

void PG_CustomCampaignsModel::registerResourceSource() {
	m_resourceScope.expire(); //an earlier open of this same model may still be registered

	const std::string id = getResourceNameSpace();
	if (id.empty() || !getFile().exists()) return;

	m_resourceScope = PG_ResourceManager::registerSource<PG_ZIPResourceSource>(getFile(), id);
	if (m_resourceScope && !m_resourceScope->isValid()) m_resourceScope.expire();
}

bool PG_CustomCampaignsModel::save() {
	//a deregistered source closes its handle and stays closed, so the file can be replaced
	m_resourceScope.expire();
	const bool result = PG_ZIP::save();
	registerResourceSource();
	return result;
}



PG_Texture PG_CustomCampaignsModel::getPreviewImg() const {
	PG_Texture texture;
	if (!m_campaignJsonContainer || m_campaignJsonContainer->getPreviewImgPath().empty()) return texture;
	if (auto previewInfo = find(PG_File(m_campaignJsonContainer->getPreviewImgPath()))) {
		texture.load(m_campaignJsonContainer->getId() + m_campaignJsonContainer->getPreviewImgPath(), [reader = previewInfo.createReader()]()->PG_Texture::ImageFormats {
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

constexpr auto dungeonsFolder = "dungeons/";

void PG_CustomCampaignsModel::importFile(const PG_File& file) {
	if (file.isEmpty() || !file.exists()) return;

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
	
}

void PG_CustomCampaignsModel::addNewDungeon(const std::string& name, const PG_DisaFileEntry& mppSD, const PG_DisaFileEntry& mppHD, int topIndex, int sideIndex, int decalIndex) {
	if (name.empty()) return;

	if (find(dungeonsFolder + name + ".mpd", false)) {
		OUT_ERROR("The dungeon with the name \"" << name << "\" already exists!");
		return;
	}

	PG_DisaFileEntry transformMpd;
	transformMpd.setParent(this);
	transformMpd.getInfo().setName(dungeonsFolder + name + ".mpd");
	transformMpd.getInfo().setType(PG_DisaFileInfo::MPD);
	transformMpd.getInfo().setSize(10);

	PG_DisaFileEntry transformSD = mppSD;
	transformSD.setParent(this);
	transformSD.getInfo().setName(dungeonsFolder + name + ".mpp");

	PG_DisaFileEntry transformHD = mppHD;
	transformHD.setParent(this);
	transformHD.getInfo().setName(dungeonsFolder + std::string("new_") + name + ".mpp");

	std::vector<std::shared_ptr<PG_DisaContainerInterface>> keepers;
	

	push_back(transformMpd);
	auto mpdModel = getItems().back().replace<PG_DisaMPDContainer>();
	mpdModel->createModel()->createBaseMap(topIndex, sideIndex, decalIndex);
	keepers.push_back(mpdModel);

	push_back(transformSD);
	keepers.push_back(getItems().back().replace<PG_DisaChangedReaderContainer>(mppSD.createReader()));
	push_back(transformHD);
	keepers.push_back(getItems().back().replace<PG_DisaChangedReaderContainer>(mppHD.createReader()));

	save();

}

void PG_CustomCampaignsModel::customRemove(std::size_t index) {
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
