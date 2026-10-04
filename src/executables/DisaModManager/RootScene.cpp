/*
 * GNU Lesser General Public License (LGPL-3.0)
 *
 * Copyright (c) 2025 ProgSys
 *
 * This code is part of a free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 * This code is distributed in the hope that it will be useful,
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

#include <Base/PG_Application.h>
#include <Base/Events/PG_KeyEvent.h>
#include <Resources/PG_Shader.h>

#include <Scene/Gui/PG_GuiMain.h>
#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Gui/PG_GuiRow.h>
#include <Scene/Gui/PG_GuiTooltip.h>
#include <Scene/Gui/PG_GuiButton.h>
#include <Scene/Gui/PG_GuiToggleButton.h>
#include <Scene/Gui/PG_GuiListView.h>
#include <Scene/Gui/PG_GuiScrollBar.h>
#include <Scene/Gui/PG_GuiImage.h>
#include <Scene/Gui/PG_GuiText.h>


#include <Disa/utils/PG_DisaInstaller.h>
#include <Disa/utils/PG_FileDialog.h>
#include <Disa/customGui/PG_PupUpDialog.h>
#include <Models/PG_DisaModManagerMainModel.h>
#include <CustomGui/PG_DisaModListItem.h>
#include <Util/Color/ColorDefined.h>

constexpr glm::vec3 backgroundColor = glm::vec3(40.f / 255.f);
constexpr glm::vec3 backgroundDecoColor = glm::vec3(45.f / 255.f);
constexpr glm::vec3 backgroundDeco2Color = glm::vec3(30.f / 255.f);
constexpr glm::vec3 backgroundDarkColor = glm::vec3(20.f / 255.f);
constexpr glm::vec3 offWeightColor = glm::vec3(200.f / 255.f);
constexpr glm::vec3 blueColor = glm::vec3(133.f, 208.f, 255.f) / 255.f;
constexpr glm::vec3 bobbleTextColor = glm::vec3(76.f, 111.f, 199.f) / 255.f;

void RootScene::init() {
	PG_Scene::init();

	m_mainModel = PG_new<PG_DisaModManagerMainModel>("PG_DisaModManagerMainModel", this);

	auto guiMain = PG_new<PG_GuiMain>("guiMain", getRoot());
	rootItem = PG_new<PG_GuiItem>("rootItem", glm::vec2(0.f), glm::vec2(300.f, 400.f), guiMain);

	{//map editor toolbar

		toolbar = PG_new<PG_GuiRectangle>("toolbar", glm::vec2(0.f), glm::vec2(10.f, 50.f), rootItem);
		toolbar->setColor(backgroundColor);
		toolbar->getAnchor(PG_Anchor::Top)->setAnchor(rootItem->getAnchor(PG_Anchor::Top));
		toolbar->getAnchor(PG_Anchor::Left)->setAnchor(rootItem->getAnchor(PG_Anchor::Left));
		toolbar->getAnchor(PG_Anchor::Right)->setAnchor(rootItem->getAnchor(PG_Anchor::Right));

		toolbarRow = PG_new<PG_GuiRow>("ToolbarRow", glm::vec2(0.f, 10.f), glm::vec2(39.f, 30.f), toolbar);
		toolbarRow->setSpacing(3.f);
		toolbarRow->setMargin(PG_Anchor::Left, 10.f);
		toolbarRow->setMargin(PG_Anchor::Right, 10.f);
		toolbarRow->fill(toolbar);
	}

	{ //default center
		PG_GuiItem* defaultWrapper = PG_new<PG_GuiItem>("defaultWrapper", rootItem);
		PG_GuiItem::anchor(defaultWrapper, PG_Anchor::Top, toolbar, PG_Anchor::Bottom);
		PG_GuiItem::anchor(defaultWrapper, PG_Anchor::Left, rootItem, PG_Anchor::Left);
		PG_GuiItem::anchor(defaultWrapper, PG_Anchor::Right, rootItem, PG_Anchor::Right);
		PG_GuiItem::anchor(defaultWrapper, PG_Anchor::Bottom, rootItem, PG_Anchor::Bottom);
		defaultWrapper->setClip(true);

		center = defaultWrapper;
	}

	{ //tooltip
		guiMain->setTooltipItem(PG_new<PG_GuiTooltip>("Tooltip", rootItem));
	}

	setupToolbar();
	setupCenter();

	postCallEvent(&RootScene::openInit, m_mainModel->getConfig().getDisaExeVersion(), PG_File(m_mainModel->getConfig().getDisaExePath()));
}

void RootScene::setupToolbar() {
	toolbarRow->clearGuiChildren();

	auto importButton = PG_new<PG_GuiButton>("importButton", glm::vec2(0.0f), glm::vec2(39.f, 30.f), toolbarRow);
	importButton->setIcon("gui/icon_export.png");
	importButton->setBackground("M_GuiRoundedRectangleAll5");
	importButton->setTooltipText("Import zip");
	importButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 10.f);

	connect(importButton, &PG_GuiButton::clicked, this, [this] {
		PG_File file = PG_FileDialog::open(this, "zip");
		if (file.isEmpty()) return;
		std::string error = m_mainModel->importZip(file);
		if (error.empty()) return;

		auto* popup = PG_PopUpDialog::createError(rootItem, "Import failed", "Failed to import mod!\nReason: " + error);
		popup->open();
	});

	auto modificationsButton = PG_new<PG_GuiToggleButton>("ModificationsButton", glm::vec2(3.f, 0.f), glm::vec2(39.f, 30.f), toolbarRow);
	modificationsButton->setToggled(false);
	modificationsButton->setIcon("gui/icon_actor.png");
	modificationsButton->setBackground("M_GuiToolButtonStart");
	modificationsButton->setTooltipText("Modifications");
	modificationsButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 1.f);
	modificationsButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Flow, PG_Anchor::HorizontalCenter);

	connect(modificationsButton, &PG_GuiButton::clicked, m_mainModel, [m = m_mainModel]() {
		m->setUiState(PG_DisaModManagerMainModel::CustomModifications);
		});
	connect(m_mainModel, &PG_DisaModManagerMainModel::uiStateChanged, modificationsButton, [a = modificationsButton](PG_DisaModManagerMainModel::EUiState state) {
		a->setToggled(state == PG_DisaModManagerMainModel::CustomModifications);
		});

	auto mapsButton = PG_new<PG_GuiToggleButton>("MapModsButton", glm::vec2(3.f, 0.f), glm::vec2(39.f, 30.f), toolbarRow);
	mapsButton->setToggled(false);
	mapsButton->setIcon("gui/disa/icon_disaTerrain.png");
	mapsButton->setBackground("M_GuiToolButtonEnd");
	mapsButton->setTooltipText("Campaigns");
	mapsButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 1.f);
	mapsButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Flow, PG_Anchor::HorizontalCenter);

	connect(mapsButton, &PG_GuiButton::clicked, m_mainModel, [m = m_mainModel]() {
		m->setUiState(PG_DisaModManagerMainModel::CustomCampaigns);
		});
	connect(m_mainModel, &PG_DisaModManagerMainModel::uiStateChanged, mapsButton, [a = mapsButton](PG_DisaModManagerMainModel::EUiState state) {
		a->setToggled(state == PG_DisaModManagerMainModel::CustomCampaigns);
		});
}

void RootScene::setupCenter() {
	auto listView = PG_new<PG_GuiListView>("PG_GuiListView", glm::vec2(3.f, 0.f), glm::vec2(39.f, 30.f), center);
	listView->fill(center);

	listView->setItem(PG_GuiListView::getDefaultCreateFunc<PG_DisaModListItem>());
	listView->setItemUpdate(PG_GuiListView::getDefaultUpdateFunc<PG_DisaModListItem>());
	listView->setCellSize(150.f);
	listView->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
	listView->setSpacing(5.f);
	listView->setMargin(7.f);

	PG_GuiScrollBar* scrollY = PG_new<PG_GuiScrollBar>("PG_GuiScrollBarY", center);
	scrollY->setSize(glm::vec2(10.0f, 0.0f));
	scrollY->setScrollBarColors({
		PG_Color::hentaiBlueFrgb * 0.5f, PG_Color::hentaiBlueFrgb * 0.7f, PG_Color::hentaiBlueFrgb
		});
	if (auto rec = dynamic_cast<PG_GuiRectangle*>(scrollY->getHandle())) {
		rec->setMaterial("M_GuiRoundedRectangleAll5");
	}
	PG_GuiItem::anchor(scrollY, PG_Anchor::Top, center, PG_Anchor::Top);
	PG_GuiItem::anchor(scrollY, PG_Anchor::Bottom, center, PG_Anchor::Bottom);
	PG_GuiItem::anchor(scrollY, PG_Anchor::Right, center, PG_Anchor::Right);
	scrollY->setMargin(PG_Anchor::Top, 3.f);
	scrollY->setMargin(PG_Anchor::Bottom, 3.f);
	scrollY->setMargin(PG_Anchor::Right, 3.f);
	listView->setupVerticalScrollBar(scrollY);
	connect(scrollY, &PG_GuiMouseArea::enabledChanged, listView, [listView](bool visible) {
		listView->setMargin(PG_Anchor::Right, visible? 17.f: 7.f);
		});

	auto* tutorialArrow = PG_new<PG_GuiImage>("tutorialArrow", glm::vec2(25.f, 0.f), glm::vec2(120.f, 84.f), center);
	PG_GuiItem::anchor(tutorialArrow, PG_Anchor::Top, center, PG_Anchor::Top);
	PG_GuiItem::anchor(tutorialArrow, PG_Anchor::Left, center, PG_Anchor::Left);
	tutorialArrow->setSource("gui/disa/pg_tutorial_arrow.png");

	auto* tutorialText = PG_new<PG_GuiText>("tutorialText", center);
	PG_GuiItem::anchor(tutorialText, PG_Anchor::Bottom, tutorialArrow, PG_Anchor::Bottom);
	PG_GuiItem::anchor(tutorialText, PG_Anchor::Left, tutorialArrow, PG_Anchor::Right);
	tutorialText->setText("Add new mods here!");
	tutorialText->setColor(PG_Color::hentaiBlueFrgb);
	tutorialText->setFontSize(20.f);
	tutorialText->setHeight(20.f);
	tutorialText->setWrapMode(PG_GuiText::None);

	connect(m_mainModel, &PG_DisaModManagerMainModel::centerModelChanged, listView, &PG_GuiListView::setModel);
	connect(m_mainModel, &PG_DisaModManagerMainModel::centerModelChanged, this, [this](PG_ListModelBase* model) {
		if (!model) return;
		auto* arrow = center->findFirstChild<PG_GuiItem>("tutorialArrow");
		auto* tutorialText = center->findFirstChild<PG_GuiItem>("tutorialText");
		if (!arrow || !tutorialText) return;
		auto updateVisibility = [arrow, tutorialText, model] {
			arrow->setVisible(model->empty());
			tutorialText->setVisible(model->empty());
		};
		connect(model, &PG_ListModelBase::sizeChanged, arrow, updateVisibility);
		updateVisibility();
	});
	m_mainModel->setUiState(PG_DisaModManagerMainModel::CustomModifications);
}

bool RootScene::event(PG_Event* e) {
	switch (e->getType())
	{
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

void RootScene::openInit(std::string version, PG_File exePath) {
	PG_DisaInstaller* install = PG_new<PG_DisaInstaller>(rootItem);
	connect(install, &PG_DisaInstaller::done, this, [this](std::string newVersion, const PG_File& newExeFile) {
		m_mainModel->getConfig().setExePath(newVersion, newExeFile);
	});

	install->runInstall(version, exePath);
}
