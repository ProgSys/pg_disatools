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

#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include <Disa/PG_DisaIdentifier.h>

#include "PG_DisaScriptCommand.h"
#include "PG_DisaScriptSlots.h"

/**
* @brief The variables one lane keeps while it runs.
* @details The game holds these in its DisaScriptMemory, one per running script.
*/
struct PG_DisaScriptLaneMemory {
	/// Ticks the lane still has to sleep before the next command is run.
	float waitTicks = 0.f;
	/// Ticks the command the lane sits on has been waiting for something, reset when it moves on.
	float waitedTicks = 0.f;

	/* RepeatStart and RepeatEnd build a for loop. The game keeps the counter globally, which is
	 * why it cannot nest, here it is per lane so a nested loop at least behaves. */
	int repeatCounter = 0;
	int repeatMax = 0;
	std::size_t repeatStart = 0; /// Command the loop jumps back to
};

/**
* @brief One running script, its commands and everything it needs to continue where it left off.
* @details Lanes run side by side, one tick advances every one of them. A lane that starts another
* script gets a new lane rather than a call, which is how the game runs a cutscene script next to
* the talk script that started it.
*/
struct PG_DisaScriptLane {
	enum EType : unsigned char {
		Talk,  /// From TALK.DAT, dialog and menus
		Script /// From SCRIPT.DAT, camera, actors and effects
	};

	std::vector<PG_DisaScriptCommand> commands;
	std::vector<char> data; /// Argument bytes of every command
	PG_DisaScriptLaneMemory memory;

	std::size_t cursor = 0; /// Command that runs next
	int parent = -1; /// Lane that started this one, -1 when the user started it
	EType type = Script;
	PG_DisaIdentifier scriptId;
	/// Talk lanes only, `restart` runs this again so a changed game state can pick another script.
	PG_DisaIdentifier search;
	std::string name; /// What the lane prints itself as, the entry's file name
	bool finished = false;

	/* Slots 64 to 95, shared with every lane of the same chain: a lane that starts another hands
	 * its own table down, so a script can reach what the script before it spawned. */
	std::shared_ptr<PG_DisaScriptSlots> slots;

	inline bool isAtEnd() const { return cursor >= commands.size(); }

	/// First argument byte of the command, null when it has no arguments.
	inline const void* getData(const PG_DisaScriptCommand& command) const {
		return command.dataSize ? data.data() + command.dataIndex : nullptr;
	}

	/**
	* @brief Appends a command and copies its arguments into the lane.
	* @param arguments May be null, then the command takes no arguments.
	*/
	void add(unsigned char opCode, PG_DisaScriptCommandPtr command, const void* arguments, std::size_t size) {
		PG_DisaScriptCommand out{ data.size(), size, command, opCode };
		if (arguments && size) {
			data.resize(out.dataIndex + size);
			memcpy(data.data() + out.dataIndex, arguments, size);
		} else {
			out.dataIndex = 0;
			out.dataSize = 0;
		}
		commands.push_back(out);
	}
};
