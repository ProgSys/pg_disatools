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

#include <Scene/Module/PG_SceneModule.h>
#include <Base/PG_Lifeline.h>
#include <glm/glm.hpp>
#include <optional>
#include <util/PG_Ray.h>
//the lifelines need the whole types
#include <Disa/PG_DisaCamera.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/map/PG_DisaActorListModel.h>
#include <Disa/combat/PG_DisaCombatCursorNode.h>

class PG_ModelNode;
class PG_KeyEvent;
class PG_MouseButtonEvent;
class PG_MouseMotionEvent;
class PG_DisaActorsViewNode;
class PG_DisaActorEditSelectionNode;

/**
* @brief Simulates the map part of a battle inside the editor
* @details Swaps in the game camera and a game cursor and works on a copy of the actors, so whatever
* happens to them is gone when the simulation stops. The map itself stays untouched.
*
* The mouse moves the cursor the way the game does it, the arrow keys move it one tile relative to
* the camera. A left click or Enter selects the unit under the cursor and shows its move range, a
* right click clears the selection.
*
* Installing the module on a PG_DisaMapScene with a loaded map starts the simulation, removing it
* with PG_Scene::removeModule stops it again.
*/
class PG_DisaCombatModule : public PG_SceneModule {
	PG_OBJECT(PG_DisaCombatModule);
	PG_PARENT(PG_SceneModule);
signals:
	void runningChanged(bool running);
	void selectionChanged();
public:
	using PG_SceneModule::PG_SceneModule;
	~PG_DisaCombatModule() override;

	/// False when the scene had no map to simulate
	bool isRunning() const { return m_running; }
	/**
	* @brief Takes over the camera and the input of the scene
	* @details Needs a PG_DisaMapScene with a loaded map, otherwise nothing runs.
	*/
	void added(PG_Scene* scene) override;
	/// Gives the camera back, drops the actor copy and shows the original actors again
	void removed(PG_Scene* scene) override;

	/// The copy of the actors the simulation works on, null while not running
	PG_DisaActorListModel* getActors() const { return m_actors.get(); }
	PG_DisaCombatCursorNode* getCursor() const { return m_cursor.get(); }

	/**
	* @brief Moves the cursor onto a tile, clamped into the map
	* @param frames Game frames (1/60 s) the cursor glides, 0 snaps it
	*/
	void moveCursorTo(glm::ivec2 tile, float frames = 0.f);

	/// The unit standing on the tile, null when there is none
	PG_DisaActorModel* findUnit(glm::ivec2 tile) const;
	/// Selects the unit under the cursor and shows its move range, does nothing when there is none
	void selectAtCursor();
	void select(PG_DisaActorModel* unit);
	void clearSelection();
	PG_DisaActorModel* getSelected() const { return m_selected.get(); }

	bool filterEvent(PG_Event* e) override;

private:
	bool onMouseMotion(PG_MouseMotionEvent* e);
	bool onMouseButton(PG_MouseButtonEvent* e);
	bool onKey(PG_KeyEvent* e);

	/**
	* @brief The tile the ray points at, like the game cursor picks it
	* @details A ray that hits the side of a tile picks the tile the side belongs to, which is the
	* higher one, so the cursor does not flicker between a step and the ground in front of it.
	*/
	std::optional<glm::ivec2> pickTile(const PG_Ray& ray) const;
	std::optional<PG_Ray> rayOf(glm::vec2 screenPosition) const;
	PG_DisaMapModel* getMapModel() const;
	void copyActors();
	void updateCursorPointer();

	bool m_running = false;
	PG_Lifeline_T<PG_DisaMapViewNode> m_mapView;
	PG_Lifeline_T<PG_CameraNode> m_previousCamera;
	PG_Lifeline_T<PG_DisaCamera> m_camera;
	bool m_wasActorsVisible = true;

	PG_Lifeline_T<PG_DisaActorListModel> m_actors; /// owned, a child of the map model
	PG_Lifeline_T<PG_ActorNode> m_root; /// holds every node the simulation adds to the map view
	PG_DisaActorsViewNode* m_actorView = nullptr;
	PG_Lifeline_T<PG_DisaCombatCursorNode> m_cursor;
	PG_ModelNode* m_rangeNode = nullptr;
	PG_DisaActorEditSelectionNode* m_selectedMarker = nullptr;
	PG_Lifeline_T<PG_DisaActorModel> m_selected;

	glm::ivec2 m_tileMin = glm::ivec2(0); /// map bounds, inclusive
	glm::ivec2 m_tileMax = glm::ivec2(0);
};
