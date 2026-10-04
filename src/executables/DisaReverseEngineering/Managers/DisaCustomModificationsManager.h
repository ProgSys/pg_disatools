#pragma once

#include <DisaCharacterFunctions.h>
#include <unordered_map>
#include <list>
#include <Util/PG_File.h>
#include <Disa/PG_DisaIdentifier.h>
#include <Disa/modifications/PG_DisaCustomModificationsModel.h>
#include <mutex>
#include <condition_variable>

class DisaCustomModificationsManager: public PG_DisaCustomModificationsModel {
public:
	constexpr static uint32_t maxKillLevelSum = 99999999u;

	using PG_DisaCustomModificationsModel::PG_DisaCustomModificationsModel;


	void loadModifications();
	void loadCharactersIntoMememory();

	struct FaceHeader {
		bool isHD;
		bool isOnlyOneImage;
		std::array<char, 0x88> header;
	};

	char* getSpriteData(unsigned int spriteId);
	FaceHeader* getFaceData(unsigned int faceId, bool isHD);


	void addCharacterToParty(const PG_DisaIdentifier& identifier, unsigned short level = 1, int8_t mentorIndex = -1) const;

	const std::unordered_map<PG_DisaIdentifier, DisaAssemblyUnitInfo>& getAssemblyHistory() const { return m_assemblyHistory; };
	void setAssemblyHistory(std::unordered_map<PG_DisaIdentifier, DisaAssemblyUnitInfo>&& history) {
		m_assemblyHistory = std::move(history);
		m_isHistoryChanged = true;
	};
	void recordEnemyKilled(DisaPartyCharacterInfo* characterInfo);
	bool isUpdatedNeededForAssemblyHistory() {
		auto v = m_isHistoryChanged;
		m_isHistoryChanged = false;
		return v;
	}
	/**
	* @brief Is called by main thread to wait for the modding thread to fully load
	* @see wakeUp
	*/
	void waitForModThread();
	void wakeUp();

	static DisaCustomModificationsManager* get();
private:
	std::unordered_map<unsigned int, std::vector<char>> m_spriteData; //loaded sprite files
	std::unordered_map<unsigned int, std::vector<char>> m_faceData; //loaded face files
	bool m_isHistoryChanged = true;
	std::unordered_map<PG_DisaIdentifier, DisaAssemblyUnitInfo> m_assemblyHistory;

	std::mutex m_mtx;
	std::condition_variable m_cv;
	bool m_paused = true;
};