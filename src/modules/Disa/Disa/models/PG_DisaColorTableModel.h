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

#include <Scene/Model/PG_ListModelBase.h>
#include <Util/Color/RGBA.h>
#include <Util/PG_File.h>
#include <vector>

/**
* @brief A list of the colors of one color table, out of a set of tables that all have the same size.
* @details The model doesn't own the tables, it edits the vector it was given with setTables. TX2 images and SH
* sprites both keep their tables as std::vector<ColorTable>, so either can be handed over directly.
* Whoever owns the tables listens to colorsModified to redraw, and calls refresh after changing them itself.
*/
class PG_DisaColorTableModel : public PG_ListModelBase {
	PG_OBJECT(PG_DisaColorTableModel);
	PG_PARENT(PG_ListModelBase);
	PG_PROPERTY(int currentTable GET getCurrentTable SET setCurrentTable CHANGED currentTableChanged)
	PG_PROPERTY(int tableCount GET getTableCount CHANGED tablesChanged)
signals:
	void currentTableChanged();
	/// A table was added or removed
	void tablesChanged();
	/// Emitted after this model changed a color or a table, not when refresh is called
	void colorsModified();
public:
	using Tables = std::vector<std::vector<glm::rgba>>;

	using PG_ListModelBase::PG_ListModelBase;

	/**
	* @brief Sets the tables to show and edit, nullptr to show nothing
	* @param colorsPerTable Size of a new table, the existing tables are expected to have this size
	*/
	void setTables(Tables* tables, std::size_t colorsPerTable);
	inline Tables* getTables() const { return m_tables; }
	/// Call after the tables were changed from outside
	void refresh();

	/// When false only colors can be changed, the number of tables stays
	inline bool isTableCountEditable() const { return m_tableCountEditable; }
	void setTableCountEditable(bool editable);

	int getTableCount() const;
	int getCurrentTable() const { return m_currentTable; }
	void setCurrentTable(int table);
	std::size_t getColorsPerTable() const { return m_colorsPerTable; }

	glm::rgba getColor(std::size_t index) const;
	void setColor(std::size_t index, const glm::rgba& color);

	/// Inserts a table after the current one and makes it current, a copy of it or all transparent black
	void insertTable(bool copyCurrent);
	void removeCurrentTable();

	/// Writes the current table as png, one pixel per color
	bool exportTable(const PG_File& file) const;
	/// Reads the pixels of an image row by row into the current table
	bool importTable(const PG_File& file);

	static std::string toHex(const glm::rgba& color);
	/// Accepts RRGGBB or RRGGBBAA, with or without a leading '#'
	static bool fromHex(std::string hex, glm::rgba& colorOut);

	bool empty() const override { return size() == 0; }
	std::size_t size() const override;
	PG_Variety getData(PG_Index index, Identity identity) const override;
	void setData(PG_Index index, const PG_Variety& data, Identity identity) override;

private:
	void resetItems(std::size_t oldSize);

	Tables* m_tables = nullptr;
	std::size_t m_colorsPerTable = 0;
	int m_currentTable = 0;
	bool m_tableCountEditable = true;
};
