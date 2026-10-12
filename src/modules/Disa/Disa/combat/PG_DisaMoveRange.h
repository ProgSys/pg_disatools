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
#include <vector>

class PG_DisaMapModel;
class PG_DisaActorListModel;
class PG_DisaSpriteModel;

/**
* @brief The tiles a unit can walk to, by the rules of FUN_004d21f0
* @details A breadth first search over the four neighbours, every step costs one. A tile can be
* entered when it exists, its mobility is 0 and it rises at most the jump power above the tile the
* step comes from, dropping is always allowed. Units of another alliance block their tile, units of
* the same alliance can be passed but not stood on. The game ignores the movement type here, so
* floating and warping units get the same range as walkers.
*/
struct PG_DisaMoveRange {
	struct Stats {
		int movement = 0; /// steps
		int jumpPower = 0; /// game height units, 12 make a tile
	};

	/// Movement and jump power from the character data, both 0 when the character is unknown
	static Stats statsOf(const PG_DisaSpriteModel& unit);

	/**
	* @returns Every tile the game highlights: the reached ones, its own tile and the tiles of allies included
	*/
	static std::vector<glm::ivec2> find(const PG_DisaMapModel& map, const PG_DisaActorListModel& actors, const PG_DisaSpriteModel& unit);
	static std::vector<glm::ivec2> find(const PG_DisaMapModel& map, const PG_DisaActorListModel& actors, const PG_DisaSpriteModel& unit,
		Stats stats);
};
