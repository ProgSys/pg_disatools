#include "PG_DisaFileInspector.h"

#include <Base/PG_Event.h>
#include <Base/PG_Index.h>
#include <Base/PG_Object.h>
#include <Util/PG_Any.h>
#include <Scene/Gui/PG_GuiButton.h>
#include <Scene/Gui/PG_GuiColumn.h>
#include <Scene/Gui/PG_GuiImage.h>
#include <Scene/Gui/PG_GuiLines.h>
#include <Scene/Gui/PG_GuiRow.h>
#include <Scene/Gui/PG_GuiText.h>
#include <Scene/Gui/PG_GuiTooltipArea.h>
#include <Scene/Model/PG_ListModelLocation.h>
#include <Util/PG_Dir.h>
#include <Util/PG_File.h>

#include <Resources/PG_Audio.h>

#include <Disa/PG_DisaColorDefines.h>
#include <Disa/customGui/PG_DisaFileIcons.h>
#include <Disa/customGui/PG_DisaPreviewItem.h>
#include <Disa/customGui/PG_PupUpDialog.h>
#include <Disa/fileformats/PG_DisaFileContainer.h>

#include <Models/PG_UiStateBaseModel.h>

#include <algorithm>
#include <vector>

constexpr float g_narrowWidth = 150.f; //below this the panel drops to the icon column
constexpr float g_buttonSize = 54.f;
constexpr float g_buttonCount = 5.f; //the action slot heads the group
constexpr float g_buttonGapRatio = 0.2f; //the gap between two buttons, as a share of their size
constexpr float g_iconRatio = 32.f / g_buttonSize;
constexpr float g_margin = 12.f;
constexpr float g_narrowMargin = 6.f;
constexpr float g_spacing = 10.f;
constexpr float g_headerHeight = 28.f;

/// U+2022, swap it for a plain '-' should the font turn out to have no glyph for it
constexpr const char* g_bullet = "\xE2\x80\xA2 ";
/// Keeps the head of the button group filled when the selected file has no action of its own
constexpr const char* g_actionStandIn = "gui/disa/icon_standin.svg";
constexpr unsigned char g_rebuildEvent = PG_Event_Behaviour_Singleton | PG_Event_Behaviour_Later;

/// Empty when the model has no selection, or is not the kind of model that tracks one
static PG_ListModelLocation lastSelectedOf(PG_Object* model) {
	if (!model) return {};
	PG_Any value = model->getPropertyValue("lastSelected");
	if (!value.is<PG_ListModelLocation>()) return {}; //get would throw on anything else
	return value.get<PG_ListModelLocation>();
}

static int selectionCountOf(PG_Object* model) {
	return model ? std::max(0, model->getPropertyValue("selectionSize").toInt(0)) : 0;
}

/// The file icon on the same rounded plate the action buttons sit on, for the narrow layout
static PG_GuiRectangle* addIconChip(PG_GuiItem* parent, float side) {
	auto* chip = PG_new<PG_GuiRectangle>("iconChip", glm::vec2(0.f), glm::vec2(side), parent);
	chip->setMaterial("M_GuiRoundedRectangleAll5");
	chip->setColor(PG_Color::darkerGrayFrgb);

	auto* icon = PG_new<PG_GuiImage>("icon", glm::vec2(0.f), glm::vec2(side * g_iconRatio), std::string(PG_DisaFileIcon::generic), chip);
	PG_GuiItem::anchor(icon, PG_Anchor::HorizontalCenter, chip, PG_Anchor::HorizontalCenter);
	PG_GuiItem::anchor(icon, PG_Anchor::VerticalCenter, chip, PG_Anchor::VerticalCenter);
	return chip;
}

namespace {

/**
* @brief Plays the selected ogg, and stops it again.
* @details Hangs off the button it drives, which is set before PG_new calls init, so it reaches that
* button itself and nothing outside has to hold it or talk to it. Nothing is read until it is pressed.
*/
class DisaPlayAudioAction : public PG_Object {
public:
	DisaPlayAudioAction(const std::string& name, const PG_ListModelLocation& location) : PG_Object(name), m_location(location) {}
	~DisaPlayAudioAction() override { m_audio.stop(); }

	void init() {
		auto* button = getParent<PG_GuiButton>();
		if (!button) return;
		button->setEnabled(true);
		connect(button, &PG_GuiButton::clicked, this, &DisaPlayAudioAction::trigger);
		updateButton();
	}

private:
	void trigger() {
		if (m_audio) {
			m_audio.stop();
			m_audio = PG_Audio(); //the samples go with it, nothing is held for a sound nobody is playing
		}
		else {
			/* The same resource the waveform preview asks for, so the two are one audio without this
			 * having to spell out the name PG_DisaFileEntry::getPreviewImage builds. */
			PG_Variety preview = m_location.getData(PG_Identity_PreviewImage);
			if (!preview.is<PG_Audio>()) return;
			m_audio = preview.get<PG_Audio>();
			m_audio.setCached(true);
			m_audio.play(); //starts itself once the decode lands, there is nothing to wait for here
			m_audio.onFinished(this, &DisaPlayAudioAction::trigger);
		}
		updateButton();
	}

	void updateButton() {
		auto* button = getParent<PG_GuiButton>();
		if (!button) return;
		const bool playing = (bool)m_audio;
		//setIcon builds a fresh item, so it gets the size the group is on rather than its own default
		const float side = button->getSize().x > 0.f ? button->getSize().x : g_buttonSize;
		button->setIcon(playing ? "gui/icon_pause.svg" : "gui/icon_play.svg", glm::vec2(side * g_iconRatio));
		button->setTooltipText(playing ? "Stop playing" : "Play the audio");
	}

	PG_ListModelLocation m_location;
	PG_Audio m_audio;
};


class DisaCOLAAction : public PG_Object {
public:
	DisaCOLAAction(const std::string& name, const PG_ListModelLocation& location) : PG_Object(name), m_location(location) {}

	void init() {
		auto* button = getParent<PG_GuiButton>();
		if (!button) return;
		button->setEnabled(true);
		connect(button, &PG_GuiButton::clicked, this, &DisaCOLAAction::trigger);
	}


private:
	void trigger() {
		if (!m_location) return;
		auto* button = getParent<PG_GuiButton>();
		if (!m_location) return;
		auto model = m_location.getModel()->getParent();
		assert(model);
		if (!model) return;
		auto funcDecompress = model->getFunction("void decompressAndReplaceLastSelected()");
		if (!funcDecompress) return;
		button->setEnabled(false);
		funcDecompress.callArgs(model);
	}

	PG_ListModelLocation m_location;
};

class DisaOpenFolderAction : public PG_Object {
public:
	DisaOpenFolderAction(const std::string& name, const PG_ListModelLocation& location) : PG_Object(name), m_location(location) {}

	void init() {
		auto* button = getParent<PG_GuiButton>();
		if (!button) return;
		button->setEnabled(true);
		connect(button, &PG_GuiButton::clicked, this, &DisaOpenFolderAction::trigger);
	}

private:
	void trigger() {
		if (!m_location) return;
		auto* button = getParent<PG_GuiButton>();
		if (!m_location) return;
		auto model = m_location.getModel()->getParent();
		assert(model);
		if (!model) return;
		auto func = model->getFunction("void openEntryLastSelected()");
		if (!func) return;
		button->setEnabled(false);
		//model->postCallEvent(func); //needed as it will self open will detroy inspector
		model->postCallEvent([model, func] { func.callArgs(model); }); //needed as it will self open will detroy inspector
	}

	PG_ListModelLocation m_location;
};

/// The file's type, or UNKNOWN when the location names no single file. Reads no file data.
PG_DisaFileInfo::Type typeOf(const PG_ListModelLocation& location) {
	if (!location) return PG_DisaFileInfo::UNKNOWN;
	auto dat = location.getData(PG_Identity_Item);
	auto entry = dat.cast<PG_DisaFileEntry>();
	return entry ? entry->getInfo().getType() : PG_DisaFileInfo::UNKNOWN;
}

} // namespace

static void addSeparator(PG_GuiItem* column) {
	auto* line = PG_new<PG_GuiLines>("separator", glm::vec2(0.f), glm::vec2(0.f, 1.f), column);
	line->makeLineX();
	line->setColor(PG_Color::darkerGrayFrgb);
	PG_GuiItem::anchor(line, PG_Anchor::Left, column, PG_Anchor::Left);
	PG_GuiItem::anchor(line, PG_Anchor::Right, column, PG_Anchor::Right);
}

void PG_DisaFileInspector::init() {
	PG_GuiRectangle::init();
	setColor(PG_Color::blackGrayFrgb);

	/* PG_GuiCell resizes a panel by moving its divider, the width then falls out of the anchors
	 * inside doUpdate and no size signal is emitted. midGuiUpdate is the one that carries it. */
	connect(this, &PG_GuiItem::midGuiUpdate, this, [this](glm::vec2*, glm::vec2* size) {
		updateMode(size->x);
		});
	connect(this, &PG_GuiItem::sizeChanged, this, [this] { updateMode(getSize().x); });

	rebuild();
}

void PG_DisaFileInspector::setModel(PG_UiStateBaseModel* model) {
	if (m_model) disconnect(m_model, nullptr, this, nullptr);

	m_stateModel = model;
	m_model = dynamic_cast<PG_Object*>(model);

	//a fresh tree comes up without any connection to the model that just went away
	rebuild();
}

void PG_DisaFileInspector::updateMode(float width) {
	if (width <= 1.f) return; //the panel has not been laid out yet

	const EMode mode = width < g_narrowWidth ? Narrow : Wide;
	if (mode == m_mode) return;
	m_mode = mode;

	/* Never tear the tree down inside a layout pass, and while the divider sits on the threshold
	 * this fires every frame, so the singleton collapses them into one rebuild. */
	postCallEvent(g_rebuildEvent, &PG_DisaFileInspector::rebuild);
}

void PG_DisaFileInspector::rebuild() {
	if (m_root) delete m_root;
	m_root = nullptr;
	m_fileIcon = nullptr;
	m_name = nullptr;
	m_infoBody = nullptr;
	m_preview = nullptr;
	m_iconArea = nullptr;
	m_actionSlot = nullptr;
	m_addButton = nullptr;
	m_replaceButton = nullptr;
	m_exportButton = nullptr;
	m_deleteButton = nullptr;

	if (m_mode == Narrow) buildNarrow();
	else buildWide();

	bindModel();
	updateSelection();
}

void PG_DisaFileInspector::buildWide() {
	m_root = PG_new<PG_GuiItem>("wide", this);
	m_root->fill(this);
	m_root->setMargin(PG_Anchor::Left, g_margin);
	m_root->setMargin(PG_Anchor::Right, g_margin);
	m_root->setMargin(PG_Anchor::Top, g_margin);

	auto* column = PG_new<PG_GuiColumn>("disaFileInspectorColumn", m_root);
	column->fill(m_root);
	column->setSpacing(g_spacing);

	{ //header, the file icon followed by the file name
		auto* header = PG_new<PG_GuiItem>("header", glm::vec2(0.f), glm::vec2(0.f, g_headerHeight), column);
		anchor(header, PG_Anchor::Left, column, PG_Anchor::Left);
		anchor(header, PG_Anchor::Right, column, PG_Anchor::Right);

		m_fileIcon = PG_new<PG_GuiImage>("icon", glm::vec2(0.f), glm::vec2(20.f), std::string(PG_DisaFileIcon::generic), header);
		anchor(m_fileIcon, PG_Anchor::Left, header, PG_Anchor::Left);
		anchor(m_fileIcon, PG_Anchor::VerticalCenter, header, PG_Anchor::VerticalCenter);

		m_name = PG_new<PG_GuiText>("name", glm::vec2(0.f), glm::vec2(0.f, g_headerHeight), header);
		anchor(m_name, PG_Anchor::Left, m_fileIcon, PG_Anchor::Right);
		anchor(m_name, PG_Anchor::Right, header, PG_Anchor::Right);
		m_name->setMargin(PG_Anchor::Left, 8.f);
		m_name->setFontSize(21.f);
		m_name->setColor(PG_Color::hentaiBlueFrgb);
		m_name->setWrapMode(PG_GuiText::Elide);
		m_name->setVerticalAlignment(PG_Anchor::VerticalCenter);
	}

	addSeparator(column);

	{ //preview, framed. The checkerboard behind an image comes from PG_DisaPreviewItem itself
		auto* frame = PG_new<PG_GuiItem>("previewFrame", glm::vec2(0.f), glm::vec2(0.f, 220.f), column);
		anchor(frame, PG_Anchor::Left, column, PG_Anchor::Left);
		anchor(frame, PG_Anchor::Right, column, PG_Anchor::Right);
		//both x anchors settle the width, so the ratio has the height follow it and the column reserves that
		frame->setLayoutProperty(PG_GuiItem::LayoutProperty_AspectRatio, glm::vec2(1.f));

		m_preview = PG_new<PG_DisaPreviewItem>("PG_DisaPreviewItem", frame);
		m_preview->fill(frame);
		m_preview->setNameVisible(false);

		auto* border = PG_new<PG_GuiRectangle>("previewBorder", frame);
		border->fill(frame);
		border->setAlpha(0.f);
		border->setBorderColor(PG_Color::darkerGrayFrgb);
		border->setBorderWidth(1.f);
	}

	addSeparator(column);

	{ //the file actions, centered and never wider than g_buttonSize
		//a negative height is the row saying it works that out from its children, @see PG_GuiItem::setSize
		auto* row = PG_new<PG_GuiRow>("buttonRow", glm::vec2(0.f), glm::vec2(0.f, -1.f), column);
		anchor(row, PG_Anchor::Left, column, PG_Anchor::Left);
		anchor(row, PG_Anchor::Right, column, PG_Anchor::Right);
		//the height follows the buttons, and only the height is capped, a width of 0 would collapse the row
		row->setLayoutProperty(PG_GuiItem::LayoutProperty_MaxSize,
			glm::vec2(std::numeric_limits<float>::max(), g_buttonSize));

		buildButtons(row);

		for (PG_GuiItem* item : { m_actionSlot, (PG_GuiItem*)m_addButton, (PG_GuiItem*)m_replaceButton,
			(PG_GuiItem*)m_exportButton, (PG_GuiItem*)m_deleteButton }) {
			item->setLayoutProperty(PG_GuiItem::LayoutProperty_Flow, PG_Anchor::HorizontalCenter);
			//fill is what lets them give way, the row only ever grows an item from its minimum
			item->setLayoutProperty(PG_GuiItem::LayoutProperty_Fill, true);
			item->setLayoutProperty(PG_GuiItem::LayoutProperty_MaxSize, glm::vec2(g_buttonSize));
			item->setLayoutProperty(PG_GuiItem::LayoutProperty_AspectRatio, glm::vec2(1.f));
		}

		/* A gap of a fifth of the button side is a length worked out from another length, which no property
		 * spells. measureSize is the one place early enough: the row shares its width out right after it,
		 * so a spacing set here is the spacing those buttons are measured against. */
		connect(row, &PG_GuiItem::measureSize, row, [row](glm::vec2, glm::vec2* size) {
			//the five items and the four gaps between them fill the row exactly
			const float side = std::min(g_buttonSize, size->x / (g_buttonCount + (g_buttonCount - 1.f) * g_buttonGapRatio));
			row->setSpacing(side * g_buttonGapRatio);
			});
	}

	addSeparator(column);

	{ //file info
		auto* title = PG_new<PG_GuiText>("infoTitle", glm::vec2(0.f), glm::vec2(0.f, 30.f), std::string("File Info"), column);
		anchor(title, PG_Anchor::Left, column, PG_Anchor::Left);
		anchor(title, PG_Anchor::Right, column, PG_Anchor::Right);
		title->setFontSize(24.f);
		title->setColor(PG_DisaColor::textFrgb);
		title->setWrapMode(PG_GuiText::None);

		m_infoBody = PG_new<PG_GuiText>("infoBody", glm::vec2(0.f), glm::vec2(0.f, 78.f), column);
		anchor(m_infoBody, PG_Anchor::Left, column, PG_Anchor::Left);
		anchor(m_infoBody, PG_Anchor::Right, column, PG_Anchor::Right);
		m_infoBody->setFontSize(20.f);
		m_infoBody->setColor(PG_DisaColor::textFrgb);
		m_infoBody->setWrapMode(PG_GuiText::None);
	}
}

void PG_DisaFileInspector::buildNarrow() {
	m_root = PG_new<PG_GuiItem>("narrow", this);
	m_root->fill(this);
	m_root->setMargin(PG_Anchor::Left, g_narrowMargin);
	m_root->setMargin(PG_Anchor::Right, g_narrowMargin);
	m_root->setMargin(PG_Anchor::Top, g_narrowMargin);

	auto* column = PG_new<PG_GuiColumn>("column", m_root);
	column->fill(m_root);

	m_iconArea = PG_new<PG_GuiTooltipArea>("iconArea", glm::vec2(0.f), glm::vec2(0.f, g_buttonSize), column);
	anchor(m_iconArea, PG_Anchor::Left, column, PG_Anchor::Left);
	anchor(m_iconArea, PG_Anchor::Right, column, PG_Anchor::Right);
	m_iconArea->setUseParent(false);

	/* The panel is against the right edge of the window, so beside means left, every time. There is
	 * no hint that says so outright. */
	m_iconArea->setTooltipPositionHint(PG_TooltipPositionHint_Right);

	//the file icon carries what the wide layout spells out, as a tooltip
	auto* chip = addIconChip(m_iconArea, g_buttonSize);
	anchor(chip, PG_Anchor::HorizontalCenter, m_iconArea, PG_Anchor::HorizontalCenter);
	anchor(chip, PG_Anchor::VerticalCenter, m_iconArea, PG_Anchor::VerticalCenter);
	m_fileIcon = chip->findFirstChild<PG_GuiImage>();

	buildButtons(column);

	for (PG_GuiItem* item : { m_actionSlot, (PG_GuiItem*)m_addButton, (PG_GuiItem*)m_replaceButton,
		(PG_GuiItem*)m_exportButton, (PG_GuiItem*)m_deleteButton }) {
		anchor(item, PG_Anchor::HorizontalCenter, column, PG_Anchor::HorizontalCenter);
	}

	/* A column owns no width, and a centre anchor places an item from a size it therefore never gets
	 * told, so everything in here is sized from the column instead. */
	connect(column, &PG_GuiItem::midGuiUpdate, column, [this, column, chip](glm::vec2*, glm::vec2* size) {
		const float side = std::min(g_buttonSize, size->x);
		column->setSpacing(side * g_buttonGapRatio);
		m_iconArea->setHeight(side);
		chip->setSize(glm::vec2(side));
		if (m_fileIcon) m_fileIcon->setSize(glm::vec2(side * g_iconRatio));
		setButtonSide(side);
		});
}

PG_GuiButton* PG_DisaFileInspector::makeGroupButton(PG_GuiItem* parent, const std::string& name,
	const std::string& iconPath, const std::string& tooltip) {
	auto* btn = PG_new<PG_GuiButton>(name, glm::vec2(0.f), glm::vec2(g_buttonSize), parent);
	btn->setIcon(iconPath, glm::vec2(g_buttonSize * g_iconRatio));
	btn->setBackground("M_GuiRoundedRectangleAll5");
	btn->setTooltipText(tooltip);
	btn->setEnabled(false);
	//a button places its icon but never sizes it, so it follows whatever side the button ends up with
	PG_Object::connect(btn, &PG_GuiItem::resolvedSizeChanged, btn, [btn] {
		if (auto* icon = btn->getIcon()) icon->setSize(glm::vec2(btn->getSize().x * g_iconRatio));
		});
	return btn;
}

void PG_DisaFileInspector::buildButtons(PG_GuiItem* parent) {
	//first in the group, and the only one whose contents are thrown away as the selection moves
	m_actionSlot = PG_new<PG_GuiItem>("actionSlot", glm::vec2(0.f), glm::vec2(g_buttonSize), parent);

	m_addButton = makeGroupButton(parent, "addButton", "gui/icon_add.svg", "Import one or more files below the selected file");
	m_replaceButton = makeGroupButton(parent, "replaceButton", "gui/icon_repeat.svg", "Replace selected file keeping its name");
	m_exportButton = makeGroupButton(parent, "exportButton", "gui/icon_file_export.svg", "Extract selected file or files");
	m_deleteButton = makeGroupButton(parent, "deleteButton", "gui/icon_delete.svg", "Delete selected file or files");

	m_deleteButton->setBackgroundColor({ PG_DisaColor::destructiveFrgb,
		PG_DisaColor::destructiveHoverFrgb, PG_DisaColor::destructivePressedFrgb });
}

void PG_DisaFileInspector::updateAction() {
	if (!m_actionSlot) return;
	m_actionSlot->clearGuiChildren();

	//a multi selection is the no action case by definition, there is no one file to act on
	const PG_ListModelLocation location = selectionCountOf(m_model) == 1 ? lastSelectedOf(m_model) : PG_ListModelLocation();

	//one case per type that has something to offer, and the only place this grows
	switch (typeOf(location)) {
	case PG_DisaFileInfo::OGG: {
		PG_GuiButton* button = makeGroupButton(m_actionSlot, "actionButton", "gui/icon_play.svg", "Play the audio");
		button->fill(m_actionSlot);
		PG_new<DisaPlayAudioAction>("playAudio", location, button);
		return;
	}
	case PG_DisaFileInfo::COLA: {
		PG_GuiButton* button = makeGroupButton(m_actionSlot, "actionButton", "gui/icon_folder.svg", "Decompress and replace");
		button->fill(m_actionSlot);
		PG_new<DisaCOLAAction>("decompress", location, button);
		return;
	}
	case PG_DisaFileInfo::OLA: {
	case PG_DisaFileInfo::SOLA: 
	case PG_DisaFileInfo::MPP: 
		PG_GuiButton* button = makeGroupButton(m_actionSlot, "actionButton", "gui/icon_folder.svg", "Open container");
		button->fill(m_actionSlot);
		PG_new<DisaOpenFolderAction>("openFolder", location, button);
		return;
	}
	default: break;
	}

	auto* standIn = PG_new<PG_GuiImage>("actionStandIn", m_actionSlot);
	standIn->setSource(std::string(g_actionStandIn));
	standIn->setAlpha(0.5f);
	standIn->fill(m_actionSlot);
}

/// Only the narrow column needs this, the wide row sizes the group through the layout properties
void PG_DisaFileInspector::setButtonSide(float side) {
	for (PG_GuiItem* item : { m_actionSlot, (PG_GuiItem*)m_addButton, (PG_GuiItem*)m_replaceButton,
		(PG_GuiItem*)m_exportButton, (PG_GuiItem*)m_deleteButton })
		item->setSize(glm::vec2(side));
}

void PG_DisaFileInspector::bindModel() {
	if (!m_model) return;

	connect(m_model, m_model->getProperty("lastSelected"), this, &PG_DisaFileInspector::updateSelection);

	if (auto func = m_model->getFunction("void importFiles(const std::vector<PG_File>&)")) {
		m_addButton->setEnabled(true);
		connect(m_addButton, &PG_GuiButton::clicked, m_model, [model = m_model, func] {
			std::vector<PG_File> results = PG_PopUpDialog::selectFiles({});
			if (results.empty()) return;
			func.callArgs(model, results);
			});
	}

	if (auto func = m_model->getFunction("void replaceLastSelected(const PG_File&,bool)")) {
		if (auto prop = m_model->getProperty("hasSingleSelection")) {
			connect(m_model, prop, m_replaceButton, m_replaceButton->getProperty("enabled"));
			m_replaceButton->setEnabled(m_model->getPropertyValue(prop).toBool());
		}
		connect(m_replaceButton, &PG_GuiButton::clicked, m_model, [model = m_model, func] {
			PG_File result = PG_PopUpDialog::selectFile({});
			if (!result) return;
			func.callArgs(model, result, false);
			});
	}

	if (auto prop = m_model->getProperty("hasSelection")) {
		//nothing removes entries yet, the button only tracks whether there would be something to remove
		connect(m_model, prop, m_deleteButton, m_deleteButton->getProperty("enabled"));
		m_deleteButton->setEnabled(m_model->getPropertyValue(prop).toBool());
	}

	auto funcExport = m_model->getFunction("void exportSelected(const PG_Dir&)");
	auto funcExportSingle = m_model->getFunction("void exportLastSelected(const PG_File&)");
	if (funcExport) {
		if (auto prop = m_model->getProperty("hasSelection")) {
			connect(m_model, prop, m_exportButton, m_exportButton->getProperty("enabled"));
			m_exportButton->setEnabled(m_model->getPropertyValue(prop).toBool());
		}
		connect(m_exportButton, &PG_GuiButton::clicked, m_model, [model = m_model, funcExport, funcExportSingle] {
			//only a single file is selected -> file name can be changed
			if (auto selectionSize = model->getPropertyValue("selectionSize");
				funcExportSingle && selectionSize && selectionSize.toInt() == 1) {
				PG_ListModelLocation loc = model->getPropertyValue("lastSelected").get<PG_ListModelLocation>();
				if (!loc) return;

				if (PG_File result = PG_PopUpDialog::saveAs(loc.getData(EIdentity::PG_Identity_Text).convert<std::string>())) {
					funcExportSingle.callArgs(model, result);
				}
				return;
			}

			//more then one file is selected -> select folder
			if (PG_Dir result = PG_PopUpDialog::selectDir(); !result.isEmpty()) {
				funcExport.callArgs(model, result);
			}
			});
	}
}

void PG_DisaFileInspector::updateSelection() {
	const int count = selectionCountOf(m_model);

	std::string title;
	if (count > 1) title = std::to_string(count) + " files selected";
	else if (count == 1) title = columnText(0);
	else if (m_stateModel) title = m_stateModel->getFileName();
	if (title.empty()) title = "No selection";
	if (m_name) m_name->setText(title);
	if (m_infoBody) m_infoBody->setText(buildInfoText());

	updateAction();

	//one file shows what its extension maps to, a pile of them has no one extension to show
	if (m_fileIcon) {
		if (count == 1) m_fileIcon->setSource(PG_DisaFileIcon::getFromName(columnText(0)));
		else m_fileIcon->setSource(PG_DisaFileIcon::generic);
	}

	if (m_iconArea) {
		if (std::string info = buildInfoText(); !info.empty()) title += "\n" + info;
		m_iconArea->setTooltipText(title);
	}

	if (m_preview) {
		if (count > 1) {
			//drop the location first, so reselecting that same single file refreshes the preview again
			m_preview->setLocation({});
			m_preview->open(std::string("gui/skip.png"));
		}
		else if (count == 1) {
			m_preview->setLocation(lastSelectedOf(m_model));
		}
		else {
			m_preview->setLocation({});
		}
	}
}

std::string PG_DisaFileInspector::buildInfoText() const {
	if (!m_model) return {};

	const int count = selectionCountOf(m_model);
	if (count > 1) {
		return g_bullet + std::string("Files: ") + std::to_string(count) +
			"\n" + g_bullet + "Total size: " + std::to_string(m_model->getPropertyValue("selectionByteSize").toInt(0));
	}
	if (count != 1) return {};

	return g_bullet + std::string("Type: ") + columnText(1) +
		"\n" + g_bullet + "Size: " + columnText(2) +
		"\n" + g_bullet + "Index: " + columnText(3);
}

std::string PG_DisaFileInspector::columnText(int column) const {
	PG_ListModelLocation loc = lastSelectedOf(m_model);
	if (!loc) return {};

	PG_Index index = loc.getIndex();
	if (index.size() < 2) return {}; //the table hands out a row and a column, anything flatter is not ours
	index.set(1, column);
	loc.setIndex(std::move(index));

	return loc.getData(EIdentity::PG_Identity_Text).convert<std::string>();
}
