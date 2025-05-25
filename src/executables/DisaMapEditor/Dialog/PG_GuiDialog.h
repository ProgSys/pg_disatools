#pragma once

#include <Scene/Gui/PG_GuiItem.h>


class PG_GuiImage;
class PG_GuiText;


class PG_GuiDialog : public PG_GuiItem
{
	PG_OBJECT(PG_GuiDialog);
	PG_PARENT(PG_GuiItem);
public:
	PG_GuiDialog(const std::string& name);

	void init() override;

	inline float getAspectRatio() const { return m_aspectRatio;}
	inline void setAspectRatio(float aspectRatio) {m_aspectRatio = aspectRatio;}

	void setText(const std::string& text);

private:

	float m_aspectRatio = 16.0f / 9.0f;
	PG_GuiItem* m_character = nullptr;
	PG_GuiImage* m_speechBubble = nullptr;
	PG_GuiText* m_text = nullptr;
	PG_GuiItem* m_characterNameplate = nullptr;
};