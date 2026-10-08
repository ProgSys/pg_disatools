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

#include <Scene/Gui/PG_GuiItem.h>
#include <Scene/Model/PG_ListModelLocation.h>

class PG_GuiText;
class PG_GuiTextEdit;
class PG_GuiRectangle;

/// A column title of the data table
class PG_DisaDataHeaderItem : public PG_GuiItem {
	PG_OBJECT(PG_DisaDataHeaderItem);
	PG_PARENT(PG_GuiItem);
public:
	using PG_GuiItem::PG_GuiItem;
	void init() override;

	void setText(const std::string& text);
private:
	PG_GuiText* m_text = nullptr;
};

/**
* @brief One editable cell of the data table.
* @details The edit is committed to the model when it loses focus or Return is pressed, Escape reverts it.
*/
class PG_DisaDataCellItem : public PG_GuiItem {
	PG_OBJECT(PG_DisaDataCellItem);
	PG_PARENT(PG_GuiItem);
public:
	using PG_GuiItem::PG_GuiItem;
	void init() override;
	bool filterEvent(PG_Event* e) override;

	const PG_ListModelLocation& getLocation() const { return m_location; }
	void setLocation(const PG_ListModelLocation& location);
	void update(const PG_ListModelLocation& location, const PG_IdentitySet& identities);
private:
	void updateText();
	void updateBackground();
	void commit();

	PG_ListModelLocation m_location;
	PG_GuiRectangle* m_background = nullptr;
	PG_GuiTextEdit* m_text = nullptr;
	bool m_editable = false;
};
