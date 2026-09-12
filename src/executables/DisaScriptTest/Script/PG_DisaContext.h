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

#include <glm/glm.hpp>

#include <Base/PG_Object.h>

#include "PG_DisaAnimationComponent.h"

class PG_DisaActorModel;
class PG_DisaActorSlotModel;
class PG_DisaMapModel;

/**
* @brief The world the scripts act on: the map, the actors they spawn and what moves over time.
* @details Commands do not reach into the scene, they go through here. It owns the actors a
* script spawns, resolves the slot numbers a script addresses them by, and keeps the list of
* running animations so a script can wait for one.
*/
class PG_DisaContext : public PG_Object {
	PG_OBJECT(PG_DisaContext);
	PG_PARENT(PG_Object);
signals:
	void dungeonChanged(unsigned int id);
	void gameFlagChanged(unsigned int flagIndex, bool value);
public:
	/// Script units in one tile, every coordinate in a script is counted in these.
	static constexpr float unitsPerTile = 120.f;
	/// How fast an actor falls when nothing says otherwise, in tiles per tick squared.
	static constexpr float defaultGravity = 0.1633f / unitsPerTile;

	/**
	* @brief The slots a script addresses that are not a plain actor index.
	* @details Read out of the game's slot resolver, the whole table is in reverse/Doku.md.
	*/
	enum ESlot : int {
		Slot_MapLast = 63,     /// 0 to 63 are the units the map placed
		Slot_AllTargets = 202, /// Every unit of the target group at once
		Slot_MapOrigin = 203,  /// Not an actor, the map's teleporter, used as a position parent
		Slot_ActingUnit = 204,
	};

	using PG_Object::PG_Object;

	void init();

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

	//==== Actors ====

	/// The map the actors stand on, the ground height and the map's own units come from it.
	PG_DisaMapModel* getMapModel() const { return m_mapModel; }
	void setMapModel(PG_DisaMapModel* model) { m_mapModel = model; }

	/// The actors the scripts spawned, a view of its own draws these over the map.
	PG_DisaActorSlotModel* getActors() const { return m_actors; }

	/**
	* @brief The actor a script means by that slot number.
	* @details Slots up to 63 are the units the map placed, 64 to 95 are the script's own. The
	* group slots the game has (101, 151, 160 and up) need a running battle, so they find nothing.
	* @returns null when the slot holds no actor.
	*/
	PG_DisaActorModel* resolveSlot(int slot) const;
	/// Every actor a slot stands for, which is more than one only for the group slots.
	std::vector<PG_DisaActorModel*> resolveSlots(int slot) const;

	/// Makes the actor of a script slot, or hands back the one already in it. @see SetActor
	PG_DisaActorModel* takeSlot(int slot);

	/**
	* @brief Position the coordinates of a slot are counted from, in tile units.
	* @details A script gives a position relative to a parent slot, and the map origin for the
	* slots that name no actor.
	*/
	glm::vec3 getSlotOrigin(int slot) const;

	/// Turns the coordinates in a script into the tile units the models are in.
	static inline glm::vec3 toTileUnits(const glm::vec3& scriptUnits) { return (scriptUnits * glm::vec3(-1.f, -1.f, 1.f)) / unitsPerTile; }

	//==== Ground ====

	/**
	* @brief Height of the ground under that spot, in tile units.
	* @details The game drops an actor onto the floor rather than letting it hang in the air, and
	* a spawn with no height of its own would otherwise sit inside the map.
	* @returns 0 when there is no tile there, which is where the map's base plane lies.
	*/
	float getGroundHeight(const glm::vec3& position) const;
	/// Puts the actor on the ground right away, without a fall.
	void snapToGround(PG_DisaActorModel* actor) const;

	//==== Animations ====

	/**
	* @brief Takes an animation over, it then runs until it reports that it is done.
	* @details A second animation on the same actor and channel ends the first, the way a second
	* SetActorPos overrules the one before it.
	*/
	void addAnimation(PG_DisaAnimationComponent* animation);

	/// True while anything still moves the actor, which is what WaitForAnimation waits on.
	bool hasAnimation(const PG_DisaActorModel* actor) const;
	bool hasAnimation(const PG_DisaActorModel* actor, PG_DisaAnimationComponent::EChannel channel) const;
	/// Ends every animation of that actor, or of every actor when none is given.
	void stopAnimations(const PG_DisaActorModel* actor = nullptr);

	inline std::size_t getAnimationCount() const { return m_animations.size(); }

	/**
	* @brief Advances every running animation and drops the ones that finished.
	* @param deltaTicks Game ticks since the last call, the game runs at 60 of them a second.
	*/
	void tick(float deltaTicks);

	/// Drops every spawned actor and everything that was moving them.
	void reset();

private:
	/* The game's flags live in one bit array that TALK.DAT also filters its scripts by. */
	std::vector<unsigned char> m_gameState = std::vector<unsigned char>(8192, 0);

	//unsigned int m_dungeonId = 701; //@todo change to PG_DisaIdentifier
	unsigned int m_dungeonId = 5105; //@todo change to PG_DisaIdentifier

	PG_DisaActorSlotModel* m_actors = nullptr;
	PG_DisaMapModel* m_mapModel = nullptr;

	/* Owned here, an animation is dropped the tick it reports that it finished. */
	std::vector<PG_DisaAnimationComponent*> m_animations;
};