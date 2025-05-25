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
#include <Resources/PG_Texture.h>

class PG_OLA;

struct PG_DisaItemModel {
	enum EPDisaCharacterModelIdentity : Identity {
		Identity_Id = EIdentity::Identity_Custom,
		Identity_Name,
		Identity_Description,
	};

	unsigned short id = 0;
	std::string name;
	std::string nameNormelized;
	std::string description;
	std::string descriptionNormelized;
	unsigned short iconIndex;
	unsigned short iconType;

	operator const std::string& () const { return nameNormelized; }

	PG_Variety getData(Identity identity) const;
};

/**
* @brief Loads disa items (weapons/armor/consumables) from MITEM.DAT
* Also the icon texture to dispay there icon
*/
class PG_DisaItemsModel : public PG_ListModel<PG_DisaItemModel> {
	PG_OBJECT(PG_DisaItemsModel);
	PG_PARENT(PG_ListModel<PG_DisaItemModel>);
public:
	using PG_ListModel<PG_DisaItemModel>::PG_ListModel;

	void init();
	void open(std::shared_ptr<PG_OLA> startDat);

	const PG_DisaItemModel& getItemModel(unsigned short id);
	const PG_Texture& getIconsTexture() const { return m_iconsTexture; };

	using  PG_ListModel<PG_DisaItemModel>::getData;

	static PG_DisaItemsModel* get();
private:
	std::shared_ptr<PG_OLA> m_startDat;
	PG_Texture m_iconsTexture;
};