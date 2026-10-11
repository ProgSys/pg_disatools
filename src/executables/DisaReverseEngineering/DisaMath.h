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

/**
* @brief Rotates (x, z) around the y axis and writes the result to the globals 0x006B791C (x) and 0x006B7A60 (z).
* Original function: FUN_004E32A0
* @param angleDegrees Wrapped into 0 to 360 once, so angles below -360 or from 720 on are not fully wrapped.
*/
void FUN_004e32a0_rotateXZ(float angleDegrees, float x, float z);

/**
* @brief On which side of the line from begin through end the point lies, in the xz plane.
* Original function: FUN_0046E410, which leaves the point on the stack for the caller to pop.
* @return 1 or -1 by side. A point on the line gives -1 before begin, 1 past end and 0 on the segment.
*/
int __fastcall FUN_0046e410_sideOfLine(const glm::vec3* begin, const glm::vec3* end, const glm::vec3* point);
