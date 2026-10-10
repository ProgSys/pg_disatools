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

class PG_MouseButtonEvent;

class PG_DisaDataTableModel;
class PG_DisaDefListModel;
class PG_GuiItem;
class PG_GuiRectangle;
class PG_GuiRow;
class PG_GuiButton;

/**
* @brief Edits Disgaea .DAT tables, their layout is described by the .DEF files in the resources.
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

private:
	void setupToolbar();
	void rebuildTable();
	void openRowMenu(PG_GuiItem* owner, std::size_t row, const PG_MouseButtonEvent* e);
	void showError(const std::string& title, const std::string& text);

	PG_DisaDataTableModel* m_model = nullptr;
	PG_DisaDefListModel* m_defListModel = nullptr;
	PG_GuiItem* m_rootItem = nullptr;
	PG_GuiRectangle* m_toolbar = nullptr;
	PG_GuiRow* m_toolbarRow = nullptr;
	PG_GuiItem* m_center = nullptr;
	PG_GuiItem* m_tableContent = nullptr;
};
