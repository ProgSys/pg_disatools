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

#include <functional>

#include <Base/PG_Object.h>

class PG_DisaCamera;

/**
* @brief One thing that moves the camera over time, a pan, a turn, a zoom or a lens change.
* @details The camera counterpart of PG_DisaAnimationComponent, and the same idea: the camera owns
* it and the PG_DisaContext ticks it, so the camera itself holds no animation of its own and only
* answers plain setters.
*
* The game keeps one animation slot per value and a second command of a kind overwrites it, which
* is what the channels are. A command that runs while another is still going simply replaces it.
*/
class PG_DisaCameraAnimationComponent : public PG_Object {
	PG_OBJECT(PG_DisaCameraAnimationComponent);
	PG_PARENT(PG_Object);
public:
	/**
	* @brief What the animation does with one slice of time.
	* @param camera The camera it runs on, never null.
	* @param deltaTicks Game ticks since the last call.
	* @returns true when it has nothing left to do and should be dropped.
	*/
	using TickFunction = std::function<bool(PG_DisaCamera* camera, float deltaTicks)>;

	/// One per value the game animates, so a pan and a turn do not get in each other's way.
	enum EChannel : unsigned char {
		Channel_Move,     /// The orbit point, what SetCameraPos writes
		Channel_Rotation, /// Pitch and heading together, the game keeps them in one slot too
		Channel_Zoom,     /// Distance to the orbit point
		Channel_View,     /// Field of view, what SetCameraView writes
		Channel_Size_
	};

	using PG_Object::PG_Object;

	/// The camera it runs on, which is the object that owns it.
	PG_DisaCamera* getCamera() const;

	inline EChannel getChannel() const { return m_channel; }
	inline void setChannel(EChannel value) { m_channel = value; }

	/// What it does with a slice of time. This is the part a later command of the same kind replaces.
	void setTickFunction(const TickFunction& value);

	/// Runs one slice, @returns true when the animation is over.
	bool tick(float deltaTicks);

	/// Ends it early, the context drops it on its next tick without the function running again.
	inline void stop() { m_finished = true; }
	inline bool isFinished() const { return m_finished; }

private:
	EChannel m_channel = Channel_Move;
	TickFunction m_tickFunction;
	bool m_finished = false;
};
