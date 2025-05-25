#pragma once

#include <Scene/Gui/PG_GuiRectangle.h>
#include <Resources/PG_Texture.h>
#include <vector>
#include <array>

class PG_GuiImage;
using PG_DisaUVItemOutlineType = std::vector<std::array<glm::vec2, 4>>;

class PG_DisaUVItem : public PG_GuiRectangle {
	PG_OBJECT(PG_DisaUVItem);
	PG_PARENT(PG_GuiRectangle);

	PG_PROPERTY(PG_Texture image GET getImage SET setImage CHANGED imageChanged);
	PG_PROPERTY(PG_DisaUVItemOutlineType outlines SET setOutlines);
signals:
	void imageChanged();
	void hoverAt(glm::ivec2 value);
	void clickedAt(glm::ivec2 value);

public:
	using PG_GuiRectangle::PG_GuiRectangle;

	void init() override;

	PG_Texture getImage() const;
	void setImage(const PG_Texture& img);

	void setOutlines(const std::vector<std::array<glm::vec2, 4>>& outlines);
private:
	glm::vec2 toImagePos(glm::vec2 value) const;

	PG_GuiImage* m_image = nullptr;
};