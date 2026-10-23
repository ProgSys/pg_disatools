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

#include <memory>
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
* The game has no "materials" folder, so a texture name is the entry path as it is. Only textures are served.
*
* The source only reads. Whoever writes a game file it reads does so inside guardModification.
*/
class PG_DisaGameResourceSource : public PG_ResourceSource {
public:
	static constexpr std::string_view nameSpace = "disa";

	/// gameDir is the folder of the exe, the one holding SUBDATA.DAT and DATA.DAT.
	explicit PG_DisaGameResourceSource(const PG_Dir& gameDir);

	std::string getDescription() const override { return m_gameDir.getDir(); }
	/// The files of the game folder only, listing what the archives carry would decompress them.
	void traverseEntries(const std::function<void(const std::string&)>& unary) const override;
	/// Textures only, the name is the entry path.
	std::string toEntryPath(PG_EResourceFolder folder, std::string_view name) const override;

	void onDeregistered(PG_ResourceManager* manager) override;

	inline const PG_Dir& getGameDir() const { return m_gameDir; }

protected:
	std::shared_ptr<PG::STREAM::In> openStream(const std::string& path) const override;

	/// Lets go of the opened archives, so the files can be written.
	void onModificationBegin() override;

private:
	/// The file of the game folder that name stands for. Empty for anything that would leave the folder.
	PG_File toGameFile(std::string_view name) const;

	PG_Dir m_gameDir;
	mutable PG_DisaArchiveWalker m_walker;
};

/**
* @brief Registers the game folder under "disa".
* @returns An empty scope when the folder holds no SUBDATA.DAT or there is no resource manager.
*/
PG_ResourceSourceScope<PG_DisaGameResourceSource> registerDisaGameSource(const PG_Dir& gameDir);
