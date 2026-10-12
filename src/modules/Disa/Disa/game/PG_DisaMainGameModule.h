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
#include <Disa/game/PG_DisaMoveRange.h>
//the lifelines need the whole types
#include <Disa/PG_DisaCamera.h>
#include <Disa/map/PG_DisaActorListModel.h>
#include <Disa/game/PG_DisaActorLogicComponentModel.h>

class PG_ModelNode;
class PG_KeyEvent;
class PG_MouseButtonEvent;
class PG_MouseMotionEvent;
class PG_DisaMapModel;
class PG_DisaMapViewNode;
class PG_DisaCursorNode;
class PG_DisaActorEditSelectionNode;

/**
* @brief Plays the map part of a battle inside the editor
* @details Swaps in the game camera, a game cursor and a copy of the actors, so whatever happens to
* them is gone when the game stops. The map itself stays untouched. Every unit of the copy carries a
* PG_DisaActorLogicComponentModel with its game state.
*
* The mouse moves the cursor the way the game does it, the arrow keys move it one tile relative to
* the camera. A left click or Enter selects the unit under the cursor and shows its move range, a
* right click clears the selection. With a player unit selected the path to the cursor is shown, and
* a left click or Enter moves the unit there. T starts the next turn.
*
* Installing the module on a PG_DisaMapScene with a loaded map starts the game, removing it with
* PG_Scene::removeModule stops it again.
*/
class PG_DisaMainGameModule : public PG_SceneModule {
	PG_OBJECT(PG_DisaMainGameModule);
	PG_PARENT(PG_SceneModule);
signals:
	void runningChanged(bool running);
	void turnChanged(int turn);
public:
	using PG_SceneModule::PG_SceneModule;
	~PG_DisaMainGameModule() override;

	/// False when the scene had no map to play
	bool isRunning() const { return m_running; }
	/**
	* @brief Takes over the camera, the actors and the input of the scene
	* @details Needs a PG_DisaMapScene with a loaded map, otherwise nothing runs.
	*/
	void added(PG_Scene* scene) override;
	/// Gives the camera and the actors of the map back
	void removed(PG_Scene* scene) override;

	/// Gives every unit its full movement back
	void nextTurn();

	/// Passes the tick on to the units, their models are not part of the tree the ticks run through
	bool event(PG_Event* e) override;
	bool filterEvent(PG_Event* e) override;

private:
	bool onMouseMotion(PG_MouseMotionEvent* e);
	bool onMouseButton(PG_MouseButtonEvent* e);
	bool onKey(PG_KeyEvent* e);
	/// Left click and Enter: moves the selected unit, or selects the unit under the cursor
	void confirmAtCursor();

	/// Moves the cursor onto a tile, clamped into the map. @param frames Game frames (1/60 s) it glides, 0 snaps
	void moveCursorTo(glm::ivec2 tile, float frames = 0.f);
	/**
	* @brief The tile the ray points at, like the game cursor picks it
	* @details A ray that hits the side of a tile picks the tile the side belongs to, which is the
	* higher one, so the cursor does not flicker between a step and the ground in front of it.
	*/
	std::optional<glm::ivec2> pickTile(const PG_Ray& ray) const;
	std::optional<PG_Ray> rayOf(glm::vec2 screenPosition) const;

	PG_DisaMapViewNode* getMapView() const;
	PG_DisaMapModel* getMapModel() const;
	/// Clones the actors of the map into a list of their own
	void copyActors();
	/// Gives a new unit its game state
	void attachLogic(PG_DisaActorModel* actor);

	/// The unit standing on the tile for the rules, null when there is none
	PG_DisaActorLogicComponentModel* findUnit(glm::ivec2 tile) const;
	void select(PG_DisaActorLogicComponentModel* unit);
	void clearSelection();
	/**
	* @brief Moves the selected unit to the destination of its range closest to the tile
	* @returns False when the selected unit can not move or has nowhere to go
	*/
	bool moveSelected(glm::ivec2 tile);

	/// Recomputes the range of the selected unit from where it stands and with what it has left
	void updateRange();
	/// Shows the way the selected unit would walk to the cursor
	void updatePath();
	void updateCursorPointer();

	bool m_running = false;

	//restored when the game stops
	PG_Lifeline_T<PG_CameraNode> m_previousCamera;
	PG_Lifeline_T<PG_DisaActorListModel> m_previousActors;

	//game logic
	PG_Lifeline_T<PG_DisaActorListModel> m_actors; /// owned, a child of the map model so the units find their ground
	PG_Lifeline_T<PG_DisaActorLogicComponentModel> m_selected;
	std::optional<PG_DisaMoveRange> m_range; /// of the selected unit
	glm::ivec2 m_tileMin = glm::ivec2(0); /// map bounds, inclusive
	glm::ivec2 m_tileMax = glm::ivec2(0);
	int m_turn = 1;

	//visuals
	PG_Lifeline_T<PG_DisaCamera> m_camera;
	PG_Lifeline_T<PG_ActorNode> m_root; /// holds every node the game adds to the map view
	PG_DisaCursorNode* m_cursor = nullptr;
	PG_ModelNode* m_rangeNode = nullptr;
	PG_ModelNode* m_pathNode = nullptr;
	PG_DisaActorEditSelectionNode* m_selectedMarker = nullptr;
};
