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

#include <Disa/map/PG_DisaSpriteModel.h>

/**
* @brief The actors a script spawned, addressed by the slot the script knows them under.
* @details A script never says "the third actor", it says "slot 65". The slots are not dense and
* they are not an order, so they cannot be the list index, but a view wants a plain list. This
* keeps both: the list the view walks, and a slot to row table beside it.
*
* Every actor a script touches lives here, the units a map placed included. The game keeps those
* two apart, this does not, which is why a script may spawn into any slot up to 95.
*/
class PG_DisaActorSlotModel : public PG_DisaActorListModel {
	PG_OBJECT(PG_DisaActorSlotModel);
	PG_PARENT(PG_DisaActorListModel);
public:
	using PG_DisaActorListModel::PG_DisaActorListModel;

	/**
	* @brief The slots that name a single actor, everything above is a group or a marker.
	* @details The game splits these into the units a map placed (up to 63) and the ones a script
	* spawns (64 to 95), but a script writes into both, so they are kept together here.
	*/
	static constexpr int firstSlot = 0;
	static constexpr int lastSlot = 95;
	static inline bool isActorSlot(int slot) { return slot >= firstSlot && slot <= lastSlot; }

	/// The actor in that slot, null when the slot is empty.
	PG_DisaSpriteModel* getSlot(int slot) const;
	/// The slot an actor sits in, -1 when it is not one of ours.
	int getSlotOf(const PG_DisaSpriteModel* actor) const;

	/**
	* @brief The actor of that slot, made when the slot is still empty.
	* @details This is what SetActor does, a second call on a taken slot hands back the actor that
	* is already there so the script can change it rather than stack a new one on top.
	*/
	PG_DisaSpriteModel* takeSlot(int slot);

	/// Removes the actor of that slot, does nothing when the slot is empty.
	void clearSlot(int slot);
	/// Removes every actor, what the end of a cutscene does.
	void clearSlots();

private:
	/* Slot to actor rather than slot to row: a removed row moves every row behind it up, a
	 * pointer the list still owns does not move. */
	std::unordered_map<int, PG_DisaSpriteModel*> m_slots;
};
