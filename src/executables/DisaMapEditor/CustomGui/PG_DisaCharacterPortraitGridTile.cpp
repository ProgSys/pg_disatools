#include "PG_DisaCharacterPortraitGridTile.h"
#include <Scene/Gui/PG_GuiImage.h>
#include <Scene/Gui/PG_GuiText.h>
#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Gui/PG_GuiMouseArea.h>
#include <Scene/Gui/PG_GuiGridView.h>
#include <Disa/PG_DisaCharactersModel.h>
#include <Util/Color/ColorConverters.h>
#include <Base/Events/PG_MouseMotionEvent.h>
#include <Drag/DisaNewCharacterDrag.h>


void PG_DisaCharacterPortraitGridTile::init() {
	PG_GuiItem::init();
	m_image = PG_new<PG_GuiImageCropped>("PG_GuiImageCropped", this);
	m_image->fill(this);
	m_image->setCropSize(glm::vec2(160.f));

	m_text = PG_new<PG_GuiText>("PG_GuiText", glm::vec2(0.f, -10.f), glm::vec2(110.f, 30.f), "Name", this);
	anchor(m_text, PG_Anchor::Bottom, this, PG_Anchor::Bottom);
	anchor(m_text, PG_Anchor::HorizontalCenter, this, PG_Anchor::HorizontalCenter);
	constexpr glm::vec3 blueColor = glm::vec3(133.f, 208.f, 255.f) / 255.f;
	m_text->setVerticalAlignment(PG_Anchor::Bottom);
	m_text->setHorizontalAlignment(PG_Anchor::HorizontalCenter);
	m_text->setFontSize(21.f);
	m_text->setColor(blueColor);
	m_text->setMaterial(PG_Material("M_GuiTextBorderSDF"));
	
	PG_GuiRectangle* selectionBorder = PG_new<PG_GuiRectangle>("DisaGridTileRec", this);
	selectionBorder->fill(this);
	selectionBorder->setAlpha(0.f);
	selectionBorder->setBorderColor(PG_Color::hexToFloatColor(0xFF85D0FF));
	selectionBorder->setBorderWidth(2.f);
	selectionBorder->setVisible(false);

	m_area = PG_new<PG_GuiMouseArea>("DisaGridTileArea", this);
	m_area->setHoverEnabled(true);
	m_area->setConferEnabled(true);
	m_area->setDragEnabled(true);
	m_area->setAcceptedButtons(PG_GuiMouseArea::Left);
	m_area->fill(this);
	connect(m_area, &PG_GuiMouseArea::hoveredChanged, selectionBorder, &PG_GuiRectangle::setVisible);

	connect(m_area, &PG_GuiMouseArea::dragChanged, this, [this](PG_MouseMotionEvent* e) {
		if (!m_location || e->getButton() != PG_BUTTON_LEFT) return;

		auto* grid = findParent<PG_GuiGridView>();
		if (!grid) return;
		auto* drag = grid->findFirstChild<DisaNewCharacterDrag>();
		if (!drag) return;

		drag->start(m_location.getDataT<unsigned short>(PG_DisaCharacterModel::Identity_Id));
	});

}


void PG_DisaCharacterPortraitGridTile::setLocation(const PG_ListModelLocation& location) {
	if (location == m_location) return;
	m_location = location;
	reload();
}

void PG_DisaCharacterPortraitGridTile::update(const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
	const bool hasItem = identities.count(Identity_Item) != 0 || m_location != location;
	if (hasItem || identities.count(Identity_Location)) {
		m_location = location;
	}

	if (hasItem || identities.count(PG_DisaCharacterModel::Identity_Portrait)) {
		reload();
	}
}

void PG_DisaCharacterPortraitGridTile::reload() {
	if (!m_location) return;
	auto img = m_location.getDataT<PG_Texture>(PG_DisaCharacterModel::Identity_Portrait);
	if (img.isValid()) {
		m_image->setImage(img);
	}
	else {
		m_image->setImage("gui/disa/disaMissingno.png");
	}

	m_text->setText(std::to_string(m_location.getDataT<unsigned short>(PG_DisaCharacterModel::Identity_Id)));
	std::string name = m_location.getDataT<std::string>(PG_DisaCharacterModel::Identity_Name);
	std::string title = m_location.getDataT<std::string>(PG_DisaCharacterModel::Identity_Title);
	if (name.empty()) name = std::move(title);
	else if (name != title) name += "\n" + title;
	m_area->setTooltipText(name);
}