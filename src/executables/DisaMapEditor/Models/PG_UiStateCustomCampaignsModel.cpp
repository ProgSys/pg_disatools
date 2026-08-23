#include "PG_UiStateCustomCampaignsModel.h"

#include <Models/PG_DisaMapEditorMainModel.h>
#include <nlohmann/json.hpp>

#include <PG_DisaDefines.h>
#include <Models/PG_CategoryFilterModel.h>
#include <Models/PG_DisaMapContainerModel.h>
#include <Disa/fileformats/PG_DisaMPDPack.h>
#include <Disa/PG_DisaMapModel.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamInOutByteVector.h>
#include <Scene/PG_CameraNode.h>
#include <Util/PG_ImageLoad.h>

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
* =============== PG_UiStateCustomCampaignsModel ===============
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


void PG_UiStateCustomCampaignsModel::setSubUiState(SubUiState state) {
	if (m_subUiState == state) return;
	auto oldState = getUiState();
	m_subUiState = state;

	updateFilter();
	emit subUiStateChanged();
	if(getUiState() != oldState)
		emit uiStateChanged();
}

PG_UiStateCustomCampaignsModel::UiState PG_UiStateCustomCampaignsModel::getUiState() const  { 
	switch (m_subUiState)
	{
	case PG_UiStateCustomCampaignsModel::SubUiState_Settings: return UiState::CustomCampaignSetting;
	case PG_UiStateCustomCampaignsModel::SubUiState_DungeonsPlan: return UiState::CustomCampaignPlan;
	default: return UiState::GridFileView;
	}
 }

std::vector<PG_UiStateCustomCampaignsModel::ModeIcon> PG_UiStateCustomCampaignsModel::getSubModeIcons() const {
	return {
		{SubUiState_Settings, "Settings", "gui/icon_tag.png"},
		{SubUiState_DungeonsPlan, "Plan", "gui/icon_flag.png"},
		{SubUiState_Dungeons, "Dungeons", "gui/icon_actor.png"},
		{SubUiState_Images, "Images", "gui/icon_texture.png"},
		{SubUiState_Scripts, "Scripts", "gui/icon_script.png"},
	};
}

void PG_UiStateCustomCampaignsModel::openUi() {
	if (!m_filterModel) {
		m_filterModel = PG_new<PG_CategoryFilterModel>("PG_CategoryFilterModel", this);
		m_filterModel->setModel(this);
	}
	updateFilter();
}

void PG_UiStateCustomCampaignsModel::closeUi() {
	if (!m_filterModel) return;
	delete m_filterModel;
	m_filterModel = nullptr;
}

void PG_UiStateCustomCampaignsModel::tileClicked(PG_ListModelLocation location) {
	auto var = location.getData(PG_Identity_PreviewImage);
	if (var.is<PG_DisaMPDPack>()) {
		auto pack = var.get<PG_DisaMPDPack>();
		PG_DisaMapEditorMainModel* mainModel = PG_DisaMapEditorMainModel::get();
		mainModel->pushStateStack(PG_DisaMapContainerModel::open(pack.getMPD(), pack.getMPPSD(), pack.getMPPHD()));
		return;
	}
}

void PG_UiStateCustomCampaignsModel::tileClicked(PG_ListModelLocation location, PG_CameraNode* camera) {
	auto var = location.getData(PG_Identity_PreviewImage);
	if (var.is<PG_DisaMPDPack>()) {
		auto pack = var.get<PG_DisaMPDPack>();
		PG_DisaMapEditorMainModel* mainModel = PG_DisaMapEditorMainModel::get();
		auto model = PG_DisaMapContainerModel::open(pack.getMPD(), pack.getMPPSD(), pack.getMPPHD());
		if(camera) model->setCameraStartTransformation(camera->getTransformation());
		mainModel->pushStateStack(model);
		return;
	}
}


void PG_UiStateCustomCampaignsModel::updateFilter() {
	if (!m_filterModel) return;
	switch (m_subUiState)
	{
	case PG_UiStateCustomCampaignsModel::SubUiState_Settings:
		break;
	default:
	case PG_UiStateCustomCampaignsModel::SubUiState_DungeonsPlan:
	case PG_UiStateCustomCampaignsModel::SubUiState_Dungeons:
		m_filterModel->setFilterCategory(PG_CategoryFilterModel::Dungeons);
		break;
	case PG_UiStateCustomCampaignsModel::SubUiState_Images:
		m_filterModel->setFilterCategory(PG_CategoryFilterModel::Image);
		break;
	case PG_UiStateCustomCampaignsModel::SubUiState_Scripts:
		m_filterModel->setFilterCategory(PG_CategoryFilterModel::Script);
		break;
	}
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