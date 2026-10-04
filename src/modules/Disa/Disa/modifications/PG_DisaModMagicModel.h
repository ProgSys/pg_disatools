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

#include <Disa/modifications/PG_DisaModRegistry.h>
#include <Disa/dataStructs/PG_DisaDataMagic.h>

struct PG_DisaModInfo;
struct PG_DisaModLoadInfo;

/**
* @brief The "magic" block (magic and skills) of every loaded modification.
*/
class PG_DisaModMagicModel {
public:
	struct MagicInfo {
		PG_DisaModInfo* modInfo = nullptr;
		PG_DisaIdentifier identifier;
		PG_DisaIdentifier scriptIdentifier; //invalid when the magic has no custom script
		PG_DisaDataMagic magicData;
	};

	/**
	* @brief Parses the "magic" block of the mod json, does nothing when the block is missing.
	* @details A magic with the identifier of an earlier one replaces it.
	*/
	void load(PG_DisaModInfo* modInfo, const PG_DisaModLoadInfo& loadInfo);
	void clear() { m_magic.clear(); }

	/// @returns 0 when unknown
	unsigned int getMagicId(const PG_DisaIdentifier& identifier) const { return m_magic.getId(identifier); }
	/// @returns A "disa" identifier for original ids, an invalid one for unknown custom ids
	PG_DisaIdentifier getMagicIdentifier(unsigned int id) const;
	/// @returns An empty entry when the id is no known custom magic id
	const MagicInfo& getMagicInfo(unsigned int id) const { return m_magic.getById(id); }

	/// Entries may be changed, but never their identifier
	auto begin() { return m_magic.begin(); }
	auto end() { return m_magic.end(); }
	auto begin() const { return m_magic.begin(); }
	auto end() const { return m_magic.end(); }

private:
	PG_DisaModRegistry<MagicInfo> m_magic{ PG_DisaIdentifier::customMagicIdStart };
};
