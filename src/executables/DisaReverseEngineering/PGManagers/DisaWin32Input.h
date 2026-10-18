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

#include <Base/PG_Key.h>
#include <Base/PG_ControllerButton.h>
#include <cstdint>
#include <string>
#include <glm/glm.hpp>

/**
* @brief Translates Win32 input into pg_engine values. The only place where virtual keys and scancodes appear.
*/
namespace DisaWin32Input {
	/**
	* @brief The key of a WM_KEYDOWN or WM_KEYUP message.
	* @details Keys are named by their US layout position, like SDL does without a layout. Shift, Ctrl and Alt
	* are told apart into left and right by the scancode and the extended bit in lParam.
	*/
	PG_EKeyCode toKeyCode(unsigned int virtualKey, std::intptr_t lParam);
	/// The physical key of a key message, from the scancode in bits 16 to 23 and the extended bit 24 of lParam
	PG_EScancode toScancode(unsigned int virtualKey, std::intptr_t lParam);
	/// The modifier keys held right now, toggles like caps lock included
	PG_KeymodValue getKeymod();
	/// The mouse buttons held right now as PG_BUTTON_*MASK bits
	int getMouseButtons();
	/// The PG_BUTTON_* index of a mouse button message, 0 when it is none
	int toMouseButton(unsigned int message, std::uintptr_t wParam);
	/// The client position packed into the lParam of a mouse message
	glm::ivec2 toMousePosition(std::intptr_t lParam);

	/// The button of an XINPUT_GAMEPAD_* bit
	PG_EControllerButton toControllerButton(unsigned short xinputButton);
	/**
	* @brief A thumb stick in the range of -1 to 1, positive y is up.
	* @details Inside the dead zone the result is zero, outside its length is clamped to 1.
	*/
	glm::vec2 toStickDirection(short xAxis, short yAxis);
	/// A trigger from 0 to 1, zero below the XInput threshold
	float toTriggerValue(unsigned char value);

	/// Encodes a code point as UTF-8, empty for an invalid one
	std::string toUtf8(char32_t codePoint);
}
