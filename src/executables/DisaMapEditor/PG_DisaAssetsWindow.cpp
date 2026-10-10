#include "PG_DisaAssetsWindow.h"
#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Gui/PG_GuiListView.h>
#include <Scene/Gui/PG_GuiButton.h>
#include <Scene/Gui/PG_GuiToggleButton.h>
#include <Scene/Gui/PG_GuiImage.h>
#include <Scene/Gui/PG_GuiText.h>
#include <Scene/Gui/PG_GuiRenderItem.h>
#include <Scene/Gui/PG_GuiMenuPopup.h>
#include <Scene/Model/PG_ListModelFilter.h>
#include <Renderer/PG_SceneRenderer.h>
#include <Util/Color/ColorConverters.h>
#include <Util/Color/ColorDefined.h>
#include <Disa/fileformats/PG_TX2.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/PG_DisaAssetsModel.h>
#include <Disa/PG_DisaModelNode.h>
#include <Disa/fileformats/PG_DisaFileContainer.h>
#include <Disa/CustomGui/PG_PupUpDialog.h>
#include <Disa/utils/PG_FileDialog.h>
#include <Disa/PG_DisaMapScene.h>
#include <Models/PG_DisaMapEditorMainModel.h>

constexpr glm::vec3 backgroundDarkColor = PG_Color::rgbToF({25,25,30});
constexpr glm::vec3 backgroundColor = PG_Color::rgbToF(glm::rgba(40));
constexpr glm::vec3 blueColor = PG_Color::rgbToF({133, 208, 255});
constexpr glm::vec3 brightColor = PG_Color::rgbToF(glm::rgba(60));


void PG_DisaAssetsListItem::init() {
	PG_GuiItem::init();
	
	auto bgLeft = PG_new<PG_GuiRectangle>("bgLeft", this);
	bgLeft->setColor(backgroundColor);
	bgLeft->setWidth(50);
	bgLeft->setMaterial("M_GuiToolButtonStart");
	bgLeft->setMargin({2,2,0,0});
	anchor(bgLeft, PG_Anchor::Left, this, PG_Anchor::Left);
	anchor(bgLeft, PG_Anchor::Top, this, PG_Anchor::Top);
	anchor(bgLeft, PG_Anchor::Bottom, this, PG_Anchor::Bottom);

	auto bgRight = PG_new<PG_GuiRectangle>("bgRight", this);
	bgRight->setPosition(glm::vec2(4.f, 0.f));
	bgRight->setColor(backgroundColor);
	bgRight->setMaterial("M_GuiToolButtonEnd");
	bgRight->setMargin({ 2,2,0,0 });
	anchor(bgRight, PG_Anchor::Left, bgLeft, PG_Anchor::Right);
	anchor(bgRight, PG_Anchor::Right, this, PG_Anchor::Right);
	anchor(bgRight, PG_Anchor::Top, this, PG_Anchor::Top);
	anchor(bgRight, PG_Anchor::Bottom, this, PG_Anchor::Bottom);
	m_rightItem = bgRight;

	auto buttonGroup = PG_new<PG_GuiItem>("buttonGroup", bgLeft);
	buttonGroup->setSize(glm::vec2(30.f, 72.f));
	anchor(buttonGroup, PG_Anchor::HorizontalCenter, bgLeft, PG_Anchor::HorizontalCenter);
	anchor(buttonGroup, PG_Anchor::VerticalCenter, bgLeft, PG_Anchor::VerticalCenter);

	m_topBtn = PG_new<PG_GuiButton>("TopBtn", glm::vec2(0.f, 0.f), glm::vec2(30.f, 34.f), buttonGroup);
	m_topBtn->setText("0");
	m_topBtn->setBackground("M_GuiRoundedRectangleTop5");
	m_topBtn->setTooltipText("More");
	//PG_GuiItem::anchor(m_topBtn, PG_Anchor::Top, buttonGroup, PG_Anchor::Top);

	auto bottomBtn = PG_new<PG_GuiButton>("TopBtn", glm::vec2(0.f, 0.f), glm::vec2(30.f, 34.f), buttonGroup);
	bottomBtn->setIcon("gui/icon_brush_draw.png");
	bottomBtn->setBackground("M_GuiRoundedRectangleBottom5");
	bottomBtn->setTooltipText("Apply");
	PG_GuiItem::anchor(bottomBtn, PG_Anchor::Bottom, buttonGroup, PG_Anchor::Bottom);
	connect(bottomBtn, &PG_GuiButton::clicked, this, [this]() {
		if (!m_location) return;
		if (m_isTexture) {
			emit PG_DisaMapEditorMainModel::get()->paintSurfacesPressed(m_location.getIndex());
		}
		else {
			emit PG_DisaMapEditorMainModel::get()->addModelPressed(m_location.getIndex());
		}
	});
}

void PG_DisaAssetsListItem::setLocation(const PG_ListModelLocation& location) {
	m_location = location;
	m_topBtn->setText(std::to_string(location.getIndex()));

	updateCenter();
}

void PG_DisaAssetsListItem::update(const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
	const bool hasItem = identities.count(PG_Identity_Item) != 0 || m_location != location;
	if (hasItem || identities.count(PG_Identity_Location)) {
		m_location = location;
		m_topBtn->setText(std::to_string(location.getIndex()));
	}

	if (hasItem || contains<PG_DisaAssetsTextureItem::Identity_SDModified, PG_DisaAssetsTextureItem::Identity_HDModified, PG_DisaAssetsTextureItem::Identity_HDNormalModified>(identities)) {
		updateCenter();
	}
}

static inline void setImage(PG_GuiImage* guiImage, const PG_DisaFileEntry& entry) {
	auto info = entry.getInfo();
	if (info.getType() != PG_DisaFileInfo::TX2) return;
	auto reader = entry.createReader();
	if (!reader) return;

	auto loadFunc = [reader]() mutable->PG_Texture::ImageFormats {
		PG_TX2 tx2;
		const bool result = tx2.open(reader.get());
		reader.reset();
		if (result) {
			PG_RGBAImage image;
			if (!decompressTX2(tx2, image)) {
				return image;
			}
			else {
				OUT_ERROR("Failed to decompress 'tx2'!");
			}
		}
		else {
			OUT_ERROR("Failed to read 'tx2'!");
		}
		
		return PG_RGBAImage();
		};
	guiImage->setSourceData(PG_Texture(PG_UNIQUE_RES("DisaResource"), loadFunc));
}

static inline std::pair<PG_GuiRenderItem*, PG_DisaMapScene*> createMapScene(PG_GuiItem* parent) {

	auto renderWindow = PG_new<PG_GuiRenderItem>("GridRenderItem", parent);
	auto sceneRenderer = PG_new<PG_SceneRenderer>("GameSceneRenderer", true, renderWindow);
	auto gameScene = PG_new<PG_DisaMapScene>("GameScene", sceneRenderer);

	return { renderWindow, gameScene };
}

static inline void addSeperator(float tileBgSize, PG_GuiItem* leftItem, PG_GuiItem* parent) {
	auto bgSep = PG_new<PG_GuiRectangle>("seperator", glm::vec2(48.f, 0.f), glm::vec2(4.f, tileBgSize), parent);
	bgSep->setColor(brightColor);
	PG_GuiItem::anchor(bgSep, PG_Anchor::VerticalCenter, parent, PG_Anchor::VerticalCenter);
	PG_GuiItem::anchor(bgSep, PG_Anchor::Left, leftItem, PG_Anchor::Right);

	auto textSD = PG_new<PG_GuiText>("textSD", glm::vec2(-5.f, 0.f), glm::vec2(30.f, 100.f), parent);
	textSD->setColor(brightColor);
	textSD->setWrapMode(PG_GuiText::None);
	textSD->setHorizontalAlignment(PG_Anchor::HorizontalCenter);
	textSD->setVerticalAlignment(PG_Anchor::VerticalCenter);
	textSD->setFontSize(20.f);
	textSD->setText("SD");
	PG_GuiItem::anchor(textSD, PG_Anchor::Right, bgSep, PG_Anchor::Left);
	PG_GuiItem::anchor(textSD, PG_Anchor::VerticalCenter, bgSep, PG_Anchor::VerticalCenter);

	auto textHD = PG_new<PG_GuiText>("textHD", glm::vec2(5.f, 0.f), glm::vec2(30.f, 100.f), parent);
	textHD->setColor(brightColor);
	textHD->setWrapMode(PG_GuiText::None);
	textHD->setHorizontalAlignment(PG_Anchor::HorizontalCenter);
	textHD->setVerticalAlignment(PG_Anchor::VerticalCenter);
	textHD->setFontSize(20.f);
	textHD->setText("HD");
	PG_GuiItem::anchor(textHD, PG_Anchor::Left, bgSep, PG_Anchor::Right);
	PG_GuiItem::anchor(textHD, PG_Anchor::VerticalCenter, bgSep, PG_Anchor::VerticalCenter);
}

static inline void exportTexture(PG_DisaAssetsListItem* owner, PG_ListModelLocation location, PG_DisaAssetsModel::ETextureType type) {
	if (!location) return;
	auto* model = location.getModel()->getParent<PG_DisaAssetsModel>();
	if (!model) return;
	std::string name;
	switch (type)
	{
	case PG_DisaAssetsModel::TextureSD: name = "textureSD"; break;
	case PG_DisaAssetsModel::TextureHD: name = "textureHD"; break;
	case PG_DisaAssetsModel::TextureNormal: name = "textureNormal"; break;
	default: return;
	}

	name += std::to_string(location.getIndex());

	PG_File file = PG_FileDialog::save(owner, "TX2;PNG", name);
	if (file) {
		model->exportTexture(file, location, type);
	}
}

static inline void importTexture(PG_DisaAssetsListItem* owner, PG_ListModelLocation location, PG_DisaAssetsModel::ETextureType type) {
	if (!location) return;
	auto* model = location.getModel()->getParent<PG_DisaAssetsModel>();
	if (!model) return;

	PG_File file = PG_FileDialog::open(owner, "TX2;PNG");
	if (!file) return;
	auto extension = file.getFileExtensionFormated();
	if (extension == "png") {
		if (auto* popup = PG_PopUpDialog::selectCompression(owner)) {
			PG_GuiItem::connect(popup, &PG_PopUpDialog::confirmClickedIndex, owner, [model, file = std::move(file), location, type](int i) {
				PG_TX2::Type tx2Type = PG_TX2::TX2ERROR;
				switch (i)
				{
				case 0: tx2Type = PG_TX2::BGRA; break;
				case 1: tx2Type = PG_TX2::DXT1; break;
				case 2: tx2Type = PG_TX2::DXT5; break;
				default:
					break;
				}
				if (tx2Type == PG_TX2::TX2ERROR) return;
				model->importTexture(file, location, type, tx2Type);

				});
			popup->open();
		}
		return;
	}
	
	model->importTexture(file, location, type);
	
}

static inline void addTextureOptions(PG_DisaAssetsListItem* owner, PG_ListModelLocation location, PG_DisaAssetsModel::ETextureType type, const PG_MouseButtonEvent* e) {
	if (!location) return;
	auto popup = PG_new<PG_GuiMenuPopup>("headerPopup", owner);
	popup->addOption("Export", owner, [owner, location, type] { exportTexture(owner, location, type); });
	popup->addOption("Import", true, owner, [owner, location, type] { importTexture(owner, location, type); });
	popup->openAndDestroyOnClose(e);
}


static inline void exportGeoemtry(PG_DisaAssetsListItem* owner, PG_ListModelLocation location, bool hd) {
	if (!location) return;
	auto* model = location.getModel()->getParent<PG_DisaAssetsModel>();
	if (!model) return;
	std::string name = hd ? "geometryHD" : "geometrySD";
	name += std::to_string(location.getIndex());

	PG_File file = PG_FileDialog::save(owner, "GEO", name);
	if (file) {
		model->exportGeometry(file, location, hd);
	}
}

static inline void importGeoemtry(PG_DisaAssetsListItem* owner, PG_ListModelLocation location, bool hd) {
	if (!location) return;
	auto* model = location.getModel()->getParent<PG_DisaAssetsModel>();
	if (!model) return;

	PG_File file = PG_FileDialog::open(owner, "GEO");
	if (file) {
		model->importGeometry(file, location, hd);
	}
}

static inline void importGeoemtryGLTF(PG_DisaAssetsListItem* owner, PG_ListModelLocation location, bool hd) {
	if (!location) return;
	auto* model = location.getModel()->getParent<PG_DisaAssetsModel>();
	if (!model) return;

	PG_File file = PG_FileDialog::open(owner, "glb");
	if (file) {
		model->importGeometryGLTF(file, location, hd);
	}
}

static inline void addGeometryOptions(PG_DisaAssetsListItem* owner, PG_ListModelLocation location, bool hd, const PG_MouseButtonEvent* e) {
	if (!location) return;
	auto popup = PG_new<PG_GuiMenuPopup>("headerPopup", owner);
	popup->addOption("Export", owner, [owner, location, hd] { exportGeoemtry(owner, location, hd); });
	popup->addOption("Import", true, owner, [owner, location, hd] { importGeoemtry(owner, location, hd); });
	popup->addOption("Import GLTF (Experimental)", true, owner, [owner, location, hd] { importGeoemtryGLTF(owner, location, hd); });
	popup->openAndDestroyOnClose(e);
}

void PG_DisaAssetsListItem::updateCenter() {
	constexpr float tileSize = 125.f;
	constexpr float tileBgSize = tileSize + 8.f;

	auto dataItem = m_location.getData(PG_Identity_Item);

	if (dataItem.is<PG_DisaAssetsTextureItem>()) {
		if (!m_isTexture) m_rightItem->clearGuiChildren();
		m_isTexture = true;

		auto imageSD = m_rightItem->findFirstChild<PG_GuiImage>("imageSD");
		if (!imageSD) {

			auto bgRec = PG_new<PG_GuiRectangle>("imageSDbg", glm::vec2(11.f, 0.f), glm::vec2(tileBgSize), m_rightItem);
			bgRec->setColor(brightColor);
			anchor(bgRec, PG_Anchor::VerticalCenter, m_rightItem, PG_Anchor::VerticalCenter);

			imageSD = PG_new<PG_GuiImage>("imageSD", glm::vec2(15.f, 0.f), glm::vec2(tileSize), m_rightItem);
			anchor(imageSD, PG_Anchor::VerticalCenter, m_rightItem, PG_Anchor::VerticalCenter);

			PG_GuiMouseArea* area = PG_new<PG_GuiMouseArea>("area", imageSD);
			area->fill(imageSD);
			area->setAcceptedButtons(PG_GuiMouseArea::Right);

			connect(area, &PG_GuiMouseArea::rightClicked, this, [this](PG_MouseButtonEvent* e) { addTextureOptions(this, m_location, PG_DisaAssetsModel::TextureSD, e); });

			//sperator
			addSeperator(tileSize, imageSD, m_rightItem);
		}
		auto imageSDModified = m_rightItem->findFirstChild<PG_GuiRectangle>("imageSDModified");
		if (!imageSDModified) {
			imageSDModified = PG_new<PG_GuiRectangle>("imageSDModified", glm::vec2(0.f), glm::vec2(10.f), m_rightItem);
			imageSDModified->setColor(PG_Color::hentaiRedFrgb);
			imageSDModified->setMaterial("M_GuiRectangleCircle");
			anchor(imageSDModified, PG_Anchor::HorizontalCenter, imageSD, PG_Anchor::Right);
			anchor(imageSDModified, PG_Anchor::VerticalCenter, imageSD, PG_Anchor::Top);
		}

		auto imageHQ = m_rightItem->findFirstChild<PG_GuiImage>("imageHQ");
		if (!imageHQ) {
			auto bgRec = PG_new<PG_GuiRectangle>("imageHQbg", glm::vec2(96.f, 0.f), glm::vec2(tileBgSize), m_rightItem);
			bgRec->setColor(brightColor);
			anchor(bgRec, PG_Anchor::Left, imageSD, PG_Anchor::Right);
			anchor(bgRec, PG_Anchor::VerticalCenter, m_rightItem, PG_Anchor::VerticalCenter);

			imageHQ = PG_new<PG_GuiImage>("imageHQ", glm::vec2(100.f, 0.f), glm::vec2(tileSize), m_rightItem);
			anchor(imageHQ, PG_Anchor::Left, imageSD, PG_Anchor::Right);
			anchor(imageHQ, PG_Anchor::VerticalCenter, m_rightItem, PG_Anchor::VerticalCenter);

			PG_GuiMouseArea* area = PG_new<PG_GuiMouseArea>("area", imageHQ);
			area->fill(imageHQ);
			area->setAcceptedButtons(PG_GuiMouseArea::Right);

			connect(area, &PG_GuiMouseArea::rightClicked, this, [this](PG_MouseButtonEvent* e) { addTextureOptions(this, m_location, PG_DisaAssetsModel::TextureHD, e); });
		}
		auto imageHQModified = m_rightItem->findFirstChild<PG_GuiRectangle>("imageHQModified");
		if (!imageHQModified) {
			imageHQModified = PG_new<PG_GuiRectangle>("imageHQModified", glm::vec2(0.f), glm::vec2(10.f), m_rightItem);
			imageHQModified->setColor(PG_Color::hentaiRedFrgb);
			imageHQModified->setMaterial("M_GuiRectangleCircle");
			anchor(imageHQModified, PG_Anchor::HorizontalCenter, imageHQ, PG_Anchor::Right);
			anchor(imageHQModified, PG_Anchor::VerticalCenter, imageHQ, PG_Anchor::Top);
		}

		auto imageNormal = m_rightItem->findFirstChild<PG_GuiImage>("imageNormal");
		if (!imageNormal) {
			auto bgRec = PG_new<PG_GuiRectangle>("imageNormalbg", glm::vec2(16.f, 0.f), glm::vec2(tileBgSize), m_rightItem);
			bgRec->setColor(brightColor);
			anchor(bgRec, PG_Anchor::Left, imageHQ, PG_Anchor::Right);
			anchor(bgRec, PG_Anchor::VerticalCenter, m_rightItem, PG_Anchor::VerticalCenter);

			imageNormal = PG_new<PG_GuiImage>("imageNormal", glm::vec2(20.f, 0.f), glm::vec2(tileSize), m_rightItem);
			anchor(imageNormal, PG_Anchor::Left, imageHQ, PG_Anchor::Right);
			anchor(imageNormal, PG_Anchor::VerticalCenter, m_rightItem, PG_Anchor::VerticalCenter);

			PG_GuiMouseArea* area = PG_new<PG_GuiMouseArea>("area", imageNormal);
			area->fill(imageNormal);
			area->setAcceptedButtons(PG_GuiMouseArea::Right);

			connect(area, &PG_GuiMouseArea::rightClicked, this, [this](PG_MouseButtonEvent* e) { addTextureOptions(this, m_location, PG_DisaAssetsModel::TextureNormal, e); });
		}
		auto imageNormalModified = m_rightItem->findFirstChild<PG_GuiRectangle>("imageNormalModified");
		if (!imageNormalModified) {
			imageNormalModified = PG_new<PG_GuiRectangle>("imageNormalModified", glm::vec2(0.f), glm::vec2(10.f), m_rightItem);
			imageNormalModified->setColor(PG_Color::hentaiRedFrgb);
			imageNormalModified->setMaterial("M_GuiRectangleCircle");
			anchor(imageNormalModified, PG_Anchor::HorizontalCenter, imageNormal, PG_Anchor::Right);
			anchor(imageNormalModified, PG_Anchor::VerticalCenter, imageNormal, PG_Anchor::Top);
		}

		//read images
		imageSD->setSourceData(m_location.getDataT<PG_Texture>(PG_DisaAssetsTextureItem::Identity_SD));
		imageHQ->setSourceData(m_location.getDataT<PG_Texture>(PG_DisaAssetsTextureItem::Identity_HD));
		imageNormal->setSourceData(m_location.getDataT<PG_Texture>(PG_DisaAssetsTextureItem::Identity_HDNormal));

		imageSDModified->setVisible(m_location.getDataT<bool>(PG_DisaAssetsTextureItem::Identity_SDModified));
		imageHQModified->setVisible(m_location.getDataT<bool>(PG_DisaAssetsTextureItem::Identity_HDModified));
		imageNormalModified->setVisible(m_location.getDataT<bool>(PG_DisaAssetsTextureItem::Identity_HDNormalModified));
	}
	else if(dataItem.is<PG_DisaAssetsGeometryItem>()) {
		if (m_isTexture) m_rightItem->clearGuiChildren();
		m_isTexture = false;

		PG_DisaMapScene* sceneSD = nullptr;
		PG_DisaMapScene* sceneHD = nullptr;
		auto geoSDbg = m_rightItem->findFirstChild<PG_GuiRectangle>("geoSDbg");
		if (!geoSDbg) {
			geoSDbg = PG_new<PG_GuiRectangle>("geoSDbg", glm::vec2(11.f, 0.f), glm::vec2(tileBgSize), m_rightItem);
			geoSDbg->setColor(brightColor);
			anchor(geoSDbg, PG_Anchor::VerticalCenter, m_rightItem, PG_Anchor::VerticalCenter);

			auto [item, gameScene] = createMapScene(geoSDbg);
			item->setSize(glm::vec2(tileSize));
			item->setPosition(glm::vec2(4.f));
			sceneSD = gameScene;

			PG_GuiMouseArea* area = PG_new<PG_GuiMouseArea>("area", item);
			area->fill(item);
			area->setAcceptedButtons(PG_GuiMouseArea::Right);

			connect(area, &PG_GuiMouseArea::rightClicked, this, [this](PG_MouseButtonEvent* e) { addGeometryOptions(this, m_location, false, e); });


			addSeperator(tileSize, geoSDbg, m_rightItem);
		}
		else {
			sceneSD = dynamic_cast<PG_DisaMapScene*>(geoSDbg->findFirstInTree(PG_Search::byName("GameScene")));
		}

		auto imageSDModified = m_rightItem->findFirstChild<PG_GuiRectangle>("imageSDModified");
		if (!imageSDModified) {
			imageSDModified = PG_new<PG_GuiRectangle>("imageSDModified", glm::vec2(0.f), glm::vec2(10.f), m_rightItem);
			imageSDModified->setColor(PG_Color::hentaiRedFrgb);
			imageSDModified->setMaterial("M_GuiRectangleCircle");
			anchor(imageSDModified, PG_Anchor::HorizontalCenter, geoSDbg, PG_Anchor::Right);
			anchor(imageSDModified, PG_Anchor::VerticalCenter, geoSDbg, PG_Anchor::Top);
		}

		auto geoHDbg = m_rightItem->findFirstChild<PG_GuiRectangle>("geoHDbg");
		if (!geoHDbg) {
			geoHDbg = PG_new<PG_GuiRectangle>("geoHDbg", glm::vec2(100.f, 0.f), glm::vec2(tileBgSize), m_rightItem);
			geoHDbg->setColor(brightColor);
			anchor(geoHDbg, PG_Anchor::Left, geoSDbg, PG_Anchor::Right);
			anchor(geoHDbg, PG_Anchor::VerticalCenter, m_rightItem, PG_Anchor::VerticalCenter);

			auto [item, gameScene] = createMapScene(geoHDbg);
			item->setSize(glm::vec2(tileSize));
			item->setPosition(glm::vec2(4.f));
			sceneHD = gameScene;

			PG_GuiMouseArea* area = PG_new<PG_GuiMouseArea>("area", item);
			area->fill(item);
			area->setAcceptedButtons(PG_GuiMouseArea::Right);

			connect(area, &PG_GuiMouseArea::rightClicked, this, [this](PG_MouseButtonEvent* e) { addGeometryOptions(this, m_location, true, e); });
		}
		else {
			sceneHD = dynamic_cast<PG_DisaMapScene*>(geoHDbg->findFirstInTree(PG_Search::byName("GameScene")));
		}
		auto imageHQModified = m_rightItem->findFirstChild<PG_GuiRectangle>("imageHQModified");
		if (!imageHQModified) {
			imageHQModified = PG_new<PG_GuiRectangle>("imageHQModified", glm::vec2(0.f), glm::vec2(10.f), m_rightItem);
			imageHQModified->setColor(PG_Color::hentaiRedFrgb);
			imageHQModified->setMaterial("M_GuiRectangleCircle");
			anchor(imageHQModified, PG_Anchor::HorizontalCenter, geoHDbg, PG_Anchor::Right);
			anchor(imageHQModified, PG_Anchor::VerticalCenter, geoHDbg, PG_Anchor::Top);
		}


		if (sceneSD) {
			sceneSD->setPreviewMode(true);
			PG_DisaModelNode* disaModel = sceneSD->getRoot()->findFirstChild<PG_DisaModelNode>("PG_DisaModelNode");
			if(!disaModel) disaModel = PG_new<PG_DisaModelNode>("PG_DisaModelNode", sceneSD->getRoot());
			disaModel->setModel(m_location.getDataT<PG_Model>(PG_DisaAssetsTextureItem::Identity_SD));
		}

		if (sceneHD) {
			sceneHD->setPreviewMode(true);
			PG_DisaModelNode* disaModel = sceneHD->getRoot()->findFirstChild<PG_DisaModelNode>("PG_DisaModelNode");
			if (!disaModel) disaModel = PG_new<PG_DisaModelNode>("PG_DisaModelNode", sceneHD->getRoot());
			disaModel->setModel(m_location.getDataT<PG_Model>(PG_DisaAssetsTextureItem::Identity_HD));
		}

		imageSDModified->setVisible(m_location.getDataT<bool>(PG_DisaAssetsTextureItem::Identity_SDModified));
		imageHQModified->setVisible(m_location.getDataT<bool>(PG_DisaAssetsTextureItem::Identity_HDModified));
	}

}

void PG_DisaAssetsWindow::init() {
	PG_GuiItem::init();

	connect(this, &PG_DisaAssetsWindow::toggleModeChanged, this, [this](bool isTexturesMode) {
		if (!m_model) return;
		m_listView->setModel(isTexturesMode ? (PG_ListModelBase*)m_model->getTextures() : (PG_ListModelBase*)m_model->getGeometries());
	});

	auto bg = PG_new<PG_GuiRectangle>("AssetBg", this);
	bg->fill(this);
	bg->setColor(backgroundDarkColor);

	auto toolbar = PG_new<PG_GuiRectangle>("toolbar", glm::vec2(0.f), glm::vec2(10.f, 50.f), bg);
	toolbar->setColor(backgroundColor);
	anchor(toolbar, PG_Anchor::Top, bg, PG_Anchor::Top);
	anchor(toolbar, PG_Anchor::Left, bg, PG_Anchor::Left);
	anchor(toolbar, PG_Anchor::Right, bg, PG_Anchor::Right);

	{

		auto addButton = PG_new<PG_GuiButton>("AddButton", glm::vec2(15.f, 10.f), glm::vec2(32.f, 30.f), toolbar);
		addButton->setIcon("gui/icon_add.png");
		addButton->setBackground("M_GuiRoundedRectangleAll5");
		PG_GuiItem::anchor(addButton, PG_Anchor::Left, toolbar, PG_Anchor::Left);

		connect(addButton, &PG_GuiButton::clicked, this, [this]() {
			if (!m_model) return;
			m_model->addItem(isShowTextures());
		});

		auto toggleCenter = PG_new<PG_GuiItem>("toggleCenter", glm::vec2(10.f, 0.f), glm::vec2(4.f, 30.f), toolbar);
		anchor(toggleCenter, PG_Anchor::HorizontalCenter, toolbar, PG_Anchor::HorizontalCenter);

		auto toggleTexture = PG_new<PG_GuiToggleButton>("ToggleTexture", glm::vec2(10.f, 10.f), glm::vec2(120.f, 30.f), toolbar);
		toggleTexture->setText("Textures");
		toggleTexture->setBackground("M_GuiToolButtonStart");
		toggleTexture->setToggled(true);
		anchor(toggleTexture, PG_Anchor::Right, toggleCenter, PG_Anchor::Left);

		connect(this, &PG_DisaAssetsWindow::toggleModeChanged, toggleTexture, [toggleTexture](bool isTexturesMode) {
			toggleTexture->setToggled(isTexturesMode);
			});
		connect(toggleTexture, &PG_GuiToggleButton::toggleChanged, this, [this](bool toggle) {
			if (toggle) postCallEvent(PG_Event_Behaviour_Replace, &PG_DisaAssetsWindow::setShowTextures, true);
			});

		auto toggleGeometry = PG_new<PG_GuiToggleButton>("toggleGeometry", glm::vec2(10.f, 10.f), glm::vec2(120.f, 30.f), toolbar);
		toggleGeometry->setText("Geometry");
		toggleGeometry->setBackground("M_GuiToolButtonEnd");
		anchor(toggleGeometry, PG_Anchor::Left, toggleCenter, PG_Anchor::Right);

		connect(this, &PG_DisaAssetsWindow::toggleModeChanged, toggleTexture, [toggleGeometry](bool isTexturesMode) {
			toggleGeometry->setToggled(!isTexturesMode);
			});
		connect(toggleGeometry, &PG_GuiToggleButton::toggleChanged, this, [this](bool toggle) {
			if(toggle) postCallEvent(PG_Event_Behaviour_Replace, &PG_DisaAssetsWindow::setShowTextures, false);
			});
	}

	m_listView = PG_new<PG_GuiListView>("AssetList", bg);
	m_listView->setMargin({0.f, 0.f, 5.f, 5.f});
	m_listView->setCellSize(164.f);
	m_listView->setClip(true);
	anchor(m_listView, PG_Anchor::Top, toolbar, PG_Anchor::Bottom);
	anchor(m_listView, PG_Anchor::Bottom, bg, PG_Anchor::Bottom);
	anchor(m_listView, PG_Anchor::Left, bg, PG_Anchor::Left);
	anchor(m_listView, PG_Anchor::Right, bg, PG_Anchor::Right);

	m_listView->setItem(PG_GuiListView::getDefaultCreateFunc<PG_DisaAssetsListItem>());
	m_listView->setItemUpdate(PG_GuiListView::getDefaultUpdateFunc<PG_DisaAssetsListItem>());
}

void PG_DisaAssetsWindow::setModel(PG_DisaAssetsModel* model) {
	if (m_model == model) return;
	m_model = model;

	m_listView->setModel(isShowTextures() ? (PG_ListModelBase*)m_model->getTextures() : (PG_ListModelBase*)m_model->getGeometries());
}