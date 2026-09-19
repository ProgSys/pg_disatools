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

#include "RootScene.h"

#include <Base/Events/PG_KeyEvent.h>
#include <Base/Events/PG_TickEvent.h>
#include <Renderer/PG_SceneRenderer.h>
#include <Resources/PG_Shader.h>
#include <Scene/Gui/PG_GuiItem.h>
#include <Scene/Gui/PG_GuiMain.h>
#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Gui/PG_GuiRenderItem.h>
#include <Scene/Gui/PG_GuiRow.h>
#include <Scene/Gui/PG_GuiTooltip.h>
#include <Scene/Gui/PG_GuiSpinBox.h>
#include <Scene/Gui/PG_GuiButton.h>
#include <Scene/Gui/PG_GuiTextField.h>
#include <Scene/Gui/PG_GuiToggleButton.h>
#include <Scene/PG_CameraNode.h>

#include <Disa/PG_DisaMapScene.h>
#include <Disa/map/PG_DisaActorListModel.h>
#include <Scene/PG_FootlooseViewNode.h>
#include <Models/PG_DisaScriptTestModel.h>
#include <Script/PG_DisaContext.h>
#include <Script/PG_DisaScriptExecutor.h>

void RootScene::init() {
	PG_Scene::init();
	m_model = PG_new<PG_DisaScriptTestModel>("PG_DisaScriptTestModel", this);
	m_model->openGameFiles();

	auto guiMain = PG_new<PG_GuiMain>("guiMain", getRoot());
	m_rootItem = PG_new<PG_GuiItem>("rootItem", glm::vec2(0.f), glm::vec2(300.f, 400.f), guiMain);
	{
		auto cam = PG_new<PG_CameraNode>("N_CameraNode", PG_CameraNode::Type_Orthographic, getRoot());
		cam->setTranslation(glm::vec3(-5.0f, 0.0f, 0.0f));
		cam->setZoom(120.0f);
		setActiveCamera(cam);
	}

	setupToolbar();
	guiMain->setTooltipItem(PG_new<PG_GuiTooltip>("Tooltip", m_rootItem));
	setupMapView();
}

bool RootScene::event(PG_Event* e) {
	switch (e->getType())
	{
		case PG_Event_Tick: {
			const double seconds = static_cast<PG_TickEvent*>(e)->getDeltaTimeSeconds();
			/* The animations run first, so a lane that waits for one sees the tick it just had
			 * rather than the one before. */
			m_model->getContext()->tick(static_cast<float>(seconds) * PG_DisaScriptExecutor::ticksPerSecond);
			m_model->getScriptExecutor()->tick(seconds);
		}break;
		case PG_Event_Key: {
			auto key = static_cast<PG_KeyEvent*>(e);
			if (key->getKey() == PG_Key_F5 && !key->isRepeat() && key->isUp()) {
				PG_Shader::reloadAll();
				key->accept();
				return true;
			}
		}break;
	}

	return PG_Scene::event(e);
}

void RootScene::setupToolbar() {
	constexpr glm::vec3 backgroundColor = glm::vec3(40.f / 255.f);

	m_toolbar = PG_new<PG_GuiRectangle>("toolbar", glm::vec2(0.f), glm::vec2(10.f, 50.f), m_rootItem);
	m_toolbar->setColor(backgroundColor);
	PG_GuiItem::anchor(m_toolbar, PG_Anchor::Top, m_rootItem, PG_Anchor::Top);
	PG_GuiItem::anchor(m_toolbar, PG_Anchor::Left, m_rootItem, PG_Anchor::Left);
	PG_GuiItem::anchor(m_toolbar, PG_Anchor::Right, m_rootItem, PG_Anchor::Right);

	m_toolbarRow = PG_new<PG_GuiRow>("ToolbarRow", glm::vec2(0.f, 10.f), glm::vec2(39.f, 30.f), m_toolbar);
	m_toolbarRow->setMargin(PG_Anchor::Left, 10.f);
	m_toolbarRow->setMargin(PG_Anchor::Right, 10.f);
	m_toolbarRow->setSpacing(0.f);
	m_toolbarRow->fill(m_toolbar);

	constexpr glm::vec2 spinBoxSize(126.f, 30.f);
	{
		auto* spinBox = PG_GuiSpinBox::createInt("spinBox", glm::vec2(0.f), spinBoxSize, 0, 99999, m_toolbarRow);
		spinBox->getBackground()->setMaterial("M_GuiRoundedRectangleLeft5");

		auto btn = PG_new<PG_GuiButton>("openMap", glm::vec2(0.0f), glm::vec2(39.f, 30.f), m_toolbarRow);
		btn->setIcon("gui/icon_play.svg");
		btn->setBackground("M_GuiRoundedRectangleRight5");
		btn->setTooltipText("Open Map");
		btn->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 25.f);

		connect(btn, &PG_GuiButton::clicked, m_model->getContext(), [model = m_model->getContext(), spinBox] {  model->setDungeonId(static_cast<int>(spinBox->getValue())); });
	}

	{
		auto* text = PG_new<PG_GuiTextField>("talkScriptId", glm::vec2(0.f), glm::vec2(256.f, 30.f), m_toolbarRow);
		text->getBackgroundRectangle()->setMaterial("M_GuiRoundedRectangleLeft5");
		text->setText("disa::1"); //the intro of the game

		auto btn = PG_new<PG_GuiButton>("execTalkScript", glm::vec2(0.0f), glm::vec2(39.f, 30.f), m_toolbarRow);
		btn->setIcon("gui/icon_play.svg");
		btn->setBackground("M_GuiRoundedRectangleRight5");
		btn->setTooltipText("Run talk script");
		btn->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 25.f);

		connect(btn, &PG_GuiButton::clicked, m_model, [model = m_model, text] { model->runTalkScript(text->getText()); });
	}

	{
		auto* text = PG_new<PG_GuiTextField>("scriptId", glm::vec2(0.f), spinBoxSize, m_toolbarRow);
		text->getBackgroundRectangle()->setMaterial("M_GuiRoundedRectangleLeft5");

		auto btn = PG_new<PG_GuiButton>("execScript", glm::vec2(0.0f), glm::vec2(39.f, 30.f), m_toolbarRow);
		btn->setIcon("gui/icon_play.svg");
		btn->setBackground("M_GuiRoundedRectangleRight5");
		btn->setTooltipText("Run Script");
		btn->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 25.f);

		connect(btn, &PG_GuiButton::clicked, m_model, [model = m_model, text] { model->runScript(text->getText()); });
	}

	{
		auto btn = PG_new<PG_GuiToggleButton>("swapCamera", glm::vec2(0.0f), glm::vec2(39.f, 30.f), m_toolbarRow);
		btn->setIcon("gui/icon_repeat.svg");
		btn->setTooltipText("Swap camera");
		btn->setBackground("M_GuiRoundedRectangleAll5");
		btn->setLayoutProperty(PG_GuiItem::LayoutProperty_Flow, PG_Anchor::Right);

		connect(btn, &PG_GuiToggleButton::toggleChanged, m_model, [model = m_model] (bool toggled) { model->useGameCamera(!toggled); });
	}
}

void RootScene::setupMapView() {
	auto item = PG_new<PG_GuiRenderItem>("MapRenderItem", m_rootItem);
	auto sceneRenderer = PG_new<PG_SceneRenderer>("GameSceneRenderer", true, 2, item);
	auto gameScene = PG_new<PG_DisaMapScene>("GameScene", sceneRenderer);

	/* The map model outlives every map it shows, so the scene is bound once here. Opening another
	 * map refills the same model. */
	gameScene->setMapModel(m_model->getMapModel());
	m_model->setScene(gameScene);

	PG_GuiItem::anchor(item, PG_Anchor::Top, m_toolbar, PG_Anchor::Bottom);
	PG_GuiItem::anchor(item, PG_Anchor::Left, m_rootItem, PG_Anchor::Left);
	PG_GuiItem::anchor(item, PG_Anchor::Right, m_rootItem, PG_Anchor::Right);
	PG_GuiItem::anchor(item, PG_Anchor::Bottom, m_rootItem, PG_Anchor::Bottom);
}

