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

#include <Disa/PG_DisaIdentifier.h>
#include <Disa/utils/PG_DisaModInfo.h>
#include <Util/PG_File.h>
#include <list>
#include <memory>
#include <optional>
#include <string>

class PG_DisaModSpritesModel;
class PG_DisaModFacesModel;
class PG_DisaModMagicModel;
class PG_DisaModCharactersModel;
class PG_DisaScriptsModel;
class PG_DisaTranslationModel;

/**
* @brief All mods in the CustomModifications folder. Each content type of a mod is kept by its own model.
* @details The sub-models are created with this model and are never null.
*/
class PG_DisaCustomModificationsModel {
public:
	struct DisplayInfo {
		unsigned short characterId = 0;
		unsigned short spriteId = 0;
		unsigned short faceId = 0;
		unsigned short charDataIndex = 0;
	};

	/**
	* @brief The "betterControls" block of a mod.json, the improved mouse control of battles is on while a loaded mod has one.
	* @details The blocks are layers: every mod with one overrides only the keys it sets, a mod higher in the load order
	* over the ones below it, the rest keep the defaults. Names without a namespace are taken from the mod that sets
	* them, so "M_X" of a mod "pg_mod" is "pg_mod::M_X". A name that starts with "::" is a global one.
	*/
	struct BetterControls {
		bool isRadialMenu = true; /// "radialMenu", the radial command menu instead of the original one
		std::string radialMenuStyle; /// "radialMenuStyle", the style json in the mod, empty for the built in style
		std::string pathMaterial = "M_DisaTilePath"; /// "pathMaterial", draws the path of a move
	};

	/**
	* @param scriptsModel Not owned, receives the "scripts" and "talkScripts" blocks. Must not be null.
	* @param translationModel Not owned, receives the "translation" block. May be null.
	*/
	PG_DisaCustomModificationsModel(PG_DisaScriptsModel* scriptsModel, PG_DisaTranslationModel* translationModel);
	~PG_DisaCustomModificationsModel();

	/**
	* @brief Loads every enabled mod in the CustomModifications folder.
	* @details Mods are loaded bottom first, so the top of the load order wins when two use the same identifier.
	* Afterwards getModifications() is sorted top first.
	*/
	void loadModifications(const PG_File& exePath);

	const std::list<PG_DisaModInfo>& getModifications() const { return m_modifications; }
	/// The settings of the improved mouse control, nullopt while no mod turns it on
	const std::optional<BetterControls>& getBetterControls() const { return m_betterControls; }

	PG_DisaModSpritesModel* getSpritesModel() const { return m_spritesModel.get(); }
	PG_DisaModFacesModel* getFacesModel() const { return m_facesModel.get(); }
	PG_DisaModMagicModel* getMagicModel() const { return m_magicModel.get(); }
	PG_DisaModCharactersModel* getCharactersModel() const { return m_charactersModel.get(); }
	PG_DisaScriptsModel* getScriptsModel() const { return m_scriptsModel; }
	PG_DisaTranslationModel* getTranslationModel() const { return m_translationModel; }

	/// @returns All zero when the character is not registered
	DisplayInfo getDisplayInfo(const PG_DisaIdentifier& identifier) const;

private:
	std::list<PG_DisaModInfo> m_modifications; //a list, the entries of the sub-models point into it
	std::optional<BetterControls> m_betterControls;

	std::unique_ptr<PG_DisaModSpritesModel> m_spritesModel;
	std::unique_ptr<PG_DisaModFacesModel> m_facesModel;
	std::unique_ptr<PG_DisaModMagicModel> m_magicModel;
	std::unique_ptr<PG_DisaModCharactersModel> m_charactersModel;

	PG_DisaScriptsModel* m_scriptsModel = nullptr;
	PG_DisaTranslationModel* m_translationModel = nullptr;
};
