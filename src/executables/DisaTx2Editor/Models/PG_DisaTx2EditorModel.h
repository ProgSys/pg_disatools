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

#include <Base/PG_Object.h>
#include <Disa/fileformats/PG_TX2.h>
#include <Resources/PG_Texture.h>
#include <Util/PG_File.h>
#include <string>
#include <vector>

class PG_DisaColorTableModel;

/**
* @brief Holds the TX2 image that is edited, and the texture it is shown with.
* @details Other images (png, tga, jpg) can be opened too, they become BGRA TX2 images.
*/
class PG_DisaTx2EditorModel : public PG_Object {
	PG_OBJECT(PG_DisaTx2EditorModel);
	PG_PARENT(PG_Object);
signals:
	/// The pixels, the size or the type changed
	void imageChanged();
	void fileChanged();
	void modifiedChanged();
public:
	struct ConvertTarget {
		std::string name;
		PG_TX2::Type type;
	};

	using PG_Object::PG_Object;
	/// Called by PG_new
	void init();

	bool open(const PG_File& file, std::string& errorOut);
	/// Saves as TX2 for .tx2 and .txp, everything else is written as png
	bool save(const PG_File& file, std::string& errorOut);
	bool convertTo(PG_TX2::Type type, std::string& errorOut);

	inline bool isOpen() const { return m_tx2.isValid(); }
	inline bool isModified() const { return m_modified; }
	inline const PG_File& getFile() const { return m_file; }
	/// True when the file can be overwritten as it is, an opened png has to be saved as TX2 first
	bool isTx2File() const;
	inline const PG_TX2& getTx2() const { return m_tx2; }
	inline const PG_Texture& getTexture() const { return m_texture; }
	inline PG_DisaColorTableModel* getColorTableModel() const { return m_colorTableModel; }

	std::string getTypeName() const;
	static const std::vector<ConvertTarget>& getConvertTargets();
	/// The number of different colors the image has with the current color table, stops counting after max + 1
	std::size_t countColors(std::size_t max) const;

private:
	void resetColorTables();
	void updateTexture();
	void setModified(bool modified);

	PG_TX2 m_tx2;
	PG_File m_file;
	PG_Texture m_texture;
	PG_DisaColorTableModel* m_colorTableModel = nullptr;
	bool m_modified = false;
};
