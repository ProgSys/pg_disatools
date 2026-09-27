/*
 * GNU Lesser General Public License (LGPL-3.0)
 *
 * Copyright (c) 2026 ProgSys
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

#include <Resources/PG_Model.h>
#include <Scene/Model/PG_ListModel.h>

class PG_DisaAssetsModel;

/**
* @brief One model an object can be placed with.
*/
struct PG_DisaObjectCatalogItem {
	bool isMod() const { return !resourceName.empty(); }

	int geometryIndex = -1;   /// index into PG_DisaAssetsModel::getGeometries(), only for Disa
	std::string resourceName; /// fully qualified, only for Mod
	std::string displayName;
	mutable PG_Model model;   /// the Mod model, requested on first use
};

/**
* @brief Every model the open map can place objects with.
* @details The geometries of the map assets come first, so their index in here is their geometry index.
* After them come the model blocks the campaign the map was opened from carries.
* PG_Identity_PreviewImage answers the PG_Model, PG_Identity_Text the name to show.
*/
class PG_DisaObjectCatalogModel : public PG_ListModel<PG_DisaObjectCatalogItem> {
	PG_OBJECT(PG_DisaObjectCatalogModel);
	PG_PARENT(PG_ListModel<PG_DisaObjectCatalogItem>);
public:
	enum ECustomIdentity : Identity {
		Identity_IsMod = PG_Identity_Custom,
		Identity_GeometryIndex,
		Identity_ResourceName,
	};

	using PG_ListModel<PG_DisaObjectCatalogItem>::PG_ListModel;

	PG_DisaAssetsModel* getAssets() const { return m_assets; }
	void setAssets(PG_DisaAssetsModel* assets);

	const std::string& getNameSpace() const { return m_nameSpace; }
	/**
	* @brief The namespace the Mod models are listed from.
	* @details Read once per rebuild. The caller sets it back to empty when the zip behind it closes.
	* @param nameSpace Empty adds no Mod models, the global namespace holds engine content and not map objects
	*/
	void setNameSpace(std::string nameSpace);

	/// Refills the list on the next event loop cycle, so a burst of changes refills it once
	void requestRebuild();
	void rebuild();

	PG_Variety getData(PG_Index index, Identity identity) const override;

private:
	PG_Model getModel(const PG_DisaObjectCatalogItem& item) const;

	void onGeometriesChanged(PG_Index startIndex, PG_Index endIndex, const PG_IdentitySet& identities);
	void onAssetsDeleted();
	void onHDViewChanged();

	PG_DisaAssetsModel* m_assets = nullptr; /// reset by onAssetsDeleted
	std::string m_nameSpace;
};
