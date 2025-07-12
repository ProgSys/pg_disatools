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
#include <Scene/Gui/PG_GuiTableView.h>
#include <Scene/Gui/PG_GuiTableHeaderView.h>
#include <Scene/Gui/PG_GuiFootlooseView.h>
#include <Scene/Gui/PG_GuiCell.h>
#include <Scene/Model/PG_ListSelectionWrapperModel.h>


#include <Disa/utils/PG_DisaInstaller.h>
#include <Disa/utils/PG_FileDialog.h>
#include <Disa/customGui/PG_PupUpDialog.h>
#include <Disa/CustomGui/PG_DisaSearchField.h>
#include <Disa/CustomGui/PG_DisaPreviewItem.h>
#include <Util/Color/ColorDefined.h>

#include <Models/PG_DisaFileManagerMainModel.h>
#include <CustomGui/PG_DisaFileTableItem.h>

constexpr glm::vec3 backgroundColor = glm::vec3(40.f / 255.f);
constexpr glm::vec3 backgroundDecoColor = glm::vec3(45.f / 255.f);
constexpr glm::vec3 backgroundDeco2Color = glm::vec3(30.f / 255.f);
constexpr glm::vec3 backgroundDarkColor = glm::vec3(20.f / 255.f);
constexpr glm::vec3 offWeightColor = glm::vec3(200.f / 255.f);
constexpr glm::vec3 blueColor = glm::vec3(133.f, 208.f, 255.f) / 255.f;
constexpr glm::vec3 bobbleTextColor = glm::vec3(76.f, 111.f, 199.f) / 255.f;

void RootScene::init() {
	PG_Scene::init();

	m_mainModel = PG_new<PG_DisaFileManagerMainModel>("PG_DisaFileManagerMainModel", this);

	auto guiMain = PG_new<PG_GuiMain>("guiMain", getRoot());
	rootItem = PG_new<PG_GuiItem>("rootItem", glm::vec2(0.f), glm::vec2(300.f, 400.f), guiMain);

	{//map editor toolbar

		toolbar = PG_new<PG_GuiRectangle>("toolbar", glm::vec2(0.f), glm::vec2(10.f, 50.f), rootItem);
		toolbar->setColor(backgroundColor);
		toolbar->getAnchor(PG_Anchor::Top)->setAnchor(rootItem->getAnchor(PG_Anchor::Top));
		toolbar->getAnchor(PG_Anchor::Left)->setAnchor(rootItem->getAnchor(PG_Anchor::Left));
		toolbar->getAnchor(PG_Anchor::Right)->setAnchor(rootItem->getAnchor(PG_Anchor::Right));

		toolbarRow = PG_new<PG_GuiRow>("toolbarRow", glm::vec2(0.f, 10.f), glm::vec2(39.f, 30.f), toolbar);
		toolbarRow->setSpacing(3.f);
		toolbarRow->setMargin(PG_Anchor::Left, 10.f);
		toolbarRow->setMargin(PG_Anchor::Right, 10.f);
		toolbarRow->fill(toolbar);
	}

	{
		navbar = PG_new<PG_GuiRectangle>("navbar", glm::vec2(0.f), glm::vec2(0.f, 30.f), rootItem);
		navbar->setColor(backgroundColor * 1.3f);
		PG_GuiItem::anchor(navbar, PG_Anchor::Top, toolbar, PG_Anchor::Bottom);
		PG_GuiItem::anchor(navbar, PG_Anchor::Left, rootItem, PG_Anchor::Left);
		PG_GuiItem::anchor(navbar, PG_Anchor::Right, rootItem, PG_Anchor::Right);

		navbarRow = PG_new<PG_GuiRow>("navbarRow", glm::vec2(0.f, 3.f), glm::vec2(39.f, 24.f), navbar);
		navbarRow->setSpacing(3.f);
		navbarRow->setMargin(PG_Anchor::Left, 10.f);
		navbarRow->setMargin(PG_Anchor::Right, 10.f);
		navbarRow->fill(navbar);

	}

	{ //default center
		PG_GuiItem* defaultWrapper = PG_new<PG_GuiItem>("defaultWrapper", rootItem);
		PG_GuiItem::anchor(defaultWrapper, PG_Anchor::Top, navbar, PG_Anchor::Bottom);
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
	setupNavbar();
	setupCenter();
}

void RootScene::setupToolbar() {
	toolbarRow->clearGuiChildren();

	auto openButton = PG_new<PG_GuiButton>("openButton", glm::vec2(0.0f), glm::vec2(39.f, 30.f), toolbarRow);
	openButton->setIcon("gui/icon_folder.png");
	openButton->setBackground("M_GuiRoundedRectangleAll5");
	openButton->setTooltipText("Open file");
	openButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 10.f);

	connect(openButton, &PG_GuiButton::clicked, this, [this] {
		PG_File file = PG_FileDialog::open(this, "dat");
		if (file.isEmpty()) return;
		if (m_mainModel->open(file)) return;

		auto* popup = PG_PopUpDialog::createError(rootItem, "Open failed", "Failed to open file!");
		popup->open();
	});


	connect(m_mainModel, &PG_DisaFileManagerMainModel::centerModelChanged, toolbarRow, [this](PG_UiStateBaseModel* oldModel, PG_UiStateBaseModel* newModel) {
		PG_DisaSearchField* searchField = toolbarRow->findFirstChild<PG_DisaSearchField>("searchField");
		if (searchField) delete searchField;
		auto* asObject = dynamic_cast<PG_Object*>(newModel);
		if (!asObject) return;
		auto prop = asObject->getProperty("searchText");
		if (!prop) return;

		searchField = PG_new<PG_DisaSearchField>("searchField", glm::vec2(0.f), glm::vec2(300.f, 30.f), toolbarRow);
		searchField->setLayoutProperty(PG_GuiItem::LayoutProperty_Flow, PG_Anchor::VerticalCenter);
		searchField->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
		searchField->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 10.f);
		searchField->setLayoutProperty(PG_GuiItem::LayoutProperty_MaxSize, glm::vec2(300.f, 30.f));
		searchField->setLayoutProperty(PG_GuiItem::LayoutProperty_MinSize, glm::vec2(80.f, 30.f));
		searchField->setText(asObject->getPropertyValue(prop).toString());

		connect(searchField, searchField->getProperty("text"), asObject, prop);
	});
}

void RootScene::setupNavbar() {
	navbarRow->clearGuiChildren();

	auto backButton = PG_new<PG_GuiButton>("backButton", glm::vec2(0.0f), glm::vec2(39.f, 24.f), navbarRow);
	backButton->setIcon("gui/icon_back.png");
	backButton->setBackground("M_GuiRoundedRectangleAll5");
	backButton->setTooltipText("Back");
	backButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 10.f);

	connect(backButton, &PG_GuiButton::clicked, m_mainModel, &PG_DisaFileManagerMainModel::navBack);

	auto navText = PG_new<PG_GuiText>("navText", glm::vec2(0.0f), glm::vec2(39.f, 24.f), navbarRow);
	navText->setText(m_mainModel->getNavText());
	navText->setColor(PG_Color::hentaiBlueFrgb);
	navText->setFontSize(20.f);
	navText->setWrapMode(PG_GuiText::Elide);
	navText->setVerticalAlignment(PG_Anchor::VerticalCenter);
	navText->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);

	connect(m_mainModel, m_mainModel->getProperty("navText"), navText, navText->getProperty("text"));
}

void RootScene::setupCenter() {

	auto cell = PG_new<PG_GuiCell>("guiCell", glm::vec2(0.0f), glm::vec2(740.f, 700.f), center);
	cell->fill(center);

	auto* content = PG_new<PG_GuiItem>("content", cell);
	cell->addLeft(content, 480.f);

	PG_GuiTableHeaderView* tableHeader = PG_new<PG_GuiTableHeaderView>("PG_GuiTableHeaderView", content);
	PG_GuiItem::anchor(tableHeader, PG_Anchor::Top, content, PG_Anchor::Top);
	PG_GuiItem::anchor(tableHeader, PG_Anchor::Left, content, PG_Anchor::Left);
	PG_GuiItem::anchor(tableHeader, PG_Anchor::Right, content, PG_Anchor::Right);
	tableHeader->setHeight(32.f);

	tableHeader->setItem([](PG_GuiItem* parent, std::size_t columnIndex) {
		auto h = PG_new<PG_DisaFileTableHeaderItem>("HeaderItem" + std::to_string(columnIndex), parent);
		switch (columnIndex) {
		case 0: h->getTextItem()->setText("File"); break;
		case 1: h->getTextItem()->setText("Type"); break;
		case 2: h->getTextItem()->setText("Size"); break;
		case 3: h->getTextItem()->setText("Index"); break;
		}
		return h;
	});

	auto bgList = PG_new<PG_GuiListView>("PG_GuiListView", content);
	bgList->setClip(true);
	bgList->setUserInteractionEnabled(false);
	bgList->setItem(PG_GuiListView::getDefaultCreateFunc<PG_DisaFileTableRowItem>());
	bgList->setItemUpdate(PG_GuiListView::getDefaultUpdateFunc<PG_DisaFileTableRowItem>());


	auto tableView = PG_new<PG_GuiTableView>("PG_GuiTableView", content);
	PG_GuiItem::anchor(tableView, PG_Anchor::Top, tableHeader, PG_Anchor::Bottom);
	PG_GuiItem::anchor(tableView, PG_Anchor::Bottom, content, PG_Anchor::Bottom);
	PG_GuiItem::anchor(tableView, PG_Anchor::Left, content, PG_Anchor::Left);
	PG_GuiItem::anchor(tableView, PG_Anchor::Right, content, PG_Anchor::Right);

	//bgList->fill(tableView);
	PG_GuiItem::anchor(bgList, PG_Anchor::Top, tableHeader, PG_Anchor::Bottom);
	PG_GuiItem::anchor(bgList, PG_Anchor::Bottom, content, PG_Anchor::Bottom);
	PG_GuiItem::anchor(bgList, PG_Anchor::Left, content, PG_Anchor::Left);
	PG_GuiItem::anchor(bgList, PG_Anchor::Right, content, PG_Anchor::Right);


	bgList->setCellSize(tableView->getCellSize().y);
	connect(tableView, &PG_GuiGridView::scrollChanged, bgList, &PG_GuiGridView::setScrollPosition);


	tableView->setColumnWidthModel(tableHeader);
	tableView->setItem(PG_GuiListView::getDefaultCreateFunc<PG_DisaFileTableItem>());
	tableView->setItemUpdate(PG_GuiListView::getDefaultUpdateFunc<PG_DisaFileTableItem>());
	tableView->setClip(true);

	PG_GuiScrollBar* scrollY = PG_new<PG_GuiScrollBar>("PG_GuiScrollBarY", content);
	scrollY->setSize(glm::vec2(10.0f, 0.0f));
	scrollY->setScrollBarColors({
		PG_Color::hentaiBlueFrgb * 0.5f, PG_Color::hentaiBlueFrgb * 0.7f, PG_Color::hentaiBlueFrgb
		});
	if (auto rec = dynamic_cast<PG_GuiRectangle*>(scrollY->getHandle())) {
		rec->setMaterial("M_GuiRoundedRectangleAll5");
	}
	PG_GuiItem::anchor(scrollY, PG_Anchor::Top, content, PG_Anchor::Top);
	PG_GuiItem::anchor(scrollY, PG_Anchor::Bottom, content, PG_Anchor::Bottom);
	PG_GuiItem::anchor(scrollY, PG_Anchor::Right, content, PG_Anchor::Right);
	scrollY->setMargin(PG_Anchor::Top, 3.f);
	scrollY->setMargin(PG_Anchor::Bottom, 3.f);
	scrollY->setMargin(PG_Anchor::Right, 3.f);
	tableView->setupVerticalScrollBar(scrollY);
	tableHeader->moveToBack();

	connect(scrollY, &PG_GuiMouseArea::enabledChanged, tableView, [tableView](bool visible) {
		tableView->setMargin(PG_Anchor::Right, visible? 17.f: 7.f);
		});

	connect(m_mainModel, &PG_DisaFileManagerMainModel::centerModelChanged, this, [this, tableView, bgList] (PG_UiStateBaseModel* oldModel, PG_UiStateBaseModel* newModel){
		if (!newModel) return;
		bgList->setModel(newModel->getListModel());
		tableView->setModel(newModel->getListModel());
		});

	auto* previewContent = PG_new<PG_GuiRectangle>("preview", cell);
	previewContent->setColor(backgroundDarkColor);

	cell->addRight(previewContent, 250.f);

	PG_DisaPreviewItem* preview = PG_new<PG_DisaPreviewItem>("PG_DisaPreviewItem", previewContent);
	preview->setHeight(250.f);
	connect(preview, &PG_DisaPreviewItem::midGuiUpdate, preview, [](glm::vec2* pos, glm::vec2* size) {
		size->y = size->x;
		});

	PG_GuiItem::anchor(preview, PG_Anchor::Top, previewContent, PG_Anchor::Top);
	PG_GuiItem::anchor(preview, PG_Anchor::Left, previewContent, PG_Anchor::Left);
	PG_GuiItem::anchor(preview, PG_Anchor::Right, previewContent, PG_Anchor::Right);
	connect(m_mainModel, &PG_DisaFileManagerMainModel::centerModelChanged, preview, [preview](PG_UiStateBaseModel* oldModel, PG_UiStateBaseModel* newModel) {
		if(oldModel) disconnect(dynamic_cast<PG_Object*>(oldModel), nullptr, preview, nullptr);

		if (!newModel) return;
		auto* asObject = dynamic_cast<PG_Object*>(newModel);
		if (!asObject) return;
		auto prop = asObject->getProperty("lastSelected");
		if (!prop) return;
		preview->openPreview(asObject->getPropertyValue(prop));
		connect(asObject, prop, preview, preview->getProperty("location"));
	});
}

bool RootScene::event(PG_Event* e) {
	switch (e->getType())
	{
		case Event_Key: {
			auto key = static_cast<PG_KeyEvent*>(e);
			if (key->getKey() == PG_F5 && !key->isRepeat() && key->isUp()) {
				PG_Shader::reloadAll();
				key->accept();
				return true;
			}
		}break;
	}

	return PG_Scene::event(e);
}
