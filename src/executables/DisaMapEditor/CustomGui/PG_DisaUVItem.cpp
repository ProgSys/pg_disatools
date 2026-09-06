#include "PG_DisaUVItem.h"

#include <Scene/Gui/PG_GuiImage.h>
#include <Scene/Gui/PG_GuiLines.h>
#include <Scene/Gui/PG_GuiMouseArea.h>
#include <Util/Color/ColorDefined.h>
#include <Util/Color/ColorConverters.h>
#include <Base/Events/PG_MouseMotionEvent.h>
#include <Base/Events/PG_MouseButtonEvent.h>

void PG_DisaUVItem::init() {
	PG_GuiRectangle::init();

	setColor(PG_Color::rgbToF(glm::rgba(60)));
	
	m_image = PG_new<PG_GuiImage>("image", this);
	m_image->fill(this);
	m_image->setMargin(1.f);

	auto area = PG_new<PG_GuiMouseArea>("area", m_image);
	area->fill(this);
	area->setHoverEnabled(true);

	connect(area, &PG_GuiMouseArea::mouseMotion, this, [this](PG_MouseMotionEvent* e) {
		emit hoverAt(toImagePos(e->getPosition()));
	});

	connect(area, &PG_GuiMouseArea::mouseClicked, this, [this](PG_MouseButtonEvent* e) {
		emit clickedAt(toImagePos(e->getPosition()));
		});
}

glm::vec2 PG_DisaUVItem::toImagePos(glm::vec2 value) const {
	value = m_image->toLocalTranslation(value);
	value /= m_image->getSize();
	return glm::round(value * glm::vec2(255.f));
}

PG_Texture PG_DisaUVItem::getImage() const {
	return std::get<PG_Texture>(m_image->getSourceData());
}

void PG_DisaUVItem::setImage(const PG_Texture& img) {
	m_image->setSourceData(img);
	emit imageChanged();
}

void PG_DisaUVItem::setOutlines(const std::vector<std::array<glm::vec2, 4>>& outlines) {
	std::vector<PG_GuiLines*> lines = m_image->findChidren<PG_GuiLines>();
	auto it = lines.begin();
	for (const auto& outline : outlines) {
		PG_GuiLines* line;
		if (it == lines.end()) {
			line = PG_new<PG_GuiLines>("Outline", m_image);
			line->fill(m_image);
			line->setColor(PG_Color::hentaiRedFrgb);
		}
		else {
			line = *it;
			++it;
		}
		std::vector<glm::vec4> transform{
			glm::vec4(outline[0], 0.f, 1.f),
			glm::vec4(outline[1], 0.f, 1.f),

			glm::vec4(outline[1], 0.f, 1.f),
			glm::vec4(outline[2], 0.f, 1.f),

			glm::vec4(outline[2], 0.f, 1.f),
			glm::vec4(outline[3], 0.f, 1.f),

			glm::vec4(outline[3], 0.f, 1.f),
			glm::vec4(outline[0], 0.f, 1.f),
		};
		line->setPoints(transform);
	}

	for (; it != lines.end(); ++it) {
		delete* it;
	}

	//lines->setColor(PG_Color::hentaiRedFrgb);
}