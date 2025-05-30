#include "PG_DisaModListItem.h"
#include <Util/Color/ColorDefined.h>
#include <Models/PG_DisaModListModel.h>
#include <Scene/Gui/PG_GuiImage.h>
#include <Scene/Gui/PG_GuiText.h>

void PG_DisaModListItem::init() {
	PG_GuiItem::init();

	setColor(PG_Color::grayFrgb);

	m_img = PG_new<PG_GuiImage>("Thumbnail", this);
	anchor(m_img, PG_Anchor::Top, this, PG_Anchor::Top);
	anchor(m_img, PG_Anchor::Bottom, this, PG_Anchor::Bottom);
	anchor(m_img, PG_Anchor::Left, this, PG_Anchor::Left);
	connect(m_img, &PG_GuiImage::midGuiUpdate, this, [this](glm::vec2* pos, glm::vec2* scale){
		scale->x = getSize().y * 1.3f;
	});


	m_title = PG_new<PG_GuiText>("title", this);
	anchor(m_title, PG_Anchor::Top, this, PG_Anchor::Top);
	anchor(m_title, PG_Anchor::Left, m_img, PG_Anchor::Right);
	anchor(m_title, PG_Anchor::Right, this, PG_Anchor::Right);
	m_title->setWrapMode(PG_GuiText::None);
	m_title->setHeight(32.f);
	m_title->setFontSize(32.f);
	m_title->setColor(PG_Color::hentaiBlueFrgb);

	m_author = PG_new<PG_GuiText>("author", this);
	anchor(m_author, PG_Anchor::Top, m_title, PG_Anchor::Bottom);
	anchor(m_author, PG_Anchor::Left, m_img, PG_Anchor::Right);
	anchor(m_author, PG_Anchor::Right, this, PG_Anchor::Right);
	m_author->setWrapMode(PG_GuiText::None);
	m_author->setHeight(22.f);
	m_author->setFontSize(22.f);
	m_author->setColor(PG_Color::hentaiBlueFrgb);

	m_decription = PG_new<PG_GuiText>("decription", this);
	anchor(m_decription, PG_Anchor::Top, m_author, PG_Anchor::Bottom);
	anchor(m_decription, PG_Anchor::Bottom, this, PG_Anchor::Bottom);
	anchor(m_decription, PG_Anchor::Left, m_img, PG_Anchor::Right);
	anchor(m_decription, PG_Anchor::Right, this, PG_Anchor::Right);
	m_decription->setFontSize(24.f);
	m_decription->setColor(PG_Color::hentaiBlueFrgb);
	m_decription->setClip(true);
}

void PG_DisaModListItem::setLocation(const PG_ListModelLocation& location) {
	if (location == m_location) return;
	m_location = location;
	
	m_img->setImage(location.getDataT<PG_Texture>(Identity_PreviewImage));
	m_title->setText(location.getDataT<std::string>(Identity_Text));
	updateAutor();
	m_decription->setText(location.getDataT<std::string>(PG_DisaModListModel::Identity_Description));
}

void PG_DisaModListItem::update(const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
	const bool hasItem = identities.count(Identity_Item) != 0 || m_location != location;
	if (hasItem || identities.count(Identity_Location)) {
		m_location = location;
	}
	if (hasItem || identities.count(Identity_PreviewImage)) {
		m_img->setImage(location.getDataT<PG_Texture>(Identity_PreviewImage));
	}
	if (hasItem || identities.count(Identity_Text)) {
		m_title->setText(location.getDataT<std::string>(Identity_Text));
	}
	if (hasItem || identities.count(PG_DisaModListModel::Identity_Author)) {
		updateAutor();
	}
	if (hasItem || identities.count(PG_DisaModListModel::Identity_Description)) {
		m_decription->setText(location.getDataT<std::string>(PG_DisaModListModel::Identity_Description));
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