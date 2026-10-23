#include "PG_DisaItemDropDown.h"
#include <Scene/Gui/PG_GuiText.h>
#include <Scene/Gui/PG_GuiImage.h>
#include <Util/Color/ColorDefined.h>
#include <Models/PG_DisaMapEditorMainModel.h>
#include <Disa/PG_DisaUiTextursModel.h>


PG_GuiItem* PG_DisaItemDropDown::createDisplayElement(PG_GuiItem* parent) const {
	auto container = PG_new<PG_GuiItem>("Container", glm::vec2(0.f), glm::vec2(30.f), parent);

	auto icon = PG_new<PG_GuiImage>("Icon", glm::vec2(0.f), glm::vec2(30.f), container);
	PG_GuiItem::anchor(icon, PG_Anchor::Top, container, PG_Anchor::Top);
	PG_GuiItem::anchor(icon, PG_Anchor::Left, container, PG_Anchor::Left);
	icon->setSourceData(PG_DisaMapEditorMainModel::get()->getUiTextursModel()->getIconsTexture());
	
	auto text = PG_new<PG_GuiText>("DropText", glm::vec2(0.f), glm::vec2(30.f), container);
	PG_GuiItem::anchor(text, PG_Anchor::Top, container, PG_Anchor::Top);
	PG_GuiItem::anchor(text, PG_Anchor::Left, icon, PG_Anchor::Right);
	PG_GuiItem::anchor(text, PG_Anchor::Right, container, PG_Anchor::Right);
	text->setFontSize(21.f);
	text->setColor(PG_Color::hentaiBlueFrgb);
	text->setVerticalAlignment(PG_Anchor::VerticalCenter);
	text->setMargin(PG_Anchor::Left, 1.f);
	text->setMargin(PG_Anchor::Right, 3.f);
	text->setWrapMode(PG_GuiText::EWrapMode::None);

	if (dynamic_cast<PG_GuiDropDown*>(parent)) {
		PG_GuiItem::anchor(container, PG_Anchor::Top, parent, PG_Anchor::Top);
		PG_GuiItem::anchor(container, PG_Anchor::Bottom, parent, PG_Anchor::Bottom);
		PG_GuiItem::anchor(container, PG_Anchor::Left, parent, PG_Anchor::Left);
		if (m_arrow) PG_GuiItem::anchor(container, PG_Anchor::Right, m_arrow, PG_Anchor::Left);
		else PG_GuiItem::anchor(container, PG_Anchor::Right, parent, PG_Anchor::Right);
	}
	else {
		container->fill(parent);
	}
	return container;
}

void PG_DisaItemDropDown::updateDisplayElement(PG_GuiItem* element, const std::string& str) const {
	element->findFirstChild<PG_GuiText>()->setText(str);
}

void PG_DisaItemDropDown::updatePreviewElement(PG_GuiItem* element, const PG_Variety& var) const {
	auto icon = element->findFirstChild<PG_GuiImage>();
	if (!icon) return;

	if (var.is<glm::vec4>()) {
		auto value = var.get<glm::vec4>();
		icon->setCutoutOffset(glm::vec2(value.x, value.y));
		icon->setCutoutSize(glm::vec2(value.z, value.w));
		icon->setVisible(true);
		return;
	}
	
	if (var.is<glm::vec2>()) {
		auto value = var.get<glm::vec2>();
		icon->setCutoutOffset(glm::vec2(value.x, value.y));
		icon->setCutoutSize(icon->getSize());
		icon->setVisible(true);
		return;
	}

	icon->setVisible(false);
}