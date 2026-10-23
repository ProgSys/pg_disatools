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
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <Util/PG_Dir.h>
#include <Util/PG_File.h>
#include <Resources/PG_ResourceSource.h>
#include <Resources/PG_ResourceSourceScope.h>
#include <Disa/resources/PG_DisaArchiveWalker.h>

/**
* @brief The game folder as a resource source, so what the archives carry loads like anything else.
* @details Served under the namespace "disa". An entry path starts with a file of the game folder and runs
* through every archive on the way, so "disa::SUBDATA.DAT/START.DAT/ITEM.TX2" is ITEM.TX2 inside START.DAT
* inside SUBDATA.DAT. Names are case insensitive like the game treats them.
* @code
* image I_disa_iconsAtlas {
*	path: "disa::SUBDATA.DAT/START.DAT/ITEM.TX2"
* }
* @endcode
*
* The game has no "materials" folder, so a texture name is the entry path as it is. Only textures are served,
* models, shaders and the like are not asked for.
*
* Only the saved files are read. Whoever writes a game file while the source is registered does so inside
* guardModification, guardFile finds the right source for a file. A file changed any other way is noticed on
* the next read, the source warns and reads it again.
*/
class PG_DisaGameResourceSource : public PG_ResourceSource {
public:
	static constexpr std::string_view nameSpace = "disa";
	/// How long a registered source keeps parsed archives after its last read. START.DAT alone is big.
	static constexpr std::chrono::seconds idleTimeout{ 30 };

	/// gameDir is the folder of the exe, the one holding SUBDATA.DAT and DATA.DAT.
	explicit PG_DisaGameResourceSource(const PG_Dir& gameDir);
	~PG_DisaGameResourceSource() override;

	std::string getDescription() const override { return m_gameDir.getDir(); }
	/// The files of the game folder only, listing what the archives carry would decompress them.
	void traverseEntries(const std::function<void(const std::string&)>& unary) const override;
	/// Textures only, the name is the entry path.
	std::string toEntryPath(PG_EResourceFolder folder, std::string_view name) const override;

	/// Keeps parsed archives between reads, until they were idle for idleTimeout.
	void onRegistered(PG_ResourceManager* manager) override;
	/// Lets go of every parsed archive.
	void onDeregistered(PG_ResourceManager* manager) override;

	inline const PG_Dir& getGameDir() const { return m_gameDir; }

	/**
	* @brief Suspends the registered game source whose folder holds file, so it can be written.
	* @returns An empty guard when no registered game source reads that file.
	* @see PG_ResourceSource::guardModification
	*/
	[[nodiscard]] static ModificationGuard guardFile(const PG_File& file);

protected:
	std::shared_ptr<PG::STREAM::In> openStream(const std::string& path) const override;

	/// Lets go of the parsed archives, so the files can be written.
	void onModificationBegin() override;
	/// Forgets the stamps, the files are read as they are now.
	void onModificationEnd() override;

private:
	/// The file of the game folder that name stands for. Empty for anything that would leave the folder.
	PG_File toGameFile(std::string_view name) const;
	/// Drops the parsed archives when the root file changed since it was opened.
	void checkStamp(std::string_view root) const;

	PG_Dir m_gameDir;
	mutable PG_DisaArchiveWalker m_walker;

	mutable std::mutex m_stampMutex;
	/// lowercased root file -> its stamp when the walker opened it
	mutable std::map<std::string, PG_File::Stamp> m_stamps;
	/// Shared with the companion that releases idle archives, set to make it go. Null without one.
	std::shared_ptr<std::atomic_bool> m_companionReleased;
};

/**
* @brief Registers the game folder under "disa".
* @returns An empty scope when the folder holds no SUBDATA.DAT or there is no resource manager.
*/
PG_ResourceSourceScope<PG_DisaGameResourceSource> registerDisaGameSource(const PG_Dir& gameDir);
