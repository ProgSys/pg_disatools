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
#include <Util/PG_File.h>
#include <memory>
#include <Resources/PG_Texture.h>

class PG_OLA;
class PG_SOLA;
class PG_PSPFS;
class PG_DisaFileContainer;
class PG_DisaCustomModificationsModel;
class PG_DisaScriptsModel;

struct PG_DisaCharacterModel {
	enum EPDisaCharacterModelIdentity : Identity {
		Identity_Id = EIdentity::PG_Identity_Custom,
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
	unsigned short jumpPower = 0;
	unsigned short movement = 0; /// tiles per turn
	unsigned short movementType = 0; /// 0 walks and jumps, 1 floats over height steps, 2 warps

	//must be buffered
	PG_Texture portrait;
	PG_DisaSprite sprite;

	operator const std::string& () const { return nameNormelized; }

	PG_Variety getData(Identity identity) const;
};


class PG_DisaCharactersModel: public PG_ListModel<PG_DisaCharacterModel> {
	PG_OBJECT(PG_DisaCharactersModel);
	PG_PARENT(PG_ListModel<PG_DisaCharacterModel>);
public: 
	//defined in the .cpp, the unique_ptr members need complete types to be destroyed
	PG_DisaCharactersModel();
	PG_DisaCharactersModel(const std::string& name);
	PG_DisaCharactersModel(std::string&& name);
	~PG_DisaCharactersModel();

	void init();
	void open(const PG_File& exePath, std::shared_ptr<PG_PSPFS> subdata, std::shared_ptr<PG_OLA> startDat);


	PG_Texture getPortrait(unsigned short characterId);
	std::pair<PG_DisaSprite, unsigned short> getSprite(unsigned short characterId); //sprite, color table offset
	const PG_DisaCharacterModel& getCharacterModel(unsigned short characterId);

	PG_Variety getData(PG_Index index, Identity identity) const override;

	/// The custom modifications, null before init
	PG_DisaCustomModificationsModel* getModificationsModel() const { return m_modificationsModel.get(); }

	static PG_DisaCharactersModel* get();
private:

	//original file data
	PG_File m_exePath;
	std::shared_ptr<PG_OLA> m_startDat;
	std::shared_ptr<PG_SOLA> m_portraitDat;

	std::unordered_map<unsigned int, std::shared_ptr<PG_DisaFileContainer>> m_spriteMapping;

	std::unique_ptr<PG_DisaScriptsModel> m_scriptsModel; //tools without the game have no other owner for it
	std::unique_ptr<PG_DisaCustomModificationsModel> m_modificationsModel;
};