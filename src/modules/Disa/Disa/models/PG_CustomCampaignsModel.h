#pragma once

#include <Disa/fileformats/PG_ZIP.h>
#include <Disa/resources/PG_ZIPResourceSource.h>
#include <Resources/PG_Texture.h>
#include <Resources/PG_ResourceSourceScope.h>
#include <Util/PG_ValueSetter.h>
#include <Scene/Model/PG_ListModel.h>
#include <nlohmann/json_fwd.hpp>

class PG_CategoryFilterModel;
class PG_CustomCampaignsModel;
class PG_CameraNode;


enum EDisaUnlockCriteria : int {
	DisaUnlockCriteria_None, //is allways unlocked
	DisaUnlockCriteria_PreviousCompleted, //previous areas/dungions were completed
	DisaUnlockCriteria_PreviousPartialCompleted, //if any other was completed
};

struct PG_DisaCustomDungeonScripts {
	std::string intro;
	std::string introSkipStory;
	std::string exit;
	std::string exitSkipStory;

	inline bool empty() const {
		return intro.empty() && introSkipStory.empty() && exit.empty() && exitSkipStory.empty();
	}
};

class PG_DisaCustomDungeonModel : public PG_Object {
	PG_OBJECT(PG_DisaCustomDungeonModel);
	PG_PARENT(PG_Object);

	PG_PROPERTY(std::string name GET getName SET setName CHANGED nameChanged);
	PG_PROPERTY(std::string identifier GET getIdentifier SET setIdentifier CHANGED identifierChanged);
	PG_PROPERTY(std::string description GET getDescription SET setDescription CHANGED descriptionChanged);
	PG_PROPERTY(std::string dungeon GET getDungeon SET setDungeon CHANGED dungeonChanged);
	PG_PROPERTY(int bonusRank GET getBonusRank SET setBonusRank CHANGED bonusRankChanged);
	PG_PROPERTY(int bgm GET getBGM SET setBGM CHANGED bgmChanged);
	PG_PROPERTY(EDisaUnlockCriteria unlockCriteria GET getUnlockCriteria SET setUnlockCriteria CHANGED unlockCriteriaChanged);

	PG_PROPERTY(std::string scriptIntro GET getScriptIntro SET setScriptIntro CHANGED scriptIntroChanged);
	PG_PROPERTY(std::string scriptIntroSkipStory GET getScriptIntroSkipStory SET setScriptIntroSkipStory  CHANGED scriptIntroSkipStoryChanged);
	PG_PROPERTY(std::string scriptExit GET getScriptExit SET setScriptExit CHANGED scriptExitChanged);
	PG_PROPERTY(std::string scriptExitSkipStory GET getScriptExitSkipStory SET setScriptExitSkipStory CHANGED scriptExitSkipStoryChanged);


	PG_PROPERTY(PG_ListModelBase* rootModel GET getRootModel);

signals:
	void nameChanged();
	void identifierChanged();
	void descriptionChanged();
	void dungeonChanged();
	void bonusRankChanged();
	void bgmChanged();
	void unlockCriteriaChanged();

	void scriptIntroChanged();
	void scriptIntroSkipStoryChanged();
	void scriptExitChanged();
	void scriptExitSkipStoryChanged();
public:
	using PG_Object::PG_Object;

	void markModified();

	void parseJson(const nlohmann::json& j);
	void toJson(nlohmann::json& j) const;

	const std::string& getName() const { return m_name; }
	void setName(const std::string& value) { PG_ValueSetter::change(m_name, value, this, &PG_DisaCustomDungeonModel::nameChanged, &PG_DisaCustomDungeonModel::markModified); }

	const std::string& getIdentifier() const { return m_identifier; }
	void setIdentifier(const std::string& value) { PG_ValueSetter::change(m_identifier, value, this, &PG_DisaCustomDungeonModel::identifierChanged, &PG_DisaCustomDungeonModel::markModified); }

	const std::string& getDescription() const { return m_description; }
	void setDescription(const std::string& value) { PG_ValueSetter::change(m_description, value, this, &PG_DisaCustomDungeonModel::descriptionChanged, &PG_DisaCustomDungeonModel::markModified); }

	const std::string& getDungeon() const { return m_dungeon; }
	void setDungeon(const std::string& value) { PG_ValueSetter::change(m_dungeon, value, this, &PG_DisaCustomDungeonModel::dungeonChanged, &PG_DisaCustomDungeonModel::markModified); }

	int getBonusRank() const { return m_bonusRank; }
	void setBonusRank(int value) { PG_ValueSetter::change(m_bonusRank, value, this, &PG_DisaCustomDungeonModel::bonusRankChanged, &PG_DisaCustomDungeonModel::markModified); }

	int getBGM() const { return m_bgm; }
	void setBGM(int value) { PG_ValueSetter::change(m_bgm, value, this, &PG_DisaCustomDungeonModel::bgmChanged, &PG_DisaCustomDungeonModel::markModified); }

	EDisaUnlockCriteria getUnlockCriteria() const { return m_unlockCriteria; }
	void setUnlockCriteria(EDisaUnlockCriteria value) { PG_ValueSetter::change(m_unlockCriteria, value, this, &PG_DisaCustomDungeonModel::unlockCriteriaChanged, &PG_DisaCustomDungeonModel::markModified); }

	const std::string& getScriptIntro() const { return m_scripts.intro; }
	void setScriptIntro(const std::string& value) { PG_ValueSetter::change(m_scripts.intro, value, this, &PG_DisaCustomDungeonModel::scriptIntroChanged, &PG_DisaCustomDungeonModel::markModified); } 

	const std::string& getScriptIntroSkipStory() const { return m_scripts.introSkipStory; }
	void setScriptIntroSkipStory(const std::string& value) { PG_ValueSetter::change(m_scripts.introSkipStory, value, this, &PG_DisaCustomDungeonModel::scriptIntroSkipStoryChanged, &PG_DisaCustomDungeonModel::markModified); }

	const std::string& getScriptExit() const { return m_scripts.exit; }
	void setScriptExit(const std::string& value) { PG_ValueSetter::change(m_scripts.exit, value, this, &PG_DisaCustomDungeonModel::scriptExitChanged, &PG_DisaCustomDungeonModel::markModified); }

	const std::string& getScriptExitSkipStory() const { return m_scripts.exitSkipStory; }
	void setScriptExitSkipStory(const std::string& value) { PG_ValueSetter::change(m_scripts.exitSkipStory, value, this, &PG_DisaCustomDungeonModel::scriptExitSkipStoryChanged, &PG_DisaCustomDungeonModel::markModified); }

	PG_CustomCampaignsModel* getRootModel() const;
receivers:
	void remove();
	void moveUp();
	void moveDown();
private:
	std::string m_name;
	std::string m_identifier;
	std::string m_description;
	std::string m_dungeon;
	int m_bonusRank = 10;
	int m_bgm = 1; //background music
	EDisaUnlockCriteria m_unlockCriteria = DisaUnlockCriteria_PreviousCompleted;
	PG_DisaCustomDungeonScripts m_scripts;
};


class PG_DisaCustomAreaModel : public PG_Object {
	PG_OBJECT(PG_DisaCustomAreaModel);
	PG_PARENT(PG_Object);

	PG_PROPERTY(std::string name GET getName SET setName CHANGED nameChanged);
	PG_PROPERTY(std::string description GET getDescription SET setDescription CHANGED descriptionChanged);
	PG_PROPERTY(std::string backgroundImage GET getBackgroundImage SET setBackgroundImage CHANGED backgroundImageChanged);
	PG_PROPERTY(EDisaUnlockCriteria unlockCriteria GET getUnlockCriteria SET setUnlockCriteria CHANGED unlockCriteriaChanged);

	PG_PROPERTY(PG_ListModelBase* dungeons GET getDungeons);
	PG_PROPERTY(PG_ListModelBase* rootModel GET getRootModel);

signals:
	void nameChanged();
	void descriptionChanged();
	void backgroundImageChanged();
	void unlockCriteriaChanged();
public:
	using PG_Object::PG_Object;

	void init();
	void markModified();

	void parseJson(const nlohmann::json& j);
	void toJson(nlohmann::json& j) const;

	const std::string& getName() const { return m_name; }
	void setName(const std::string& value) { PG_ValueSetter::change(m_name, value, this, &PG_DisaCustomAreaModel::nameChanged, &PG_DisaCustomAreaModel::markModified); }

	const std::string& getDescription() const { return m_description; }
	void setDescription(const std::string& value) { PG_ValueSetter::change(m_description, value, this, &PG_DisaCustomAreaModel::descriptionChanged, &PG_DisaCustomAreaModel::markModified); }

	const std::string& getBackgroundImage() const { return m_backgroundImage; }
	void setBackgroundImage(const std::string& value) { PG_ValueSetter::change(m_backgroundImage, value, this, &PG_DisaCustomAreaModel::backgroundImageChanged, &PG_DisaCustomAreaModel::markModified); }

	EDisaUnlockCriteria getUnlockCriteria() const { return m_unlockCriteria; }
	void setUnlockCriteria(EDisaUnlockCriteria value) { PG_ValueSetter::change(m_unlockCriteria, value, this, &PG_DisaCustomAreaModel::unlockCriteriaChanged, &PG_DisaCustomAreaModel::markModified); }

	PG_ListModel<PG_DisaCustomDungeonModel*>* getDungeons() const { return m_dungeons; }
	PG_CustomCampaignsModel* getRootModel() const;
receivers:
	void add();
	void remove();
	void moveUp();
	void moveDown();
private:
	std::string m_name;
	std::string m_description;
	std::string m_backgroundImage;

	EDisaUnlockCriteria m_unlockCriteria = DisaUnlockCriteria_PreviousCompleted;

	PG_ListModel<PG_DisaCustomDungeonModel*>* m_dungeons = nullptr;
};

/**
* @brief Conainer for modefierd data
*/
class PG_DisaCampaignJsonContainer : public PG_DisaChangedContainer {
	PG_OBJECT(PG_DisaCampaignJsonContainer);
	PG_PARENT(PG_DisaChangedContainer);

	PG_PROPERTY(std::string name GET getName SET setName CHANGED nameChanged);
	PG_PROPERTY(std::string author GET getAuthor SET setAuthor CHANGED authorChanged);
	PG_PROPERTY(std::string description GET getDescription SET setDescription CHANGED descriptionChanged);
	PG_PROPERTY(std::string previewImgPath GET getPreviewImgPath SET setPreviewImgPath CHANGED previewImgPathChanged);
	PG_PROPERTY(std::string backgroundImgPath GET getBackgroundImgPath SET setBackgroundImgPath CHANGED backgroundImgPathChanged);

	PG_PROPERTY(PG_ListModelBase* areas GET getAreas);
	PG_PROPERTY(PG_ListModelBase* rootModel GET getRootModel);
signals:
	void uiStateChanged();
	void subUiStateChanged();

	void idChanged();
	void nameChanged();
	void authorChanged();
	void descriptionChanged();
	void previewImgPathChanged();
	void backgroundImgPathChanged();
public:
	using PG_DisaChangedContainer::PG_DisaChangedContainer;

	void init();

	bool open(std::shared_ptr<PG::STREAM::In> reader) override;
	bool refresh(std::shared_ptr<PG::STREAM::In> reader) override;

	bool save(std::shared_ptr<PG::STREAM::Out> writer) const override;

	bool isModified() const override { return m_isModified; }
	void markModified();

	const std::string& getId() const { return m_id; }
	void setId(const std::string& id) { PG_ValueSetter::change(m_id, id, this, &PG_DisaCampaignJsonContainer::idChanged, &PG_DisaCampaignJsonContainer::markModified); }

	const std::string& getName() const { return m_name; }
	void setName(const std::string& name) { PG_ValueSetter::change(m_name, name, this, &PG_DisaCampaignJsonContainer::nameChanged, &PG_DisaCampaignJsonContainer::markModified); }

	const std::string& getAuthor() const { return m_author; }
	void setAuthor(const std::string& author) { PG_ValueSetter::change(m_author, author, this, &PG_DisaCampaignJsonContainer::authorChanged, &PG_DisaCampaignJsonContainer::markModified); }

	const std::string& getDescription() const { return m_description; }
	void setDescription(const std::string& value) { PG_ValueSetter::change(m_description, value, this, &PG_DisaCampaignJsonContainer::descriptionChanged, &PG_DisaCampaignJsonContainer::markModified); }

	const std::string& getPreviewImgPath() const { return m_previewImgPath; }
	void setPreviewImgPath(const std::string& value) { PG_ValueSetter::change(m_previewImgPath, value, this, &PG_DisaCampaignJsonContainer::previewImgPathChanged, &PG_DisaCampaignJsonContainer::markModified); }

	const std::string& getBackgroundImgPath() const { return m_backgroundImgPath; }
	void setBackgroundImgPath(const std::string& value) { PG_ValueSetter::change(m_backgroundImgPath, value, this, &PG_DisaCampaignJsonContainer::backgroundImgPathChanged, &PG_DisaCampaignJsonContainer::markModified); }

	PG_ListModel<PG_DisaCustomAreaModel*>* getAreas() const { return m_areas; }
	PG_CustomCampaignsModel* getRootModel() const;
receivers:
	void addArea();
private:
	bool m_isModified = false;

	std::string m_id;
	std::string m_name;
	std::string m_author;
	std::string m_description;
	std::string m_previewImgPath;
	std::string m_backgroundImgPath;
	std::string m_namespaceMod;

	PG_ListModel<PG_DisaCustomAreaModel*>* m_areas = nullptr;
};

class PG_CustomCampaignsModel : public PG_ZIP {
	PG_OBJECT(PG_CustomCampaignsModel);
	PG_PARENT(PG_ZIP);
	PG_PROPERTY(PG_Object* campaignJson GET getCampaignJsonContainer);
public:
	enum SubUiState: int {
		SubUiState_Settings,
		SubUiState_DungeonsPlan,
		SubUiState_Dungeons,
		SubUiState_Images,
		SubUiState_Scripts,
		SubUiState_SIZE,
	};

	using PG_ZIP::PG_ZIP;

	void createNew(std::string name);

	PG_DisaCampaignJsonContainer* getCampaignJsonContainer() const { return m_campaignJsonContainer.get(); }
	/// The namespace the campaign content is offered to the resource manager under, empty when it has no id
	std::string getResourceNameSpace() const { return m_campaignJsonContainer ? m_campaignJsonContainer->getId() : std::string(); }

	PG_Texture getPreviewImg() const;

receivers:
	void importFile(const PG_File& file);
	void addNewDungeon(const std::string& name, const PG_DisaFileEntry& mppSD, const PG_DisaFileEntry& mppHD, int topIndex = 1, int sideIndex = 2, int decalIndex = 0);
	void customRemove(std::size_t index); //called when item shall be removed

	void saveToFile() { save(); }

private:
	void afterOpen(zipUPtr& zip) final;
	/**
	* @brief Offers everything the campaign carries to the resource manager under its own id.
	* @details A texture of it then loads as "<id>::<path in the zip>", and letting the campaign
	* go takes all of it back out. Does nothing for a campaign with no id.
	*/
	void registerResourceSource();

	std::shared_ptr<PG_DisaCampaignJsonContainer> m_campaignJsonContainer;
	/// The registration, so the campaigns content lives exactly as long as the campaign does
	PG_ResourceSourceScope<PG_ZIPResourceSource> m_resourceScope;
};