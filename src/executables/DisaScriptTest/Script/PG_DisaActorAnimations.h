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

#include <string>

#include <glm/glm.hpp>

class PG_DisaActorModel;
class PG_DisaContext;

/**
* @brief The things a script can set an actor doing over time.
* @details Each one makes a PG_DisaAnimationComponent that the context then owns and ticks. They
* all return right away, the movement happens over the ticks that follow, which is why a script
* has to wait for them.
*/
namespace PG_DisaActorAnimations {

/**
* @brief How SetActorPos gets the actor to where it is going.
*/
enum EMoveMode : int {
	Move_Straight = 0, /// Duration is the number of ticks it takes
	Move_Jump = 1,     /// Follows an arc, the height of the target is how high the arc goes
	Move_Speed = 2,    /// Duration is a speed, the travel time is the distance divided by it
};

/**
* @brief Moves the actor to a place, and lets it drop when it ends up above the ground.
* @param target Where it goes, in tile units.
* @param durationTicks Ticks it takes, or the speed for Move_Speed. Zero puts it there at once.
* @details The game plays no animation of its own while this runs, a script that wants the actor
* to walk sets the animation itself.
*/
void move(PG_DisaContext& context, PG_DisaActorModel* actor, const glm::vec3& target,
	float durationTicks, EMoveMode mode = Move_Straight);

/**
* @brief Lets the actor fall until it stands on the ground under it.
* @param gravity Tiles per tick squared, the game's default is a bit over a hundredth of a tile.
* @details Nothing happens when it already stands on or under the ground, which is what keeps a
* freshly spawned actor out of the floor.
*/
void fall(PG_DisaContext& context, PG_DisaActorModel* actor, float gravity);

/**
* @brief Turns the actor to an angle.
* @param degree Facing around y, in the space PG_DisaActorModel counts its rotation in.
* @param durationTicks Zero sets it at once, otherwise it turns there over that many ticks.
*/
void rotateTo(PG_DisaContext& context, PG_DisaActorModel* actor, float degree, float durationTicks);

/**
* @brief Turns the actor by an angle, on top of the way it already faces.
* @param degree How far to turn around y. More than a full turn spins.
* @param durationTicks Zero turns it at once, otherwise the turn is spread over that many ticks.
*/
void rotateBy(PG_DisaContext& context, PG_DisaActorModel* actor, float degree, float durationTicks);

/**
* @brief Puts the actor into an animation set and keeps it busy while that set runs.
* @param loops How often it plays before the actor counts as free again, zero never ends by
* itself and only a later animation stops it.
* @details Only the busy state ends, the sprite itself keeps playing either way. A pose the script
* named before is dropped, the set and the angle pick the sheet again.
*/
void play(PG_DisaContext& context, PG_DisaActorModel* actor, int animationSet, int loops = 1);

/// Holds one named sheet, the way SetActorPose does. A negative number is the mirrored sheet.
void pose(PG_DisaActorModel* actor, int animation);

/**
* @brief Tints the actor's sprite, fading there over the duration.
* @param color One per channel, 1 leaves it as it is.
* @param durationTicks Zero sets it at once.
*/
void tint(PG_DisaContext& context, PG_DisaActorModel* actor, const glm::vec4& color, float durationTicks);

} //namespace PG_DisaActorAnimations
