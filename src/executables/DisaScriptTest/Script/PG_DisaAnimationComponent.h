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
* @details Like a PG_TickComponent, except that it is not the scene that ticks it. It puts itself
* into the PG_DisaContext when it is made and takes itself out again the moment its function
* reports that it is done. The context walking one list is what lets a script ask whether a given
* actor is still busy, which is all `WaitForAnimation` needs.
*
* Two of them on the same actor and the same channel cannot both be right, so the newer one
* replaces the older, the same way a second SetActorPos overrules the first.
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

	/// What an actor can run one of at a time, a move and a turn do not get in each other's way.
	enum EChannel : unsigned char {
		Channel_Move,      /// Position, including the fall onto the ground
		Channel_Rotation,
		Channel_Animation, /// The sprite animation itself
		Channel_Size_
	};

	/**
	* @brief Registers itself with the context, which owns it from then on.
	* @param context Ticks it and drops it when it is done.
	*/
	PG_DisaAnimationComponent(PG_DisaContext* context, PG_DisaActorModel* actor, EChannel channel,
		const TickFunction& tickFunction, const std::string& name = "PG_DisaAnimationComponent");

	inline PG_DisaActorModel* getActor() const { return m_actor; }
	inline EChannel getChannel() const { return m_channel; }

	/// Runs one slice, @returns true when the animation is over.
	bool tick(float deltaTicks);

	/// Ends it early, the context drops it on its next tick without the function running again.
	inline void stop() { m_finished = true; }
	inline bool isFinished() const { return m_finished; }

private:
	PG_DisaActorModel* m_actor = nullptr;
	EChannel m_channel = Channel_Move;
	TickFunction m_tickFunction;
	bool m_finished = false;
};
