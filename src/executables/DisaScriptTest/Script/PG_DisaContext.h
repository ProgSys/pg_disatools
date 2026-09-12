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

#include <memory>
#include <vector>

#include <Base/PG_Object.h>


class PG_DisaContext : public PG_Object {
	PG_OBJECT(PG_DisaContext);
signals:
	void dungeonChanged(unsigned int id);
	void gameFlagChanged(unsigned int flagIndex, bool value);
public:
	using PG_Object::PG_Object;

	unsigned int getDungeonId() const { return m_dungeonId; }
	void setDungeonId(unsigned int id) {
		if (m_dungeonId == id) return;
		m_dungeonId = id;
		emit dungeonChanged(m_dungeonId);
	}
	/**
	* @brief The global game state the talk `set` and `clear` and the script `SetFlag` write.
	* @param flagIndex Index of the bit, the same number an `If` compares against.
	*/
	bool getGameFlag(unsigned int flagIndex) const; //@todo use DisaGameStateFilter (move to seperate file)
	void setGameFlag(unsigned int flagIndex, bool value);

private:
	/* The game's flags live in one bit array that TALK.DAT also filters its scripts by. */
	std::vector<unsigned char> m_gameState = std::vector<unsigned char>(8192, 0);

	unsigned int m_dungeonId = 701; //@todo change to PG_DisaIdentifier 
};