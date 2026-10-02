#pragma once
#include <string>
#include <vector>
#include <stack>
#include <variant>
#include <Util/PG_File.h>
#include <glm/glm.hpp>
#include <Disa/PG_DisaIdentifier.h>
#include <Disa/utils/PG_DisaModInfo.h>

class DisaSettings;
class PG_DisaScriptsModel;

/**
* @brief Manages custom campaigns
* They are located inside the folder "mod/CustomCampaigns".
* Manages the menu and data retrieval of current selected map (zip).
*/
class DisaCustomCampaignManager {
public:
	enum EMenuState: int {
		None = 0,
		CampaignList,
		CampaignAreaList,
		CampaignDungeonList
	};

	struct TalkScripts {
		PG_DisaIdentifier intro;
		PG_DisaIdentifier introSkipStory;
		PG_DisaIdentifier exit;
		PG_DisaIdentifier exitSkipStory;
	};

	struct CampaignDungeon {
		std::string name;
		std::string description;
		unsigned int bonusRank = 10;
		std::string mpd;
		std::string mpp;
		std::string mppHD;
		unsigned int bgm = 0;
		TalkScripts scripts;
	};

	struct CampaignArea {
		std::string name;
		std::string backgroundImage;
		std::vector<CampaignDungeon> entries;
	};

	struct Campaign : PG_DisaModInfo{
		std::string description;
		std::string author;
		std::string previewImage;
		std::string backgroundImage;

		std::vector<CampaignArea> entries;

		inline bool operator==(const Campaign& other) const { return id == other.id; }

		using PG_DisaModInfo::open;
		/// Puts the zip in use and reads the areas and dungeons from it.
		void open();
	};

	using EntriesVariant = std::variant<DisaCustomCampaignManager*, Campaign*, CampaignArea*>;

	DisaCustomCampaignManager(PG_DisaScriptsModel* scriptsModel);

	bool isActive() const { return !m_entriesState.empty(); }
	bool isEmpty() const { return m_campaigns.empty(); }
	unsigned int size() const { return m_campaigns.size(); }

	void loadCampaigns();

	void activate();
	void reactivate();
	void diactivate();
	/**
	* @brief Will prosses managers game state
	* Will procces user events and render, with the intent to replace the original code 
	* @returs true when process is active
	* @see isActive
	*/
	bool process();

	void readMPP(int* dataLocation, bool hdTextures) const;
	void readMPD(int* dataLocation) const;

	/**
	* @brief Will trigger the game to load a test map
	* Map shall be located at "mod/CustomCampaigns/" with "debugMap.mpd" and "debugMap.MPP"/"new_debugMap.MPP"
	*/
	void runTestMap();
	void readMPPTestMap(int* dataLocation, bool hdTextures) const;
	void readMPDTestMap(int* dataLocation) const;
	
	/**
	* @brief Get the talk stripts of the current loaded custom map
	* @returns TalkScripts
	*/
	TalkScripts* getActiveTalkScripts() const; //first campain identifier, second talk scripts

	/**
	* @brief Returns the current selected background music of selected/loaded map
	*/
	unsigned int getBGM() const;
	/// The namespace the resources of the active campaign resolve in, empty without one
	std::string getActiveResourceNameSpace() const { return m_activeCampaign ? m_activeCampaign->namespaceMod : std::string(); }
	/**
	* @returns Singelton
	*/
	static DisaCustomCampaignManager* get();

private:
	void processUserEvents();
	void processUserRender();

	void renderCustomImage(const std::string& id, const std::string& path);
	void renderCustomImage(const std::string& id, const std::string& path, const glm::vec2& pos, const glm::vec2& size);

	std::vector<Campaign> m_campaigns;

	struct EntriesState {
		EMenuState type = None;
		EntriesVariant entry;
		int size = 0;
		int width = 0;
		int scrollOffsetIndex = 0;
		int hoverIndex = 0;
		bool isCustomBackground = false;

		std::string titleText;
		std::string infoText;

		inline int getIndex() const { return std::min(scrollOffsetIndex + hoverIndex, size - 1); }
		EntriesState createChildEntry(int index);

		const std::string& getEntryName(int index) const;
		const std::string& getDescription() const;
		const std::string& getBackgroundImagePath() const;
		const std::string& getPreviewImagePath() const;
		const std::string& getAutor() const;
		unsigned int getBGM() const;
		
		inline void resetHover() {
			hoverIndex = 0;
			scrollOffsetIndex = 0;
		}
	};

	EntriesState m_selectEntry; //the entry that is currently hovered
	std::vector<EntriesState> m_entriesState;
	Campaign* m_activeCampaign = nullptr;
	PG_DisaScriptsModel* m_scriptsModel = nullptr;
};