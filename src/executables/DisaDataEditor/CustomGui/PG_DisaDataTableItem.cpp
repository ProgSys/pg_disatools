#include "PG_DisaDataTableItem.h"

#include <Base/Events/PG_KeyEvent.h>
#include <Scene/Gui/PG_GuiLines.h>
#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Gui/PG_GuiText.h>
#include <Scene/Gui/PG_GuiTextEdit.h>
#include <Scene/Gui/Validator/PG_GuiTextValidators.h>
#include <Scene/Model/PG_ListModelBase.h>
#include <Util/Color/ColorDefined.h>

#include <Models/PG_DisaDataTableModel.h>

namespace {
	constexpr float g_fontSize = 18.f;
	constexpr char g_font[] = "ClearSans/ClearSans-Regular.ttf";
}

void PG_DisaDataHeaderItem::init() {
	PG_GuiItem::init();

	m_text = PG_new<PG_GuiText>("title", this);
	m_text->fill(this);
	m_text->setWrapMode(PG_GuiText::Elide);
	m_text->setFontSize(g_fontSize);
	m_text->setFont(g_font);
	m_text->setColor(PG_Color::hentaiBlueFrgb);
	m_text->setMargin(PG_Anchor::Left, 5.f);
	m_text->setMargin(PG_Anchor::Right, 5.f);
	m_text->setVerticalAlignment(PG_GuiText::ETextAlignment::VerticalCenter);

	auto* line = PG_new<PG_GuiLines>("separator", this);
	line->makeLineY();
	anchor(line, PG_Anchor::Top, this, PG_Anchor::Top);
	anchor(line, PG_Anchor::Bottom, this, PG_Anchor::Bottom);
	anchor(line, PG_Anchor::Right, this, PG_Anchor::Right);
	line->setColor(PG_Color::hentaiBlueFrgb * 0.7f);
}

void PG_DisaDataHeaderItem::setText(const std::string& text) {
	m_text->setText(text);
}

void PG_DisaDataCellItem::init() {
	PG_GuiItem::init();

	m_background = PG_new<PG_GuiRectangle>("background", this);
	m_background->fill(this);
	m_background->setVisible(false);

	m_text = PG_new<PG_GuiTextEdit>("cellEdit", this);
	m_text->fill(this);
	m_text->setMargin(PG_Anchor::Left, 5.f);
	m_text->setMargin(PG_Anchor::Right, 5.f);
	m_text->setFont(g_font);
	m_text->setFontSize(g_fontSize);
	m_text->setColor(PG_Color::hentaiBlueFrgb);
	m_text->setSelectionColor(PG_Color::hentaiDarkBlueFrgb);
	m_text->setCursorColor(PG_Color::bluishWhiteFrgb);
	m_text->setWrapMode(PG_GuiTextEdit::None);
	m_text->setVerticalAlignment(PG_GuiText::ETextAlignment::VerticalCenter);
	m_text->setClip(true);
	m_text->addEventFilter(this);

	connect(m_text, &PG_GuiTextEdit::activateChanged, this, [this](bool active) {
		if (!active) commit();
	});
}

bool PG_DisaDataCellItem::filterEvent(PG_Event* e) {
	if (e->getType() != EEventTypes::PG_Event_Key) return false;
	auto key = static_cast<PG_KeyEvent*>(e);
	if (!key->isDown() || !m_text->isKeysGrabbed()) return false;

	switch (key->getKey()) {
	case PG_Key_Return2:
	case PG_Key_Return:
		m_text->unfocus();
		return true;
	case PG_Key_Escape:
		updateText();
		m_text->unfocus();
		return true;
	default: return false;
	}
}

void PG_DisaDataCellItem::setLocation(const PG_ListModelLocation& location) {
	if (location == m_location) return;
	m_text->unfocus();
	m_location = location;
	updateText();
	updateBackground();
}

void PG_DisaDataCellItem::update(const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
	const bool hasItem = identities.contains(PG_Identity_Item) || m_location != location;
	if (hasItem) {
		m_text->unfocus();
		m_location = location;
	}

	if (hasItem || identities.contains(PG_Identity_Text) || identities.contains(PG_Identity_Location)) {
		updateText();
		updateBackground();
	}
}

void PG_DisaDataCellItem::updateText() {
	if (!m_location) {
		m_text->setText("");
		return;
	}
	m_editable = m_location.getDataT<bool>(PG_DisaDataTableModel::Identity_Editable);
	if (m_editable) m_text->setValidator(PG_SingleLineValidator());
	else m_text->setValidator([](std::string&) { return false; });
	m_text->setColor(m_editable ? PG_Color::hentaiBlueFrgb : PG_Color::grayFrgb);
	m_text->setText(m_location.getDataT<std::string>(PG_Identity_Text));
}

void PG_DisaDataCellItem::updateBackground() {
	if (!m_location) {
		m_background->setVisible(false);
		return;
	}

	const glm::vec4 color = m_location.getDataT<glm::vec4>(PG_DisaDataTableModel::Identity_Background);
	if (color.a > 0.f) {
		m_background->setColor(glm::vec3(color));
		m_background->setAlpha(color.a);
		m_background->setVisible(true);
		return;
	}

	const bool isStriped = m_location.getIndex().get(0) % 2 == 0;
	m_background->setColor(PG_Color::grayFrgb);
	m_background->setAlpha(0.1f);
	m_background->setVisible(isStriped);
}

void PG_DisaDataCellItem::commit() {
	if (!m_location) return;
	if (m_editable && m_text->getText() != m_location.getDataT<std::string>(PG_Identity_Text)) {
		m_location.setData(PG_Variety(m_text->getText()), PG_Identity_Text);
	}
	//the model may have rejected or reformatted the value
	updateText();
	updateBackground();
}
