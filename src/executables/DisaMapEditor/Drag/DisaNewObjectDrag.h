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

#include <Base/PG_Object.h>
#include <Resources/PG_Model.h>
#include <Scene/Model/PG_ListModelLocation.h>
#include <optional>

class PG_MouseMotionEvent;
class PG_ActorNode;
class PG_DisaModelNode;
class PG_DisaMapScene;
class PG_DisaObjectModel;

/**
* @brief Drags a model of the object catalog onto the map and places it there on release.
* @details Lives under the grid of the catalog. While dragging it holds the mouse, shows a see through
* copy of the model where the object would land, and a right click cancels.
* @see PG_DisaObjectCatalogModel
*/
class DisaNewObjectDrag : public PG_Object {
	PG_OBJECT(DisaNewObjectDrag);
public:
	using PG_Object::PG_Object;
	~DisaNewObjectDrag() override;

	/// @param location An entry of PG_DisaObjectCatalogModel, read once here
	void start(const PG_ListModelLocation& location);

	/**
	* If returns true, then event will not be propagate farther in tree.
	*/
	bool event(PG_Event* e) override;

private:
	PG_DisaMapScene* findGameScene() const;
	void mouseMoved(PG_MouseMotionEvent* e);
	void released();
	void reset();
	/// The indicator lives in the scene, which may go before this does
	void onIndicatorDeleted();

	PG_ActorNode* m_indicator = nullptr;
	PG_DisaModelNode* m_indicatorModel = nullptr;
	/// Holds the snapped position, so the indicator lands where the object will
	PG_DisaObjectModel* m_objectMoc = nullptr;

	PG_Model m_model;
	int m_geometryIndex = -1;
	std::string m_resourceName; /// empty for a Disa model
	std::optional<glm::vec3> m_position; /// where the object lands, empty while the cursor is off the map
	bool m_dragging = false;
};
