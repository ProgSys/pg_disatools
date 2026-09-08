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
#include <string>
#include <vector>

class PG_DisaSearchField;
class PG_GuiButton;
class PG_GuiImage;
class PG_GuiPopup;
class PG_GuiPopupInstance;
class PG_GuiRectangle;

/**
* @brief A search field with a funnel button next to it, dropping a panel of file type toggles.
* @details The two sit side by side and are rounded only on their outer edges, so they read as one
* control. Any number of types can be on at once and the panel stays open until the user clicks past it.
* Nothing toggled means nothing filtered, which is what it starts out as.
*/
class PG_DisaFilterSearchField : public PG_GuiItem {
	PG_OBJECT(PG_DisaFilterSearchField);
	PG_PARENT(PG_GuiItem);
	PG_PROPERTY(std::string text GET getText SET setText CHANGED textChanged);
	PG_PROPERTY(std::vector<std::string> fileTypes GET getFileTypes SET setFileTypes CHANGED fileTypesChanged);
signals:
	void textChanged();
	void fileTypesChanged();
public:
	using PG_GuiItem::PG_GuiItem;

	void init() override;

	const std::string& getText() const;
	void setText(const std::string& text);

	/// The extensions currently toggled on, upper case and without the dot
	inline const std::vector<std::string>& getFileTypes() const { return m_fileTypes; }
	void setFileTypes(std::vector<std::string> types);

private:
	void buildPopup(PG_GuiPopupInstance* instance);
	void updateFilterIcon();
	/// Squares off the two bottom corners while the panel hangs underneath them
	void updateCorners(bool open);
	void setTypeSelected(const std::string& type, bool selected);

	std::vector<std::string> m_fileTypes;
	PG_DisaSearchField* m_searchField = nullptr;
	PG_GuiButton* m_filterButton = nullptr;
	PG_GuiRectangle* m_buttonBackground = nullptr;
	PG_GuiImage* m_filterIcon = nullptr;
	PG_GuiPopup* m_popup = nullptr;
};
