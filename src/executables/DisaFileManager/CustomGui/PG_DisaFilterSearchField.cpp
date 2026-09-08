#include "PG_DisaFilterSearchField.h"

#include <Scene/Gui/PG_GuiButton.h>
#include <Scene/Gui/PG_GuiGrid.h>
#include <Scene/Gui/PG_GuiImage.h>
#include <Scene/Gui/PG_GuiMouseArea.h>
#include <Scene/Gui/PG_GuiPopup.h>
#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Gui/PG_GuiTextField.h>
#include <Scene/Gui/PG_GuiToggleButton.h>

#include <Disa/PG_DisaColorDefines.h>
#include <Disa/customGui/PG_DisaSearchField.h>

#include <algorithm>
#include <array>
#include <utility>

using FileType = std::pair<const char*, const char*>; //extension, icon

constexpr std::array<FileType, 11> g_fileTypes = { {
	{ "TX2", "gui/icon_script.svg" }, { "DAT", "gui/icon_script.svg" }, { "GEO", "gui/icon_script.svg" },
	{ "MPD", "gui/icon_script.svg" }, { "MPP", "gui/icon_script.svg" }, { "OGG", "gui/icon_script.svg" },
	{ "PBD", "gui/icon_script.svg" }, { "FFM", "gui/icon_script.svg" }, { "PNG", "gui/icon_script.svg" },
	{ "FAD", "gui/icon_script.svg" }, { "PHD", "gui/icon_script.svg" },
} };

constexpr glm::vec2 g_cellSize = glm::vec2(90.f, 30.f);
constexpr int g_columnCount = 3;
constexpr float g_cellSpacing = 4.f;
constexpr float g_panelPadding = 6.f;
constexpr float g_filterButtonWidth = 30.f;
constexpr float g_filterIconSize = 20.f;

/// Eleven types and the one that clears them, so the last row comes out full
constexpr int g_cellCount = (int)g_fileTypes.size() + 1;
constexpr int g_rowCount = (g_cellCount + g_columnCount - 1) / g_columnCount;
/// What the cells need. The panel follows the field's width and only falls back on this when narrower.
constexpr float g_minPanelWidth =
	g_columnCount * g_cellSize.x + (g_columnCount - 1) * g_cellSpacing + 2.f * g_panelPadding;
constexpr float g_panelHeight =
	g_rowCount * g_cellSize.y + (g_rowCount - 1) * g_cellSpacing + 2.f * g_panelPadding;

void PG_DisaFilterSearchField::init() {
	PG_GuiItem::init();

	/* The two are rounded on their outer edges only, so a field and a button read as one control.
	 * @see updateCorners, which squares the bottom off again while the panel hangs below. */
	m_filterButton = PG_new<PG_GuiButton>("filterButton", glm::vec2(0.f), glm::vec2(g_filterButtonWidth), this);
	m_filterButton->setTooltipText("Filter by file type");
	anchor(m_filterButton, PG_Anchor::Right, this, PG_Anchor::Right);
	anchor(m_filterButton, PG_Anchor::Top, this, PG_Anchor::Top);
	anchor(m_filterButton, PG_Anchor::Bottom, this, PG_Anchor::Bottom);

	//replaces the one PG_GuiButton::init built, so updateCorners has something it can hold on to
	m_buttonBackground = PG_new<PG_GuiRectangle>("filterButtonBackground", m_filterButton);
	m_buttonBackground->setMaterial("M_GuiToolButtonEnd");
	m_filterButton->setBackground(m_buttonBackground);

	m_filterIcon = PG_new<PG_GuiImage>("filterIcon", glm::vec2(0.f), glm::vec2(g_filterIconSize),
		std::string("gui/icon_filter.svg"), m_filterButton);
	m_filterButton->setIcon(m_filterIcon);

	m_searchField = PG_new<PG_DisaSearchField>("textField", glm::vec2(0.f), glm::vec2(100.f, g_cellSize.y), this);
	m_searchField->setBackground("M_GuiToolButtonStart");
	anchor(m_searchField, PG_Anchor::Left, this, PG_Anchor::Left);
	anchor(m_searchField, PG_Anchor::Right, m_filterButton, PG_Anchor::Left);
	anchor(m_searchField, PG_Anchor::Top, this, PG_Anchor::Top);
	anchor(m_searchField, PG_Anchor::Bottom, this, PG_Anchor::Bottom);

	connect(m_searchField, &PG_GuiTextField::textChanged, this, &PG_DisaFilterSearchField::textChanged);

	m_popup = PG_new<PG_GuiPopup>("filterPopup", this);
	m_popup->setCreator([this](PG_GuiPopupInstance* instance) { buildPopup(instance); });
	connect(m_filterButton, &PG_GuiButton::clicked, m_popup, &PG_GuiPopup::open);
	connect(m_popup, &PG_GuiPopup::activeChanged, this, &PG_DisaFilterSearchField::updateCorners);
}

const std::string& PG_DisaFilterSearchField::getText() const {
	static const std::string empty;
	return m_searchField ? m_searchField->getText() : empty;
}

void PG_DisaFilterSearchField::setText(const std::string& text) {
	if (m_searchField) m_searchField->setText(text);
}

void PG_DisaFilterSearchField::buildPopup(PG_GuiPopupInstance* instance) {
	/* First, so it ends up underneath everything the panel holds. A mouse event walks the children
	 * backwards, so a click on a toggle stops at the toggle and only one that misses the panel gets
	 * this far and closes the popup. */
	auto* outside = PG_new<PG_GuiMouseArea>("outsideArea", instance);
	outside->fill(instance);
	outside->setConferEnabled(false);
	connect(outside, &PG_GuiMouseArea::clicked, instance, &PG_GuiPopupInstance::close);

	auto* panel = PG_new<PG_GuiRectangle>("filterPanel", glm::vec2(0.f), glm::vec2(g_minPanelWidth, g_panelHeight), instance);
	panel->setColor(PG_Color::blackGrayFrgb);
	panel->setMaterial("M_GuiRoundedRectangleBottom5");
	panel->setLayoutProperty(PG_GuiItem::LayoutProperty_MinSize, glm::vec2(g_minPanelWidth, g_panelHeight));
	//both edges, so the panel keeps the field's width even while the toolbar resizes
	anchor(panel, PG_Anchor::Left, this, PG_Anchor::Left);
	anchor(panel, PG_Anchor::Right, this, PG_Anchor::Right);
	anchor(panel, PG_Anchor::Top, this, PG_Anchor::Bottom);

	auto* grid = PG_new<PG_GuiGrid>("filterGrid", panel);
	grid->fill(panel);
	grid->setMargin(g_panelPadding);
	grid->setColumnCount(g_columnCount);
	grid->setCellHeight(g_cellSize.y);
	grid->setSpacing(glm::vec2(g_cellSpacing));

	const auto makeButton = [grid](const std::string& name, const char* icon, const char* text) {
		auto* btn = PG_new<PG_GuiToggleButton>(name, glm::vec2(0.f), g_cellSize, grid);
		btn->setIcon(icon); //sized off the button, the same 27 the toolbar buttons come out at
		btn->setText(std::string(text));
		btn->setBackground("M_GuiRoundedRectangleAll5");
		//a column is as wide as the panel allows, the button keeps its own width in the middle of it
		btn->setLayoutProperty(PG_GuiItem::LayoutProperty_Flow, PG_Anchor::HorizontalCenter);
		return btn;
		};

	std::vector<PG_GuiToggleButton*> toggles;
	toggles.reserve(g_fileTypes.size());
	for (const auto& [name, icon] : g_fileTypes) {
		auto* btn = makeButton(std::string("filter") + name, icon, name);
		btn->setToggled(std::ranges::find(m_fileTypes, name) != m_fileTypes.end());
		connect(btn, &PG_GuiToggleButton::toggleChanged, this, [this, extension = std::string_view(name)](bool toggled) {
			setTypeSelected(std::string(extension), toggled);
			});
		toggles.push_back(btn);
	}

	auto* clearButton = makeButton("filterClear", "gui/icon_delete.svg", "DEL");
	clearButton->setAutoToggle(false); //it acts, it does not latch
	clearButton->setBackgroundColor({ PG_DisaColor::destructiveFrgb,
		PG_DisaColor::destructiveHoverFrgb, PG_DisaColor::destructivePressedFrgb });

	connect(clearButton, &PG_GuiButton::clicked, this, [this, toggles = std::move(toggles)] {
		if (m_fileTypes.empty()) return;
		//emptied first, so every toggleChanged below finds nothing to remove and stays quiet
		m_fileTypes.clear();
		for (PG_GuiToggleButton* btn : toggles) btn->setToggled(false);
		updateFilterIcon();
		emit fileTypesChanged();
		});
}

void PG_DisaFilterSearchField::updateCorners(bool open) {
	const auto reshape = [open](PG_GuiRectangle* rec, const char* corner) {
		if (!rec) return;
		const PG_Material& material = rec->getMaterial();
		if (open && !material.isInstance()) {
			PG_Material instance = material.toInstance();
			instance.set(corner, 0.f);
			rec->setMaterial(instance);
		}
		else if (!open && material.isInstance()) {
			rec->setMaterial(material.toDesolveInstance());
		}
		};

	reshape(m_searchField ? m_searchField->getBackgroundRectangle() : nullptr, "radiusBottomLeft");
	reshape(m_buttonBackground, "radiusBottomRight");
}

void PG_DisaFilterSearchField::setFileTypes(std::vector<std::string> types) {
	if (m_fileTypes == types) return;
	m_fileTypes = std::move(types);
	updateFilterIcon();
	emit fileTypesChanged();
}

void PG_DisaFilterSearchField::setTypeSelected(const std::string& type, bool selected) {
	const auto findIt = std::ranges::find(m_fileTypes, type);
	if (selected == (findIt != m_fileTypes.end())) return;

	if (selected) m_fileTypes.push_back(type);
	else m_fileTypes.erase(findIt);

	updateFilterIcon();
	emit fileTypesChanged();
}

void PG_DisaFilterSearchField::updateFilterIcon() {
	if (!m_filterIcon) return;
	m_filterIcon->setSource(m_fileTypes.empty() ? "gui/icon_filter.svg" : "gui/icon_filter_warning.svg");
}
