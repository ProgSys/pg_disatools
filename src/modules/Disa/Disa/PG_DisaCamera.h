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
#include <optional>
#include <string>
#include <Scene/PG_FlyCameraNode.h>

/**
* @brief Camera which orbits around a point on the map.
* @details The camera is always placed at `orbit point - view dir * orbit distance`, so the camera
* follows whenever the orbit point moves. WASD and dragging move the orbit point, the mouse wheel
* pulls the camera towards it.
*/
class PG_DisaCamera : public PG_FlyCameraNode {
	PG_OBJECT(PG_DisaCamera);
	PG_PARENT(PG_FlyCameraNode);
public:
	using PG_FlyCameraNode::PG_FlyCameraNode;

	void init() override;

	/// Point the camera looks at and is attached to.
	inline const glm::vec3& getOrbitPoint() const { return m_orbitPoint; }
	/**
	* @brief Moves the orbit point, the camera follows it and keeps its view dir and distance.
	* @param time Duration of the transition in milliseconds, zero or smaller is instant.
	*/
	void setOrbitPoint(glm::vec3 point, float time = 0.f);

	inline float getOrbitDistance() const { return m_orbitDistance; }
	/**
	* @brief Moves the camera closer to or further away from the orbit point.
	* @param distance Clamped into the min and max orbit distance.
	* @param time Duration of the transition in milliseconds, zero or smaller is instant.
	*/
	void setOrbitDistance(float distance, float time = 0.f);

	/// Direction the camera looks in, pointing from the camera to the orbit point.
	inline const glm::vec3& getForwardOrbit() const { return m_orbitForward; }
	/**
	* @brief Rotates the camera around the orbit point, the orbit point itself stays in place.
	* @param viewDir Ignored when it has no length or points straight up or down.
	* @param time Duration of the transition in milliseconds, zero or smaller is instant.
	* @param rotationAxis Axis the transition turns around, it keeps the view tilt towards that axis.
	* Without one the transition takes the shortest path over the sphere, which tilts the view on the way.
	*/
	void setForwardOrbit(glm::vec3 viewDir, float time = 0.f, std::optional<glm::vec3> rotationAxis = std::nullopt);

	/// Rotation of the camera around the orbit point on the y axis in degree.
	float getCameraOrbit() const;
	/**
	* @brief Turns the camera around the y axis of the orbit point, the pitch is kept.
	* @param degree Absolute orbit angle, not a delta.
	* @param time Duration of the transition in milliseconds, zero or smaller is instant.
	*/
	void setCameraOrbit(float degree, float time = 0.f);

	/// Tilt of the view against the ground plane in degree, negative while the camera looks down.
	float getOrbitPitch() const;
	/**
	* @brief Turns the camera around the orbit point to a tilt and a heading at once.
	* @param pitchDegree Negative looks down. Clamped short of straight down, which leaves no heading.
	* @param orbitDegree Absolute orbit angle, the one getCameraOrbit reads. @see setCameraOrbit
	*/
	void setOrbitAngles(float pitchDegree, float orbitDegree);

	inline float getMinOrbitDistance() const { return m_minOrbitDistance; }
	inline void setMinOrbitDistance(float value) { m_minOrbitDistance = value; }

	inline float getMaxOrbitDistance() const { return m_maxOrbitDistance; }
	inline void setMaxOrbitDistance(float value) { m_maxOrbitDistance = value; }

protected:
	void onMove(float deltaTime) override;
	bool onMouseMotion(PG_MouseMotionEvent* e) override;
	bool onMouseWheel(PG_MouseWheelEvent* e)  override;
	bool onKey(PG_KeyEvent* key) override;

	/// View dir flattened onto the ground plane, the direction forward movement follows.
	glm::vec3 getGroundForward() const;
	/**
	* @brief Scale the orbit point movement runs at.
	* @details Grows with the orbit distance, so the ground moves along with the cursor, but only by
	* its square root, so zooming far out does not turn every step into a jump.
	*/
	float getMoveScale() const;
	/// Places the camera at the orbit point, pushed back along the view dir.
	void updateOrbitTransformation();
	/**
	* @brief Starts a linear animation and replaces the one running under the same name.
	* @param apply Called every tick with the progress in [0,1].
	*/
	void startOrbitAnimation(const std::string& name, float time, std::function<void(float)> apply);
	/// Stops the animation running under the given name, if there is one.
	void stopOrbitAnimation(const std::string& name);

	glm::vec3 m_orbitPoint = glm::vec3(0.f);
	glm::vec3 m_orbitForward = glm::vec3(0.f, 0.f, 1.f);
	float m_orbitDistance = 64.f;
	float m_minOrbitDistance = 2.f;
	float m_maxOrbitDistance = 300.f;
};
