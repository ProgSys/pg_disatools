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
#include <Scene/Gui/PG_GuiScrollComponent.h>
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
#include <CustomGui/PG_DisaFilterSearchField.h>
#include <Disa/PG_DisaColorDefines.h>

#include <Models/PG_DisaFileManagerMainModel.h>
#include <CustomGui/PG_DisaFileInspector.h>
#include <CustomGui/PG_DisaFileTableItem.h>

void RootScene::init() {
	PG_Scene::init();

	m_mainModel = PG_new<PG_DisaFileManagerMainModel>("PG_DisaFileManagerMainModel", this);

	auto guiMain = PG_new<PG_GuiMain>("guiMain", getRoot());
	rootItem = PG_new<PG_GuiItem>("rootItem", glm::vec2(0.f), glm::vec2(300.f, 400.f), guiMain);

	{//map editor toolbar

		toolbar = PG_new<PG_GuiRectangle>("toolbar", glm::vec2(0.f), glm::vec2(10.f, 50.f), rootItem);
		toolbar->setColor(PG_Color::blackGrayFrgb);
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
		navbar->setColor(PG_Color::blackGrayFrgb * 1.3f);
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

	{
		auto openButton = PG_new<PG_GuiButton>("openButton", glm::vec2(0.0f), glm::vec2(39.f, 30.f), toolbarRow);
		openButton->setIcon("gui/icon_folder.png");
		openButton->setBackground("M_GuiRoundedRectangleAll5");
		openButton->setTooltipText("Open file");
		openButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 20.f);

		connect(openButton, &PG_GuiButton::clicked, this, [this] {
			PG_File file = PG_FileDialog::open(this, "dat");
			if (file.isEmpty()) return;
			if (m_mainModel->open(file)) return;

			auto* popup = PG_PopUpDialog::createError(rootItem, "Open failed", "Failed to open file!");
			popup->open();
		});
	}

	{
		auto saveButton = PG_new<PG_GuiButton>("saveButton", glm::vec2(0.0f), glm::vec2(39.f, 30.f), toolbarRow);
		saveButton->setIcon("gui/icon_save.png");
		saveButton->setBackground("M_GuiRoundedRectangleAll5");
		saveButton->setTooltipText("Save file");
		saveButton->setEnabled(false);
		saveButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 10.f);
		if (auto prop = m_mainModel->getProperty("isModified")) {
			connect(m_mainModel, prop, saveButton, saveButton->getProperty("enabled"));
			saveButton->setEnabled(m_mainModel->getPropertyValue(prop).toBool());
		}
		connect(saveButton, &PG_GuiButton::clicked, m_mainModel, m_mainModel->getFunction("void save()"));
	}

	{
		auto saveAsButton = PG_new<PG_GuiButton>("saveAsButton", glm::vec2(0.0f), glm::vec2(39.f, 30.f), toolbarRow);
		saveAsButton->setIcon("gui/icon_saveAs.png");
		saveAsButton->setBackground("M_GuiRoundedRectangleAll5");
		saveAsButton->setTooltipText("Save as");
		saveAsButton->setEnabled(false);

		connect(m_mainModel, &PG_DisaFileManagerMainModel::centerModelChanged, saveAsButton, [saveAsButton](PG_UiStateBaseModel* oldModel, PG_UiStateBaseModel* newModel) {
			saveAsButton->setEnabled(newModel != nullptr);
			});

		connect(saveAsButton, &PG_GuiButton::clicked, this, [this] {
			const PG_File originFile = m_mainModel->getOriginFile();
			if (originFile.isEmpty()) return;

			PG_File file = PG_FileDialog::save(this, originFile.getFileExtensionFormated(), originFile.getFile());
			if (file.isEmpty()) return;
			m_mainModel->saveAs(file);
			});
	}


	connect(m_mainModel, &PG_DisaFileManagerMainModel::centerModelChanged, toolbarRow, [this](PG_UiStateBaseModel* oldModel, PG_UiStateBaseModel* newModel) {
		PG_DisaFilterSearchField* searchField = toolbarRow->findFirstChild<PG_DisaFilterSearchField>("searchField");
		if (searchField) delete searchField;
		auto* asObject = dynamic_cast<PG_Object*>(newModel);
		if (!asObject) return;
		auto prop = asObject->getProperty("searchText");
		auto typesProp = asObject->getProperty("fileTypes");
		if (!prop || !typesProp) return;

		searchField = PG_new<PG_DisaFilterSearchField>("searchField", glm::vec2(0.f), glm::vec2(300.f, 30.f), toolbarRow);
		searchField->setLayoutProperty(PG_GuiItem::LayoutProperty_Flow, PG_Anchor::VerticalCenter);
		searchField->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
		searchField->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 10.f);
		searchField->setLayoutProperty(PG_GuiItem::LayoutProperty_MaxSize, glm::vec2(300.f, 30.f));
		searchField->setLayoutProperty(PG_GuiItem::LayoutProperty_MinSize, glm::vec2(80.f, 30.f));
		searchField->setText(asObject->getPropertyValue(prop).toString());

		connect(searchField, searchField->getProperty("text"), asObject, prop);
		connect(searchField, searchField->getProperty("fileTypes"), asObject, typesProp);
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
	content->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
	cell->addLeft(content, 480.f);

	PG_GuiTableHeaderView* tableHeader = PG_new<PG_GuiTableHeaderView>("PG_GuiTableHeaderView", content);
	PG_GuiItem::anchor(tableHeader, PG_Anchor::Top, content, PG_Anchor::Top);
	PG_GuiItem::anchor(tableHeader, PG_Anchor::Left, content, PG_Anchor::Left);
	PG_GuiItem::anchor(tableHeader, PG_Anchor::Right, content, PG_Anchor::Right);
	tableHeader->setHeight(32.f);

	tableHeader->setItem([](PG_GuiItem* parent, std::size_t columnIndex) {
		auto h = PG_new<PG_DisaFileTableHeaderItem>("HeaderItem" + std::to_string(columnIndex), parent);
		switch (columnIndex) {
		case 0: 
			h->getTextItem()->setText("File");
			h->setWidth(200.f);
			break;
		case 1: h->getTextItem()->setText("Type"); break;
		case 2: h->getTextItem()->setText("Size"); break;
		case 3: h->getTextItem()->setText("Index"); break;
		}
		
		return h;
	});

	auto bgList = PG_new<PG_GuiListView>("PG_GuiListView", content);
	bgList->setClip(true);
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
	connect(tableView, &PG_GuiItem::contentOffsetChanged, bgList, [tableView, bgList] {
		bgList->setContentOffset(glm::vec2(0.f, tableView->getContentOffset().y));
	});


	tableView->setColumnWidthModel(tableHeader);
	tableView->setItem(PG_GuiListView::getDefaultCreateFunc<PG_DisaFileTableFileItem>());
	tableView->setItemUpdate(PG_GuiListView::getDefaultUpdateFunc<PG_DisaFileTableFileItem>());
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
	auto scrollComponent = PG_new<PG_GuiScrollComponent>(tableView);
	scrollComponent->setVerticalScrollBar(scrollY);

	PG_GuiScrollBar* scrollX = PG_new<PG_GuiScrollBar>("PG_GuiScrollBarX", content);
	scrollX->setSize(glm::vec2(0.0f, 10.0f));
	scrollX->setScrollBarColors(scrollY->getScrollBarColors());
	PG_GuiItem::anchor(scrollX, PG_Anchor::Left, content, PG_Anchor::Left);
	PG_GuiItem::anchor(scrollX, PG_Anchor::Right, scrollY, PG_Anchor::Left);
	PG_GuiItem::anchor(scrollX, PG_Anchor::Bottom, content, PG_Anchor::Bottom);
	scrollX->setMargin(PG_Anchor::Left, 3.f);
	scrollX->setMargin(PG_Anchor::Right, 3.f);
	scrollX->setMargin(PG_Anchor::Bottom, 3.f);
	scrollComponent->setHorizontalScrollBar(scrollX);
	if (auto rec = dynamic_cast<PG_GuiRectangle*>(scrollX->getHandle())) {
		rec->setMaterial("M_GuiRoundedRectangleAll5");
	}
	tableHeader->moveToBack();

	connect(scrollY, &PG_GuiMouseArea::enabledChanged, tableView, [tableView](bool visible) {
		tableView->setMargin(PG_Anchor::Right, visible? 17.f: 7.f);
		});
	connect(scrollX, &PG_GuiMouseArea::enabledChanged, tableView, [tableView](bool visible) {
		tableView->setMargin(PG_Anchor::Bottom, visible ? 17.f : 0.f);
		});

	connect(m_mainModel, &PG_DisaFileManagerMainModel::centerModelChanged, this, [this, tableView, bgList] (PG_UiStateBaseModel* oldModel, PG_UiStateBaseModel* newModel){
		if (!newModel) return;
		bgList->setModel(newModel->getListModel());
		tableView->setModel(newModel->getListModel());
		});

	// ========= RIGHT SIDE =========
	auto* inspector = PG_new<PG_DisaFileInspector>("inspector", cell);
	cell->addRight(inspector, 250.f);

	connect(m_mainModel, &PG_DisaFileManagerMainModel::centerModelChanged, inspector,
		[inspector](PG_UiStateBaseModel* oldModel, PG_UiStateBaseModel* newModel) { inspector->setModel(newModel); });
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
