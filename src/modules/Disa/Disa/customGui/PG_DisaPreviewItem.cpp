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
#include <Scene/Gui/PG_GuiSpinBox.h>
#include <Scene/Gui/PG_GuiSprite.h>
#include <Renderer/PG_SceneRenderer.h>
#include <Disa/PG_DisaMapScene.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/PG_DisaMapModel.h>
#include <Disa/fileformats/PG_PSPFS.h>
#include <Disa/resources/PG_DisaSprite.h>
#include <Disa/customGui/PG_PupUpDialog.h>
#include <Disa/utils/PG_SaveTexture.h>
#include <Resources/PG_Audio.h>
#include <Util/PG_Dir.h>
#include <Util/PG_ImageLoad.h>
#include <Util/Color/ColorConverters.h>
#include <Util/Color/ColorDefined.h>
#include <Util/Volume/PG_AABB.h>
#include <optional>
#include <unordered_map>

namespace {

constexpr glm::uvec2 g_waveformSize(256);

/// Frames looked at per animation while measuring, walking every frame of every animation is not worth it
constexpr int g_spriteBoundsSamples = 8;

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


class DisaSpritePreview : public PG_Object {
public:
	using PG_Object::PG_Object;

	void open(const PG_Sprite& sprite) {
		PG_GuiSprite* guiSprite = getParent<PG_GuiSprite>();

		//the load of whatever was here before is no longer wanted, and cannot be reached once it is replaced
		guiSprite->getSprite().disconnect(this);
		guiSprite->setSprite(sprite);

		PG_Sprite& target = guiSprite->getSprite();
		//the speed the map view plays its actors at, so a sprite moves the same in both places
		target.setAnimationSpeed(PG_Sprite::defaultTimeStep / 2.f);

		target.connect(this, [this] {
			PG_GuiSprite* guiSprite = getParent<PG_GuiSprite>();
			PG_Sprite& loaded = guiSprite->getSprite();
			if (!loaded.isLoaded()) return; //a load that was cancelled ends here too

			/* An item or weapon sheet carries cutouts for another sprite to pull in and nothing to play,
			 * so the sheet itself is what there is to look at. */
			if (loaded.getAnimationSize() <= 0) {
				openSheets(guiSprite);
				return;
			}
			clearSheet(guiSprite); //what a sheet preview before this one left would cover what plays here

			m_measurements = measureSprite(loaded);
			
			auto targetIndex = loaded.getAnimationIndex("1001"); //idle standing
			if (targetIndex < 0) targetIndex = loaded.getAnimationIndex("1");
			if (targetIndex < 0) targetIndex = 0;
			loaded.setAnimationIndex(targetIndex);

			fitSpriteIntoItem(guiSprite);
			openAnimationBox(guiSprite);
		});
	}

	/**
	* @brief Shows the sprite sheets themselves, for a sprite that holds cutouts and no animation.
	* @details Only the sheet on screen is resolved to colour. Most of these files hold a single sheet and
	* nobody steps through them, so composing all of them to show the first would be spent on nothing.
	*/
	void openSheets(PG_GuiSprite* spriteItem) {
		const int sheetCount = PG_DisaSpriteSheets::getSize(spriteItem->getSprite());
		if (sheetCount <= 0) {
			OUT_WARNING("The sprite \"" << spriteItem->getSprite().getName() << "\" has nothing to show!");
			clearSpinBox(spriteItem); //the box of the preview before this one has nothing left to step
			return;
		}

		auto* image = spriteItem->findFirstChild<PG_GuiImage>("sheetImage");
		if (!image) {
			image = PG_new<PG_GuiImage>("sheetImage", spriteItem);
			image->fill(spriteItem);
			image->setFitType(PG_GuiImage::MaintainAspectFit);
		}

		showSheet(spriteItem, 0);
		openSheetBox(spriteItem, sheetCount);
	}

	/// Resolves one sheet to colour and hands it over, the sheet before it goes with the texture it replaces
	void showSheet(PG_GuiSprite* spriteItem, int sheetIndex) {
		auto* image = spriteItem->findFirstChild<PG_GuiImage>("sheetImage");
		if (!image) return;

		PG_RGBAImage sheet = PG_DisaSpriteSheets::buildImage(spriteItem->getSprite(), sheetIndex);
		if (sheet.empty()) return;
		m_sheetTexture.load(PG_UNIQUE_RES("DisaSheetPreview"), sheet, { false, PG_Texture::Nearest, PG_Texture::Clamp });
		image->setSourceData(m_sheetTexture);
	}

	void clearSheet(PG_GuiSprite* spriteItem) {
		if (auto* image = spriteItem->findFirstChild<PG_GuiImage>("sheetImage")) delete image;
		m_sheetTexture = PG_Texture();
	}

	void openSheetBox(PG_GuiSprite* spriteItem, int sheetCount) {
		if (sheetCount <= 1) { //a single sheet is not something to step through
			clearSpinBox(spriteItem);
			return;
		}

		PG_GuiSpinBox* spinBox = createSpinBox(spriteItem);
		spinBox->setValidator([sheetCount](double v) -> double {
			return glm::clamp((int)v, 0, sheetCount - 1);
		});
		spinBox->setValueToText([](double value) { return "Sheet " + std::to_string((int)value); });
		//the shown text is not a number, so typing over it with one is what moves the box
		spinBox->setTextToValue([](const std::string& text) -> double { return std::stod(text); });
		connect(spinBox, &PG_GuiSpinBox::valueModefied, this, [this](double value) {
			if (PG_GuiSprite* guiSprite = getParent<PG_GuiSprite>()) showSheet(guiSprite, (int)value);
		});
		spinBox->setValue(0);
	}

	void clearSpinBox(PG_GuiSprite* spriteItem) {
		if (auto* spinBox = spriteItem->findFirstChild<PG_GuiSpinBox>()) delete spinBox;
	}

	/**
	* @brief The spin box under the preview, replacing the one the preview before this one left.
	* @details A fresh box rather than a reused one, so that no lambda and no connection of the previous
	* preview can still be on it.
	*/
	PG_GuiSpinBox* createSpinBox(PG_GuiSprite* spriteItem) {
		clearSpinBox(spriteItem);

		constexpr glm::vec2 spinBoxSize(126.f, 26.f);
		auto* spinBox = PG_GuiSpinBox::createInt("spinBox", glm::vec2(0.f), spinBoxSize, spriteItem);
		PG_GuiItem::anchor(spinBox, PG_Anchor::HorizontalCenter, spriteItem, PG_Anchor::HorizontalCenter);
		PG_GuiItem::anchor(spinBox, PG_Anchor::Bottom, spriteItem, PG_Anchor::Bottom);
		spinBox->setMargin(PG_Anchor::Bottom, 6.f);
		return spinBox;
	}

	void openAnimationBox(PG_GuiSprite* spriteItem) {
		PG_Sprite& sprite = spriteItem->getSprite();
		const auto size = sprite.getAnimationSize();
		if (!sprite.isValid() || size <= 0) {
			clearSpinBox(spriteItem);
			return;
		}

		PG_GuiSpinBox* spinBox = createSpinBox(spriteItem);
		spinBox->setValidator([size](double v) -> double {
			return  glm::clamp((int)v, 0, size - 1);
		});

		spinBox->setValueToText([sprite](double value) { 
			auto name = sprite.getAnimationName((int)value);
			auto idxStr = std::to_string((int)value);
			return name.empty() ? "Idx " + idxStr :  (name + " ("+ idxStr+")");
		});
		spinBox->setTextToValue([sprite, spinBox](const std::string& text) -> double {
			const int index = sprite.getAnimationIndex(text);
			return index < 0 ? spinBox->getValue() : (double)index; //a name the sheet has no animation for changes nothing
		});
		connect(spinBox, &PG_GuiSpinBox::valueModefied, this, [this](double value) {
			PG_GuiSprite* guiSprite = getParent<PG_GuiSprite>();
			if (!guiSprite) return;
			guiSprite->getSprite().setAnimationIndex((int)value, true);
			fitSpriteIntoItem(guiSprite);
		});
		spinBox->setValue(sprite.getAnimationIndex());
	}

	/**
	* @brief The box every animation of the sprite stays inside, in the units the sprite draws in.
	* @details Corners are placed the way DisaSprite.vert places them, so what is measured is what is drawn.
	* Every animation is walked, not just the current one, so the sprite keeps its size as the animation
	* changes and two animations stay comparable by eye.
	* @param owner The node the sprite draws under, a traverser reads properties off it.
	* @returns Nothing when the sprite draws no cutout at all.
	*/
	std::unordered_map<std::string, PG_AABB2D> measureSprite(PG_Sprite sprite) {
		std::unordered_map<std::string, PG_AABB2D> out;

		const int animationCount = sprite.getAnimationSize();
		for (int animationIndex = 0; animationIndex < animationCount; ++animationIndex) {
			const std::string animationName = sprite.getAnimationName(animationIndex);
			if (animationName.empty()) continue;

			sprite.setAnimationIndex(animationIndex, true);
			const auto endFrame = sprite.getSegment();
			const int sampleCount = endFrame.range.y > 0 ? g_spriteBoundsSamples : 1;
			PG_AABB2D bounds;
			for (int sample = 0; sample < sampleCount; ++sample) {
				sprite.setFrame(float(endFrame.range.y * sample) / float(sampleCount));
				sprite.traverse(this, glm::mat4(1.f), [&bounds](PG_SpriteTraverserser*, const PG_Geometry&,
								const PG_Material&, const PG_Texture&, glm::vec2, glm::vec2 cutoutSize, float,
								PG_SpriteEnums::EStencilDrawMode, bool, const glm::mat4& matrix, std::vector<glm::mat4>*) {
					const glm::vec2 quad = cutoutSize * 0.02f; //what the vertex shader scales the unit quad by
					for (const glm::vec2& corner : { glm::vec2(0.f), glm::vec2(quad.x, 0.f), glm::vec2(0.f, quad.y), quad }) {
						const glm::vec2 at = glm::vec2(matrix * glm::vec4(corner, 0.f, 1.f));
						bounds.add(at);
					}
				});
			}
			if (bounds.isValid()) out[animationName] = bounds;
		}

		return out;
	}


	/// Scales and centres the sprite so that every animation of it stays inside the item
	void fitSpriteIntoItem(PG_GuiSprite* spriteItem) {
		PG_Transformation transformation;
		auto findIt = m_measurements.find(spriteItem->getSprite().getAnimationName());
		if (findIt == m_measurements.end()) {
			OUT_WARNING("The sprite \"" << spriteItem->getSprite().getName() << "\" has nothing to measure!");
			/// Used when a sheet measures to nothing, roughly the size an ordinary character comes out at
			constexpr float spriteFallbackScale = 0.25f;
			transformation.setScale(glm::vec3(spriteFallbackScale));
		}
		else {
			/// The share of the preview a sprite is allowed to take up
			constexpr float spriteFill = 0.9f;

			const glm::vec2 extent = findIt->second.getDimensions();
			const float largest = std::max(extent.x, extent.y);
			const float scale = spriteFill / largest;
			transformation.setScale(glm::vec3(scale));
			transformation.setTranslation(-findIt->second.getCenter() * scale);
		}

		spriteItem->setOffsetTransformation(transformation);
	}


	std::unordered_map<std::string, PG_AABB2D> m_measurements;
	PG_Texture m_sheetTexture; /// The one sheet a cutout only sprite currently shows
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
	area->setHoverEnabled(true); //the map preview freezes its renderer while nobody looks at it
	area->setConferEnabled(true);
	area->setAcceptedButtons(PG_GuiMouseArea::Right);
	area->fill(this);
	connect(area, &PG_GuiMouseArea::rightClicked, this, [this] {
		auto popup = PG_new<PG_GuiMenuPopup>("previewPopup", this);
		//only the export the open preview can actually do is offered, the two never apply at once
		if (getPreviewTexture()) popup->addOption("Export Image", this, &PG_DisaPreviewItem::exportImage);
		else if (getPreviewSprite()) popup->addOption("Export Sprite Sheets", this, &PG_DisaPreviewItem::exportSpriteSheets);
		popup->addOption(m_previewEnabled ? "Disable Preview" : "Enable Preview", true, this, [this] { setPreviewEnabled(!m_previewEnabled); });
		popup->openAndDestroyOnClose();
		});
}

const PG_Texture* PG_DisaPreviewItem::getPreviewTexture() const {
	PG_GuiImage* imgItem = m_container ? m_container->findFirstChild<PG_GuiImage>() : nullptr;
	return imgItem ? std::get_if<PG_Texture>(&imgItem->getSourceData()) : nullptr;
}

void PG_DisaPreviewItem::exportImage() {
	const PG_Texture* texture = getPreviewTexture();
	if (!texture) {
		OUT_ERROR("This kind of preview can't be exported as a image!");
		return;
	}

	PG_FileDialog::save(*texture, m_location.getDataT<std::string>(EIdentity::PG_Identity_Text));
}

const PG_Sprite* PG_DisaPreviewItem::getPreviewSprite() const {
	PG_GuiSprite* spriteItem = m_container ? m_container->findFirstChild<PG_GuiSprite>() : nullptr;
	if (!spriteItem || !spriteItem->getSprite().isLoaded()) return nullptr;
	return &spriteItem->getSprite();
}

void PG_DisaPreviewItem::exportSpriteSheets() {
	const PG_Sprite* sprite = getPreviewSprite();
	if (!sprite) {
		OUT_ERROR("There is no sprite to export!");
		return;
	}

	const PG_Dir folder = PG_PopUpDialog::selectDir();
	if (folder.isEmpty()) return; //the user said no

	const std::vector<PG_RGBAImage> sheets = PG_DisaSpriteSheets::buildImages(*sprite);
	if (sheets.empty()) {
		OUT_ERROR("This sprite has no sprite sheet to export!");
		return;
	}

	const std::string baseName = PG_File(m_location.getDataT<std::string>(EIdentity::PG_Identity_Text)).getName();
	for (std::size_t i = 0; i < sheets.size(); ++i) {
		if (sheets[i].empty()) continue; //a sheet that could not be read back has already said so
		const PG_File target(folder.toFilePath(baseName + "_" + std::to_string(i) + ".png"));
		if (!PG_ImageLoad::save(target, sheets[i])) OUT_ERROR("Failed to save: " << target << "!");
	}
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
	if (var.is<PG_Sprite>()) {
		open(var.get<PG_Sprite>());
		return;
	}
	if (var.is<PG_DisaMPPPack>()) {
		open(var.get<PG_DisaMPPPack>());
		return;
	}
	if (var.is<PG_DisaGEOPack>()) {
		open(var.get<PG_DisaGEOPack>());
		return;
	}
	if (var.is<PG_Model>()) {
		open(var.get<PG_Model>());
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
	DisaAudioPreview* preview = imgItem->findFirstChild<DisaAudioPreview>("audioPreview");
	if (!preview) preview = PG_new<DisaAudioPreview>("audioPreview", imgItem);
	preview->open(audio, imgItem);
	if (m_checker) m_checker->setVisible(true);
}

PG_GuiSprite* PG_DisaPreviewItem::getOrCreateSprite() {
	PG_GuiSprite* spriteItem = m_container->findFirstChild<PG_GuiSprite>();
	if (!spriteItem) {
		m_container->clearGuiChildren();
		spriteItem = PG_new<PG_GuiSprite>("PreviewSprite", m_container);
		spriteItem->fill(m_container);
		spriteItem->setModelAnchor(PG_GuiSprite::Center);
		/* A sprite matrix carries the flip that turns sprite space into the scene's y up world, and gui
		 * space is y down and never mirrored, so both axes go back. */
		spriteItem->setFlipHorizontal(true);
		spriteItem->setFlipVertical(true);
	}
	return spriteItem;
}

void PG_DisaPreviewItem::open(const PG_Sprite& sprite) {
	PG_GuiSprite* spriteItem = getOrCreateSprite();
	DisaSpritePreview* preview = spriteItem->findFirstChild<DisaSpritePreview>("spritePreview");
	if (!preview) preview = PG_new<DisaSpritePreview>("spritePreview", spriteItem);
	preview->open(sprite);
	if (m_checker) m_checker->setVisible(true);
	
}

void PG_DisaPreviewItem::clearImage() {
	m_container->clearGuiChildren();
}

void PG_DisaPreviewItem::clear() {
	clearImage();
	if (m_text) m_text->setText({});
}

PG_DisaMapScene* PG_DisaPreviewItem::getOrCreateGameScene() {
	PG_GuiRenderItem* renderItem = m_container->findFirstChild<PG_GuiRenderItem>();
	if (!renderItem) {
		m_container->clearGuiChildren();
		renderItem = PG_new<PG_GuiRenderItem>("MapRenderItem", m_container);
		renderItem->fill(m_container);
		//a map that nobody looks at costs a draw per frame, so it only turns while the mouse is on it
		renderItem->setFreez(true);
		connect(findFirstChild<PG_GuiMouseArea>("PreviewArea"), &PG_GuiMouseArea::hoveredChanged,
			renderItem, [renderItem](bool hover) { renderItem->setFreez(!hover); });
	}

	PG_SceneRenderer* sceneRenderer = renderItem->findFirstChild<PG_SceneRenderer>();
	if (!sceneRenderer) sceneRenderer = PG_new<PG_SceneRenderer>("GameSceneRenderer", true, 2, renderItem);

	if (!m_gameScene) {
		m_gameScene = PG_new<PG_DisaMapScene>("GameScene", sceneRenderer);
		m_gameScene->setPreviewMode(true);
		m_gameScene->setActorsVisible(false);
		m_gameScene->setGeoPanelsVisible(false);

		connect(m_gameScene, &PG_Object::onDeleted, this, [this] { m_gameScene = nullptr; });
		connect(m_gameScene->getMapNode(), &PG_DisaMapViewNode::loaded, renderItem, &PG_GuiRenderItem::forceRenderNext);
	}

	//the checkerboard tells a transparent image from the background, a scene has nothing to tell apart
	if (m_checker) m_checker->setVisible(false);
	renderItem->forceRenderNext();
	return m_gameScene;
}

void PG_DisaPreviewItem::open(const PG_DisaMPDPack& mapFiles) {
	auto mpd = mapFiles.getMPD();
	if (!(mpd && mpd.getInfo().getType() == PG_DisaFileInfo::MPD)) {
		OUT_ERROR("Failed to find MPD!");
		clear();
		return;
	}

	auto mppSD = mapFiles.getMPPSD();
	/* No mpp is not a reason to show nothing. The assets stay empty, and the tiles come out on
	 * M_MapDefaultTile, so the map is still there to look at, just without its textures. */
	if (!mppSD) OUT_WARNING("Failed to find the mpp of \"" << mpd.getName() << "\", the map stays untextured!");

	auto container = PG_DisaMPDContainer::createFromMPD(mpd, mppSD, mapFiles.getMPPHD());
	assert(container); //most likely the entry is already open as another container type
	if (!container) {
		OUT_ERROR("Failed to open \"" << mpd.getName() << "\" as a map!");
		clear();
		return;
	}
	getOrCreateGameScene()->setModel(container);
}

void PG_DisaPreviewItem::open(const PG_DisaMPPPack& mppPack) {
	auto mpp = mppPack.getMPP();
	if (!(mpp && mpp.getInfo().getType() == PG_DisaFileInfo::MPP)) {
		OUT_ERROR("Failed to find MPP!");
		clear();
		return;
	}

	const auto style = mppPack.getStyle();
	getOrCreateGameScene()->setModel(
		PG_DisaMPDContainer::createFromMPP(mpp, style.topIndex, style.sideIndex, style.decalIndex));
}

void PG_DisaPreviewItem::open(const PG_DisaGEOPack& geoPack) {
	auto geo = geoPack.getGEO();
	if (!(geo && geo.getInfo().getType() == PG_DisaFileInfo::GEO)) {
		OUT_ERROR("Failed to find GEO!");
		clear();
		return;
	}

	getOrCreateGameScene()->setModel(PG_DisaMPDContainer::createFromGEO(geo));
}

void PG_DisaPreviewItem::open(const PG_Model& model) {
	if (!model.isValid()) {
		clearImage();
		return;
	}
	getOrCreateGameScene()->setModel(PG_DisaMPDContainer::createFromModel(model));
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
