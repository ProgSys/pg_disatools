#include "PG_DisaColorTableView.h"

#include <Base/Events/PG_KeyEvent.h>
#include <Base/Events/PG_MouseButtonEvent.h>
#include <Scene/Gui/PG_GuiButton.h>
#include <Scene/Gui/PG_GuiGridView.h>
#include <Scene/Gui/PG_GuiImage.h>
#include <Scene/Gui/PG_GuiMouseArea.h>
#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Gui/PG_GuiRow.h>
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
	constexpr float g_swatchSize = 40.f;

	/// Black or white, whatever reads better on the color
	glm::vec3 contrastColor(const glm::rgba& color) {
		const float luminance = (0.299f * color.r + 0.587f * color.g + 0.114f * color.b) / 255.f;
		return color.a > 127 && luminance > 0.6f ? PG_Color::blackFrgb : PG_Color::whiteFrgb;
	}
}

void PG_DisaColorSwatchItem::init() {
	PG_GuiItem::init();

	auto* border = PG_new<PG_GuiRectangle>("border", this);
	border->fill(this);
	border->setColor(PG_Color::blackFrgb);

	auto* pattern = PG_new<PG_GuiImage>("pattern", border);
	pattern->fill(border);
	for (auto side : { PG_Anchor::Left, PG_Anchor::Right, PG_Anchor::Top, PG_Anchor::Bottom }) pattern->setMargin(side, 1.f);
	pattern->setFitType(PG_GuiImage::Repeat);
	pattern->setSource("/gui/pattern.png");

	m_color = PG_new<PG_GuiRectangle>("color", pattern);
	m_color->fill(pattern);

	m_selection = PG_new<PG_GuiRectangle>("selection", pattern);
	m_selection->fill(pattern);
	m_selection->setColor(PG_Color::whiteFrgb);
	m_selection->setVisible(false);

	m_indexText = PG_new<PG_GuiText>("index", glm::vec2(3.f, 1.f), glm::vec2(g_swatchSize - 4.f, 14.f), pattern);
	m_indexText->setFontSize(12.f);
	m_indexText->setWrapMode(PG_GuiText::None);

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
	m_selection->setVisible(selected);
	if (selected) m_indexText->setColor(PG_Color::blackFrgb);
	else if (m_location) m_indexText->setColor(contrastColor(m_location.getDataT<glm::rgba>(PG_Identity_Item)));
}

void PG_DisaColorSwatchItem::updateColor() {
	if (!m_location) return;
	const glm::rgba color = m_location.getDataT<glm::rgba>(PG_Identity_Item);
	m_color->setColor(glm::vec3(color) / 255.f);
	m_color->setAlpha(color.a / 255.f);
	m_indexText->setText(std::to_string(m_location.getIndex().get(0)));
	m_area->setTooltipText(m_location.getDataT<std::string>(PG_Identity_Tooltip));
	auto* view = findParent<PG_DisaColorTableView>();
	setSelected(view && view->getSelected() == m_location.getIndex().get(0));
}

void PG_DisaColorTableView::init() {
	PG_GuiItem::init();

	//===== toolbar, styled like the toolbars of the editors =====
	auto* toolbar = PG_new<PG_GuiRectangle>("colorTableToolbar", glm::vec2(0.f), glm::vec2(10.f, 50.f), this);
	toolbar->setColor(PG_Color::blackGrayFrgb);
	anchor(toolbar, PG_Anchor::Top, this, PG_Anchor::Top);
	anchor(toolbar, PG_Anchor::Left, this, PG_Anchor::Left);
	anchor(toolbar, PG_Anchor::Right, this, PG_Anchor::Right);

	auto* toolbarRow = PG_new<PG_GuiRow>("colorTableToolbarRow", glm::vec2(0.f, 10.f), glm::vec2(39.f, 30.f), toolbar);
	toolbarRow->setSpacing(3.f);
	toolbarRow->setMargin(PG_Anchor::Left, 10.f);
	toolbarRow->setMargin(PG_Anchor::Right, 10.f);
	toolbarRow->fill(toolbar);

	const auto createButton = [toolbarRow](const std::string& name, const std::string& icon, const std::string& tooltip) {
		auto* button = PG_new<PG_GuiButton>(name, glm::vec2(0.f), glm::vec2(39.f, 30.f), toolbarRow);
		button->setIcon(icon);
		button->setBackground("M_GuiRoundedRectangleAll5");
		button->setTooltipText(tooltip);
		return button;
	};

	m_prevButton = createButton("prevTableButton", "gui/icon_prevFrame.svg", "Previous color table");
	m_tableText = PG_new<PG_GuiText>("tableText", glm::vec2(0.f), glm::vec2(56.f, 30.f), toolbarRow);
	m_tableText->setFontSize(18.f);
	m_tableText->setColor(PG_Color::hentaiBlueFrgb);
	m_tableText->setHorizontalAlignment(PG_GuiText::HorizontalCenter);
	m_tableText->setVerticalAlignment(PG_GuiText::VerticalCenter);
	m_nextButton = createButton("nextTableButton", "gui/icon_nextFrame.svg", "Next color table");
	m_nextButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 20.f);
	m_addButton = createButton("addTableButton", "gui/icon_add.svg", "Add an empty color table");
	m_copyButton = createButton("copyTableButton", "gui/icon_duplicate.svg", "Add a copy of this color table");
	m_removeButton = createButton("removeTableButton", "gui/icon_delete.svg", "Remove this color table");
	m_removeButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 20.f);
	m_importButton = createButton("importTableButton", "gui/icon_folder.svg", "Import color table from image");
	m_exportButton = createButton("exportTableButton", "gui/icon_export.svg", "Export color table as png");
	m_exportButton->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 20.f);

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

	//the selected color and its hex edit
	m_selectedColor = PG_new<PG_GuiRectangle>("selectedColor", glm::vec2(0.f), glm::vec2(30.f), toolbarRow);
	m_selectedColor->setMaterial("M_GuiRoundedRectangleAll5");
	m_selectedText = PG_new<PG_GuiText>("selectedText", glm::vec2(0.f), glm::vec2(80.f, 30.f), toolbarRow);
	m_selectedText->setFontSize(18.f);
	m_selectedText->setColor(PG_Color::hentaiBlueFrgb);
	m_selectedText->setVerticalAlignment(PG_GuiText::VerticalCenter);

	m_hexField = PG_new<PG_GuiTextField>("hexField", glm::vec2(0.f), glm::vec2(130.f, 30.f), toolbarRow);
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
	auto* gridArea = PG_new<PG_GuiItem>("colorGridArea", this);
	anchor(gridArea, PG_Anchor::Top, toolbar, PG_Anchor::Bottom);
	anchor(gridArea, PG_Anchor::Bottom, this, PG_Anchor::Bottom);
	anchor(gridArea, PG_Anchor::Left, this, PG_Anchor::Left);
	anchor(gridArea, PG_Anchor::Right, this, PG_Anchor::Right);
	gridArea->setClip(true);

	auto* background = PG_new<PG_GuiImage>("colorGridBackground", gridArea);
	background->fill(gridArea);
	background->setFitType(PG_GuiImage::Repeat);
	background->setSource("/gui/pattern.png");

	m_grid = PG_new<PG_GuiGridView>("colorGrid", gridArea);
	m_grid->fill(gridArea);
	m_grid->setMargin(PG_Anchor::Top, 6.f);
	m_grid->setMargin(PG_Anchor::Bottom, 6.f);
	m_grid->setMargin(PG_Anchor::Left, 6.f);
	m_grid->setMargin(PG_Anchor::Right, 18.f);
	m_grid->setCellSize(glm::ivec2(int(g_swatchSize)));
	m_grid->setSpacing(0.f);
	m_grid->setClip(true);
	m_grid->setItem([](PG_GuiItem* parent, const PG_ListModelLocation& location) -> PG_GuiItem* {
		auto* swatch = PG_new<PG_DisaColorSwatchItem>("swatch", parent);
		swatch->setLocation(location);
		return swatch;
	});
	m_grid->setItemUpdate([](PG_GuiItem* item, const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
		if (auto* swatch = dynamic_cast<PG_DisaColorSwatchItem*>(item)) swatch->update(location, identities);
	});

	auto* scrollBar = PG_new<PG_GuiScrollBar>("colorGridScrollBar", gridArea);
	scrollBar->setSize(glm::vec2(10.f, 0.f));
	scrollBar->setScrollBarColors({ PG_Color::hentaiBlueFrgb * 0.5f, PG_Color::hentaiBlueFrgb * 0.7f, PG_Color::hentaiBlueFrgb });
	if (auto* handle = dynamic_cast<PG_GuiRectangle*>(scrollBar->getHandle())) handle->setMaterial("M_GuiRoundedRectangleAll5");
	anchor(scrollBar, PG_Anchor::Top, gridArea, PG_Anchor::Top);
	anchor(scrollBar, PG_Anchor::Bottom, gridArea, PG_Anchor::Bottom);
	anchor(scrollBar, PG_Anchor::Right, gridArea, PG_Anchor::Right);
	for (auto side : { PG_Anchor::Top, PG_Anchor::Bottom, PG_Anchor::Right }) scrollBar->setMargin(side, 3.f);
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
