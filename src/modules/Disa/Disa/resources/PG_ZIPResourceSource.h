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

#include <map>
#include <mutex>
#include <string>
#include <Resources/PG_ResourceSource.h>
#include <Disa/fileformats/PG_ZIP.h>

/**
* @brief A zip archive as a resource source, so what it carries loads like anything else.
* @details Entry paths are the paths inside the archive, so a campaign that ships
* "materials/heroes/portrait.png" serves it under that name in its own namespace.
*
* The archive belongs to whoever opened it and not to this, which is why it is held weakly. A
* read that can no longer reach it simply fails, and that is the whole of what happens when a
* campaign closes while something was still loading. What a read hands back is a stream over its
* own copy of the bytes, so once it is open it needs the archive no more.
*/
class PG_ZIPResourceSource : public PG_ResourceSource {
public:
	PG_ZIPResourceSource(const std::shared_ptr<PG_ZIP>& zip, std::string nameSpace);

	std::string getDescription() const override { return m_description; }
	void traverseEntries(const std::function<void(const std::string&)>& unary) const override;

	/// Whether the archive behind this is still there.
	inline bool isAlive() const { return !m_zip.expired(); }

protected:
	std::shared_ptr<PG::STREAM::In> openStream(const std::string& path) const override;
	void traversePGR(const std::function<void(const std::string&)>& unary) const override;

private:
	std::weak_ptr<PG_ZIP> m_zip;
	/// path inside the archive -> its entry, so a read is a lookup and not a scan of every row
	std::map<std::string, PG_DisaFileEntry> m_entries;
	/// one handle serves the whole archive, so the loader threads take turns reading from it
	mutable std::mutex m_mutex;
	/// kept, because it is still worth naming in a log once the archive itself is gone
	std::string m_description;
};
