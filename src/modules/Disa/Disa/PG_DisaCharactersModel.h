/*
 * GNU Lesser General Public License (LGPL-3.0)
 *
 * Copyright (c) 2024 ProgSys
 *
 * This code is part of a free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 * This code is distributed in the hope that it will be useful, without any
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 * or https://www.gnu.org/licenses/lgpl-3.0.md
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <Scene/Model/PG_ListModel.h>
#include <Disa/resources/PG_DisaSprite.h>
#include <Disa/PG_DisaIdentifier.h>
#include <Disa/dataStructs/PG_DisaDataCharacter.h>
#include <Disa/dataStructs/PG_DisaDataMagic.h>
#include <Disa/utils/PG_DisaModInfo.h>
#include <Resources/PG_Texture.h>

class PG_OLA;
class PG_SOLA;
class PG_PSPFS;
class PG_DisaFileContainer;
class PG_DisaModLoadInfo;
class PG_DisaScriptsModel;

struct PG_DisaCharacterModel {
	enum EPDisaCharacterModelIdentity : Identity {
		Identity_Id = EIdentity::Identity_Custom,
		Identity_Name,
		Identity_Title,
		Identity_Portrait,
	};

	unsigned short id = 0;
	std::string name;
	std::string nameNormelized;//without strage half utf8 stuff
	std::string title;
	std::string titleNormelized; //without strage half utf8 stuff
	unsigned short spriteId = 0; //also protrait it
	unsigned short spriteColorOffset = 0;

	//must be buffered
	PG_Texture portrait;
	PG_DisaSprite sprite;

	operator const std::string& () const { return nameNormelized; }

	PG_Variety getData(Identity identity) const;
};

class PG_DisaCustomCharactersModel {
public:
	struct CharacterInfo {
		enum ERecruitmentType {
			RecruitmentType_None, //can't be recruted
			RecruitmentType_Normal, //used normal recruitment logic
			RecruitmentType_Defeated, //at least once killed
			RecruitmentType_Monster, //at least once killed, plus mana cost scaling on number of killed
		};

		PG_DisaIdentifier identifier;
		PG_DisaIdentifier spriteIdentifier;
		PG_DisaIdentifier faceIdentifier;

		PG_DisaDataCharacter charData;
		unsigned int charDataIndex = 0;
		ERecruitmentType recruitmentType = RecruitmentType_Normal; //can be recruted in assebly
		std::string recruitGroupName;
		std::vector<std::pair<PG_DisaIdentifier, uint16_t>> skills; //identifier, unlock level

		static inline std::string toString(ERecruitmentType type) {
			switch (type) {
			case RecruitmentType_None: return "NONE";
			case RecruitmentType_Normal: return "NORMAL";
			case RecruitmentType_Defeated : return "DEFEATED";
			case RecruitmentType_Monster: return "MONSTER";
			default: return {};
			}
		}

		static void fromString(ERecruitmentType& type, std::string str);
	};

	struct MagicInfo {
		PG_DisaIdentifier identifier;

		PG_DisaDataMagic magicData;
	};

	struct CharacterMod : public PG_DisaModInfo {
		std::vector<CharacterInfo> characters;
		std::vector<MagicInfo> magics;
	};

	struct Info {
		CharacterMod* mod = nullptr;
		PG_DisaIdentifier identifier;
		PG_File fileInZip;
		PG_File secondfileInZip;
	};

	struct DisplayInfo {
		unsigned short characterId; 
		unsigned short spriteId;
		unsigned short faceId;
		unsigned short charDatIndex;
	};

	PG_DisaCustomCharactersModel();
	PG_DisaCustomCharactersModel(PG_DisaScriptsModel* scriptsModel);

	std::pair<PG::STREAM::In*, const Info&> createSpriteReader(const std::string& baseDir, unsigned int spriteId);
	std::pair<PG::STREAM::In*, const Info&> createFaceReader(const std::string& baseDir, unsigned int faceId, bool isHD);

	inline const Info& getCharacterInfo(unsigned int id) const { return getInfo(id, m_characters); }
	inline const Info& getSpriteInfo(unsigned int id) const { return getInfo(id, m_sprites); }
	inline const Info& getFaceInfo(unsigned int id) const { return getInfo(id, m_faces); }
	const Info& getCharacterInfoIndex(unsigned int index) const;

	const std::list<CharacterMod>& getMods() const { return m_mods; };

	unsigned int getCharacterId(const PG_DisaIdentifier& identifier) const {
		if (!identifier) return 0;
		auto findIt = m_charactersMapper.find(identifier.getIdentifier());
		if (findIt == m_charactersMapper.end()) return 0;
		return findIt->second + PG_DisaIdentifier::customIdStart;
	}
	PG_DisaIdentifier getCharacterIdentifier(unsigned int id) {
		if (PG_DisaIdentifier::isCustomId(id)) {
			id -= PG_DisaIdentifier::customIdStart;
			if (id >= m_characters.size()) return PG_DisaIdentifier();
			return m_characters[id].identifier;
		}
		else {
			return PG_DisaIdentifier(std::to_string(id), "disa");
		}
	}
	unsigned int getMagicId(const PG_DisaIdentifier& identifier) const {
		if (!identifier) return 0;
		auto findIt = m_magicsMapper.find(identifier.getIdentifier());
		if (findIt == m_magicsMapper.end()) return 0;
		return findIt->second + PG_DisaIdentifier::customMagicIdStart;
	}
	PG_DisaIdentifier getMagicIdentifier(unsigned int id) {
		if (PG_DisaIdentifier::isCustomMagicId(id)) {
			id -= PG_DisaIdentifier::customMagicIdStart;
			if (id >= m_magics.size()) return PG_DisaIdentifier();
			return m_magics[id].identifier;
		}
		else {
			return PG_DisaIdentifier(std::to_string(id), "disa");
		}
	}
	unsigned short getSpriteId(const PG_DisaIdentifier& identifier) const {
		if (!identifier) return 0;
		auto findIt = m_spritesMapper.find(identifier.getIdentifier());
		if (findIt == m_spritesMapper.end()) return 2210; //prinny
		return findIt->second + PG_DisaIdentifier::customIdStart;
	}

	DisplayInfo getDisplayInfo(const PG_DisaIdentifier& identifier) const;
	DisplayInfo getDisplayInfo(const CharacterInfo& c) const;

	PG_DisaScriptsModel* getDisaScriptsModel() const { return m_scriptsModel; }

protected:
	/**
	* @brief Load characters created by the user
	*/
	void loadCustomCharacters(const PG_File& exePath);

	static const Info& getInfo(unsigned int id, const std::vector<Info>& list);
	static const Info& getInfo(const std::string& id, const std::vector<Info>& list, const std::map<std::string, unsigned int>& mapper);

	/**
	* Added to searchable info
	* @returns Bool if added and index
	*/
	static std::pair<bool, unsigned int> addInfo(Info&& info, std::vector<Info>& list, std::map<std::string, unsigned int>& mapper);

	//void loadCharactersFromZip(const PG_File& file);
	void loadCharacters(CharacterMod& mod, const PG_DisaModLoadInfo& info);

	//mod file data
	std::list<CharacterMod> m_mods;

	std::vector<Info> m_characters;
	std::map<std::string, unsigned int> m_charactersMapper;
	std::vector<Info> m_sprites;
	std::map<std::string, unsigned int> m_spritesMapper;
	std::vector<Info> m_faces; //portraits
	std::map<std::string, unsigned int> m_facesMapper;
	std::vector<Info> m_magics; //skills
	std::map<std::string, unsigned int> m_magicsMapper;

	PG_DisaScriptsModel* m_scriptsModel = nullptr;
};


class PG_DisaCharactersModel: public PG_ListModel<PG_DisaCharacterModel>, public PG_DisaCustomCharactersModel {
	PG_OBJECT(PG_DisaCharactersModel);
	PG_PARENT(PG_ListModel<PG_DisaCharacterModel>);
public: 
	using PG_ListModel<PG_DisaCharacterModel>::PG_ListModel;

	void init();
	void open(const PG_File& exePath, std::shared_ptr<PG_PSPFS> subdata, std::shared_ptr<PG_OLA> startDat);


	PG_Texture getPortrait(unsigned short characterId);
	std::pair<PG_DisaSprite, unsigned short> getSprite(unsigned short characterId); //sprite, color table offset
	const PG_DisaCharacterModel& getCharacterModel(unsigned short characterId);

	PG_Variety getData(PG_Index index, Identity identity) const override;

	static PG_DisaCharactersModel* get();
private:

	//original file data
	PG_File m_exePath;
	std::shared_ptr<PG_OLA> m_startDat;
	std::shared_ptr<PG_SOLA> m_portraitDat;

	std::unordered_map<unsigned int, std::shared_ptr<PG_DisaFileContainer>> m_spriteMapping;
};