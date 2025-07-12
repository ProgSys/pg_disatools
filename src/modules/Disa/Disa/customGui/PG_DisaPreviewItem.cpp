#include "PG_DisaPreviewItem.h"
#include <Scene/Gui/PG_GuiRenderItem.h>
#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Gui/PG_GuiMouseArea.h>
#include <Scene/Gui/PG_GuiText.h>
#include <Scene/Gui/PG_GuiMenuPopup.h>
#include <Scene/Gui/PG_GuiImage.h>
#include <Renderer/PG_SceneRenderer.h>
#include <Util/Color/ColorConverters.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/fileformats/PG_PSPFS.h>

void PG_DisaPreviewItem::init() {
	PG_GuiItem::init();
	setName("PG_DisaPreviewItem");
	m_container = PG_new<PG_GuiItem>("Container", this);
	m_container->fill(this);

	constexpr glm::vec3 blueColor = glm::vec3(133.f, 208.f, 255.f) / 255.f;
	m_text = PG_new<PG_GuiText>("text", glm::vec2(0.f, 0.f), glm::vec2(110.f, 30.f),  this);
	anchor(m_text, PG_Anchor::Bottom, this, PG_Anchor::Bottom);
	anchor(m_text, PG_Anchor::Left, this, PG_Anchor::Left);
	anchor(m_text, PG_Anchor::Right, this, PG_Anchor::Right);
	m_text->setHorizontalAlignment(PG_Anchor::HorizontalCenter);
	m_text->setFontSize(21.f);
	m_text->setColor(blueColor);
	m_text->setMaterial(PG_Material("M_GuiTextBorderSDF"));
	m_text->setBaselineScale(0.3f);

	m_border = PG_new<PG_GuiRectangle>("DisaGridTileRec", this);
	m_border->fill(this);
	m_border->setAlpha(0.f);
	m_border->setBorderColor(PG_Color::hexToFloatColor(0xFF85D0FF));
	m_border->setBorderWidth(2.f);
	m_border->setVisible(false);
}

void PG_DisaPreviewItem::openPreview(const PG_Variety& var) {
	if (var.is<PG_DisaMPDPack>()) {
		open(var.get<PG_DisaMPDPack>());
		return;
	}
	if (var.is<PG_Texture>()) {
		open(var.get<PG_Texture>());
		return;
	}
	if (var.is<PG_DisaMPPPack>()) {
		open(var.get<PG_DisaMPPPack>());
		return;
	}
	if (var.is<std::string>()) {
		open(var.get<std::string>());
		return;
	}
	clear();
}

void PG_DisaPreviewItem::open(const PG_Texture& img) {
	PG_GuiImage* imgItem = m_container->findFirstChild<PG_GuiImage>();
	if (!imgItem) {
		m_container->clearGuiChildren();
		imgItem = PG_new<PG_GuiImage>("PreviewImage", m_container);
		imgItem->setFitType(PG_GuiImage::MentainAspectFit);
		imgItem->fill(m_container);
	}
	imgItem->setImage(img);
}

void PG_DisaPreviewItem::open(const std::string& imgPath) {
	PG_GuiImage* imgItem = m_container->findFirstChild<PG_GuiImage>();
	if (!imgItem) {
		m_container->clearGuiChildren();
		imgItem = PG_new<PG_GuiImage>("PreviewImage", m_container);
		imgItem->setFitType(PG_GuiImage::MentainAspectFit);
		imgItem->fill(m_container);
	}
	imgItem->setImage(imgPath);
}

void PG_DisaPreviewItem::clear() {
	m_container->clearGuiChildren();
	if (m_text) m_text->setText({});
}

void PG_DisaPreviewItem::createGameScene() {
	clear();
	return;
	/*
	PG_GuiRenderItem* renderItem = m_container->findFirstChild<PG_GuiRenderItem>();
	PG_SceneRenderer* sceneRenderer = nullptr;
	if (!renderItem) {
		m_container->clearGuiChildren();
		renderItem = PG_new<PG_GuiRenderItem>("GridRenderItem", m_container);
		renderItem->fill(m_container);
		renderItem->setFreez(true);

		auto area = findFirstChild<PG_GuiMouseArea>();
		connect(area, &PG_GuiMouseArea::hoveredChanged, renderItem, [renderItem](bool hover) {
			renderItem->setFreez(!hover);
			});

	}

	sceneRenderer = renderItem->findFirstChild<PG_SceneRenderer>();
	if (!sceneRenderer) {
		sceneRenderer = PG_new<PG_SceneRenderer>("GameSceneRenderer", true, renderItem);
	}

	if (!m_gameScene) {
		m_gameScene = PG_new<PG_DisaMapSceneView>("GameScene", sceneRenderer);
		m_gameScene->setPreviewMode(true);
		m_gameScene->setActorsVisible(false);
		m_gameScene->setGeoPanelsVisible(false);

		connect(m_gameScene, &PG_DisaMapViewNode::onDeleted, this, [this] {
			m_gameScene = nullptr;
			});
		connect(m_gameScene->getMapNode(), &PG_DisaMapViewNode::loaded, renderItem, &PG_GuiRenderItem::forceRenderNext);
	}
	renderItem->forceRenderNext();
	assert(m_gameScene);
	*/
}

void PG_DisaPreviewItem::open(const PG_DisaMPDPack& mapFiles) {

	auto mpd = mapFiles.getMPD();
	auto mppSD = mapFiles.getMPPSD();

	if (!(mpd && (mpd.getInfo().getType() == PG_DisaFileInfo::MPD) && mppSD)) {
		OUT_ERROR("Failed to find MPD or MPP!");
		return;
	}

	createGameScene();

	//m_gameScene->setModel(PG_DisaMapContainerModel::open(mpd, mppSD, mapFiles.getMPPHD()));
}


void PG_DisaPreviewItem::open(const PG_DisaMPPPack& mppPack) {
	auto mpp = mppPack.getMPP();

	if (!(mpp && (mpp.getInfo().getType() == PG_DisaFileInfo::MPP))) {
		OUT_ERROR("Failed to find MPP!");
		return;
	}

	createGameScene();
	auto style = mppPack.getStyle();

	//m_gameScene->setModel(PG_DisaMapContainerModel::open(mppPack.getMPP(), style.topIndex, style.sideIndex, style.decalIndex));
}

void PG_DisaPreviewItem::setLocation(const PG_ListModelLocation& location) {
	if (m_location == location) return;
	m_location = location;
	if (!m_location) {
		clear();
		return;
	}

	if (m_text) m_text->setText(location.getDataT<std::string>(Identity_Text));

	openPreview(location.getData(Identity_PreviewImage));
	emit locationChanged();
}

void PG_DisaPreviewItem::update(const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
	const bool hasItem = identities.count(Identity_Item) != 0 || m_location != location;
	if (hasItem || identities.count(Identity_Location)) {
		m_location = location;
	}

	if (!location) return;
	if (hasItem || identities.count(Identity_Text)) {
		if (m_text) m_text->setText(location.getDataT<std::string>(Identity_Text));
	}

	if (hasItem || identities.count(Identity_PreviewImage)) {
		openPreview(location.getData(Identity_PreviewImage));
	}
}