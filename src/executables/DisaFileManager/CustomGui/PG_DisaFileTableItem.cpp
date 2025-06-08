#include "PG_DisaFileTableItem.h"
#include <Util/Color/ColorDefined.h>
#include <Scene/Gui/PG_GuiText.h>


void PG_DisaFileTableItem::init() {
	PG_GuiItem::init();

	m_text = PG_new<PG_GuiText>("title", this);
	m_text->fill(this);
	m_text->setWrapMode(PG_GuiText::Elide);
	m_text->setHeight(20.f);
	m_text->setFontSize(20.f);
	m_text->setColor(PG_Color::hentaiBlueFrgb);
	m_text->setMargin(PG_Anchor::Top, 0.f);
	m_text->setMargin(PG_Anchor::Left, 0.f);
}

void PG_DisaFileTableItem::setLocation(const PG_ListModelLocation& location) {
	if (location == m_location) return;
	m_location = location;

	m_text->setText(location.getDataT<std::string>(Identity_Text));
}

void PG_DisaFileTableItem::update(const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
	const bool hasItem = identities.count(Identity_Item) != 0 || m_location != location;

	if (hasItem || identities.count(Identity_Text)) {
		m_text->setText(location.getDataT<std::string>(Identity_Text));
	}
}