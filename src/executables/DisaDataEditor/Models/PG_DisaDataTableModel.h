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
#include <Disa/fileformats/PG_Def.h>
#include <Util/PG_File.h>
#include <Util/PG_Dir.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

/**
* @brief A two dimensional model of a Disgaea .DAT table, its layout is given by a .DEF definition file.
* @details Every cell is held as text. Numbers are parsed and range checked when they are edited and
* written back to binary on save.
*/
class PG_DisaDataTableModel : public PG_ListModelBase {
	PG_OBJECT(PG_DisaDataTableModel);
	PG_PARENT(PG_ListModelBase);
signals:
	void fileChanged();
	void modifiedChanged();
public:
	enum EIdentities : Identity {
		Identity_Background = EIdentity::PG_Identity_Custom, /// glm::vec4, alpha is zero when the column has no color
		Identity_Editable /// bool
	};

	using PG_ListModelBase::PG_ListModelBase;

	/**
	* @brief Finds the definition file whose name is the longest prefix of the given file name.
	* @returns An empty file when there is none.
	*/
	static PG_File findDefFile(const PG_File& dataFile);
	static PG_Dir getDefDir();

	/**
	* @brief Opens the given .DAT file, the definition is looked up by its name.
	* @param errorOut Is set to a message for the user when it fails.
	*/
	bool open(const PG_File& file, std::string& errorOut);
	bool save(const PG_File& file);
	bool exportCSV(const PG_File& file) const;

	inline const PG_File& getFile() const { return m_file; }
	inline bool isOpen() const { return !m_def.header.empty(); }
	inline bool isModified() const { return m_modified; }

	std::size_t getColumnCount() const { return m_def.header.size(); }
	std::string getColumnName(std::size_t column) const;
	/// Suggested start width of a column in pixels
	float getColumnWidth(std::size_t column) const;

	/**
	* @brief Inserts a copy of the row at the given index, or of the last row when it is the end.
	* @details Columns of type INDEX of all following rows are increased by one.
	*/
	void insertRow(std::size_t at);
	/// Index columns of all following rows are decreased by one
	void removeRow(std::size_t at);
	/// Swaps two neighbouring rows, index columns keep their position
	void moveRow(std::size_t from, std::size_t to);

	bool empty() const override { return m_rows.empty(); }
	std::size_t dimension() const override { return 2; }
	std::size_t size() const override { return m_rows.size(); }
	std::size_t sizeOfDimension(std::size_t dim) const override;
	bool isIndexValid(const PG_Index& index) const override;
	PG_Variety getData(PG_Index index, Identity identity) const override;
	void setData(PG_Index index, const PG_Variety& data, Identity identity) override;

private:
	using Row = std::vector<std::string>;

	const PG_DefDefinition::RowFormat* getFormat(std::size_t column) const;
	bool isEditable(std::size_t column) const;
	glm::vec4 getBackground(std::size_t row, std::size_t column) const;
	/// Returns false when value doesn't fit the format, value may be shortened
	bool validate(const PG_DefDefinition::RowFormat& format, std::string& value) const;
	void shiftIndexColumns(std::size_t startRow, long long amount);
	void setModified(bool modified);

	PG_File m_file;
	PG_Def m_def;
	std::vector<std::string> m_headerValues; /// One per header format, empty for ZERO
	std::vector<Row> m_rows;
	std::vector<char> m_trailingData; /// Whatever follows the last row, written back unchanged
	bool m_modified = false;
};
