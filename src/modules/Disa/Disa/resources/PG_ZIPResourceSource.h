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

#include <atomic>
#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <Util/PG_File.h>
#include <Resources/PG_ResourceSource.h>

class zip;
using zipUPtr = std::unique_ptr<zip, void(*)(zip*)>;

/**
* @brief A zip archive on disk as a resource source, so what it carries loads like anything else.
* @details Entry paths are the paths inside the archive, so a campaign that ships
* "materials/heroes/portrait.png" serves it under that name in its own namespace.
*
* The source is the only one holding the libzip handle. It opens it on the first read and, while
* registered, closes it again once nothing was read for idleTimeout, so the file is not kept locked.
* A read hands back a stream over its own copy of the bytes, so an open read never needs the handle.
*
* The indexes are built once, in the constructor. A file that changed while the handle was closed
* can therefore no longer be read, the source has to be built and registered again.
* Once deregistered it does not open the file again, so its owner can overwrite it.
*/
class PG_ZIPResourceSource : public PG_ResourceSource {
public:
	/// How long a registered source keeps the handle open after its last read.
	static constexpr std::chrono::seconds idleTimeout{ 10 };

	PG_ZIPResourceSource(const PG_File& zipFile, std::string nameSpace);
	~PG_ZIPResourceSource() override;

	std::string getDescription() const override { return m_file.getPath(); }
	void traverseEntries(const std::function<void(const std::string&)>& unary) const override;

	/// Starts closing the handle when idle.
	void onRegistered(PG_ResourceManager* manager) override;
	/// Closes the handle and stops reading, so the file may be overwritten.
	void onDeregistered(PG_ResourceManager* manager) override;

	/// False when the file could not be opened when the source was built, it then carries nothing.
	inline bool isValid() const { return m_isValid; }
	inline const PG_File& getFile() const { return m_file; }
	/// Whether the libzip handle is open right now. Thread safe.
	bool isOpen() const;

	/// Closes the handle when nothing was read for longer than timeout. Thread safe.
	void closeIfIdle(std::chrono::steady_clock::duration timeout);
	/// Closes the handle, the next read opens it again. Thread safe.
	void closeNow();

protected:
	std::shared_ptr<PG::STREAM::In> openStream(const std::string& path) const override;
	void traversePGR(const std::function<void(const std::string&)>& unary) const override;

private:
	struct Entry {
		std::uint64_t index = 0; /// libzip index
		std::uint64_t size = 0;  /// uncompressed
	};
	/**
	* @brief Opens the handle if it is closed. Expects m_mutex to be locked.
	* @returns false when the source was deregistered, or the file is gone or changed.
	*/
	bool ensureOpen() const;

	PG_File m_file;
	PG_File::Stamp m_stamp; /// of the file the indexes were built from
	bool m_isValid = false;
	/// path inside the archive -> its entry, so a read is a lookup and not a scan of every row
	std::map<std::string, Entry> m_entries;

	/// libzip handles are not thread safe, and the loader threads read concurrently
	mutable std::mutex m_mutex;
	mutable zipUPtr m_archive; /// null while closed
	mutable std::chrono::steady_clock::time_point m_lastRead;
	mutable bool m_isChangeReported = false;
	bool m_isDeregistered = false;
	/// Shared with the companion that closes the handle when idle, set to make it go
	std::shared_ptr<std::atomic_bool> m_companionReleased;
};
