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

#include <vector>
#include <Disa/fileformats/PG_MPD.h>
#include <Util/PG_DataSerializer.h>

/**
* @brief Custom content in the metadata block of a binary MPD, shared by the editor and the game.
* @details The custom models are stored as a list that mirrors the chunks of the file:
* ```
* customModelsVersion: 1
* chunks: [ { index: 0, models: [ { name, x, y, z, xRot, yRot, zRot, xScale, yScale, zScale } ] } ]
* ```
* Only chunks with custom models get an entry, index is the chunk of the file it belongs to.
*/
namespace PG_MPDCustom {
	constexpr int modelsVersion = 1;

	/**
	* @brief Replaces the models of one chunk of the file.
	* @details Empty models remove the entry of the chunk, and the chunks list with the version once
	* no chunk is left in it.
	*/
	void writeModels(PG_DataSerializer& metaData, int chunkIndex, const std::vector<PG_MPDCustomModel>& models);
	/// The models of one chunk of the file, empty when it has none.
	std::vector<PG_MPDCustomModel> readModels(const PG_DataSerializer& metaData, int chunkIndex);
}
