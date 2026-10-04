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
#include <Stream/PG_StreamIn.h>
#include <Util/PG_File.h>

struct PG_DisaModInfo;
struct PG_DisaModLoadInfo;

/**
* @brief The "sprites" block of every loaded modification.
*/
class PG_DisaModSpritesModel {
public:
	struct SpriteInfo {
		PG_DisaModInfo* modInfo = nullptr;
		PG_DisaIdentifier identifier;
		PG_File file; //path inside the zip
	};

	/**
	* @brief Parses the "sprites" block of the mod json, does nothing when the block is missing.
	* @details A sprite with the identifier of an earlier one replaces it.
	*/
	void load(PG_DisaModInfo* modInfo, const PG_DisaModLoadInfo& loadInfo);
	void clear() { m_sprites.clear(); }

	/// @returns 0 for an invalid identifier, the prinny sprite when unknown
	unsigned short getSpriteId(const PG_DisaIdentifier& identifier) const;
	/// @returns 0 when no mod carries the sprite
	unsigned short findSpriteId(const PG_DisaIdentifier& identifier) const { return static_cast<unsigned short>(m_sprites.getId(identifier)); }
	const SpriteInfo& getSpriteInfo(unsigned int id) const { return m_sprites.getById(id); }

	/**
	* @brief Opens the sprite, a loose file in "mod/CustomSprite" wins over the one in the zip.
	* @param spriteId A custom id, or an original id that a mod may replace.
	* @returns The reader to delete by the caller, nullptr when nothing provides the sprite.
	*/
	std::pair<PG::STREAM::In*, const SpriteInfo&> createSpriteReader(const std::string& baseDir, unsigned int spriteId) const;

private:
	const SpriteInfo& getInfoOrReplacement(unsigned int spriteId) const;

	PG_DisaModRegistry<SpriteInfo> m_sprites{ PG_DisaIdentifier::customIdStart };
};
