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

#include <Scene/Gui/PG_GuiRectangle.h>
#include <string>

class PG_DisaPreviewItem;
class PG_GuiButton;
class PG_GuiColumn;
class PG_GuiImage;
class PG_GuiText;
class PG_GuiTooltipArea;
class PG_ListModelLocation;
class PG_UiStateBaseModel;
struct PG_DisaExtraField;

/**
* @brief The right hand panel of the file manager, describing whatever is selected in the table.
* @details Wide it shows the file name, a framed preview, the four file actions and a block of file
* info. Once the cell divider leaves it less than a button and its margins wide it drops to a column
* of the icon and those same actions, with the info moved into the icons tooltip.
*/
class PG_DisaFileInspector : public PG_GuiRectangle {
	PG_OBJECT(PG_DisaFileInspector);
	PG_PARENT(PG_GuiRectangle);
public:
	using PG_GuiRectangle::PG_GuiRectangle;

	void init() override;

	/**
	* @brief Points the panel at the archive that is currently open
	* @param model May be null, the panel then shows nothing and offers nothing
	*/
	void setModel(PG_UiStateBaseModel* model);

private:
	enum EMode { Wide, Narrow };

	/// Picks the mode for the given width and posts a rebuild when it changed
	void updateMode(float width);
	/// Drops the current tree and grows the one m_mode asks for
	void rebuild();
	void buildWide();
	void buildNarrow();
	void buildButtons(PG_GuiItem* parent);
	/// One of the group's buttons, styled and wired to rescale its icon with itself
	PG_GuiButton* makeGroupButton(PG_GuiItem* parent, const std::string& name, const std::string& iconPath,
		const std::string& tooltip);
	/// Squares every item of the button group to the given side and scales the icons along with them
	void setButtonSide(float side);
	void bindModel();
	/// Fills the action slot with what the selected file offers, or with the stand in
	void updateAction();
	void updateSelection();
	/// Grows a row per value the selected file's container describes, or leaves the block empty
	void updateExtraData();
	/// One row of the extra data block, the widget picked by what the field asks for
	void buildExtraField(const PG_ListModelLocation& location, const PG_DisaExtraField& field);
	/// The header name only, for after an edit the container answered with a rename
	void updateName();

	std::string buildInfoText() const;
	/// The extra data as 'title: value' lines, what the narrow layout shows instead of the rows
	std::string buildExtraDataText() const;
	/// The text the file table would show in that column of the selected row
	std::string columnText(int column) const;

	EMode m_mode = Wide;
	PG_UiStateBaseModel* m_stateModel = nullptr;
	PG_Object* m_model = nullptr; /// the same model as m_stateModel, for property and function access

	PG_GuiItem* m_root = nullptr;
	PG_GuiImage* m_fileIcon = nullptr; /// follows the selected file's extension, both layouts have one
	PG_GuiText* m_name = nullptr;
	PG_GuiText* m_infoBody = nullptr;
	PG_DisaPreviewItem* m_preview = nullptr;
	PG_GuiTooltipArea* m_iconArea = nullptr;
	/// Holds a row per extra data value, and measures to nothing for a file that has none
	PG_GuiColumn* m_extraDataBlock = nullptr;
	/// Heads the button group and holds either an action button or the stand in, @see updateAction
	PG_GuiItem* m_actionSlot = nullptr;
	PG_GuiButton* m_addButton = nullptr;
	PG_GuiButton* m_replaceButton = nullptr;
	PG_GuiButton* m_exportButton = nullptr;
	PG_GuiButton* m_deleteButton = nullptr;
};
