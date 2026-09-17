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

#include <unordered_map>

#include <Base/PG_Lifeline.h>
#include <Disa/map/PG_DisaActorListModel.h>
#include <Disa/map/PG_DisaSpriteModel.h>

/**
* @brief Slots 64 to 95 of one script chain.
* @details The game gives every script its own table and clears it when the script returns, so a
* script cannot see what the script that started it spawned. The retail scripts are not written
* that way: 54 of the 398 that touch these slots address one they never spawned into, which in the
* real game finds nothing. Scoping the table to the whole chain instead makes those do what they
* plainly meant.
*
* The items themselves live in the map's list, next to the units the map placed, so one view draws
* the lot. Only the numbering is private to the chain.
*/
class PG_DisaScriptSlots {
public:
	explicit PG_DisaScriptSlots(PG_DisaActorListModel* actors) : m_actors(actors) {}
	~PG_DisaScriptSlots() { clear(); }

	PG_DisaScriptSlots(const PG_DisaScriptSlots&) = delete;
	void operator=(const PG_DisaScriptSlots&) = delete;

	static bool isScriptSlot(int slot) { return PG_DisaActorListModel::isScriptSlot(slot); }

	/// The actor of that slot, null when the slot is empty.
	PG_DisaSpriteModel* getSlot(int slot) const;
	/// The actor of that slot, made when the slot is still empty.
	PG_DisaSpriteModel* takeActorSlot(int slot);

	/// Removes the actor of that slot, does nothing when the slot is empty.
	void clearSlot(int slot);
	/// Removes every actor the chain spawned, what the end of a chain does.
	void clear();

	/// Every actor the chain holds, which is what slot 202 stands in for while no battle runs.
	std::vector<PG_DisaSpriteModel*> getActors() const;

private:
	/* Lifelines rather than pointers: the list may drop a row from anywhere, and a slot that lost
	 * its actor has to read as empty instead of pointing at something that is gone. */
	PG_Lifeline_T<PG_DisaActorListModel> m_actors;
	std::unordered_map<int, PG_Lifeline_T<PG_DisaSpriteModel>> m_slots;
};
