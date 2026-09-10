#include "PG_DisaPreviewItem.h"
#include <Base/PG_AudioDirector.h>
#include <Base/Events/PG_MouseButtonEvent.h>
#include <Scene/Gui/PG_GuiRenderItem.h>
#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Gui/PG_GuiMouseArea.h>
#include <Scene/Gui/PG_GuiText.h>
#include <Scene/Gui/PG_GuiLines.h>
#include <Scene/Gui/PG_GuiMenuPopup.h>
#include <Scene/Gui/PG_GuiImage.h>
#include <Renderer/PG_SceneRenderer.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/fileformats/PG_PSPFS.h>
#include <Disa/utils/PG_SaveTexture.h>
#include <Resources/PG_Audio.h>
#include <Util/Color/ColorConverters.h>
#include <Util/Color/ColorDefined.h>

namespace {

constexpr glm::uvec2 g_waveformSize(256);

class DisaAudioTracking : public PG_GuiMouseArea {
public:
	DisaAudioTracking(const std::string& name, std::string audioName) :
		PG_GuiMouseArea(name), m_audio(audioName) {};

	void init() {
		assert(m_audio);
		PG_GuiImage* target = getParent<PG_GuiImage>();
		assert(target);
		fill(target);
		auto tracker = PG_new<PG_GuiLines>("tracker", glm::vec2(0.f), glm::vec2(1.f, 0.f), this);
		tracker->makeLineY();
		tracker->setColor(PG_Color::hentaiRedFrgb);
		tracker->anchor(tracker, PG_Anchor::Top, this, PG_Anchor::Top);
		tracker->anchor(tracker, PG_Anchor::Bottom, this, PG_Anchor::Bottom);
		tracker->setVisible(false);
		tracker->setAlpha(0.f);
		tracker->update();

		m_audio.onProgress(tracker, [tracker](float v) {
			if (v <= 0.001f || v >= 1.f) {
				tracker->setVisible(false);
				return;
			}
			tracker->setVisible(true);
			const float width = tracker->getParent<PG_GuiItem>()->getSize().x;
			const float at = width * v;
			tracker->setPosition(glm::vec2(at, 0.f));
			const float borderSize = std::min(width * 0.05f, 15.f);
			if (at < borderSize) {
				tracker->setAlpha(std::min(at / borderSize, 1.f));
			}
			else if (at > (width - borderSize)) {
				tracker->setAlpha(std::min(-((at - width) / borderSize), 1.f));
			}
			else {
				tracker->setAlpha(1.f);
			}
			
			tracker->updateModelMatrixWorld();
		});
		m_audio.onFinished(this, &DisaAudioTracking::deleteLater);

		connect(this, &PG_GuiMouseArea::mouseClicked, this, [this](PG_MouseButtonEvent* e) {
			if (m_audio) {
				const float clickAt = toLocalTranslation(e->getPosition()).x;
				m_audio.setProgress(glm::clamp(clickAt / getSize().x, 0.0F, 1.0F));
			}
			
		});
	}

	PG_Audio m_audio;
};

/**
* @brief Holds what an audio preview loads, so that deleting it is what cancels the load.
* @details The audio lives here and nowhere else until the samples are in, so a holder that goes away
* takes the last handle with it and the decode has nobody left to decode for.
*/
class DisaAudioPreview : public PG_Object {
public:
	using PG_Object::PG_Object;

	void open(const PG_Audio& audio, PG_GuiImage* target) {
		auto name = audio.getName();
		if (name.empty()) return;
		m_audio = audio;

		PG_Material material("M_GuiImageAudio");
		material.set("color", PG_Color::hentaiBlueFrgb);
		material.set("colorFade", PG_Color::hentaiDarkBlueFrgb);
		target->setMaterial(material);

		m_audio.connect(this, [this, target] {
			if (!m_audio.isLoaded()) return;
			//the samples are wanted for the one pass that measures them, so they go with it
			m_texture.load(PG_UNIQUE_RES("DisaAudioPreview"), [audio = std::move(m_audio)]()->PG_Texture::ImageFormats {
				PG_RGBImage preview;
				preview.resize(g_waveformSize);
				preview.fill(glm::rgb(0)); //createPreview writes every pixel, this is only a safe start
				audio.createPreview(preview);
				return { std::move(preview) };
				});
			target->setSourceData(m_texture);
			});

		auto* audioDirector = PG_AudioDirector::get();
		connect(audioDirector, &PG_AudioDirector::audioStartedPlaying, this, [this, name = std::move(name), target](const std::string& audioName) {
			if (audioName != name) return;
			PG_new<DisaAudioTracking>("DisaAudioTracking", audioName, target);
		});
	}
private:
	PG_Audio m_audio;
	PG_Texture m_texture;
};

}

void PG_DisaPreviewItem::init() {
	PG_GuiItem::init();
	setName("PG_DisaPreviewItem");

	/* Created first so it sits behind everything else. Only an image preview shows it, a scene has
	 * nothing transparent to tell apart from black. */
	m_checker = PG_new<PG_GuiImage>("Checker", this);
	m_checker->fill(this);
	m_checker->setFitType(PG_GuiImage::Repeat);
	m_checker->setSource("gui/pattern.png");

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

	PG_GuiMouseArea* area = PG_new<PG_GuiMouseArea>("PreviewArea", this);
	area->setConferEnabled(true);
	area->setAcceptedButtons(PG_GuiMouseArea::Right);
	area->fill(this);
	connect(area, &PG_GuiMouseArea::rightClicked, this, [this] {
		auto popup = PG_new<PG_GuiMenuPopup>("previewPopup", this);
		popup->addOption("Export Image", hasPreview(), this, &PG_DisaPreviewItem::exportImage);
		popup->addOption(m_previewEnabled ? "Disable Preview" : "Enable Preview", true, this, [this] { setPreviewEnabled(!m_previewEnabled); });
		popup->openAndDestroyOnClose();
		});
}

void PG_DisaPreviewItem::exportImage() {
	if (!hasPreview()) return;

	//find the component the preview is displayed with
	PG_GuiImage* imgItem = m_container->findFirstChild<PG_GuiImage>();
	/* An audio preview has its image before it has anything in it, and a vector source is triangles
	 * rather than a texture, so neither is a picture this can hand over. */
	const PG_Texture* texture = imgItem ? std::get_if<PG_Texture>(&imgItem->getSourceData()) : nullptr;
	if (!texture) {
		OUT_ERROR("This kind of preview can't be exported as a image!");
		return;
	}

	PG_FileDialog::save(*texture, m_location.getDataT<std::string>(EIdentity::PG_Identity_Text));
}

bool PG_DisaPreviewItem::isNameVisible() const {
	return m_text && m_text->isVisible();
}

void PG_DisaPreviewItem::setNameVisible(bool visible) {
	if (m_text) m_text->setVisible(visible);
}

void PG_DisaPreviewItem::setPreviewEnabled(bool enabled) {
	if (m_previewEnabled == enabled) return;
	m_previewEnabled = enabled;

	if (!m_previewEnabled) {
		clearImage();
		return;
	}

	if (m_location) openPreview(m_location.getData(PG_Identity_PreviewImage));
}

void PG_DisaPreviewItem::openPreview(const PG_Variety& var) {
	if (!m_previewEnabled) {
		clearImage();
		return;
	}

	if (var.is<PG_DisaMPDPack>()) {
		open(var.get<PG_DisaMPDPack>());
		return;
	}
	if (var.is<PG_Texture>()) {
		open(var.get<PG_Texture>());
		return;
	}
	if (var.is<PG_Audio>()) {
		open(var.get<PG_Audio>());
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

PG_GuiImage* PG_DisaPreviewItem::getOrCreateImage() {
	PG_GuiImage* imgItem = m_container->findFirstChild<PG_GuiImage>();
	if (!imgItem) {
		m_container->clearGuiChildren();
		imgItem = PG_new<PG_GuiImage>("PreviewImage", m_container);
		imgItem->setFitType(PG_GuiImage::MaintainAspectFit);
		imgItem->fill(m_container);
	}
	imgItem->resetMaterial();
	/* Whatever the last preview was is over. The holder going away cancels a decode still running and
	 * stops a waveform that is already on its way from landing on top of the next preview. */
	if (auto* previous = imgItem->findFirstChild<DisaAudioPreview>()) delete previous;
	return imgItem;
}

void PG_DisaPreviewItem::open(const PG_Texture& img) {
	getOrCreateImage()->setSourceData(img);
	if (m_checker) m_checker->setVisible(true);
}

void PG_DisaPreviewItem::open(const std::string& imgPath) {
	getOrCreateImage()->setSource(imgPath);
	if (m_checker) m_checker->setVisible(true);
}

void PG_DisaPreviewItem::open(const PG_Audio& audio) {
	PG_GuiImage* imgItem = getOrCreateImage();
	PG_new<DisaAudioPreview>("audioPreview", imgItem)->open(audio, imgItem);
	if (m_checker) m_checker->setVisible(true);
}

void PG_DisaPreviewItem::clearImage() {
	m_container->clearGuiChildren();
}

void PG_DisaPreviewItem::clear() {
	clearImage();
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

	if (m_text) m_text->setText(location.getDataT<std::string>(PG_Identity_Text));

	if (m_previewEnabled) openPreview(location.getData(PG_Identity_PreviewImage));
	else clearImage();
	emit locationChanged();
}

void PG_DisaPreviewItem::update(const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
	const bool hasItem = identities.count(PG_Identity_Item) != 0 || m_location != location;
	if (hasItem || identities.count(PG_Identity_Location)) {
		m_location = location;
	}

	if (!location) return;
	if (hasItem || identities.count(PG_Identity_Text)) {
		if (m_text) m_text->setText(location.getDataT<std::string>(PG_Identity_Text));
	}

	if (hasItem || identities.count(PG_Identity_PreviewImage)) {
		if (m_previewEnabled) openPreview(location.getData(PG_Identity_PreviewImage));
		else clearImage();
	}
}
