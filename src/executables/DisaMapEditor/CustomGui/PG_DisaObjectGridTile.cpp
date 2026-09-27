#include "PG_DisaObjectGridTile.h"
#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Gui/PG_GuiMouseArea.h>
#include <Scene/Gui/PG_GuiGridView.h>
#include <Base/Events/PG_MouseMotionEvent.h>
#include <Drag/DisaNewObjectDrag.h>
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

	//hover stays with the preview area below, it is what unfreezes the preview
	auto* dragArea = PG_new<PG_GuiMouseArea>("DragArea", this);
	dragArea->setConferEnabled(true);
	dragArea->setDragEnabled(true);
	dragArea->setAcceptedButtons(PG_GuiMouseArea::Left);
	dragArea->fill(this);
	connect(dragArea, &PG_GuiMouseArea::dragChanged, this, [this](PG_MouseMotionEvent* e) {
		if (!getLocation() || e->getButton() != PG_BUTTON_LEFT) return;

		auto* grid = findParent<PG_GuiGridView>();
		if (!grid) return;
		if (auto* drag = grid->findFirstChild<DisaNewObjectDrag>()) drag->start(getLocation());
	});
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
