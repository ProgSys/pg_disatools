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

#include <cstddef>

class PG_DisaScriptExecutor;

/**
* @brief What a command reports back after it was run.
*/
enum class EDisaScriptResult : unsigned char {
	Done,    /// Finished, the lane moves on to the next command.
	Jump,    /// Finished and the command placed the cursor itself, so the lane must not advance it.
	Running, /// Not finished, the same command runs again on the next tick.
	Stop,    /// The lane ends here.
};

/**
* @brief One executed command of a script.
* @param executor The executor the lane belongs to, commands reach other lanes through it.
* @param laneIndex Lane the command belongs to. It is an index and not a reference because a
* command may start another lane, which can move the lanes in memory.
* @param deltaTicks Game ticks since the last call, the game runs at 60 of them a second.
* @param data First argument byte, PG_DisaScriptCommand::dataSize byte long.
*/
using PG_DisaScriptCommandPtr = EDisaScriptResult(*)(PG_DisaScriptExecutor& executor, std::size_t laneIndex, float deltaTicks, const void* data);

/**
* @brief A single parsed command, the arguments stay raw bytes inside the lane's data.
* @details Built the same way a PG_RenderCommand is: the command is a plain function pointer and
* its arguments are a slice of one shared byte buffer, so a lane is two flat vectors.
*/
struct PG_DisaScriptCommand {
	std::size_t dataIndex = 0; /// First argument byte inside PG_DisaScriptLane::data
	std::size_t dataSize = 0; /// Number of argument bytes
	PG_DisaScriptCommandPtr command = nullptr;
	unsigned char opCode = 0; /// Kept for logging and for the menu commands, which search by op code
};
