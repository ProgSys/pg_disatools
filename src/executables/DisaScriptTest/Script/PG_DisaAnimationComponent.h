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
#include <string>

#include <Base/PG_Object.h>

class PG_DisaActorModel;
class PG_DisaContext;

/**
* @brief One thing that moves an actor over time, a move, a turn or a fall.
* @details Like a PG_TickComponent, except that it is not the scene that ticks it. The actor it
* runs on owns it, and the PG_DisaContext keeps a lifeline to it so it can tick the lot and ask
* whether a given actor is still busy.
*
* Only one of these exists per actor and channel. A second command of the same kind takes the
* running one over rather than starting beside it, so what it does is replaceable and what it set
* up is not, see setTickFunction and setEndFunction.
*/
class PG_DisaAnimationComponent : public PG_Object {
	PG_OBJECT(PG_DisaAnimationComponent);
	PG_PARENT(PG_Object);
public:
	/**
	* @brief What the animation does with one slice of time.
	* @param actor The actor it runs on, never null.
	* @param deltaTicks Game ticks since the last call.
	* @returns true when it has nothing left to do and should be dropped.
	*/
	using TickFunction = std::function<bool(PG_DisaActorModel* actor, float deltaTicks)>;

	/// Runs when the animation goes away, however it goes. The actor is null when it took it along.
	using EndFunction = std::function<void(PG_DisaActorModel* actor)>;

	/// What an actor can run one of at a time, a move and a turn do not get in each other's way.
	enum EChannel : unsigned char {
		Channel_Move,      /// Position, including the fall onto the ground
		Channel_Rotation,
		Channel_Animation, /// The sprite animation itself
		Channel_Color,     /// The colour the sprite is tinted with
		Channel_Size_
	};

	using PG_Object::PG_Object;
	~PG_DisaAnimationComponent() override;

	/// The actor it runs on, which is the object that owns it.
	PG_DisaActorModel* getActorModel() const;

	inline EChannel getChannel() const { return m_channel; }
	inline void setChannel(EChannel value) { m_channel = value; }

	/// What it does with a slice of time. This is the part a later command of the same kind replaces.
	void setTickFunction(const TickFunction& value);

	/**
	* @brief What to put back when the animation ends, whoever ends it.
	* @details Set by whatever started the animation and left alone by anything that takes it over,
	* so a walk that is retargeted halfway still remembers what the actor was doing before it.
	*/
	inline void setEndFunction(const EndFunction& value) { m_endFunction = value; }
	inline bool hasEndFunction() const { return static_cast<bool>(m_endFunction); }

	/// Runs one slice, @returns true when the animation is over.
	bool tick(float deltaTicks);

	/// Ends it early, the context drops it on its next tick without the function running again.
	inline void stop() { m_finished = true; }
	inline bool isFinished() const { return m_finished; }

private:
	EChannel m_channel = Channel_Move;
	TickFunction m_tickFunction;
	EndFunction m_endFunction;
	bool m_finished = false;
};
