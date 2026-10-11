/*
 * GNU Lesser General Public License (LGPL-3.0)
 *
 * Copyright (c) 2026 ProgSys
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

#include <Base/Events/PG_KeyEvent.h>
#include <Base/Events/PG_MouseButtonEvent.h>
#include <Resources/PG_Shader.h>

#include <Scene/Gui/PG_GuiMain.h>
#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Gui/PG_GuiRow.h>
#include <Scene/Gui/PG_GuiTooltip.h>
#include <Scene/Gui/PG_GuiButton.h>
#include <Scene/Gui/PG_GuiMouseArea.h>
#include <Scene/Gui/PG_GuiMenuPopup.h>
#include <Scene/Gui/PG_GuiScrollBar.h>
#include <Scene/Gui/PG_GuiScrollComponent.h>
#include <Scene/Gui/PG_GuiTableView.h>
#include <Scene/Gui/PG_GuiTableHeaderView.h>
#include <Scene/Gui/PG_GuiListView.h>

#include <Disa/utils/PG_FileDialog.h>
#include <Disa/customGui/PG_PupUpDialog.h>
#include <Disa/PG_DisaColorDefines.h>

#include <Models/PG_DisaDataTableModel.h>
#include <Models/PG_DisaDefListModel.h>
#include <CustomGui/PG_DisaDataTableItem.h>

#define APP_TITLE "Disa Data Editor"

void RootScene::init() {
	PG_Scene::init();

	m_model = PG_new<PG_DisaDataTableModel>("dataTableModel", this);
	m_defListModel = PG_new<PG_DisaDefListModel>("defListModel", this);
	m_defListModel->reload();

	auto guiMain = PG_new<PG_GuiMain>("guiMain", getRoot());
	m_rootItem = PG_new<PG_GuiItem>("rootItem", glm::vec2(0.f), glm::vec2(300.f, 400.f), guiMain);

	m_toolbar = PG_new<PG_GuiRectangle>("toolbar", glm::vec2(0.f), glm::vec2(10.f, 50.f), m_rootItem);
	m_toolbar->setColor(PG_Color::blackGrayFrgb);
	PG_GuiItem::anchor(m_toolbar, PG_Anchor::Top, m_rootItem, PG_Anchor::Top);
	PG_GuiItem::anchor(m_toolbar, PG_Anchor::Left, m_rootItem, PG_Anchor::Left);
	PG_GuiItem::anchor(m_toolbar, PG_Anchor::Right, m_rootItem, PG_Anchor::Right);

	m_toolbarRow = PG_new<PG_GuiRow>("toolbarRow", glm::vec2(0.f, 10.f), glm::vec2(39.f, 30.f), m_toolbar);
	m_toolbarRow->setSpacing(3.f);
	m_toolbarRow->setMargin(PG_Anchor::Left, 10.f);
	m_toolbarRow->setMargin(PG_Anchor::Right, 10.f);
	m_toolbarRow->fill(m_toolbar);

	m_center = PG_new<PG_GuiItem>("center", m_rootItem);
	PG_GuiItem::anchor(m_center, PG_Anchor::Top, m_toolbar, PG_Anchor::Bottom);
	PG_GuiItem::anchor(m_center, PG_Anchor::Left, m_rootItem, PG_Anchor::Left);
	PG_GuiItem::anchor(m_center, PG_Anchor::Right, m_rootItem, PG_Anchor::Right);
	PG_GuiItem::anchor(m_center, PG_Anchor::Bottom, m_rootItem, PG_Anchor::Bottom);
	m_center->setClip(true);

	guiMain->setTooltipItem(PG_new<PG_GuiTooltip>("Tooltip", m_rootItem));

	const auto updateTitle = [this] { emit titleChanged(getTitle()); };
	connect(m_model, &PG_DisaDataTableModel::fileChanged, this, updateTitle);
	connect(m_model, &PG_DisaDataTableModel::modifiedChanged, this, updateTitle);

	setupToolbar();
	rebuildTable();
}

bool RootScene::open(const PG_File& file) {
	if (file.isEmpty()) return false;
	std::string error;
	if (!m_model->open(file, error)) {
		showError("Open failed", error);
		return false;
	}
	rebuildTable();
	return true;
}

std::string RootScene::getTitle() const {
	if (!m_model || !m_model->isOpen()) return APP_TITLE;
	return std::string(APP_TITLE " - ") + m_model->getFile().getPath() + (m_model->isModified() ? "*" : "");
}

void RootScene::setupToolbar() {
	auto createButton = [this](const std::string& name, const std::string& icon, const std::string& tooltip) {
		auto button = PG_new<PG_GuiButton>(name, glm::vec2(0.0f), glm::vec2(39.f, 30.f), m_toolbarRow);
		button->setIcon(icon);
		button->setBackground("M_GuiRoundedRectangleAll5");
		button->setTooltipText(tooltip);
		return button;
	};

	auto openButton = createButton("openButton", "I_icon_folder", "Open file");
	openButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 20.f);
	connect(openButton, &PG_GuiButton::clicked, this, [this] {
		open(PG_FileDialog::open(this, "DAT"));
	});

	auto saveButton = createButton("saveButton", "I_icon_save", "Save file");
	connect(saveButton, &PG_GuiButton::clicked, this, [this] {
		if (!m_model->isOpen()) return;
		auto* popup = PG_PopUpDialog::createConfirm(m_rootItem, "Save?", "Do you want to save and overwrite \"" + m_model->getFile().getFile() + "\"?");
		connect(popup, &PG_PopUpDialog::confirmClicked, this, [this] {
			if (!m_model->save(m_model->getFile())) showError("Save failed", "Failed to save \"" + m_model->getFile().getPath() + "\"!");
		});
		popup->open();
	});

	auto saveAsButton = createButton("saveAsButton", "I_icon_saveAs", "Save as");
	connect(saveAsButton, &PG_GuiButton::clicked, this, [this] {
		if (!m_model->isOpen()) return;
		PG_File file = PG_FileDialog::save(this, "DAT", m_model->getFile().getFile());
		if (file.isEmpty()) return;
		if (!m_model->save(file)) showError("Save failed", "Failed to save \"" + file.getPath() + "\"!");
	});

	auto exportButton = createButton("exportCsvButton", "I_icon_export", "Export as CSV");
	connect(exportButton, &PG_GuiButton::clicked, this, [this] {
		if (!m_model->isOpen()) return;
		PG_File file = PG_FileDialog::save(this, "csv", m_model->getFile().getName() + ".csv");
		if (file.isEmpty()) return;
		if (!m_model->exportCSV(file)) showError("Export failed", "Failed to export \"" + file.getPath() + "\"!");
	});

	const auto updateEnabled = [this, saveButton, saveAsButton, exportButton] {
		saveButton->setEnabled(m_model->isOpen() && m_model->isModified());
		saveAsButton->setEnabled(m_model->isOpen());
		exportButton->setEnabled(m_model->isOpen());
	};
	connect(m_model, &PG_DisaDataTableModel::fileChanged, this, updateEnabled);
	connect(m_model, &PG_DisaDataTableModel::modifiedChanged, this, updateEnabled);
	updateEnabled();
}

void RootScene::rebuildTable() {
	if (m_tableContent) {
		m_tableContent->setParent(nullptr);
		m_tableContent->deleteLater();
	}

	auto* content = PG_new<PG_GuiItem>("tableContent", m_center);
	content->fill(m_center);
	m_tableContent = content;

	//behind the table, it gets the right clicks the cells ignore
	auto* tableArea = PG_new<PG_GuiMouseArea>("tableArea", content);
	tableArea->fill(content);
	tableArea->setAcceptedButtons(PG_GuiMouseArea::Right);

	PG_GuiTableHeaderView* tableHeader = PG_new<PG_GuiTableHeaderView>("tableHeader", content);
	PG_GuiItem::anchor(tableHeader, PG_Anchor::Top, content, PG_Anchor::Top);
	PG_GuiItem::anchor(tableHeader, PG_Anchor::Left, content, PG_Anchor::Left);
	PG_GuiItem::anchor(tableHeader, PG_Anchor::Right, content, PG_Anchor::Right);
	tableHeader->setHeight(32.f);
	//without a file the available definitions are listed
	const bool isOpen = m_model->isOpen();
	tableHeader->setItem([this, isOpen](PG_GuiItem* parent, std::size_t columnIndex) {
		auto* header = PG_new<PG_DisaDataHeaderItem>("HeaderItem" + std::to_string(columnIndex), parent);
		header->setText(isOpen ? m_model->getColumnName(columnIndex) : m_defListModel->getColumnName(columnIndex));
		header->setWidth(isOpen ? m_model->getColumnWidth(columnIndex) : m_defListModel->getColumnWidth(columnIndex));
		return header;
	});

	auto tableView = PG_new<PG_GuiTableView>("tableView", content);
	PG_GuiItem::anchor(tableView, PG_Anchor::Top, tableHeader, PG_Anchor::Bottom);
	PG_GuiItem::anchor(tableView, PG_Anchor::Bottom, content, PG_Anchor::Bottom);
	PG_GuiItem::anchor(tableView, PG_Anchor::Left, content, PG_Anchor::Left);
	PG_GuiItem::anchor(tableView, PG_Anchor::Right, content, PG_Anchor::Right);
	tableView->setColumnWidthModel(tableHeader);
	tableView->setItem(PG_GuiListView::getDefaultCreateFunc<PG_DisaDataCellItem>());
	tableView->setItemUpdate(PG_GuiListView::getDefaultUpdateFunc<PG_DisaDataCellItem>());
	tableView->setClip(true);

	PG_GuiScrollBar* scrollY = PG_new<PG_GuiScrollBar>("scrollBarY", content);
	scrollY->setSize(glm::vec2(10.0f, 0.0f));
	scrollY->setScrollBarColors({ PG_Color::hentaiBlueFrgb * 0.5f, PG_Color::hentaiBlueFrgb * 0.7f, PG_Color::hentaiBlueFrgb });
	if (auto rec = dynamic_cast<PG_GuiRectangle*>(scrollY->getHandle())) rec->setMaterial("M_GuiRoundedRectangleAll5");
	PG_GuiItem::anchor(scrollY, PG_Anchor::Top, content, PG_Anchor::Top);
	PG_GuiItem::anchor(scrollY, PG_Anchor::Bottom, content, PG_Anchor::Bottom);
	PG_GuiItem::anchor(scrollY, PG_Anchor::Right, content, PG_Anchor::Right);
	scrollY->setMargin(PG_Anchor::Top, 3.f);
	scrollY->setMargin(PG_Anchor::Bottom, 3.f);
	scrollY->setMargin(PG_Anchor::Right, 3.f);
	auto scrollComponent = PG_new<PG_GuiScrollComponent>(tableView);
	scrollComponent->setVerticalScrollBar(scrollY);

	PG_GuiScrollBar* scrollX = PG_new<PG_GuiScrollBar>("scrollBarX", content);
	scrollX->setSize(glm::vec2(0.0f, 10.0f));
	scrollX->setScrollBarColors(scrollY->getScrollBarColors());
	if (auto rec = dynamic_cast<PG_GuiRectangle*>(scrollX->getHandle())) rec->setMaterial("M_GuiRoundedRectangleAll5");
	PG_GuiItem::anchor(scrollX, PG_Anchor::Left, content, PG_Anchor::Left);
	PG_GuiItem::anchor(scrollX, PG_Anchor::Right, scrollY, PG_Anchor::Left);
	PG_GuiItem::anchor(scrollX, PG_Anchor::Bottom, content, PG_Anchor::Bottom);
	scrollX->setMargin(PG_Anchor::Left, 3.f);
	scrollX->setMargin(PG_Anchor::Right, 3.f);
	scrollX->setMargin(PG_Anchor::Bottom, 3.f);
	scrollComponent->setHorizontalScrollBar(scrollX);
	tableHeader->moveToBack();

	connect(scrollY, &PG_GuiMouseArea::enabledChanged, tableView, [tableView](bool visible) {
		tableView->setMargin(PG_Anchor::Right, visible ? 17.f : 7.f);
	});
	connect(scrollX, &PG_GuiMouseArea::enabledChanged, tableView, [tableView](bool visible) {
		tableView->setMargin(PG_Anchor::Bottom, visible ? 17.f : 0.f);
	});

	connect(tableArea, &PG_GuiMouseArea::mouseClicked, this, [this, tableArea, tableView](PG_MouseButtonEvent* e) {
		if (e->getButton() != PG_BUTTON_RIGHT || !tableView->isInside(e->getPosition())) return;
		const float y = tableView->toContentSpace(tableView->toLocalTranslation(e->getPosition())).y;
		const float rowHeight = tableView->getCellSize().y + tableView->getSpacing();
		if (y < 0.f || rowHeight <= 0.f) return;
		//below the last row only inserting is offered
		openRowMenu(tableArea, std::min(std::size_t(y / rowHeight), m_model->size()), e);
	});

	if (isOpen) tableView->setModel(m_model);
	else tableView->setModel(m_defListModel);
}

void RootScene::openRowMenu(PG_GuiItem* owner, std::size_t row, const PG_MouseButtonEvent* e) {
	if (!m_model->isOpen()) return;
	auto* model = m_model;
	const std::size_t size = model->size();
	auto popup = PG_new<PG_GuiMenuPopup>("rowMenu", owner);

	if (row < size) {
		popup->addOption("Insert above", model, [model, row] { model->insertRow(row); });
		popup->addOption("Insert below", model, [model, row] { model->insertRow(row + 1); });
		popup->addOption("Insert at top", model, [model] { model->insertRow(0); });
		popup->addOption("Insert at bottom", model, [model] { model->insertRow(model->size()); });
		popup->addOption("Move up", row > 0, model, [model, row] { model->moveRow(row, row - 1); });
		popup->addOption("Move down", row + 1 < size, model, [model, row] { model->moveRow(row, row + 1); });
		popup->addOption("Delete row " + std::to_string(row + 1), model, [model, row] { model->removeRow(row); });
	}
	else {
		popup->addOption("Insert row", model, [model] { model->insertRow(model->size()); });
	}
	popup->openAndDestroyOnClose(e);
}

void RootScene::showError(const std::string& title, const std::string& text) {
	auto* popup = PG_PopUpDialog::createError(m_rootItem, title, text);
	popup->open();
}

bool RootScene::event(PG_Event* e) {
	if (e->getType() == PG_Event_Key) {
		auto key = static_cast<PG_KeyEvent*>(e);
		if (key->getKey() == PG_Key_F5 && !key->isRepeat() && key->isUp()) {
			PG_Shader::reloadAll();
			key->accept();
			return true;
		}
	}
	return PG_Scene::event(e);
}
