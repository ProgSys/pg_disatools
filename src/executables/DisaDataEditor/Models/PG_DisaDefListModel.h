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
#include <string>
#include <vector>

/**
* @brief Read only table of all available .DEF files, shown while no data file is open.
* @details The descriptions come from descriptions.txt in the definition folder, one "NAME:Description" per line.
*/
class PG_DisaDefListModel : public PG_ListModelBase {
	PG_OBJECT(PG_DisaDefListModel);
	PG_PARENT(PG_ListModelBase);
public:
	using PG_ListModelBase::PG_ListModelBase;

	/// Reads the definition folder again
	void reload();

	std::size_t getColumnCount() const { return 3; }
	std::string getColumnName(std::size_t column) const;
	float getColumnWidth(std::size_t column) const;

	bool empty() const override { return m_rows.empty(); }
	std::size_t dimension() const override { return 2; }
	std::size_t size() const override { return m_rows.size(); }
	std::size_t sizeOfDimension(std::size_t dim) const override;
	bool isIndexValid(const PG_Index& index) const override;
	PG_Variety getData(PG_Index index, Identity identity) const override;

private:
	struct Row {
		std::string fileName;
		std::string description;
		std::string startName;
	};
	std::vector<Row> m_rows;
};
