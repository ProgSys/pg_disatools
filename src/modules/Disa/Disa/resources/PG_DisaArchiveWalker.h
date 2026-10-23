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

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class PG_DisaFileContainer;
class PG_DisaFileEntry;
namespace PG {
	namespace STREAM {
		class In;
	}
}

/**
* @brief Reads an entry through every game archive on its path, "SUBDATA.DAT/START.DAT/ITEM.TX2" and the like.
* @details The root, "SUBDATA.DAT" here, is handed to the RootOpener. Every following segment is looked up case
* insensitive in the archive the segment before it is. What an archive is, is read from its data, so a PSPFS, an
* OLA or any other container PG_Disa::createFileContainer knows can sit at any level. A compressed OLA is
* decompressed in memory once, the file itself is never written.
*
* Opened archives are kept, so the next read of a neighbouring entry does not parse them again. All of it
* is thread safe, reads are served one after the other and hand back their own copy of the bytes.
*/
class PG_DisaArchiveWalker {
public:
	/**
	* @brief Opens the outermost file, "SUBDATA.DAT".
	* @details Called with the walkers lock held, so it must not read through this walker.
	* @returns Null when it is not there. The walker keeps the stream for as long as it keeps the archive.
	*/
	using RootOpener = std::function<std::shared_ptr<PG::STREAM::In>(std::string_view root)>;

	explicit PG_DisaArchiveWalker(RootOpener opener);
	~PG_DisaArchiveWalker();

	PG_DisaArchiveWalker(const PG_DisaArchiveWalker&) = delete;
	PG_DisaArchiveWalker& operator=(const PG_DisaArchiveWalker&) = delete;

	/**
	* @brief Reads the whole entry at the end of the path, its first segment is the root.
	* @returns Empty when a segment is missing, is no archive, or the path is malformed ("", "a//b", "..").
	*/
	std::optional<std::vector<char>> read(std::string_view path) const;

	/// Forgets every opened archive and lets go of the root streams, the next read opens them again.
	void clear();
	/// Like clear, but only when nothing was read for longer than timeout.
	void releaseIdle(std::chrono::steady_clock::duration timeout);
	/// Whether any archive is kept right now.
	bool isHoldingArchives() const;

private:
	/**
	* @brief The archive the reader holds, built and cached under key. Expects m_mutex to be locked.
	* @param entry The entry in the parent archive, null for the root. The archive is set as its container.
	* @returns Null when the data is no archive.
	*/
	std::shared_ptr<PG_DisaFileContainer> openArchive(const std::string& key, std::shared_ptr<PG::STREAM::In> reader,
		PG_DisaFileEntry* entry = nullptr) const;

	RootOpener m_opener;

	mutable std::mutex m_mutex;
	/// lowercased path of the archive -> the archive, like "subdata.dat/start.dat"
	mutable std::map<std::string, std::shared_ptr<PG_DisaFileContainer>> m_archives;
	mutable std::chrono::steady_clock::time_point m_lastRead;
};
