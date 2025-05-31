/*
 * GNU Lesser General Public License (LGPL-3.0)
 * 
 * Copyright (c) 2020 ProgSys
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
#include <Scene/Model/PG_ListModelLocation.h>
#include <Disa/utils/PG_DisaConfig.h>

class PG_DisaModManagerMainModel;

class PG_GuiItem;
class PG_GuiRectangle;
class PG_GuiRow;
class PG_GuiButton;
class PG_GuiText;

struct RootGui {
    PG_GuiItem* rootItem = nullptr;
    PG_GuiRectangle* toolbar = nullptr;
    PG_GuiRow* toolbarRow = nullptr;
    PG_GuiButton* openButton = nullptr;

    PG_GuiText* fpsItem = nullptr;
    PG_GuiItem* center = nullptr;
};

class RootScene : public PG_Scene, public RootGui
{
	PG_OBJECT(RootScene);
signals:
    void cleanUp();
public:

	using PG_Scene::PG_Scene;

	void init() override;

	bool event(PG_Event* e) override;

private:
    void setupToolbar();
    void setupCenter();

    void openInit(std::string version, PG_File exePath);
    void openExe(PG_File exePath);


    PG_DisaModManagerMainModel* m_mainModel = nullptr;
};
