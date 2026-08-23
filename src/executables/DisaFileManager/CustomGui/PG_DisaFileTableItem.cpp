#include "PG_DisaFileTableItem.h"
#include <Util/Color/ColorDefined.h>
#include <Scene/Gui/PG_GuiText.h>
#include <Scene/Gui/PG_GuiLines.h>
#include <Scene/Gui/PG_GuiMouseArea.h>
#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Model/PG_ListModelBase.h>
#include <Base/Events/PG_MouseButtonEvent.h> 
#include <Disa/fileformats/PG_DisaFileContainer.h>



void PG_DisaFileTableItem::init() {
	PG_GuiItem::init();

	m_text = PG_new<PG_GuiText>("title", this);
	m_text->fill(this);
	m_text->setWrapMode(PG_GuiText::Elide);
	m_text->setHeight(20.f);
	m_text->setFontSize(20.f);
	m_text->setFont("ClearSans/ClearSans-Regular.ttf");
	m_text->setColor(PG_Color::hentaiBlueFrgb);
	m_text->setMargin(PG_Anchor::Top, 0.f);
	m_text->setMargin(PG_Anchor::Left, 5.f);
	m_text->setVerticalAlignment(PG_GuiText::ETextAlignment::VerticalCenter);
}

void PG_DisaFileTableItem::setLocation(const PG_ListModelLocation& location) {
	if (location == m_location) return;
	m_location = location;

	m_text->setText(location.getDataT<std::string>(PG_Identity_Text));
}

void PG_DisaFileTableItem::update(const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
	const bool hasItem = identities.count(PG_Identity_Item) != 0 || m_location != location;

	if (hasItem) {
		m_location = location;
	}
	
	if (hasItem || identities.count(PG_Identity_Text) || identities.count(PG_DisaFileEntry::Identity_Modified)) {
		std::string str = location.getDataT<std::string>(PG_Identity_Text);
		if (location.getIndex().get(1) == 0 && location.getDataT<bool>(PG_DisaFileEntry::Identity_Modified)) {
			str += "*";
		}
		m_text->setText(str);
	}
}

void PG_DisaFileTableHeaderItem::init() {
	PG_DisaFileTableItem::init();

	auto* line = PG_new<PG_GuiLines>("sperator", this);
	line->makeLineY();
	anchor(line, PG_Anchor::Top, this, PG_Anchor::Top);
	anchor(line, PG_Anchor::Bottom, this, PG_Anchor::Bottom);
	anchor(line, PG_Anchor::Right, this, PG_Anchor::Right);
	line->setColor(PG_Color::hentaiBlueFrgb * 0.7f);
}


void PG_DisaFileTableRowItem::init() {
	PG_GuiItem::init();
	m_bg = PG_new<PG_GuiRectangle>("sperator", this);
	m_bg->setColor(PG_Color::grayFrgb);
	m_bg->setAlpha(0.1f);
	m_bg->fill(this);

	m_area = PG_new<PG_GuiMouseArea>("PG_GuiMouseArea", this);
	m_area->fill(this);
	m_area->setHoverEnabled(true);
	connect(m_area, &PG_GuiMouseArea::hoveredChanged, this, &PG_DisaFileTableRowItem::updateColor);

	connect(m_area, &PG_GuiMouseArea::mouseClicked, this, [this](PG_MouseButtonEvent* e) {
		if (e->isUp() && e->getButton() & PG_BUTTON_LEFT) {
			auto parent = getParent<PG_GuiItem>();
			if (!parent || !m_location) return;
			auto lastSelected = parent->getLayoutProperty(LayoutProperty_LastSelect, PG_Index());
			auto* model = m_location.getModel();
			if (!model->isIndexValid(lastSelected)) {
				lastSelected = m_location.getIndex();
				parent->setLayoutProperty(LayoutProperty_LastSelect, lastSelected);
			}

			if (e->getKeyMod() & PG_KMod_Shift) {
				auto start = lastSelected;
				auto end = m_location.getIndex();
				if (start > end) std::swap(start, end);
				std::vector<PG_Index> changeRange = parent->getLayoutProperty(LayoutProperty_Data1, std::vector<PG_Index>{});
				if (!changeRange.empty()) {
					model->setData(lastSelected, PG_ListSelectionRangeValuesData{ std::move(changeRange), false, false }, PG_Identity_Selected);
				}

				while (start != end && changeRange.size() < 100000) {
					if (!model->getData(start, PG_Identity_Selected).get<bool>()) changeRange.push_back(start);
					start = model->next(start);
				}
				if (!model->getData(start, PG_Identity_Selected).get<bool>()) changeRange.push_back(start);

				model->setData(lastSelected, PG_ListSelectionRangeValuesData{ changeRange, true, false }, PG_Identity_Selected);
				parent->setLayoutProperty(LayoutProperty_Data1, std::move(changeRange));
			}
			else if (e->isDoubleClick()) {
				parent->setLayoutProperty(LayoutProperty_LastSelect, m_location.getIndex());
				m_location.setData( PG_ListSelectionData{ true, true }, PG_Identity_Selected );
				auto mainModel = m_location.getModelRoot()->getParent();
				if (!mainModel) return;
				auto func = mainModel->getFunction("void openEntry(const PG_Index&)");
				if (!func) return;
				func.callArgs(mainModel, m_location.getIndex());
			}
			else {
				parent->setLayoutProperty(LayoutProperty_LastSelect, m_location.getIndex());
				parent->setLayoutProperty(LayoutProperty_Data1, std::vector<PG_Index>{});
				m_location.setData(
					PG_ListSelectionData{ true, !(e->getKeyMod() & PG_KMod_Ctrl)},
					PG_Identity_Selected
				);
			}
		}
		});

}

void PG_DisaFileTableRowItem::setLocation(const PG_ListModelLocation& location) {
	if (location == m_location) return;
	m_location = location;
	updateColor();
	
}

void PG_DisaFileTableRowItem::update(const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
	const bool hasItem = identities.count(PG_Identity_Item) != 0 || m_location != location;

	if (hasItem) {
		m_location = location;
		m_area->unfocus();
	}

	if (hasItem || identities.contains(PG_Identity_Selected)) {
		updateColor();
	}
}

void PG_DisaFileTableRowItem::updateColor() {
	if (!m_location.isValid()) return;
	const bool isHovered = m_area->isHovered();
	const bool isSelected = m_location.getDataT<bool>(PG_Identity_Selected);
	const bool isMod = m_location.getIndex().get(0) % 2 == 0;

	if (isHovered) {
		m_bg->setColor(isSelected? PG_Color::hentaiBlueFrgb: PG_Color::whiteFrgb);
		m_bg->setVisible(true);
		m_bg->setAlpha(0.4f);
	}else if (isSelected) {
		m_bg->setColor(PG_Color::hentaiBlueFrgb);
		m_bg->setVisible(true);
		m_bg->setAlpha(isMod? 0.2f: 0.3f);
	}
	else {
		m_bg->setAlpha(0.1f);
		m_bg->setColor(PG_Color::grayFrgb);
		m_bg->setVisible(isMod);
	}

}