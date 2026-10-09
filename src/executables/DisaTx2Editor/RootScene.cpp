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
#include <Base/Events/PG_MouseMotionEvent.h>
#include <Base/Events/PG_MouseWheelEvent.h>
#include <Resources/PG_Shader.h>

#include <Scene/Gui/PG_GuiMain.h>
#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Gui/PG_GuiRow.h>
#include <Scene/Gui/PG_GuiTooltip.h>
#include <Scene/Gui/PG_GuiButton.h>
#include <Scene/Gui/PG_GuiToggleButton.h>
#include <Scene/Gui/PG_GuiMouseArea.h>
#include <Scene/Gui/PG_GuiImage.h>
#include <Scene/Gui/PG_GuiText.h>
#include <Scene/Gui/PG_GuiDropDown.h>
#include <Scene/Gui/PG_GuiPopup.h>
#include <Scene/Gui/PG_GuiUtil.h>
#include <Scene/Model/PG_ListModel.h>

#include <Disa/utils/PG_FileDialog.h>
#include <Disa/customGui/PG_PupUpDialog.h>
#include <Disa/customGui/PG_DisaColorTableView.h>
#include <Disa/models/PG_DisaColorTableModel.h>
#include <Disa/PG_DisaColorDefines.h>

#include <Models/PG_DisaTx2EditorModel.h>

#define APP_TITLE "Disa TX2 Editor"

namespace {
	constexpr float g_colorTablePanelWidth = 330.f;
	constexpr float g_minZoom = 0.05f;
	constexpr float g_maxZoom = 64.f;
}

void RootScene::init() {
	PG_Scene::init();

	m_model = PG_new<PG_DisaTx2EditorModel>("tx2EditorModel", this);

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

	setupView();
	setupColorTable();
	setupToolbar();

	guiMain->setTooltipItem(PG_new<PG_GuiTooltip>("Tooltip", m_rootItem));

	const auto updateTitle = [this] { emit titleChanged(getTitle()); };
	connect(m_model, &PG_DisaTx2EditorModel::fileChanged, this, updateTitle);
	connect(m_model, &PG_DisaTx2EditorModel::modifiedChanged, this, updateTitle);
	connect(m_model, &PG_DisaTx2EditorModel::imageChanged, this, &RootScene::updateInfo);
	updateInfo();
}

bool RootScene::open(const PG_File& file) {
	if (file.isEmpty()) return false;
	std::string error;
	if (!m_model->open(file, error)) {
		showError("Open failed", error);
		return false;
	}
	m_keepFocused = true;
	focusImage();
	return true;
}

std::string RootScene::getTitle() const {
	if (!m_model || !m_model->isOpen()) return APP_TITLE;
	return std::string(APP_TITLE " - ") + m_model->getFile().getPath() + (m_model->isModified() ? "*" : "");
}

void RootScene::focusImage() {
	if (!m_model->isOpen()) return;
	const glm::vec2 areaSize = m_viewArea->getSize();
	const glm::vec2 imageSize(m_model->getTx2().getWidth(), m_model->getTx2().getHeight());
	if (areaSize.x <= 0.f || areaSize.y <= 0.f || imageSize.x <= 0.f || imageSize.y <= 0.f) return;

	constexpr float border = 20.f;
	const glm::vec2 fitSize = glm::max(areaSize - border * 2.f, glm::vec2(1.f));
	const float scale = glm::clamp(glm::compMin(fitSize / imageSize), g_minZoom, g_maxZoom);
	m_imageScroll->setScale(glm::vec3(scale, scale, 1.f));
	m_imageScroll->setPosition(glm::round((areaSize - imageSize * scale) * 0.5f));
}

void RootScene::setupToolbar() {
	auto createButton = [this](const std::string& name, const std::string& icon, const std::string& tooltip) {
		auto button = PG_new<PG_GuiButton>(name, glm::vec2(0.0f), glm::vec2(39.f, 30.f), m_toolbarRow);
		button->setIcon(icon);
		button->setBackground("M_GuiRoundedRectangleAll5");
		button->setTooltipText(tooltip);
		return button;
	};
	auto createToggleButton = [this](const std::string& name, const std::string& icon, const std::string& tooltip) {
		auto button = PG_new<PG_GuiToggleButton>(name, glm::vec2(0.0f), glm::vec2(39.f, 30.f), m_toolbarRow);
		button->setIcon(icon);
		button->setBackground("M_GuiRoundedRectangleAll5");
		button->setTooltipText(tooltip);
		return button;
	};

	auto openButton = createButton("openButton", "gui/icon_folder.svg", "Open file");
	connect(openButton, &PG_GuiButton::clicked, this, [this] {
		open(PG_FileDialog::open(this, "tx2,txp,png,tga,jpg"));
	});

	auto saveButton = createButton("saveButton", "gui/icon_save.svg", "Save file");
	connect(saveButton, &PG_GuiButton::clicked, this, [this] {
		if (!m_model->isOpen() || !m_model->isTx2File()) return;
		std::string error;
		if (!m_model->save(m_model->getFile(), error)) showError("Save failed", error);
	});

	auto saveAsButton = createButton("saveAsButton", "gui/icon_saveAs.svg", "Save as TX2 or png");
	saveAsButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 20.f);
	connect(saveAsButton, &PG_GuiButton::clicked, this, [this] {
		if (!m_model->isOpen()) return;
		const PG_File file = PG_FileDialog::save(this, "tx2;png", m_model->getFile().getName());
		if (file.isEmpty()) return;
		std::string error;
		if (!m_model->save(file, error)) showError("Save failed", error);
	});

	auto focusButton = createButton("focusButton", "gui/icon_target.svg", "Show the entire image");
	connect(focusButton, &PG_GuiButton::clicked, this, [this] {
		m_keepFocused = true;
		focusImage();
	});

	auto convertButton = createButton("convertButton", "gui/icon_repeat.svg", "Convert to");
	connect(convertButton, &PG_GuiButton::clicked, this, &RootScene::openConvertDialog);

	m_infoButton = createToggleButton("infoButton", "gui/icon_tag.svg", "Image info");
	m_infoButton->setToggled(true);
	connect(m_infoButton, &PG_GuiToggleButton::toggleChanged, this, &RootScene::updateInfo);

	auto spacer = PG_new<PG_GuiItem>("toolbarSpacer", glm::vec2(0.f), glm::vec2(1.f, 30.f), m_toolbarRow);
	spacer->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);

	m_colorTableButton = createToggleButton("colorTableButton", "gui/icon_pick.svg", "Color table");
	connect(m_colorTableButton, &PG_GuiToggleButton::toggleChanged, this, [this](bool toggled) {
		m_colorTablePanel->setVisible(toggled);
		m_viewArea->setMargin(PG_Anchor::Right, toggled ? g_colorTablePanelWidth : 0.f);
	});

	const auto updateEnabled = [this, saveButton, saveAsButton, focusButton, convertButton] {
		const bool isOpen = m_model->isOpen();
		saveButton->setEnabled(isOpen && m_model->isTx2File() && m_model->isModified());
		saveAsButton->setEnabled(isOpen);
		focusButton->setEnabled(isOpen);
		convertButton->setEnabled(isOpen);
	};
	connect(m_model, &PG_DisaTx2EditorModel::fileChanged, this, updateEnabled);
	connect(m_model, &PG_DisaTx2EditorModel::modifiedChanged, this, updateEnabled);
	updateEnabled();
}

void RootScene::setupView() {
	m_viewArea = PG_new<PG_GuiItem>("viewArea", m_rootItem);
	PG_GuiItem::anchor(m_viewArea, PG_Anchor::Top, m_toolbar, PG_Anchor::Bottom);
	PG_GuiItem::anchor(m_viewArea, PG_Anchor::Left, m_rootItem, PG_Anchor::Left);
	PG_GuiItem::anchor(m_viewArea, PG_Anchor::Right, m_rootItem, PG_Anchor::Right);
	PG_GuiItem::anchor(m_viewArea, PG_Anchor::Bottom, m_rootItem, PG_Anchor::Bottom);
	m_viewArea->setClip(true);

	auto background = PG_new<PG_GuiImage>("viewBackground", m_viewArea);
	background->fill(m_viewArea);
	background->setFitType(PG_GuiImage::Repeat);
	background->setSource("/gui/pattern.png");

	m_navigationArea = PG_new<PG_GuiMouseArea>("navigationArea", m_viewArea);
	m_navigationArea->fill(m_viewArea);
	m_navigationArea->setAcceptedButtons(PG_GuiMouseArea::Middle | PG_GuiMouseArea::Wheel);

	m_imageScroll = PG_new<PG_GuiItem>("imageScroll", glm::vec2(0.f), glm::vec2(-1.f), m_viewArea);
	m_image = PG_new<PG_GuiImage>("image", glm::vec2(0.f), glm::vec2(1.f), m_imageScroll);
	m_image->setVisible(false);

	connect(m_navigationArea, &PG_GuiMouseArea::mouseMotion, this, [this](PG_MouseMotionEvent* e) {
		if (e->getButton() != PG_BUTTON_MIDDLE) return;
		m_keepFocused = false;
		m_imageScroll->setPosition(m_imageScroll->getPosition() + glm::vec2(e->getPositionRelative()));
	});
	connect(m_navigationArea, &PG_GuiMouseArea::wheelChanged, this, [this](PG_MouseWheelEvent* e) {
		m_keepFocused = false;
		const float oldScale = m_imageScroll->getScale().x;
		const float scale = glm::clamp(oldScale + oldScale * (e->y() / 10.f), g_minZoom, g_maxZoom);
		m_imageScroll->setScale(glm::vec3(scale, scale, 1.f));

		//zoom around the mouse
		const glm::vec2 pos = m_imageScroll->getPosition() - m_navigationArea->toLocalTranslation(glm::vec2(e->getPosition()));
		m_imageScroll->setPosition(m_imageScroll->getPosition() - (pos - pos * (scale / oldScale)));
	});
	//emitted in the middle of the layout pass, so the refit waits for it to end
	connect(m_viewArea, &PG_GuiItem::resolvedSizeChanged, this, [this] {
		if (m_keepFocused) postCallEvent([this] { focusImage(); });
	});

	connect(m_model, &PG_DisaTx2EditorModel::imageChanged, this, [this] {
		if (!m_model->isOpen()) {
			m_image->setVisible(false);
			m_image->setSourceData(std::monostate{});
			return;
		}
		m_image->setSize(glm::vec2(m_model->getTx2().getWidth(), m_model->getTx2().getHeight()));
		m_image->setSourceData(m_model->getTexture());
		m_image->setVisible(true);
	});

	//===== info overlay, top right =====
	m_infoOverlay = PG_new<PG_GuiRectangle>("infoOverlay", glm::vec2(0.f), glm::vec2(250.f, 136.f), m_viewArea);
	m_infoOverlay->setColor(PG_Color::blackFrgb);
	m_infoOverlay->setAlpha(0.6f);
	m_infoOverlay->setMaterial("M_GuiRoundedRectangleAll5");
	PG_GuiItem::anchor(m_infoOverlay, PG_Anchor::Top, m_viewArea, PG_Anchor::Top);
	PG_GuiItem::anchor(m_infoOverlay, PG_Anchor::Right, m_viewArea, PG_Anchor::Right);
	m_infoOverlay->setMargin(PG_Anchor::Top, 10.f);
	m_infoOverlay->setMargin(PG_Anchor::Right, 10.f);

	m_infoText = PG_new<PG_GuiText>("infoText", m_infoOverlay);
	m_infoText->fill(m_infoOverlay);
	m_infoText->setMargin(PG_Anchor::Left, 10.f);
	m_infoText->setMargin(PG_Anchor::Top, 8.f);
	m_infoText->setFontSize(18.f);
	m_infoText->setColor(PG_Color::hentaiBlueFrgb);
	m_infoText->setWrapMode(PG_GuiText::None);
}

void RootScene::setupColorTable() {
	m_colorTablePanel = PG_new<PG_GuiRectangle>("colorTablePanel", glm::vec2(0.f), glm::vec2(g_colorTablePanelWidth, 10.f), m_rootItem);
	m_colorTablePanel->setColor(PG_Gui::backgroundColor);
	PG_GuiItem::anchor(m_colorTablePanel, PG_Anchor::Top, m_toolbar, PG_Anchor::Bottom);
	PG_GuiItem::anchor(m_colorTablePanel, PG_Anchor::Right, m_rootItem, PG_Anchor::Right);
	PG_GuiItem::anchor(m_colorTablePanel, PG_Anchor::Bottom, m_rootItem, PG_Anchor::Bottom);
	m_colorTablePanel->setVisible(false);

	m_colorTableView = PG_new<PG_DisaColorTableView>("colorTableView", m_colorTablePanel);
	m_colorTableView->fill(m_colorTablePanel);
	m_colorTableView->setModel(m_model->getColorTableModel());
}

void RootScene::updateInfo() {
	m_infoOverlay->setVisible(m_infoButton && m_infoButton->isToggled() && m_model->isOpen());
	if (!m_model->isOpen()) return;

	const PG_TX2& tx2 = m_model->getTx2();
	const auto* colorTables = m_model->getColorTableModel();
	std::string text = "Type: " + m_model->getTypeName();
	text += "\nWidth: " + std::to_string(tx2.getWidth());
	text += "\nHeight: " + std::to_string(tx2.getHeight());
	text += "\nColors: " + std::to_string(tx2.header.colortableSize);
	text += "\nColor tables: " + std::to_string(colorTables->getTableCount());
	if (colorTables->getTableCount() > 1) text += " (shown " + std::to_string(colorTables->getCurrentTable() + 1) + ")";
	m_infoText->setText(text);
}

void RootScene::openConvertDialog() {
	if (!m_model->isOpen()) return;

	auto popup = PG_new<PG_PopUpDialog>("convertDialog", m_rootItem);
	popup->setCreator([](PG_GuiPopupInstance* parent) {
		auto blockArea = PG_new<PG_GuiMouseArea>("blockArea", parent);
		blockArea->fill(parent);
		blockArea->setHoverEnabled(true);
		blockArea->setConferEnabled(false);

		auto background = PG_new<PG_GuiRectangle>("dialogBackground", parent);
		background->setColor(PG_Color::blackFrgb);
		background->setAlpha(0.4f);
		background->fill(parent);

		auto window = PG_new<PG_GuiRectangle>("convertWindow", glm::vec2(0.f), glm::vec2(420.f, 230.f), background);
		window->setColor(PG_Gui::backgroundColor);
		window->setMaterial("M_GuiRoundedRectangleAll5");
		PG_GuiItem::anchor(window, PG_Anchor::HorizontalCenter, background, PG_Anchor::HorizontalCenter);
		PG_GuiItem::anchor(window, PG_Anchor::VerticalCenter, background, PG_Anchor::VerticalCenter);

		auto title = PG_new<PG_GuiText>("convertTitle", glm::vec2(0.f, 20.f), glm::vec2(420.f, 36.f), window);
		title->setFontSize(30.f);
		title->setColor(PG_Color::hentaiBlueFrgb);
		title->setHorizontalAlignment(PG_GuiText::HorizontalCenter);
		title->setText("Convert to");

		auto dropDown = PG_new<PG_GuiDropDown>("convertTargetDropDown", glm::vec2(30.f, 85.f), glm::vec2(360.f, 30.f), window);
		auto listModel = PG_new<PG_ListModel<std::string>>(dropDown);
		std::vector<std::string> names;
		for (const auto& target : PG_DisaTx2EditorModel::getConvertTargets()) names.push_back(target.name);
		listModel->set(std::move(names));
		dropDown->setModel(listModel);
		dropDown->setIndex(0);

		auto confirm = PG_new<PG_GuiButton>("convertConfirmButton", glm::vec2(30.f, 165.f), glm::vec2(170.f, 42.f), window);
		confirm->setText("Convert");
		confirm->setBackground("M_GuiRoundedRectangleAll5");
		confirm->centerText();

		auto cancel = PG_new<PG_GuiButton>("convertCancelButton", glm::vec2(220.f, 165.f), glm::vec2(170.f, 42.f), window);
		cancel->setText("Cancel");
		cancel->setBackground("M_GuiRoundedRectangleAll5");
		cancel->centerText();

		auto dialog = dynamic_cast<PG_PopUpDialog*>(parent->getOrigin());
		assert(dialog);
		connect(cancel, &PG_GuiButton::clicked, dialog, &PG_PopUpDialog::cancelClicked);
		connect(confirm, &PG_GuiButton::clicked, dialog, [dialog, dropDown] { emit dialog->confirmClickedIndex(dropDown->getIndex()); });
		connect(cancel, &PG_GuiButton::clicked, parent, &PG_GuiPopupInstance::close, PG_ConnectionType_Event);
		connect(confirm, &PG_GuiButton::clicked, parent, &PG_GuiPopupInstance::close, PG_ConnectionType_Event);
	});
	connect(popup, &PG_PopUpDialog::activeChanged, popup, [popup](bool active) {
		if (!active) popup->deleteLater();
	});
	connect(popup, &PG_PopUpDialog::confirmClickedIndex, this, [this](int index) {
		postCallEvent([this, index] { convertTo(index); });
	});
	popup->open();
}

void RootScene::convertTo(int targetIndex) {
	const auto& targets = PG_DisaTx2EditorModel::getConvertTargets();
	if (targetIndex < 0 || targetIndex >= int(targets.size())) return;
	const PG_TX2::Type type = targets[targetIndex].type;

	const auto doConvert = [this, type] {
		std::string error;
		if (!m_model->convertTo(type, error)) showError("Convert failed", error);
	};

	std::size_t maxColors = 0;
	if (type == PG_TX2::COLORTABLE_RGBA16 || type == PG_TX2::COLORTABLE_BGRA16) maxColors = 16;
	else if (type == PG_TX2::COLORTABLE_RGBA256 || type == PG_TX2::COLORTABLE_BGRA256) maxColors = 256;

	if (maxColors && m_model->countColors(maxColors) > maxColors) {
		auto* popup = PG_PopUpDialog::createConfirm(m_rootItem, "Continue?",
			"The image has more than " + std::to_string(maxColors) + " colors, some will be lost. Convert anyway?");
		connect(popup, &PG_PopUpDialog::confirmClicked, this, doConvert);
		popup->open();
		return;
	}
	doConvert();
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
