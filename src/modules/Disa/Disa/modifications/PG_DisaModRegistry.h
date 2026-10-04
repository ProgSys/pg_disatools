/*
 * GNU Lesser General Public License (LGPL-3.0)
 *
 * Copyright (c) 2024 ProgSys
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

#include <Disa/PG_DisaIdentifier.h>
#include <map>
#include <optional>
#include <string>
#include <vector>

/**
* @brief Table of mod entries that can be found by identifier or by id.
* @details The id of an entry is its index plus idStart. An entry added with an identifier that is already
* present replaces the old one and keeps its index, so the last loaded mod wins.
* @tparam TEntry Needs a default constructor and a PG_DisaIdentifier member named identifier.
*/
template<typename TEntry>
class PG_DisaModRegistry {
public:
	explicit PG_DisaModRegistry(unsigned int idStart) : m_idStart(idStart) {}

	/**
	* @returns true when added, false when it replaced an entry or the identifier is invalid, plus the index of the entry
	*/
	std::pair<bool, unsigned int> add(TEntry entry) {
		if (!entry.identifier) return { false, 0 };
		std::string key = entry.identifier.getIdentifier();
		if (auto findIt = m_mapper.find(key); findIt != m_mapper.end()) {
			m_entries[findIt->second] = std::move(entry);
			return { false, findIt->second };
		}
		const unsigned int index = static_cast<unsigned int>(m_entries.size());
		m_mapper.emplace(std::move(key), index);
		m_entries.push_back(std::move(entry));
		return { true, index };
	}

	/// @returns An empty entry when out of range
	const TEntry& getByIndex(unsigned int index) const {
		if (index >= m_entries.size()) return getEmpty();
		return m_entries[index];
	}

	/// @returns An empty entry when the id is below idStart or unknown
	const TEntry& getById(unsigned int id) const {
		if (id < m_idStart) return getEmpty();
		return getByIndex(id - m_idStart);
	}

	std::optional<unsigned int> findIndex(const PG_DisaIdentifier& identifier) const {
		if (!identifier) return std::nullopt;
		auto findIt = m_mapper.find(identifier.getIdentifier());
		if (findIt == m_mapper.end()) return std::nullopt;
		return findIt->second;
	}

	/// @returns nullptr when unknown
	const TEntry* find(const PG_DisaIdentifier& identifier) const {
		auto index = findIndex(identifier);
		return index ? &m_entries[*index] : nullptr;
	}

	/// @returns 0 when unknown
	unsigned int getId(const PG_DisaIdentifier& identifier) const {
		auto index = findIndex(identifier);
		return index ? *index + m_idStart : 0;
	}

	bool containsId(unsigned int id) const { return id >= m_idStart && id - m_idStart < m_entries.size(); }

	void clear() {
		m_entries.clear();
		m_mapper.clear();
	}

	std::size_t size() const { return m_entries.size(); }
	bool empty() const { return m_entries.empty(); }

	/// Entries may be changed, but never their identifier
	auto begin() { return m_entries.begin(); }
	auto end() { return m_entries.end(); }
	auto begin() const { return m_entries.begin(); }
	auto end() const { return m_entries.end(); }

private:
	static const TEntry& getEmpty() {
		static const TEntry empty{};
		return empty;
	}

	std::vector<TEntry> m_entries;
	std::map<std::string, unsigned int> m_mapper;
	unsigned int m_idStart;
};
