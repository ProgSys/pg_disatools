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
#include <glm/glm.hpp>
#include <deque>
#include <vector>
#include <Disa/game/PG_DisaMoveRange.h>

class PG_DisaMapModel;
class PG_DisaSpriteModel;

/**
* @brief The game state of a unit, a child of its PG_DisaSpriteModel
* @details Where the unit stands for the rules and how far it may still walk this turn. A move takes
* effect at once, the sprite only follows: it walks the path tile by tile, the way FUN_004475c0 walks
* a map unit, and further moves queue up behind the walk.
*/
class PG_DisaActorLogicComponentModel : public PG_Object {
	PG_OBJECT(PG_DisaActorLogicComponentModel);
	PG_PARENT(PG_Object);
public:
	using PG_Object::PG_Object;

	/// Takes the tile and the stats of the unit it was put under
	void init();
	/// Walks the sprite on every tick, which PG_DisaMainGameModule hands on
	bool event(PG_Event* e) override;

	/// The unit this belongs to, its parent
	PG_DisaSpriteModel* getUnit() const;

	/// Where the unit stands for the rules, which is already the end of a walk the sprite is still on
	glm::ivec2 getTile() const { return m_tile; }
	const PG_DisaMoveRange::Stats& getStats() const { return m_stats; }
	/// Steps the unit can still walk this turn
	int getMovementLeft() const { return m_movementLeft; }
	/// Only the units of the player can be moved
	bool isMovable() const;
	bool isWalking() const { return !m_walk.empty(); }

	/**
	* @brief Moves the unit to the end of the path at once and lets the sprite walk it
	* @param path From the tile the unit stands on, every step costs one point of movement
	*/
	void moveAlong(const std::vector<glm::ivec2>& path);
	/// Gives the unit its full movement back
	void resetTurn();

	/// Middle of the tile on its ground
	static glm::vec3 standPosition(PG_DisaMapModel* map, glm::ivec2 tile);

private:
	void walk(float frames);
	void startStep(const glm::vec3& target);
	void endStep();

	glm::ivec2 m_tile = glm::ivec2(0);
	PG_DisaMoveRange::Stats m_stats;
	int m_movementLeft = 0;

	std::deque<glm::ivec2> m_walk; /// tiles the sprite still has to walk through, in order
	glm::vec3 m_stepStart = glm::vec3(0.f);
	float m_stepFrames = 0.f; /// game frames already spent on the current step, 0 before it started
	bool m_isStepJump = false;
	float m_gravityBeforeJump = 0.f;
};
