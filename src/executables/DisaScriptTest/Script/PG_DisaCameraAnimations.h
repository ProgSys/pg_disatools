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

#include <glm/glm.hpp>

class PG_DisaContext;

/**
* @brief The things a script can set the game camera doing over time.
* @details The counterpart of PG_DisaActorAnimations. Each one makes a
* PG_DisaCameraAnimationComponent that the context then ticks, and each returns right away.
*
* Every one of them moves linearly, because the game does too: it works out a step per tick when
* the command runs and adds that step until the ticks are used up. A duration of zero or less sets
* the value at once and drops whatever was animating it.
*
* The angles are the camera's own, so a caller that holds the game's yaw converts it first.
* @see PG_DisaCameraAnimations::orbitOfGameYaw
*/
namespace PG_DisaCameraAnimations {

/// Moves the point the camera looks at, in tile units.
void moveTo(PG_DisaContext& context, const glm::vec3& target, float durationTicks);

/// Pulls the camera to a distance from the point it looks at, in tile units.
void zoomTo(PG_DisaContext& context, float distance, float durationTicks);

/**
* @brief Turns the camera to a tilt and a heading.
* @param pitchDegree Negative looks down. @see PG_DisaCamera::setOrbitAngles
* @param orbitDegree Absolute heading, the turn takes the short way round.
*/
void rotateTo(PG_DisaContext& context, float pitchDegree, float orbitDegree, float durationTicks);

/**
* @brief Turns the camera by an angle, on top of where it already looks.
* @details More than a full turn really goes around more than once, the game keeps the step and
* not the target.
*/
void rotateBy(PG_DisaContext& context, float pitchDegree, float orbitDegree, float durationTicks);

/// Opens or narrows the lens, in degree of vertical field of view.
void fovTo(PG_DisaContext& context, float fovDegree, float durationTicks);

/// The camera heading that matches a yaw out of a script. The two count in opposite directions.
inline float orbitOfGameYaw(float yawDegree) { return 270.f - yawDegree; }

/**
* @brief Vertical field of view in degree for the lens value of SetCameraFOV.
* @param value Already scaled by the 1/16 the command applies, zero picks the game's default.
*/
float fovOfLensValue(float value);

} //namespace PG_DisaCameraAnimations
