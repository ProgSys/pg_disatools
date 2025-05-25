#pragma once

#include <Scene/PG_SpriteNode.h>

class PG_DisaSpriteNode : public PG_SpriteNode {
	PG_OBJECT(PG_DisaSpriteNode);
	PG_PARENT(PG_SpriteNode);

	PG_PROPERTY(int colorTableOffset GET getColorTableOffset SET setColorTableOffset)
public:
	using PG_SpriteNode::PG_SpriteNode;

	int getColorTableOffset() const { return m_colorTableOffset; }
	void setColorTableOffset(int value) { m_colorTableOffset = value; }

private:
	int m_colorTableOffset = 0;
};