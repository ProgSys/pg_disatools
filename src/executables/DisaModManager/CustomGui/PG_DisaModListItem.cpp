#include "PG_DisaModListItem.h"
#include <Util/Color/ColorDefined.h>
#include <Models/PG_DisaModListModel.h>
#include <Scene/Gui/PG_GuiImage.h>
#include <Scene/Gui/PG_GuiText.h>
#include <Scene/Gui/PG_GuiRow.h>
#include <Scene/Gui/PG_GuiButton.h>
#include <Scene/Gui/PG_GuiToggleButton.h>
#include <Disa/customGui/PG_PupUpDialog.h>
#include <Resources/PG_ResourceSynchronisator.h>

static inline void callFunc(PG_ListModelLocation loc, const std::string& name) {
	if (!loc) return;
	auto func = loc.getModel()->getFunction(name);
	if (!func) return;
	func.callArgs(loc.getModel(), loc);
}

void PG_DisaModListItem::init() {
	PG_GuiItem::init();

	setColor(PG_Color::blackGrayFrgb);
	setMaterial("M_GuiRoundedRectangleAll5");
	setClip(true);

	m_img = PG_new<PG_GuiImage>("Thumbnail", this);
	anchor(m_img, PG_Anchor::Top, this, PG_Anchor::Top);
	anchor(m_img, PG_Anchor::Bottom, this, PG_Anchor::Bottom);
	anchor(m_img, PG_Anchor::Left, this, PG_Anchor::Left);
	connect(m_img, &PG_GuiImage::midGuiUpdate, this, [this](glm::vec2* pos, glm::vec2* scale){
		scale->x = getSize().y * 1.5f;
	});

	auto seperator = PG_new<PG_GuiRectangle>("seperator", this);
	anchor(seperator, PG_Anchor::Top, this, PG_Anchor::Top);
	anchor(seperator, PG_Anchor::Bottom, this, PG_Anchor::Bottom);
	anchor(seperator, PG_Anchor::Left, m_img, PG_Anchor::Right);
	seperator->setColor(PG_Color::hentaiBlueFrgb);
	seperator->setWidth(4.f);

	constexpr float leftMargin = 15.f;
	m_title = PG_new<PG_GuiText>("title", this);
	anchor(m_title, PG_Anchor::Top, this, PG_Anchor::Top);
	anchor(m_title, PG_Anchor::Left, m_img, PG_Anchor::Right);
	anchor(m_title, PG_Anchor::Right, this, PG_Anchor::Right);
	m_title->setWrapMode(PG_GuiText::None);
	m_title->setHeight(32.f);
	m_title->setFontSize(32.f);
	m_title->setColor(PG_Color::hentaiBlueFrgb);
	m_title->setMargin(PG_Anchor::Top, 5.f);
	m_title->setMargin(PG_Anchor::Left, leftMargin);

	m_author = PG_new<PG_GuiText>("author", this);
	anchor(m_author, PG_Anchor::Top, m_title, PG_Anchor::Bottom);
	anchor(m_author, PG_Anchor::Left, m_img, PG_Anchor::Right);
	anchor(m_author, PG_Anchor::Right, this, PG_Anchor::Right);
	m_author->setWrapMode(PG_GuiText::None);
	m_author->setHeight(18.f);
	m_author->setFontSize(18.f);
	m_author->setColor(PG_Color::hentaiBlueFrgb);
	m_author->setMargin(PG_Anchor::Top, 3.f);
	m_author->setMargin(PG_Anchor::Left, leftMargin);

	m_decription = PG_new<PG_GuiText>("decription", this);
	anchor(m_decription, PG_Anchor::Top, m_author, PG_Anchor::Bottom);
	anchor(m_decription, PG_Anchor::Bottom, this, PG_Anchor::Bottom);
	anchor(m_decription, PG_Anchor::Left, m_img, PG_Anchor::Right);
	anchor(m_decription, PG_Anchor::Right, this, PG_Anchor::Right);
	m_decription->setFontSize(24.f);
	m_decription->setColor(PG_Color::hentaiBlueFrgb);
	m_decription->setClip(true);
	m_decription->setMargin(PG_Anchor::Top, 10.f);
	m_decription->setMargin(PG_Anchor::Left, leftMargin);

	auto toolbarRow = PG_new<PG_GuiRow>("ToolbarRow", glm::vec2(0.f), glm::vec2(156.f + 10.f, 30.f), this);
	anchor(toolbarRow, PG_Anchor::Top, this, PG_Anchor::Top);
	anchor(toolbarRow, PG_Anchor::Right, this, PG_Anchor::Right);


	{
		auto btn = PG_new<PG_GuiButton>("DeleteButton", glm::vec2(3.f, 0.f), glm::vec2(39.f, 30.f), toolbarRow);
		btn->setIcon("gui/icon_delete.png");
		PG_Material mat("M_GuiToolButtonStart");
		mat.makeInstance();
		mat.set("radiusTopLeft", 0.f);
		btn->setBackground(mat);

		btn->setTooltipText("Delete");
		btn->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 1.f);

		connect(btn, &PG_GuiButton::clicked, this, [this] {
			auto* popup = PG_PopUpDialog::createConfirm(this, "Delete mod?", "Do you really want to delete the mod \"" + m_location.getDataT<std::string>(PG_Identity_Text) + "\"? ");
			connect(popup, &PG_PopUpDialog::confirmClicked, this, [this] {
				callFunc(m_location, "void deleteMod(PG_ListModelLocation)");
				});
			popup->open();
			});
	}

	{
		auto btn = PG_new<PG_GuiToggleButton>("DisableButton", glm::vec2(3.f, 0.f), glm::vec2(39.f, 30.f), toolbarRow);
		btn->setIcon("gui/disa/icon_disabled.png");
		btn->setBackground("M_GuiToolButton");
		btn->setTooltipText("Disable");
		btn->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 1.f);
		m_disableBtn = btn;

		connect(btn, &PG_GuiButton::clicked, this, [this] {
			callFunc(m_location, "void disableMod(PG_ListModelLocation)");
			});
	}


	{
		auto btn = PG_new<PG_GuiButton>("MoveUpButton", glm::vec2(3.f, 0.f), glm::vec2(39.f, 30.f), toolbarRow);
		btn->setIcon("gui/disa/icon_up.png");
		btn->setBackground("M_GuiToolButton");
		btn->setTooltipText("Move up");
		btn->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 1.f);

		connect(btn, &PG_GuiButton::clicked, this, [this] {
			callFunc(m_location, "void moveUp(PG_ListModelLocation)");
			});
	}

	{
		auto btn = PG_new<PG_GuiButton>("MoveDownButton", glm::vec2(3.f, 0.f), glm::vec2(45.f, 30.f), toolbarRow);
		btn->setIcon("gui/disa/icon_down.png");
		btn->setBackground("M_GuiToolButton");
		btn->setTooltipText("Move down");
		btn->setLayoutProperty(PG_GuiItem::LayoutProperty_Spacing, 1.f);

		connect(btn, &PG_GuiButton::clicked, this, [this] {
			callFunc(m_location, "void moveDown(PG_ListModelLocation)");
			});
	}
}

void PG_DisaModListItem::setLocation(const PG_ListModelLocation& location) {
	if (location == m_location) return;
	m_location = location;
	
	setColor((location.getIndex() % 2 == 0? 0.9f: 1.0f) * PG_Color::blackGrayFrgb);
	updateImage();
	m_title->setText(location.getDataT<std::string>(PG_Identity_Text));
	updateAutor();
	m_decription->setText(location.getDataT<std::string>(PG_DisaModListModel::Identity_Description));
	m_disableBtn->setToggled(location.getDataT<bool>(PG_DisaModListModel::Identity_Disabled));
}

void PG_DisaModListItem::update(const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
	const bool hasItem = identities.count(PG_Identity_Item) != 0 || m_location != location;
	if (hasItem || identities.count(PG_Identity_Location)) {
		m_location = location;
		setColor((location.getIndex() % 2 == 0 ? 0.8f : 1.0f) * PG_Color::blackGrayFrgb);
	}
	if (hasItem || identities.count(PG_Identity_PreviewImage)) {
		updateImage();
	}
	if (hasItem || identities.count(PG_Identity_Text)) {
		m_title->setText(location.getDataT<std::string>(PG_Identity_Text));
	}
	if (hasItem || identities.count(PG_DisaModListModel::Identity_Author)) {
		updateAutor();
	}
	if (hasItem || identities.count(PG_DisaModListModel::Identity_Description)) {
		m_decription->setText(location.getDataT<std::string>(PG_DisaModListModel::Identity_Description));
	}

	if (hasItem || identities.count(PG_DisaModListModel::Identity_Disabled)) {
		m_disableBtn->setToggled(location.getDataT<bool>(PG_DisaModListModel::Identity_Disabled));
	}
}
void PG_DisaModListItem::updateAutor() {
	auto test = m_location.getDataT<std::string>(PG_DisaModListModel::Identity_Author);
	if (test.empty()) {
		m_author->setText(std::string());
		m_author->setHeight(0.f);
	}
	else {
		m_author->setText("by " + test);
		m_author->setHeight(m_author->getFontSize());
	}
}

void PG_DisaModListItem::updateImage() {
	constexpr auto missingImg = "gui/disa/pg_img_missing_thumbnail.png";
	if (!m_location) {
		m_img->setSource(missingImg);
		return;
	}
	auto img = m_location.getDataT<PG_Texture>(PG_Identity_PreviewImage);
	if (!img.isValid()) {
		m_img->setSource(missingImg);
		return;
	}

	if (img.isLoaded()) {
		m_img->setSourceData(img);
		return;
	}
	m_img->setSource(missingImg);
	auto* fence = PG_new<PG_ResourceSynchronisator>("PG_ResourceSynchronisator", this);
	fence->add(img);
	connect(fence, &PG_ResourceSynchronisator::loaded, this, [this, image = std::move(img)] {
		m_img->setSourceData(image);
		});
	fence->start();
}