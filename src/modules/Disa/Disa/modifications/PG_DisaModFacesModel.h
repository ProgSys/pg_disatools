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
* @brief The "faces" block (portraits) of every loaded modification.
*/
class PG_DisaModFacesModel {
public:
	struct FaceInfo {
		PG_DisaModInfo* modInfo = nullptr;
		PG_DisaIdentifier identifier;
		PG_File fileHD; //path inside the zip, always set for a loaded face
		PG_File fileSD; //path inside the zip, may be empty
	};

	/**
	* @brief Parses the "faces" block of the mod json, does nothing when the block is missing.
	* @details A face with the identifier of an earlier one replaces it.
	*/
	void load(PG_DisaModInfo* modInfo, const PG_DisaModLoadInfo& loadInfo);
	void clear() { m_faces.clear(); }

	/// @returns 0 when no mod carries the face
	unsigned short getFaceId(const PG_DisaIdentifier& identifier) const { return static_cast<unsigned short>(m_faces.getId(identifier)); }
	const FaceInfo& getFaceInfo(unsigned int id) const { return m_faces.getById(id); }

	/**
	* @brief Opens the face, a loose file in "mod/CustomImg" wins over the one in the zip.
	* @param faceId A custom id, or an original id that a mod may replace.
	* @returns The reader to delete by the caller, nullptr when nothing provides the face.
	*/
	std::pair<PG::STREAM::In*, const FaceInfo&> createFaceReader(const std::string& baseDir, unsigned int faceId, bool isHD) const;

private:
	const FaceInfo& getInfoOrReplacement(unsigned int faceId) const;

	PG_DisaModRegistry<FaceInfo> m_faces{ PG_DisaIdentifier::customIdStart };
};
