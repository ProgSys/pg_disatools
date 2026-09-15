#pragma once

#include <Scene/PG_SpriteNode.h>

class PG_DisaSpriteNode : public PG_SpriteNode {
	PG_OBJECT(PG_DisaSpriteNode);
	PG_PARENT(PG_SpriteNode);

	PG_PROPERTY(int colorTableOffset GET getColorTableOffset SET setColorTableOffset)
	PG_PROPERTY(glm::vec4 colorMult GET getColorMult SET setColorMult)
public:
	using PG_SpriteNode::PG_SpriteNode;

	int getColorTableOffset() const { return m_colorTableOffset; }
	void setColorTableOffset(int value) { m_colorTableOffset = value; }

	glm::vec4 getColorMult() const { return m_colorMult; }
	void setColorMult(glm::vec4 value) { m_colorMult = value; }

private:
	int m_colorTableOffset = 0;
	glm::vec4 m_colorMult = glm::vec4(1.f);
};