#include "PG_DisaItemSelector.h"
#include <Scene/Model/PG_ListModel.h>
#include <Scene/Gui/PG_GuiPopup.h>
#include <Scene/Gui/PG_GuiMouseArea.h>
#include <Scene/Gui/PG_GuiMain.h>
#include <Scene/Gui/PG_GuiListView.h>
#include <Scene/Gui/PG_GuiScrollBar.h>
#include <Scene/Gui/PG_GuiMenuPopup.h>
#include <Util/Color/ColorDefined.h>
#include <PG_DisaGridTile.h>
#include <Models/PG_CategoryFilterModel.h>
#include <Disa/CustomGui/PG_PupUpDialog.h>

void PG_DisaItemSelector::init() {
	PG_GuiRectangle::init();

	setColor(glm::vec3(60.f / 255.f));
	setMaterial("M_GuiRoundedRectangleAll5");
	setClip(true);

	m_tile = PG_new<PG_DisaGridTile>("Tile", this);
	m_tile->fill(this);

	disconnect(m_tile, &PG_DisaGridTile::clicked, m_tile, nullptr);
	disconnect(m_tile, &PG_DisaGridTile::rightClicked, m_tile, nullptr);

	m_popup = PG_new<PG_GuiPopup>("PopupMenu", this);
	m_popup->setCreator([this](PG_GuiPopupInstance* parent) {
		initPopupMenu(parent);
	});

	connect(m_tile, &PG_DisaGridTile::clicked, this, &PG_DisaItemSelector::open);
	connect(m_tile, &PG_DisaGridTile::rightClicked, this, &PG_DisaItemSelector::rightClicked);
}

void PG_DisaItemSelector::setModel(PG_ListModelBase* model) {
	if (model == m_model) return;
	if (m_tile->getLocation()) {
		m_tile->setLocation({});
		emit selectedChanged();
	}

	if (m_model) {
		disconnect(m_model, nullptr, this, nullptr);
	}

	m_model = model;

	if (m_model) {
		connect(m_model, &PG_ListModelBase::itemsAdded, this, &PG_DisaItemSelector::findSelection, ConnectionType_EventSingelton);
		connect(m_model, &PG_ListModelBase::itemsRemoved, this, &PG_DisaItemSelector::findSelection, ConnectionType_EventSingelton);
	}
}

PG_ListModelLocation PG_DisaItemSelector::getSelected() const {
	return m_tile->getLocation();
}

void PG_DisaItemSelector::open() {
	if (!m_popup || !m_model || m_model->empty()) return;
	m_popup->open();
}

void PG_DisaItemSelector::close() {
	if (m_popup) m_popup->close();
}

void PG_DisaItemSelector::initPopupMenu(PG_GuiPopupInstance* parent) {
	auto area = PG_new<PG_GuiMouseArea>("DropDownOutsideArea",  parent);
	area->fill(parent);
	area->setConferEnabled(false);
	area->setAcceptedButtons(PG_GuiMouseArea::Left | PG_GuiMouseArea::Wheel | PG_GuiMouseArea::Right | PG_GuiMouseArea::Middle);
	connect(area, &PG_GuiMouseArea::clicked, parent, &PG_GuiPopupInstance::close);

	auto bg = PG_new<PG_GuiRectangle>("Dark", parent);
	bg->fill(parent);
	bg->setColor(PG_Color::blackFrgb);
	bg->setAlpha(0.3f);

	float heightMain = 0.f;
	if (auto main = findParent<PG_GuiMain>()) {
		heightMain = main->getRootItem()->getSize().y;
	}
	heightMain = std::max(heightMain, parent->getSize().y);
	constexpr float margin = 10.f;
	const float scale = getScaleWorld().y;
	const glm::vec2 localSize = parent->toLocalTranslation(toWorldTranslation(getSize(),false), false);
	const float height = std::ceil(std::min(std::min(m_model->size() * localSize.y, heightMain - margin * 2.f), 9000.f));

	auto menu = PG_new<PG_GuiRectangle>("BackgroundMenu", glm::vec2(0.f), glm::vec2(getSize().x, height/ scale), parent);
	menu->setMaterial("M_GuiRoundedRectangleAll5");
	menu->setColor(getColor());
	menu->setClip(true);
	anchor(menu, PG_Anchor::HorizontalCenter, this, PG_Anchor::HorizontalCenter);
	anchor(menu, PG_Anchor::Top, parent, PG_Anchor::Top);
	menu->setScale(getScaleWorld());

	const float center = parent->toLocalTranslation(toWorldTranslation(getPosition(PG_Anchor::VerticalCenter))).y;
	float pos = std::max(margin, center - height / 2.f);
	if (float diff = heightMain - (pos + height + margin); diff < 0.f) {
		pos += diff; //push up
	}
	menu->setPosition(glm::vec2(0.f, pos));

	auto listView = PG_new<PG_GuiListView>("PG_GuiListView", glm::vec2(0.f), glm::vec2(300.f), menu);
	listView->fill(menu);
	listView->setCellSize(getSize().y);
	listView->setItem([this](PG_GuiItem* item, const PG_ListModelLocation& location) {
		auto itemlist = PG_new<PG_DisaGridTile>("PG_DisaGridTile", item);

		disconnect(itemlist, &PG_DisaGridTile::clicked, itemlist, nullptr);
		disconnect(itemlist, &PG_DisaGridTile::rightClicked, itemlist, nullptr);
		itemlist->setLocation(location);
		connect(itemlist, &PG_DisaGridTile::clickedLocation, this, &PG_DisaItemSelector::selected);
		return itemlist;
		});
	listView->setItemUpdate(PG_GuiListView::getDefaultUpdateFunc<PG_DisaGridTile>());
	listView->setModel(m_model);

	if (getSize().x < 50.f) return; //scroll bar would take too much space
	PG_GuiScrollBar* scrollY = PG_new<PG_GuiScrollBar>("PG_GuiScrollBarY", menu);
	scrollY->setSize(glm::vec2(10.0f, 0.0f));
	scrollY->setScrollBarColors({
		PG_Color::hentaiBlueFrgb * 0.5f, PG_Color::hentaiBlueFrgb * 0.7f, PG_Color::hentaiBlueFrgb
		});
	if (auto rec = dynamic_cast<PG_GuiRectangle*>(scrollY->getHandle())) {
		rec->setMaterial("M_GuiRoundedRectangleAll5");
	}
	anchor(scrollY, PG_Anchor::Top, menu, PG_Anchor::Top);
	anchor(scrollY, PG_Anchor::Bottom, menu, PG_Anchor::Bottom);
	anchor(scrollY, PG_Anchor::Right, menu, PG_Anchor::Right);
	scrollY->setMargin(PG_Anchor::Top, 3.f);
	scrollY->setMargin(PG_Anchor::Bottom, 3.f);
	scrollY->setMargin(PG_Anchor::Right, 3.f);
	listView->setupVerticalScrollBar(scrollY);
}

void PG_DisaItemSelector::selected(PG_ListModelLocation loc) {
	close();
	if (m_tile->getLocation() == loc) return;

	m_selectedName = loc? loc.getDataT<std::string>(Identity_Edit): std::string();
	m_tile->setLocation(loc);
	emit selectedChanged();
}

std::string PG_DisaItemSelector::getName() const {
	return m_selectedName;
}

void PG_DisaItemSelector::setSelectedName(const std::string& name) {
	if (m_selectedName == name) return;
	m_selectedName = name;
	if (m_selectedName.empty()) clear();
	else postCallEvent(&PG_DisaItemSelector::findSelection);
}

void PG_DisaItemSelector::findSelection() {
	if (!m_model || m_selectedName.empty()) return;
	for (int i = 0; i < m_model->size(); ++i) {
		if (auto item = m_model->getData(i, Identity_Edit); item && item.get<std::string>() == m_selectedName) {
			if (m_tile->getLocation().getIndex() != i) {
				m_tile->setLocation(m_model->getLocation(i));
				emit selectedChanged();
			}
			return;
		}
	}
	m_tile->setLocation({});
}

void PG_DisaItemSelector::clear() {
	if (!m_tile->getLocation()) return;
	m_tile->setLocation({});
	emit selectedChanged();
}

static inline PG_DisaItemSelector* createSelector(const std::string& name, PG_GuiItem* root, PG_ListModelBase* model, PG_CategoryFilterModel::ECategory category) {
	auto item = PG_new<PG_DisaItemSelector>(name, glm::vec2(0.f), glm::vec2(200.f, 114.f), root);
	PG_CategoryFilterModel* filterModel = PG_new<PG_CategoryFilterModel>("FilterModel", item);
	filterModel->setFilterCategory(category);
	filterModel->setModel(model);
	item->setModel(filterModel);

	return item;
}

PG_DisaItemSelector* PG_DisaItemSelector::createImageSelector(const std::string& name, PG_GuiItem* root, PG_ListModelBase* model) {
	auto* item = createSelector(name, root, model, PG_CategoryFilterModel::Image);

	PG_Object::connect(item, &PG_DisaItemSelector::rightClicked, item, [item] {
		if (!item->getModel()) return;
		auto popup = PG_new<PG_GuiMenuPopup>("headerPopup", item);
		
		auto* model = item->getModel();
		if (auto* filterModel = dynamic_cast<PG_ListModelFilter*>(model))
			model = filterModel->getModel();

		if (auto func = model->getFunction("void importFile(const PG_File&)")) {
			popup->addOption("Import Image", item, [item, func, model] {

				if (PG_File result = PG_PopUpDialog::selectFile("png")) {
					func.callArgs(model, result);
					item->postCallEvent([item, name = "img/" + result.getFile()] {
						item->setSelectedName(name);
						});

				}

				});
		}
		popup->addOption("Clear", true, item, [item] { item->clear(); });
		popup->openAndDestroyOnClose();
		});
	return item;
}

PG_DisaItemSelector* PG_DisaItemSelector::createDungeonSelector(const std::string& name, PG_GuiItem* root, PG_ListModelBase* model) {
	auto* item = createSelector(name, root, model, PG_CategoryFilterModel::Dungeons);

	PG_Object::connect(item, &PG_DisaItemSelector::rightClicked, item, [item, model] {
		auto popup = PG_new<PG_GuiMenuPopup>("headerPopup", item);
		popup->addOption("Clear", true, item, [item] { item->clear(); });
		popup->openAndDestroyOnClose();
	});

	return item;
}