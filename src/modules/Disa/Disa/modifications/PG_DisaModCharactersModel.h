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

#include <Disa/modifications/PG_DisaModRegistry.h>
#include <Disa/dataStructs/PG_DisaDataCharacter.h>
#include <list>

struct PG_DisaModInfo;
struct PG_DisaModLoadInfo;
class PG_DisaModSpritesModel;
class PG_DisaModMagicModel;

/**
* @brief The "characters" block of every loaded modification.
* @details Loading only parses. A character gets its id once it is registered, which needs the sprites to be resolved first.
*/
class PG_DisaModCharactersModel {
public:
	struct CharacterInfo {
		enum ERecruitmentType {
			RecruitmentType_None, //can't be recruted
			RecruitmentType_Normal, //used normal recruitment logic
			RecruitmentType_Defeated, //at least once killed
			RecruitmentType_Monster, //at least once killed, plus mana cost scaling on number of killed
		};

		PG_DisaModInfo* modInfo = nullptr;
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

	/**
	* @brief Parses the "characters" block of the mod json, does nothing when the block is missing.
	* @details "deriveFrom" only finds characters of the same mod that come earlier in the block.
	*/
	void load(PG_DisaModInfo* modInfo, const PG_DisaModLoadInfo& loadInfo);
	/// Removes the parsed and the registered characters
	void clear();

	/// Every parsed character in parse order. Characters with the same identifier are all kept.
	std::vector<CharacterInfo>& getCharacters() { return m_characters; }
	const std::vector<CharacterInfo>& getCharacters() const { return m_characters; }
	/**
	* @brief Indices into getCharacters(), grouped by mod in the order of modifications.
	* @details Registration has to follow this order, it decides the ids of the characters.
	*/
	std::vector<unsigned int> getIndicesInOrder(const std::list<PG_DisaModInfo>& modifications) const;

	struct Registration {
		unsigned int characterIndex = 0; //index into getCharacters()
		unsigned int registeredIndex = 0; //custom id = registeredIndex + PG_DisaIdentifier::customIdStart
		bool added = false; //false when it took over the id of an earlier character with the same identifier
	};

	/**
	* @brief Registers every character whose sprite can be found and resolves its sprite, skill and character id.
	* @details Drops earlier registrations. Characters are registered in the order of getIndicesInOrder, which decides
	* their ids. A character in the "disa" namespace replaces an original and gets that original id.
	* @returns The registered characters in registration order.
	*/
	std::vector<Registration> resolve(const std::list<PG_DisaModInfo>& modifications, const PG_DisaModSpritesModel& spritesModel,
		const PG_DisaModMagicModel& magicModel);

	/// @returns 0 when not registered
	unsigned int getCharacterId(const PG_DisaIdentifier& identifier) const { return m_registered.getId(identifier); }
	/// @returns A "disa" identifier for original ids, an invalid one for unknown custom ids
	PG_DisaIdentifier getCharacterIdentifier(unsigned int id) const;
	/// @returns nullptr when the id is no registered custom id
	const CharacterInfo* findCharacter(unsigned int id) const;
	/// @returns nullptr when not registered
	const CharacterInfo* findCharacter(const PG_DisaIdentifier& identifier) const;

private:
	struct RegisteredInfo {
		PG_DisaIdentifier identifier;
		unsigned int characterIndex = 0;
	};

	/// @returns true when the id is new, plus the registered index
	std::pair<bool, unsigned int> registerCharacter(unsigned int characterIndex);

	std::vector<CharacterInfo> m_characters;
	PG_DisaModRegistry<RegisteredInfo> m_registered{ PG_DisaIdentifier::customIdStart };
};
