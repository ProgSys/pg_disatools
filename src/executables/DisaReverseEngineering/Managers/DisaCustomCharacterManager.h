#pragma once

#include <DisaCharacterFunctions.h>
#include <unordered_map>
#include <list>
#include <Util/PG_File.h>
#include <Disa/PG_DisaIdentifier.h>
#include <Disa/PG_DisaCharactersModel.h>


class DisaCustomCharacterManager: public PG_DisaCustomCharactersModel{
public:
	void loadCharacters();
	void loadCharactersIntoMememory();

	struct FaceHeader {
		bool isHD;
		bool isOnlyOneImage;
		std::array<char, 0x88> header;
	};

	char* getSpriteData(unsigned int spriteId);
	FaceHeader* getFaceData(unsigned int faceId, bool isHD);

	void addCharacterToPary(const PG_DisaIdentifier& identifier, unsigned short level = 1, int8_t mentorIndex = -1) const;

	static DisaCustomCharacterManager* get();
private:
	std::unordered_map<unsigned int, std::vector<char>> m_spriteData; //loaded sprite files
	std::unordered_map<unsigned int, std::vector<char>> m_faceData; //loaded face files
};