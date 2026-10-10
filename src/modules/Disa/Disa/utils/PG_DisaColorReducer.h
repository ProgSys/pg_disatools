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

#include <Util/PG_Image.h>
#include <Util/Color/RGBA.h>
#include <vector>

namespace PG_DisaColorReducer {
	struct Result {
		std::vector<glm::rgba> palette; /// At most maxColors entries
		std::vector<unsigned char> indices; /// One palette index per pixel, row by row
	};

	/**
	* @brief Builds a palette of at most maxColors colors for the image and maps every pixel to its closest entry.
	* @details Images that already fit keep their exact colors. Otherwise the colors are split by median cut in OKLab
	* space with alpha as a fourth axis, then refined with a few k-means passes. Each distinct color counts with the
	* square root of its pixel count, so a big area of similar colors, like a gradient background, can't take over the
	* whole palette and leave nothing for the small details. Fully transparent pixels share one transparent entry.
	* @param maxColors 2 to 256
	*/
	Result reduce(const PG_RGBAImage& image, std::size_t maxColors);
}
