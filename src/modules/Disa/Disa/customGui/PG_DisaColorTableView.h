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
#include <array>

class PG_DisaColorTableModel;
class PG_GuiButton;
class PG_GuiGridView;
class PG_GuiMouseArea;
class PG_GuiRectangle;
class PG_GuiRow;
class PG_GuiText;
class PG_GuiColorPicker;
class PG_Timer;

/// One color of a PG_DisaColorTableView
class PG_DisaColorSwatchItem : public PG_GuiItem {
	PG_OBJECT(PG_DisaColorSwatchItem);
	PG_PARENT(PG_GuiItem);
public:
	using PG_GuiItem::PG_GuiItem;
	void init() override;

	const PG_ListModelLocation& getLocation() const { return m_location; }
	void setLocation(const PG_ListModelLocation& location);
	void update(const PG_ListModelLocation& location, const PG_IdentitySet& identities);
	void setSelected(bool selected);

private:
	void updateColor();

	PG_ListModelLocation m_location;
	PG_GuiRectangle* m_color = nullptr;
	PG_GuiItem* m_selection = nullptr; /// Frame around the selected tile
	std::array<PG_GuiRectangle*, 4> m_selectionSides{};
	PG_GuiText* m_indexText = nullptr;
	PG_GuiMouseArea* m_area = nullptr;
};

/**
* @brief Shows and edits the color tables of a PG_DisaColorTableModel.
* @details The toolbar steps through the tables, adds, copies, removes, imports or exports them, and edits the
* clicked color with a color picker. Below it is a grid of all colors of the current table, a double click opens
* the full color dialog.
*/
class PG_DisaColorTableView : public PG_GuiItem {
	PG_OBJECT(PG_DisaColorTableView);
	PG_PARENT(PG_GuiItem);
signals:
	void selectedChanged(int index);
public:
	using PG_GuiItem::PG_GuiItem;
	void init() override;

	inline PG_DisaColorTableModel* getModel() const { return m_model; }
	void setModel(PG_DisaColorTableModel* model);

	/// -1 when no color is selected
	inline int getSelected() const { return m_selected; }
	void setSelected(int index);
	/// Selects the color and opens the full color dialog for it
	void editColor(int index);

private:
	void updateControls();
	/// The full color dialog for the selected color, previewed once a change rests for 100ms, canceling restores the color
	void openColorDialog();
	/// Writes the color the toolbar picker holds back, right away instead of after the delay
	void applyPendingColor();

	PG_DisaColorTableModel* m_model = nullptr;
	int m_selected = -1;

	PG_GuiText* m_tableText = nullptr;
	PG_GuiButton* m_prevButton = nullptr;
	PG_GuiButton* m_nextButton = nullptr;
	PG_GuiButton* m_addButton = nullptr;
	PG_GuiButton* m_copyButton = nullptr;
	PG_GuiButton* m_removeButton = nullptr;
	PG_GuiButton* m_importButton = nullptr;
	PG_GuiButton* m_exportButton = nullptr;
	PG_GuiGridView* m_grid = nullptr;
	PG_GuiText* m_selectedText = nullptr;
	PG_GuiColorPicker* m_picker = nullptr;
	bool m_applyingPickerColor = false; /// True while a change of the picker is written to the model
	PG_Timer* m_pickerTimer = nullptr;
	int m_pendingIndex = -1; /// Color waiting for the timer, -1 when there is none
	glm::vec4 m_pendingColor{ 0.f };
};
