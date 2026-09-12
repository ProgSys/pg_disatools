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
#include <Disa/fileformats/PG_DisaFileContainer.h>

#include "PG_DisaScriptLane.h"

class PG_Script;
class PG_TalkScript;

/**
* @brief Runs the game's two script languages side by side.
* @details A script becomes a PG_DisaScriptLane, a flat list of commands plus the memory it needs
* to continue. Every tick advances all lanes until one of them has to wait, which is how the game
* keeps a cutscene script running next to the talk script that started it.
*
* Commands do not act on the world yet, they log themselves with OUT_DEBUG. What already behaves
* like the game is the flow: starting scripts, sleeping, loops, menus and the waits, so
* `script(0);` inside a talk script really does block until the scripts it started are done.
*
* @note A SCRIPT.DAT entry reaches until the next script's address, and some scripts are entered
* in the middle of another one, so those entries end mid command. Such a lane logs a warning and
* stops, the game would simply read on into the data that follows.
*/
class PG_DisaScriptExecutor : public PG_Object {
	PG_OBJECT(PG_DisaScriptExecutor);
	PG_PARENT(PG_Object);
signals:
	/// Emitted when the last running lane finished.
	void allLanesFinished();
public:
	using PG_Object::PG_Object;

	/// Game ticks in a second, every duration in a script is counted in these.
	static constexpr float ticksPerSecond = 60.f;

	/**
	* @brief The archives scripts are looked up in.
	* @details Without them only the entries handed to runScript directly can run, a script that
	* starts another one finds nothing.
	*/
	void setContainers(std::shared_ptr<PG_Script> scripts, std::shared_ptr<PG_TalkScript> talkScripts);

	/// Runs a SCRIPT.DAT entry, it starts on the next tick.
	bool runScript(PG_DisaFileEntry entry);
	/// Runs a TALK.DAT entry, it starts on the next tick.
	bool runTalkScript(PG_DisaFileEntry entry);

	/**
	* @brief Looks a script up by the ID it is stored under and runs it.
	* @returns false when no script has that ID, or when no archive was set.
	*/
	bool runScriptById(unsigned int scriptId, int parentLane = -1);
	bool runTalkScriptById(unsigned int scriptId, int parentLane = -1);

	/**
	* @brief Searches a talk script the way the game does and runs it.
	* @details This is what the script command LoadTalkScript does. Like the game it refuses while
	* another talk script is running, there is only ever one of them.
	* @returns false when nothing matches the search, or a talk script is already running.
	*/
	bool loadTalkScript(const PG_DisaTalkScriptSearch& search, int parentLane = -1);

	/**
	* @brief The dungeon the scripts run in, a talk script search with no dungeon of its own uses it.
	* @details Map group times 100 plus the map number, so MP00701 is 701. SetDungeon writes it, and
	* a LoadTalkScript given 0xFFFF as its dungeon means "whatever is loaded right now".
	*/
	unsigned int getDungeonId() const { return m_dungeonId; }
	void setDungeonId(unsigned int dungeonId) { m_dungeonId = dungeonId; }

	/**
	* @brief Advances every lane.
	* @param deltaTimeSeconds Time since the last tick, a lane sleeping for 60 ticks needs a second of it.
	*/
	void tick(double deltaTimeSeconds);

	/// Drops every lane, running or not.
	void stopAll();

	bool isRunning() const;

	//==== Used by the commands ====

	PG_DisaScriptLane& getLane(std::size_t index) { return m_lanes[index]; }
	const PG_DisaScriptLane& getLane(std::size_t index) const { return m_lanes[index]; }
	std::size_t getLaneCount() const { return m_lanes.size(); }

	/**
	* @brief Replaces what a lane runs with a talk script, the lane keeps its parent.
	* @details This is what the talk command `call` does, the game has no call stack so the script
	* that called never continues.
	*/
	bool callTalkScript(std::size_t laneIndex, unsigned int scriptId);

	/**
	* @brief Runs the search the lane was started with again and continues in whatever it finds.
	* @details The talk command `restart` does not simply jump back, it searches once more, so a
	* flag a menu just set can bring up a different script.
	*/
	bool restartTalkScript(std::size_t laneIndex);

	/// True while any lane started by the given one is still running.
	bool hasRunningChildren(std::size_t laneIndex) const;

	/// True while a talk script is running, only one of them can be active at a time.
	bool hasRunningTalkLane() const;

	/**
	* @brief The global game state the talk `set` and `clear` and the script `SetFlag` write.
	* @param flagIndex Index of the bit, the same number an `If` compares against.
	*/
	bool getGameFlag(unsigned int flagIndex) const;
	void setGameFlag(unsigned int flagIndex, bool value);

	/// Ticks a confirm waits before it continues, there is no dialog box to click yet.
	float getConfirmTicks() const { return m_confirmTicks; }
	void setConfirmTicks(float ticks) { m_confirmTicks = ticks; }

	/// Ticks a wait on a game flag gives up after, so a script cannot hang the tool.
	float getWaitTimeoutTicks() const { return m_waitTimeoutTicks; }
	void setWaitTimeoutTicks(float ticks) { m_waitTimeoutTicks = ticks; }

private:
	/// Reads the entry and parses it into a new lane, returns the lane index or -1.
	int addLane(const PG_DisaFileEntry& entry, PG_DisaScriptLane::EType type, unsigned int scriptId, int parentLane);
	/// Finds the entry a script ID is stored under.
	PG_DisaFileEntry findScript(unsigned int scriptId) const;
	/**
	* @brief The talk script the game would pick, the first entry of the file table that fits.
	* @details A search value of zero matches anything, and on top of that every positive game state
	* filter of the entry has to be set and every negative one clear.
	*/
	PG_DisaFileEntry findTalkScript(const PG_DisaTalkScriptSearch& search) const;
	/// Refills a lane with a talk script, used by `call` and `restart` which have no call stack.
	bool replaceLane(std::size_t laneIndex, const PG_DisaFileEntry& entry, const PG_DisaTalkScriptSearch& search);

	void runLane(std::size_t laneIndex, float deltaTicks);

	std::vector<PG_DisaScriptLane> m_lanes;
	std::shared_ptr<PG_Script> m_scripts;
	std::shared_ptr<PG_TalkScript> m_talkScripts;

	/* The game's flags live in one bit array that TALK.DAT also filters its scripts by. */
	std::vector<unsigned char> m_gameState = std::vector<unsigned char>(8192, 0);

	unsigned int m_dungeonId = 0;
	float m_confirmTicks = 60.f;
	float m_waitTimeoutTicks = 300.f;
};
