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
#include <Disa/fileformats/PG_DisaFileContainer.h>

/**
* @brief Presents files lying in a folder as container entries
* @details An entry reads through its parent container, so a file on disk has nothing to read
* through. Every other container parses a packed archive, this one is the stand in for those.
* Nothing is read until somebody asks an entry for a reader, and nothing can be saved.
* Must be held in a shared_ptr, opening an entry as a container reaches for shared_from_this.
*/
class PG_DisaLooseFileContainer : public PG_DisaFileContainer {
public:
	using PG_DisaFileContainer::PG_DisaFileContainer;

	/**
	* @brief Adds the file as an entry, without touching its data
	* @returns An invalid entry when the file is missing or empty
	*/
	PG_DisaFileEntry add(const PG_File& file);

	using PG_DisaFileInterface::open;
	/// There is no archive to parse, entries only arrive through add()
	bool open(std::shared_ptr<PG::STREAM::In> reader, const std::function<void(PG_DisaFileEntry&)>& visitor) final {
		return FAILURE;
	}
	std::shared_ptr<PG::STREAM::In> createEntryReaderOriginal(const PG_DisaFileEntry& entry) const final;
};
