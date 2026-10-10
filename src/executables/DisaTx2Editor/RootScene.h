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

#include <Scene/PG_Scene.h>
#include <Util/PG_File.h>
#include <glm/glm.hpp>
#include <optional>

class PG_DisaTx2EditorModel;
class PG_DisaColorTableView;
class PG_GuiItem;
class PG_GuiImage;
class PG_GuiMouseArea;
class PG_GuiRectangle;
class PG_GuiCell;
class PG_GuiRow;
class PG_GuiText;
class PG_GuiToggleButton;

/**
* @brief Views and edits Disgaea TX2 images.
* @details Mouse wheel zooms, the middle mouse button pans. The view keeps the image fitted until it is moved.
*/
class RootScene : public PG_Scene {
	PG_OBJECT(RootScene);
	PG_PARENT(PG_Scene);
signals:
	void titleChanged(std::string title);
public:
	using PG_Scene::PG_Scene;

	void init() override;
	bool event(PG_Event* e) override;

	/// Opens the file and shows an error popup when it fails
	bool open(const PG_File& file);
	std::string getTitle() const;
	/// Scales and centers the image so all of it is in view
	void focusImage();

private:
	void setupToolbar();
	void setupView();
	void setupColorTable();
	void updateInfo();
	/// The image pixel under a window position, std::nullopt outside of the image
	std::optional<glm::ivec2> pixelAt(const glm::vec2& windowPosition) const;
	/// Moves the coordinate label to the cursor, hidden while the info is off or the cursor is not on the image
	void updateCursorInfo(const glm::vec2& windowPosition);
	void openConvertDialog();
	void convertTo(int targetIndex);
	void showError(const std::string& title, const std::string& text);

	PG_DisaTx2EditorModel* m_model = nullptr;
	PG_GuiItem* m_rootItem = nullptr;
	PG_GuiRectangle* m_toolbar = nullptr;
	PG_GuiRow* m_toolbarRow = nullptr;
	PG_GuiItem* m_viewArea = nullptr;
	PG_GuiMouseArea* m_navigationArea = nullptr;
	PG_GuiItem* m_imageScroll = nullptr;
	PG_GuiImage* m_image = nullptr;
	PG_GuiItem* m_originMarker = nullptr; /// Two lines through the top left corner of the image, shown with the info
	PG_GuiRectangle* m_infoOverlay = nullptr;
	PG_GuiText* m_infoText = nullptr;
	PG_GuiRectangle* m_cursorInfo = nullptr;
	PG_GuiText* m_cursorInfoText = nullptr;
	PG_GuiToggleButton* m_infoButton = nullptr;
	PG_GuiToggleButton* m_colorTableButton = nullptr;
	PG_GuiCell* m_cell = nullptr;
	PG_GuiItem* m_hiddenItems = nullptr; /// Holds the color table while it is not part of the cell
	PG_GuiItem* m_colorTablePanel = nullptr;
	float m_colorTableHeight = 260.f;
	PG_DisaColorTableView* m_colorTableView = nullptr;
	bool m_keepFocused = true; /// The image is refitted when the view resizes, until it is zoomed or panned
};
