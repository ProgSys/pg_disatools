#include "PG_DisaGridTile.h"
#include <Scene/Gui/PG_GuiRenderItem.h>
#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Gui/PG_GuiMouseArea.h>
#include <Scene/Gui/PG_GuiText.h>
#include <Scene/Gui/PG_GuiMenuPopup.h>
#include <Scene/Gui/PG_GuiImage.h>
#include <Renderer/PG_SceneRenderer.h>
#include <Util/Color/ColorConverters.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <Models/PG_DisaMapContainerModel.h>
#include <Models/PG_UiStateCustomCampaignsModel.h>
#include <Views/PG_DisaMapSceneView.h>
#include <PG_DisaMapListModel.h>
#include <RootScene.h>
#include <Disa/utils/PG_SaveTexture.h>

#include <Disa/fileformats/PG_PSPFS.h>



void PG_DisaGridTile::init(){
	PG_GuiItem::init();
	setName("PG_DisaGridTile");
	m_container = PG_new<PG_GuiItem>("Container", this);
	m_container->fill(this);

	constexpr glm::vec3 blueColor = glm::vec3(133.f, 208.f, 255.f) / 255.f;
	m_text = PG_new<PG_GuiText>("MapName", glm::vec2(0.f, 0.f), glm::vec2(110.f, 30.f), "Name", this);
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

	PG_GuiMouseArea* area = PG_new<PG_GuiMouseArea>("DisaGridTileArea", this);
	area->setHoverEnabled(true);
	area->setConferEnabled(true);
	area->setAcceptedButtons(PG_GuiMouseArea::Left | PG_GuiMouseArea::Right);
	area->fill(this);

	connect(area, &PG_GuiMouseArea::clicked, this, [this] {
		emit clicked();
		emit clickedLocation(m_location);
		});
	connect(area, &PG_GuiMouseArea::rightClicked, this, &PG_DisaGridTile::rightClicked);

	connect(this, &PG_DisaGridTile::clicked, this, [this]() {
		auto model = m_location.getModelRoot();
		if (!model) return;
		if (m_gameScene) {
			auto func = model->getFunction("void tileClicked(PG_ListModelLocation,PG_CameraNode*)");
			if (func) {
				func.callArgs(model, getLocation(), m_gameScene->getActiveCamera());
				return;
			}
		}

		auto func = model->getFunction("void tileClicked(PG_ListModelLocation)");
		if (!func) return;
		func.callArgs(model, getLocation());
		});

	connect(this, &PG_DisaGridTile::rightClicked, this, [this] {
		auto rootModel = m_location.getModelRoot();
		if (auto mapsModel = dynamic_cast<PG_DisaMapListModel*>(rootModel); mapsModel && m_location) {
			//TODO depricated/dosn't work
			auto popup = PG_new<PG_GuiMenuPopup>("headerPopup", this);
			popup->addOption("Export MPD", this, [this, mapsModel] { mapsModel->exportData(m_location); });
			popup->addOption("Export Json", true, this, [this, mapsModel] { mapsModel->exportJSON(m_location); });
			popup->addOption("", false);
			popup->addOption("Import MPD", this, [this, mapsModel] { mapsModel->importData(m_location); });
			popup->addOption("Import Json", true, this, [this, mapsModel] { mapsModel->importJSON(m_location); });
			popup->openAndDestroyOnClose();
		}
		else if (dynamic_cast<PG_UiStateCustomCampaignsModel*>(rootModel)) {
			auto* model = m_location.getModel();
			auto popup = PG_new<PG_GuiMenuPopup>("headerPopup", this);
			popup->addOption("Remove", model, [model, i = m_location.getIndex()] {
				if (auto func = model->getFunction("void customRemove(std::size_t)")) {
					func.callArgs(model, std::size_t(i));
					return;
				}
				model->remove(i); 
				});
			popup->openAndDestroyOnClose();
		}
		else if (PG_GuiImage* imgItem = m_container->findFirstChild<PG_GuiImage>()) {
			if (!imgItem->getImage()) return;
			auto popup = PG_new<PG_GuiMenuPopup>("headerPopup", this);
			popup->addOption("Export Image", this, [this, imgItem] { 
				PG_FileDialog::save(imgItem->getImage(), m_location.getDataT<std::string>(EIdentity::PG_Identity_Text));
			});
			popup->openAndDestroyOnClose();
		}
	});
}

void PG_DisaGridTile::openPreview(const PG_Variety& var) {
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
	m_container->clearGuiChildren();
}

void PG_DisaGridTile::open(const PG_Texture& img) {
	PG_GuiImage* imgItem = m_container->findFirstChild<PG_GuiImage>();
	if (!imgItem) {
		m_container->clearGuiChildren();
		imgItem = PG_new<PG_GuiImage>("PreviewImage", m_container);
		imgItem->fill(m_container);
	}
	imgItem->setImage(img);
	imgItem->setFitType(PG_GuiImage::MentainAspect);
}

void PG_DisaGridTile::open(const std::string& imgPath) {
	PG_GuiImage* imgItem = m_container->findFirstChild<PG_GuiImage>();
	if (!imgItem) {
		m_container->clearGuiChildren();
		imgItem = PG_new<PG_GuiImage>("PreviewImage", m_container);
		imgItem->fill(m_container);
	}
	imgItem->setImage(imgPath);
	imgItem->setFitType(PG_GuiImage::MentainAspectFit);
}

void PG_DisaGridTile::createGameScene() {
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
}

void PG_DisaGridTile::open(const PG_DisaMPDPack& mapFiles) {

	auto mpd = mapFiles.getMPD();
	auto mppSD = mapFiles.getMPPSD();

	if (!(mpd && (mpd.getInfo().getType() == PG_DisaFileInfo::MPD) && mppSD)) {
		OUT_ERROR("Failed to find MPD or MPP!");
		return;
	}

	createGameScene();

	m_gameScene->setModel(PG_DisaMapContainerModel::open(mpd, mppSD, mapFiles.getMPPHD()));
}


void PG_DisaGridTile::open(const PG_DisaMPPPack& mppPack) {
	auto mpp = mppPack.getMPP();

	if (!(mpp && (mpp.getInfo().getType() == PG_DisaFileInfo::MPP))) {
		OUT_ERROR("Failed to find MPP!");
		return;
	}

	createGameScene();
	auto style = mppPack.getStyle();

	m_gameScene->setModel(PG_DisaMapContainerModel::open(mppPack.getMPP(), style.topIndex, style.sideIndex, style.decalIndex));
}

void PG_DisaGridTile::setLocation(const PG_ListModelLocation& location) {
	m_location = location;
	if (!m_location) {
		m_container->clearGuiChildren();
		if (m_text) m_text->setText(std::string());
		return;
	}

	if (m_text) m_text->setText(location.getDataT<std::string>(PG_Identity_Text));

	auto area = findFirstChild<PG_GuiMouseArea>("DisaGridTileArea");
	if (m_location.getData(PG_Identity_Selected)) {
		connect(area, &PG_GuiMouseArea::hoveredChanged, this, &PG_DisaGridTile::updateBorder);
		connect(this, &PG_DisaGridTile::selectionChanged, this, &PG_DisaGridTile::updateBorder);
		connect(area, &PG_GuiMouseArea::clicked, this, [this]() {
			m_location.setData(true, PG_Identity_Selected);
			});
	}
	else {
		connect(area, &PG_GuiMouseArea::hoveredChanged, m_border, &PG_GuiRectangle::setVisible);
	}

	openPreview(location.getData(PG_Identity_PreviewImage));
}

void PG_DisaGridTile::update(const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
	const bool hasItem = identities.count(PG_Identity_Item) != 0 || m_location != location;
	if (hasItem || identities.count(PG_Identity_Location)) {
		m_location = location;
	}

	if (!location) return;
	if (hasItem || identities.count(PG_Identity_Text)) {
		if (m_text) m_text->setText(location.getDataT<std::string>(PG_Identity_Text));
	}

	if (hasItem || identities.count(PG_Identity_PreviewImage)) {
		openPreview(location.getData(PG_Identity_PreviewImage));
	}

	if (hasItem || identities.count(PG_Identity_Selected)) {
		emit selectionChanged();
	}
}

void PG_DisaGridTile::updateBorder() {
	auto area = findFirstChild<PG_GuiMouseArea>("DisaGridTileArea");
	const bool display = area->isHovered() || m_location.getDataT<bool>(PG_Identity_Selected);
	m_border->setVisible(display);
}