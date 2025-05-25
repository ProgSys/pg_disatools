#include "PG_DisaEventTileListItem.h"
#include <Scene/Gui/PG_GuiSpinBox.h>
#include <Scene/Gui/PG_GuiButton.h>
#include <Disa/map/PG_DisaEventTileModel.h>


void PG_DisaEventTileListItem::init() {
	PG_GuiItem::init();

	m_spinbox = PG_GuiSpinBox::createInt("ValueSpinBox", glm::vec2(0.f), glm::vec2(110.f, 30.f), 0, 99999, this);
	m_spinbox->setMargin(PG_Anchor::Right, 10.f);

	connect(m_spinbox, &PG_GuiSpinBox::valueModefied, this, [this](double value) {
		m_location.setDataT<unsigned short>(static_cast<unsigned short>(value), PG_DisaEventTileModel::Identity_Value);
		});

	m_removeBtn = PG_new<PG_GuiButton>("RemoveButton", glm::vec2(0.f), glm::vec2(39.f, 30.f), this);
	m_removeBtn->setIcon("gui/icon_delete.png");
	m_removeBtn->setBackground("M_GuiRoundedRectangleAll5");
	m_removeBtn->setTooltipText("Remove");

	connect(m_removeBtn, &PG_GuiButton::clicked, this, [this]() {
		m_location.getModel()->remove(m_location.getIndex());
		});

	anchor(m_removeBtn, PG_Anchor::Right, this, PG_Anchor::Right);
	anchor(m_removeBtn, PG_Anchor::Top, this, PG_Anchor::Top);

	anchor(m_spinbox, PG_Anchor::Left, this, PG_Anchor::Left);
	anchor(m_spinbox, PG_Anchor::Right, m_removeBtn, PG_Anchor::Left);
	anchor(m_spinbox, PG_Anchor::Top, this, PG_Anchor::Top);


}


void PG_DisaEventTileListItem::setLocation(const PG_ListModelLocation& location) {
	if (location == m_location) return;
	m_location = location;
	m_spinbox->setValue(static_cast<double>(location.getDataT<unsigned short>(PG_DisaEventTileModel::Identity_Value)));
}

void PG_DisaEventTileListItem::update(const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
	const bool hasItem = identities.count(Identity_Item) != 0 || m_location != location;
	if (hasItem || identities.count(Identity_Location)) {
		m_location = location;
	}

	if (hasItem || identities.count(PG_DisaEventTileModel::Identity_Value)) {
		m_spinbox->setValue(static_cast<double>(location.getDataT<unsigned short>(PG_DisaEventTileModel::Identity_Value)));
	}
}
