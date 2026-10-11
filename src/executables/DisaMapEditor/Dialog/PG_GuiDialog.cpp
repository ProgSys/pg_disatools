#include "PG_GuiDialog.h"
#include <glm/glm.hpp>
#include <Scene/PG_NodeTypes.h>

#include <Scene/Gui/PG_GuiImage.h>
#include <Scene/Gui/PG_GuiModel.h>
#include <Scene/Gui/PG_GuiSprite.h>
#include <Scene/Gui/PG_GuiText.h>
#include <Scene/Gui/PG_GuiTextEdit.h>

PG_GuiDialog::PG_GuiDialog(const std::string& name) :
	PG_GuiItem(name, PG_TypeInfo::create<PG_GuiDialog>(PG_Type::Node_GuiDialog, { PG_Type::Node_GuiItem, PG_Type::Node_GuiDialog })){

	m_text = PG_new<PG_GuiTextEdit>("IntroText", "Disa/DisaFont-Regular.ttf", this);
}

void PG_GuiDialog::init() {
	PG_GuiItem::init();

	auto sprite = PG_new<PG_GuiSprite>("N_Anna", glm::vec2(0.0f), glm::vec2(400.0f, 200.0f), PG_Sprite("f_char_anna_idle01.pgsp"), this);
	sprite->setModelAnchor(PG_GuiSprite::BottomCenter);
	sprite->setFitType(PG_GuiSprite::MaintainAspectHorizontal);
	sprite->setFlipHorizontal(true);
	sprite->setFlipVertical(true);
	sprite->getOffsetTransformation().setTranslation(glm::vec3(-0.18f, 0.17f, 0.0f));
	sprite->getOffsetTransformation().setScale(0.015f);
	anchor(sprite, PG_Anchor::Left, this, PG_Anchor::Left);
	anchor(sprite, PG_Anchor::Bottom, this, PG_Anchor::Bottom);
	m_character = sprite;

	m_speechBubble = PG_new<PG_GuiImage>("SpeechBubble", glm::vec2(0.0f), glm::vec2(1100.0f, 300.0f), this);
	m_speechBubble->setSource("dialog/speech_bubble.png");
	m_speechBubble->setFitType(PG_GuiImage::Stretch);
	anchor(m_speechBubble, PG_Anchor::HorizontalCenter, this, PG_Anchor::HorizontalCenter);
	anchor(m_speechBubble, PG_Anchor::Bottom, this, PG_Anchor::Bottom);

	connect(m_speechBubble, &PG_GuiImage::afterGuiUpdate, this, &PG_GuiDialog::update);

	constexpr glm::vec3 bobbleTextColor = glm::vec3(76.f, 111.f, 199.f) / 255.f;

	assert(m_text);
	m_text->moveToBack();
	m_text->setFontSize(26.f);
	m_text->setColor(bobbleTextColor);
	m_text->setMaterial(PG_Material("M_GuiTextBorderSDF"));
	m_text->setBorderColor(glm::vec3(0.9f));
	m_text->setHorizontalAlignment(PG_Anchor::Left);
	m_text->setVerticalAlignment(PG_Anchor::VerticalCenter);
	anchor(m_text, PG_Anchor::HorizontalCenter, m_speechBubble, PG_Anchor::HorizontalCenter);
	anchor(m_text, PG_Anchor::VerticalCenter, m_speechBubble, PG_Anchor::VerticalCenter);

	auto nameplate = PG_new<PG_GuiSprite>("NameplateRem", glm::vec2(0.0f), glm::vec2(400.0f, 200.0f), PG_Sprite("f_char_anna_name_tag.pgsp"), this);
	nameplate->setFlipHorizontal(true);
	nameplate->setFlipVertical(true);
	nameplate->setModelAnchor(PG_GuiSprite::Center);
	nameplate->getOffsetTransformation().setTranslation(glm::vec3(0.f, -0.2f, 0.0f));
	nameplate->getOffsetTransformation().setScale(0.8f);
	anchor(nameplate, PG_Anchor::Top, m_speechBubble, PG_Anchor::Top);
	anchor(nameplate, PG_Anchor::Left, m_speechBubble, PG_Anchor::Left);
	m_characterNameplate = nameplate;

	connect(this, &PG_GuiItem::midGuiUpdate, this, [this](glm::vec2* pos, glm::vec2* size) {
		if (size->x < 0.0f || size->y < 0.0f) return;
		const glm::vec2 old = *size;
		size->x = std::min(std::max(size->y * m_aspectRatio, 256.0f) * glm::clamp(500.0f / size->y, 1.0f, 3.0f), old.x);
		pos->x -= (size->x - old.x) / 2.0f;
		pos->y -= (size->y - old.y);



		assert(m_speechBubble);
		const glm::vec2 sizeImageBubble = m_speechBubble->getSourceSize();
		float scaleBubble = glm::clamp(600.0f / size->x, 0.4f, 1.0f);

		if(size->x/ size->y < sizeImageBubble.x/ sizeImageBubble.y)
			scaleBubble *= size->x / sizeImageBubble.x;
		else
			scaleBubble *= size->y / sizeImageBubble.y;

		const glm::vec2 sizeBubble = sizeImageBubble * scaleBubble;
		m_speechBubble->setSize(sizeBubble);


		assert(m_character);
		m_character->setSize(glm::vec2(size->x / 4.0f, std::max(size->y, 500.0f)));
		float a = glm::clamp((((old.x - sizeBubble.x) / sizeBubble.x) * -150.0f) + 80.0f, -200.0f, 0.0f);
		m_character->setPosition(glm::vec2( a , 0.0f));


		assert(m_text);
		m_text->setSize(sizeBubble * 0.9f);
		const float yScale = sizeBubble.y / 9.0f;

		m_text->setFontSize(yScale);
		m_text->setBaselineScale(1.5f);

		assert(m_characterNameplate);
		const glm::vec2 sizeImageNameplate(200.f, 70.f); 
		m_characterNameplate->setSize(sizeImageNameplate * 0.8f * scaleBubble);
		m_characterNameplate->setPosition(glm::vec2((size->x - sizeBubble.x)/55.0f, -1.0f * yScale));
	});
}

void PG_GuiDialog::setText(const std::string& text) {
	if (m_text) m_text->setText(text);
}
