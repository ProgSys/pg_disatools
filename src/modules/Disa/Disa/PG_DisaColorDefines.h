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

#include <Util/Color/ColorDefined.h>

/**
* @brief The colors the Disa tools paint their gui with, on top of the engine defined ones.
* @details Only what PG_Color has no equivalent for lives here. The panel background is
* PG_Color::blackGray, a separator is PG_Color::darkerGray and a highlight is PG_Color::hentaiBlue.
* Reach for those before adding anything below.
*/
namespace PG_DisaColor {
	D_COLOR(backgroundDeco, glm::rgba(45, 45, 45, 255));
	D_COLOR(backgroundDeco2, glm::rgba(30, 30, 30, 255));
	D_COLOR(backgroundDark, glm::rgba(20, 20, 20, 255));
	D_COLOR(text, glm::rgba(200, 200, 200, 255));

	/// The three states a button gets when pressing it throws something away. PG_Color::hentaiRed, dimmed.
	D_COLOR(destructive, glm::rgba(87, 36, 42, 255));
	D_COLOR(destructiveHover, glm::rgba(127, 52, 61, 255));
	D_COLOR(destructivePressed, glm::rgba(69, 28, 33, 255));
}
