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

#include <Scene/PG_Scene.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

class PG_DisaObjectModel;
class PG_DisaMapViewNode;
class PG_DisaMapModel;
class PG_DisaMPDContainer;

/**
* @brief The scene a disa map is shown in, both to edit it and to preview it
* @details Holds the camera and a PG_DisaMapViewNode, and installs the editing module the five
* interaction controllers hang off. Preview mode turns the editing off, orbits the camera and swallows
* the input, so the same scene serves a small tile in a grid and the editor window.
*/
class PG_DisaMapScene : public PG_Scene
{
public:
	using PG_Scene::PG_Scene;

	void init() override;
	/// Takes over the given scene's camera and map
	void copy(PG_DisaMapScene* scene);

	bool event(PG_Event* e) override;

	/**
	* @returns true when the scene shows the map rather than lets it be edited
	*/
	inline bool getPreviewMode() const { return m_previewMode; }
	/**
	* @brief Turns the map in front of the camera and stops taking input
	*/
	void setPreviewMode(bool isPreviewMode);
	/// The floor grid and the coordinate cross
	void setAxisVisible(bool visible);
	void setShowHiddenModel(bool visible);

	bool isActorsVisible() const;
	void setActorsVisible(bool visible);

	bool isGeoPanelsVisible() const;
	void setGeoPanelsVisible(bool visible);

	void setEnableEditing(bool enable);

	/// @see setCombatSimulation
	bool isCombatSimulation() const;
	/**
	* @brief Plays the map like a battle of the game, on a copy of the actors
	* @details Turns the editing off, the map and its actors are back as they were once it stops.
	* @see PG_DisaCombatModule
	*/
	void setCombatSimulation(bool enable);

	bool isHDMode() const;
	void setHDMode(bool state);

	/**
	* @brief Puts a new object of the given model on the map
	* @returns Null when no map is open
	*/
	PG_DisaObjectModel* addModel(int index, glm::vec3 pos);
	/**
	* @brief Puts a new mod object on the map, drawn by a PG_Model resource
	* @details Such an object lives in the editor only, the MPD has no way to name it and saving skips it.
	* @param resourceName Without its namespace, "D_Wall". The map model supplies the namespace.
	* @returns Null when no map is open
	*/
	PG_DisaObjectModel* addResourceModel(const std::string& resourceName, glm::vec3 pos);
	/**
	* @brief Puts a new object where the camera looks at the map
	*/
	void addModelAtViewCenter(int index);
	/**
	* @brief Copies the object at the given index, offset along x so it does not hide in the original
	*/
	void duplicateObject(int objectIndex);

	PG_DisaMapViewNode* getMapNode() const { return m_mapNode; }
	PG_DisaMapModel* getMapModel() const;

	/**
	* @brief Shows the map inside the container and keeps the container alive
	* @details Also takes the camera start the container names, if it names one.
	*/
	void setModel(const std::shared_ptr<PG_DisaMPDContainer>& container);
	/**
	* @brief Shows the given map, which somebody else owns
	* @details The camera stays where it is, a map on its own names no start point.
	*/
	void setMapModel(PG_DisaMapModel* model);

private:
	void orbitCamera(float value);
	/// Adds an object to the last chunk, snapped to the object grid at pos
	PG_DisaObjectModel* addObject(char modelId, const std::string& resourceName, glm::vec3 pos);

	bool m_previewMode = false;
	PG_DisaMapViewNode* m_mapNode = nullptr;
};
