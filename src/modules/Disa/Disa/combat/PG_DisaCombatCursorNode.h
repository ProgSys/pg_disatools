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

#include <Scene/PG_ActorNode.h>

class PG_ModelNode;
class PG_DisaMapModel;
class PG_DisaActorEditSelectionNode;

/**
* @brief The map cursor of a battle: a marker on the ground plus a pointer that bobs above it
* @details The node stands in the middle of the tile it points at, on top of its ground. Like the
* game cursor it can glide to a new tile over a number of frames, see translateTo.
*/
class PG_DisaCombatCursorNode : public PG_ActorNode {
	PG_OBJECT(PG_DisaCombatCursorNode);
	PG_PARENT(PG_ActorNode);
signals:
	void tileChanged();
public:
	using PG_ActorNode::PG_ActorNode;

	void init() override;
	bool event(PG_Event* e) override;

	/// The tile the cursor points at, also while it is still on its way there
	glm::ivec2 getTile() const { return m_tile; }
	/**
	* @brief Puts the cursor onto a tile, standing on its ground
	* @param frames Game frames (1/60 s) the cursor glides, 0 snaps it there at once
	*/
	void moveToTile(glm::ivec2 tile, float frames = 0.f);
	/**
	* @brief Moves the cursor to a position, the way FUN_0046cab0 moves a map unit
	* @param frames Game frames (1/60 s) the move takes, 0 snaps it there at once
	*/
	void translateTo(const glm::vec3& target, float frames = 0.f);
	bool isMoving() const { return m_framesLeft > 0.f; }

	/// The bobbing pointer, the ground marker always stays
	void setPointerVisible(bool visible);

	/// Rebuilds the ground marker, needed when the ground under the cursor changed
	void updateMarker();

	/// The ground height of a tile, 0 where the map has no tile
	static float groundHeight(PG_DisaMapModel* map, glm::ivec2 tile);

private:
	PG_DisaMapModel* getMapModel() const;

	PG_ModelNode* m_marker = nullptr;
	PG_DisaActorEditSelectionNode* m_pointer = nullptr;

	glm::ivec2 m_tile = glm::ivec2(0);
	glm::vec3 m_target = glm::vec3(0.f);
	glm::vec3 m_step = glm::vec3(0.f); /// per game frame
	float m_framesLeft = 0.f;
};
