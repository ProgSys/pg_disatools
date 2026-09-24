#include "PG_DisaObjectGridTile.h"
#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Gui/PG_GuiMouseArea.h>
#include <Util/Color/ColorDefined.h>
#include <Models/PG_DisaObjectCatalogModel.h>

void PG_DisaObjectGridTile::init() {
	PG_DisaPreviewItem::init();
	setName("PG_DisaObjectGridTile");

	m_modMarker = PG_new<PG_GuiRectangle>("ModMarker", glm::vec2(0.f), glm::vec2(10.f), this);
	m_modMarker->setColor(PG_Color::hentaiRedFrgb);
	m_modMarker->setMaterial("M_GuiRectangleCircle");
	anchor(m_modMarker, PG_Anchor::Top, this, PG_Anchor::Top);
	anchor(m_modMarker, PG_Anchor::Right, this, PG_Anchor::Right);
	m_modMarker->setMargin(PG_Anchor::Top, 4.f);
	m_modMarker->setMargin(PG_Anchor::Right, 4.f);
	m_modMarker->setVisible(false);
}

void PG_DisaObjectGridTile::setLocation(const PG_ListModelLocation& location) {
	PG_DisaPreviewItem::setLocation(location);
	updateInfo();
}

void PG_DisaObjectGridTile::update(const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
	PG_DisaPreviewItem::update(location, identities);
	updateInfo();
}

void PG_DisaObjectGridTile::updateInfo() {
	const PG_ListModelLocation& location = getLocation();
	if (m_modMarker) m_modMarker->setVisible(location && location.getDataT<bool>(PG_DisaObjectCatalogModel::Identity_IsMod));

	if (auto* area = findFirstChild<PG_GuiMouseArea>("PreviewArea")) {
		area->setTooltipText(location ? location.getDataT<std::string>(PG_Identity_Tooltip) : std::string());
	}
}
