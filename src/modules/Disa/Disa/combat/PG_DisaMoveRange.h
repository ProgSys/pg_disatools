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

#include <glm/glm.hpp>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <Util/PG_iVec2Hasher.h>

class PG_DisaMapModel;
class PG_DisaSpriteModel;

/**
* @brief The tiles a unit can walk to, by the rules of FUN_004d21f0, and the paths to them
* @details A breadth first search over the four neighbours, every step costs one. A tile can be
* entered when it exists, its mobility is 0 and it rises at most the jump power above the tile the
* step comes from, dropping is always allowed. Units of another alliance block their tile, units of
* the same alliance can be passed but not stood on. The game ignores the movement type here, so
* floating and warping units get the same range as walkers.
*/
class PG_DisaMoveRange {
public:
	struct Stats {
		int movement = 0; /// steps
		int jumpPower = 0; /// game height units, 12 make a tile
	};

	/// A unit standing on the map, which blocks or only occupies its tile
	struct Occupant {
		glm::ivec2 tile;
		unsigned char alliance;
	};

	/// Movement and jump power from the character data, both 0 when the character is unknown
	static Stats statsOf(const PG_DisaSpriteModel& unit);

	PG_DisaMoveRange() = default;
	/**
	* @param occupants Every other unit, the moving one must not be part of it
	*/
	PG_DisaMoveRange(const PG_DisaMapModel& map, glm::ivec2 start, unsigned char alliance, Stats stats, std::span<const Occupant> occupants);

	glm::ivec2 getStart() const { return m_start; }
	/// Every tile the game highlights: the reached ones, the start and the tiles of allies included
	const std::vector<glm::ivec2>& getTiles() const { return m_tiles; }
	/// Steps to the tile, -1 when it was not reached
	int getDistance(glm::ivec2 tile) const;
	/// Reached and free, the start does not count as a destination
	bool isDestination(glm::ivec2 tile) const;
	/**
	* @brief The destination closest to the tile, the tile itself when it is one
	* @returns The start when the unit can not go anywhere
	*/
	glm::ivec2 nearestDestination(glm::ivec2 tile) const;

	/**
	* @brief The way from the start to a reached tile, traced backwards like FUN_004d1e50
	* @returns Start and target included, empty when the tile was not reached
	*/
	std::vector<glm::ivec2> buildPath(glm::ivec2 target) const;

private:
	int heightOf(glm::ivec2 tile) const;

	glm::ivec2 m_start = glm::ivec2(0);
	int m_jumpPower = 0;
	std::vector<glm::ivec2> m_tiles;
	std::unordered_map<glm::ivec2, int, PG_iVec2Hasher> m_distances;
	std::unordered_map<glm::ivec2, int, PG_iVec2Hasher> m_heights; /// middle of the top in game units, negative is up
	std::unordered_set<glm::ivec2, PG_iVec2Hasher> m_allies;
};
