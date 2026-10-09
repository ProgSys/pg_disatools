#include "PG_DisaColorTableView.h"

#include <Base/Events/PG_KeyEvent.h>
#include <Base/Events/PG_MouseButtonEvent.h>
#include <Scene/Gui/PG_GuiButton.h>
#include <Scene/Gui/PG_GuiGridView.h>
#include <Scene/Gui/PG_GuiImage.h>
#include <Scene/Gui/PG_GuiMouseArea.h>
#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Gui/PG_GuiScrollBar.h>
#include <Scene/Gui/PG_GuiScrollComponent.h>
#include <Scene/Gui/PG_GuiText.h>
#include <Scene/Gui/PG_GuiTextEdit.h>
#include <Scene/Gui/PG_GuiTextField.h>
#include <Scene/Gui/PG_GuiUtil.h>
#include <Util/Color/ColorDefined.h>

#include <Disa/models/PG_DisaColorTableModel.h>
#include <Disa/utils/PG_FileDialog.h>

#include <cctype>

namespace {
	constexpr float g_buttonSize = 28.f;
	constexpr float g_swatchSize = 26.f;
}

void PG_DisaColorSwatchItem::init() {
	PG_GuiItem::init();

	auto* pattern = PG_new<PG_GuiImage>("pattern", this);
	pattern->fill(this);
	pattern->setMargin(PG_Anchor::Left, 2.f);
	pattern->setMargin(PG_Anchor::Right, 2.f);
	pattern->setMargin(PG_Anchor::Top, 2.f);
	pattern->setMargin(PG_Anchor::Bottom, 2.f);
	pattern->setFitType(PG_GuiImage::Repeat);
	pattern->setSource("/gui/pattern.png");

	m_color = PG_new<PG_GuiRectangle>("color", pattern);
	m_color->fill(pattern);

	m_selection = PG_new<PG_GuiRectangle>("selection", this);
	m_selection->fill(this);
	m_selection->setColor(PG_Color::whiteFrgb);
	m_selection->setAlpha(0.f);
	m_selection->moveToBack();

	m_area = PG_new<PG_GuiMouseArea>("swatchArea", this);
	m_area->fill(this);
	connect(m_area, &PG_GuiMouseArea::clicked, this, [this] {
		if (!m_location) return;
		if (auto* view = findParent<PG_DisaColorTableView>()) view->setSelected(m_location.getIndex().get(0));
	});

	if (auto* view = findParent<PG_DisaColorTableView>()) {
		connect(view, &PG_DisaColorTableView::selectedChanged, this, [this](int index) {
			setSelected(m_location && m_location.getIndex().get(0) == index);
		});
	}
}

void PG_DisaColorSwatchItem::setLocation(const PG_ListModelLocation& location) {
	m_location = location;
	updateColor();
}

void PG_DisaColorSwatchItem::update(const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
	m_location = location;
	updateColor();
}

void PG_DisaColorSwatchItem::setSelected(bool selected) {
	m_selection->setAlpha(selected ? 1.f : 0.f);
}

void PG_DisaColorSwatchItem::updateColor() {
	if (!m_location) return;
	const glm::rgba color = m_location.getDataT<glm::rgba>(PG_Identity_Item);
	m_color->setColor(glm::vec3(color) / 255.f);
	m_color->setAlpha(color.a / 255.f);
	m_area->setTooltipText(m_location.getDataT<std::string>(PG_Identity_Tooltip));
	auto* view = findParent<PG_DisaColorTableView>();
	setSelected(view && view->getSelected() == m_location.getIndex().get(0));
}

void PG_DisaColorTableView::init() {
	PG_GuiItem::init();

	auto* buttonRow = PG_new<PG_GuiItem>("buttonRow", glm::vec2(0.f), glm::vec2(10.f, g_buttonSize), this);
	anchor(buttonRow, PG_Anchor::Top, this, PG_Anchor::Top);
	anchor(buttonRow, PG_Anchor::Left, this, PG_Anchor::Left);
	anchor(buttonRow, PG_Anchor::Right, this, PG_Anchor::Right);
	buttonRow->setMargin(PG_Anchor::Top, 6.f);

	float x = 6.f;
	const auto createButton = [&x, buttonRow](const std::string& name, const std::string& icon, const std::string& tooltip) {
		auto* button = PG_new<PG_GuiButton>(name, glm::vec2(x, 0.f), glm::vec2(g_buttonSize), buttonRow);
		button->setIcon(icon);
		button->setBackground("M_GuiRoundedRectangleAll5");
		button->setTooltipText(tooltip);
		x += g_buttonSize + 3.f;
		return button;
	};

	m_prevButton = createButton("prevTableButton", "gui/icon_prevFrame.svg", "Previous color table");
	m_tableText = PG_new<PG_GuiText>("tableText", glm::vec2(x, 0.f), glm::vec2(56.f, g_buttonSize), buttonRow);
	m_tableText->setFontSize(18.f);
	m_tableText->setColor(PG_Color::hentaiBlueFrgb);
	m_tableText->setHorizontalAlignment(PG_GuiText::HorizontalCenter);
	m_tableText->setVerticalAlignment(PG_GuiText::VerticalCenter);
	x += 59.f;
	m_nextButton = createButton("nextTableButton", "gui/icon_nextFrame.svg", "Next color table");
	x += 8.f;
	m_addButton = createButton("addTableButton", "gui/icon_add.svg", "Add an empty color table");
	m_copyButton = createButton("copyTableButton", "gui/icon_duplicate.svg", "Add a copy of this color table");
	m_removeButton = createButton("removeTableButton", "gui/icon_delete.svg", "Remove this color table");
	x += 8.f;
	m_importButton = createButton("importTableButton", "gui/icon_folder.svg", "Import color table from image");
	m_exportButton = createButton("exportTableButton", "gui/icon_export.svg", "Export color table as png");

	connect(m_prevButton, &PG_GuiButton::clicked, this, [this] {
		if (!m_model || m_model->getTableCount() == 0) return;
		const int count = m_model->getTableCount();
		m_model->setCurrentTable((m_model->getCurrentTable() + count - 1) % count);
	});
	connect(m_nextButton, &PG_GuiButton::clicked, this, [this] {
		if (!m_model || m_model->getTableCount() == 0) return;
		m_model->setCurrentTable((m_model->getCurrentTable() + 1) % m_model->getTableCount());
	});
	connect(m_addButton, &PG_GuiButton::clicked, this, [this] { if (m_model) m_model->insertTable(false); });
	connect(m_copyButton, &PG_GuiButton::clicked, this, [this] { if (m_model) m_model->insertTable(true); });
	connect(m_removeButton, &PG_GuiButton::clicked, this, [this] { if (m_model) m_model->removeCurrentTable(); });
	connect(m_importButton, &PG_GuiButton::clicked, this, [this] {
		if (!m_model) return;
		if (PG_File file = PG_FileDialog::open(this, "png")) m_model->importTable(file);
	});
	connect(m_exportButton, &PG_GuiButton::clicked, this, [this] {
		if (!m_model) return;
		const std::string name = "colorTable" + std::to_string(m_model->getCurrentTable() + 1) + ".png";
		if (PG_File file = PG_FileDialog::save(this, "png", name)) m_model->exportTable(file);
	});

	//===== edit of the selected color =====
	auto* editRow = PG_new<PG_GuiItem>("editRow", glm::vec2(0.f), glm::vec2(10.f, 30.f), this);
	anchor(editRow, PG_Anchor::Bottom, this, PG_Anchor::Bottom);
	anchor(editRow, PG_Anchor::Left, this, PG_Anchor::Left);
	anchor(editRow, PG_Anchor::Right, this, PG_Anchor::Right);
	editRow->setMargin(PG_Anchor::Bottom, 6.f);

	m_selectedColor = PG_new<PG_GuiRectangle>("selectedColor", glm::vec2(6.f, 0.f), glm::vec2(30.f), editRow);
	m_selectedText = PG_new<PG_GuiText>("selectedText", glm::vec2(42.f, 0.f), glm::vec2(70.f, 30.f), editRow);
	m_selectedText->setFontSize(18.f);
	m_selectedText->setColor(PG_Color::hentaiBlueFrgb);
	m_selectedText->setVerticalAlignment(PG_GuiText::VerticalCenter);

	m_hexField = PG_new<PG_GuiTextField>("hexField", glm::vec2(116.f, 0.f), glm::vec2(130.f, 30.f), editRow);
	m_hexField->setBackground("M_GuiRoundedRectangleAll5");
	m_hexField->getBackgroundRectangle()->setColor(PG_Gui::backgroundFullColor);
	m_hexField->setPlaceholderText("RRGGBBAA");
	m_hexField->getTextItem()->setMaxChars(9);
	m_hexField->getTextItem()->setValidator([](std::string& str) {
		return std::ranges::all_of(str, [](char c) { return std::isxdigit((unsigned char)c) || c == '#'; });
	});
	m_hexField->getTextItem()->addEventFilter(this);
	connect(m_hexField->getTextItem(), &PG_GuiTextEdit::activateChanged, this, [this](bool active) {
		if (!active) commitHex();
	});

	//===== grid =====
	m_grid = PG_new<PG_GuiGridView>("colorGrid", this);
	anchor(m_grid, PG_Anchor::Top, buttonRow, PG_Anchor::Bottom);
	anchor(m_grid, PG_Anchor::Bottom, editRow, PG_Anchor::Top);
	anchor(m_grid, PG_Anchor::Left, this, PG_Anchor::Left);
	anchor(m_grid, PG_Anchor::Right, this, PG_Anchor::Right);
	m_grid->setMargin(PG_Anchor::Top, 6.f);
	m_grid->setMargin(PG_Anchor::Bottom, 6.f);
	m_grid->setMargin(PG_Anchor::Left, 6.f);
	m_grid->setMargin(PG_Anchor::Right, 16.f);
	m_grid->setCellSize(glm::ivec2(int(g_swatchSize)));
	m_grid->setClip(true);
	m_grid->setItem([](PG_GuiItem* parent, const PG_ListModelLocation& location) -> PG_GuiItem* {
		auto* swatch = PG_new<PG_DisaColorSwatchItem>("swatch", parent);
		swatch->setLocation(location);
		return swatch;
	});
	m_grid->setItemUpdate([](PG_GuiItem* item, const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
		if (auto* swatch = dynamic_cast<PG_DisaColorSwatchItem*>(item)) swatch->update(location, identities);
	});

	auto* scrollBar = PG_new<PG_GuiScrollBar>("colorGridScrollBar", this);
	scrollBar->setSize(glm::vec2(8.f, 0.f));
	scrollBar->setScrollBarColors({ PG_Color::hentaiBlueFrgb * 0.5f, PG_Color::hentaiBlueFrgb * 0.7f, PG_Color::hentaiBlueFrgb });
	anchor(scrollBar, PG_Anchor::Top, m_grid, PG_Anchor::Top);
	anchor(scrollBar, PG_Anchor::Bottom, m_grid, PG_Anchor::Bottom);
	anchor(scrollBar, PG_Anchor::Right, this, PG_Anchor::Right);
	scrollBar->setMargin(PG_Anchor::Right, 4.f);
	auto* scrollComponent = PG_new<PG_GuiScrollComponent>(m_grid);
	scrollComponent->setVerticalScrollBar(scrollBar);

	updateControls();
}

bool PG_DisaColorTableView::filterEvent(PG_Event* e) {
	if (e->getType() != EEventTypes::PG_Event_Key) return false;
	auto* key = static_cast<PG_KeyEvent*>(e);
	if (!key->isDown() || !m_hexField->getTextItem()->isKeysGrabbed()) return false;
	switch (key->getKey()) {
	case PG_Key_Return:
	case PG_Key_Return2:
		m_hexField->getTextItem()->unfocus();
		return true;
	case PG_Key_Escape:
		setSelected(m_selected); //puts the current value back into the field
		m_hexField->getTextItem()->unfocus();
		return true;
	default: return false;
	}
}

void PG_DisaColorTableView::setModel(PG_DisaColorTableModel* model) {
	if (model == m_model) return;
	if (m_model) disconnect(m_model, nullptr, this, nullptr);
	m_model = model;
	m_grid->setModel(model);
	if (m_model) {
		connect(m_model, &PG_DisaColorTableModel::currentTableChanged, this, &PG_DisaColorTableView::updateControls);
		connect(m_model, &PG_DisaColorTableModel::tablesChanged, this, &PG_DisaColorTableView::updateControls);
		connect(m_model, &PG_ListModelBase::itemsChanged, this, [this] { setSelected(m_selected); });
	}
	updateControls();
}

void PG_DisaColorTableView::setSelected(int index) {
	if (!m_model || index < 0 || index >= int(m_model->size())) index = -1;
	const bool changed = index != m_selected;
	m_selected = index;

	m_hexField->getTextItem()->setEnabled(index >= 0);
	if (index >= 0) {
		const glm::rgba color = m_model->getColor(index);
		m_selectedColor->setColor(glm::vec3(color) / 255.f);
		m_selectedColor->setAlpha(color.a / 255.f);
		m_selectedText->setText("Index " + std::to_string(index));
		if (!m_hexField->getTextItem()->isKeysGrabbed()) m_hexField->setText(PG_DisaColorTableModel::toHex(color));
	}
	else {
		m_selectedColor->setAlpha(0.f);
		m_selectedText->setText("");
		m_hexField->setText("");
	}
	if (changed) emit selectedChanged(m_selected);
}

void PG_DisaColorTableView::updateControls() {
	const int count = m_model ? m_model->getTableCount() : 0;
	const bool editable = m_model && m_model->isTableCountEditable();
	m_tableText->setText(count ? std::to_string(m_model->getCurrentTable() + 1) + "/" + std::to_string(count) : "-");
	m_prevButton->setEnabled(count > 1);
	m_nextButton->setEnabled(count > 1);
	m_addButton->setEnabled(editable);
	m_copyButton->setEnabled(editable && count > 0);
	m_removeButton->setEnabled(editable && count > 1);
	m_importButton->setEnabled(count > 0);
	m_exportButton->setEnabled(count > 0);
	setSelected(m_selected);
}

void PG_DisaColorTableView::commitHex() {
	if (!m_model || m_selected < 0) return;
	glm::rgba color;
	if (PG_DisaColorTableModel::fromHex(m_hexField->getText(), color)) m_model->setColor(m_selected, color);
	setSelected(m_selected);
}
