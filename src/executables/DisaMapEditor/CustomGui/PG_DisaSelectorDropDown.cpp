#include "PG_DisaSelectorDropDown.h"
#include <Scene/Gui/PG_GuiTextEdit.h>
#include <Scene/Gui/PG_GuiButton.h>
#include <Scene/Gui/PG_GuiImage.h>
#include <Models/PG_CategoryFilterModel.h>
#include <Disa/CustomGui/PG_PupUpDialog.h>
#include <Util/Color/ColorDefined.h>
#include <Util/PG_StringUtil.h>
#include <Scene/Gui/Validator/PG_GuiTextValidators.h>
#include <Base/Events/PG_MouseButtonEvent.h>
#include <Base/Events/PG_MouseMotionEvent.h>

void PG_DisaSelectorDropDown::init() {

	if (!m_display) {
		auto textEdit = PG_new<PG_GuiTextEdit>("spriteSheetEditSelector", glm::vec2(0.f, 0.f), glm::vec2(200.f, 30.f), this);
		textEdit->setFontSize(21.f);
		textEdit->setColor(PG_Color::hentaiBlueFrgb);
		textEdit->setVerticalAlignment(PG_Anchor::VerticalCenter);
		textEdit->setMargin(PG_Anchor::Left, 10.f);
		textEdit->setMargin(PG_Anchor::Right, 3.f);
		textEdit->setSelectionColor(PG_Color::hentaiDarkBlueFrgb);
		textEdit->setColor(PG_Color::hentaiBlueFrgb);
		textEdit->setCursorColor(PG_Color::bluishWhiteFrgb);
		textEdit->setWrapMode(PG_GuiText::EWrapMode::None);
		textEdit->setValidator(PG_SingleWordValidator());
		textEdit->setClip(true);

		m_display = textEdit;
		anchor(m_display, PG_Anchor::Top, this, PG_Anchor::Top);
		anchor(m_display, PG_Anchor::Bottom, this, PG_Anchor::Bottom);
		anchor(m_display, PG_Anchor::Left, this, PG_Anchor::Left);

		connect(this, &PG_DisaSelectorDropDown::indexChanged, textEdit, [this](int index) {
			if (!getModel() || getIndex() < 0 || getIndex() >= getModel()->size()) return;

			if (auto textEdit = dynamic_cast<PG_GuiTextEdit*>(m_display)) {
				if (PG_Variety var = getModel()->getData(getIndex(), PG_Identity_Text)) {
					PG_File file(var.get<std::string>());
					textEdit->setText(file.getName());
					textEdit->setScroll(glm::vec2(0.f));
				}
			}
		});

		connect(textEdit, &PG_GuiTextEdit::textChanged, this, [this] {
			auto textEdit = static_cast<PG_GuiText*>(m_display);
			if (!getModel() || getModel()->empty() || textEdit->getText().empty()) {
				setIndex(-1);
				return;
			}

			for (int i = 0; i < getModel()->size(); ++i) {
				if (PG_Variety var = getModel()->getData(i, PG_Identity_Text)) {
					PG_File file(var.get<std::string>());

					if (PG_StringUtil::isEqualCaseless(file.getName(), textEdit->getText())) {
						setIndex(i);
						return;
					}
				}
			}

			
		});

	}

	PG_GuiDropDown::init();

	if (m_arrow) {
		anchor(m_display, PG_Anchor::Right, m_arrow, PG_Anchor::Left);
		static_cast<PG_GuiImage*>(m_arrow)->setImage("gui/icon_target.png");
	}
	else anchor(m_display, PG_Anchor::Right, this, PG_Anchor::Right);
}

bool PG_DisaSelectorDropDown::event(PG_Event* e) {
	if (isEnabled() && m_display) {
		switch (e->getType()) {
		case PG_Event_MouseButton:
		{
			auto button = static_cast<PG_MouseButtonEvent*>(e);
			if (button->getButton() == PG_BUTTON_RIGHT) {
				if (button->isUp() && isInside(button->getPosition())) {
					emit rightClicked();
				}
				return true;
			}

			if (m_display->isInside(button->getPosition())) {
				return PG_Object::event(e); //skip mause area
			}
		}break;
		case PG_Event_MouseMotion:
		{
			auto button = static_cast<PG_MouseMotionEvent*>(e);
			if (m_display->isInside(button->getPosition())) {
				return PG_Object::event(e); //skip mause area
			}
		}break;
		default:
			break;
		};
	}
	return PG_GuiDropDown::event(e);
}

void PG_DisaSelectorDropDown::updateDisplayElement(PG_GuiItem* element, const std::string& str) const {
	if (element == m_display) return;
	static_cast<PG_GuiText*>(element)->setText(str);
}