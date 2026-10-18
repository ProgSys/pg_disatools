#include "DisaRadialMenuStyle.h"

#include <Scene/Gui/PG_GuiButton.h>
#include <Scene/Gui/PG_GuiRectangle.h>
#include <Scene/Gui/PG_GuiText.h>
#include <Util/Color/ColorConverters.h>
#include <Util/Debug/PG_DebugOut.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <utility>

namespace {
	using Json = nlohmann::json;

	void warn(const std::string& path, const std::string& problem) {
		OUT_WARNING("Radial menu style: " << path << " " << problem << ", it is skipped");
	}

	/**
	* @brief Calls read with every key of the object and its path, read returns false for a key it doesn't know.
	* @returns false when the value is no object.
	*/
	template<typename Read>
	bool readObject(const Json& object, const std::string& path, Read&& read) {
		if (!object.is_object()) {
			warn(path, "is not an object");
			return false;
		}
		for (const auto& [key, value] : object.items()) {
			const std::string keyPath = path.empty() ? key : path + "." + key;
			if (!read(key, value, keyPath)) warn(keyPath, "is unknown");
		}
		return true;
	}

	void read(const Json& value, const std::string& path, std::optional<float>& out) {
		if (value.is_number()) out = value.get<float>();
		else warn(path, "is not a number");
	}

	void read(const Json& value, const std::string& path, std::optional<unsigned int>& out) {
		if (value.is_number_unsigned()) out = value.get<unsigned int>();
		else warn(path, "is not a whole number of 0 or more");
	}

	void read(const Json& value, const std::string& path, std::optional<bool>& out) {
		if (value.is_boolean()) out = value.get<bool>();
		else warn(path, "is not true or false");
	}

	void read(const Json& value, const std::string& path, std::optional<std::string>& out) {
		if (value.is_string()) out = value.get<std::string>();
		else warn(path, "is not a string");
	}

	void readColor(const Json& value, const std::string& path, std::optional<glm::vec3>& out) {
		if (value.is_string()) {
			if (const std::optional<glm::vec4> color = PG_Color::fromHex(value.get<std::string>())) {
				out = glm::vec3(*color);
				return;
			}
		}
		warn(path, "is not a color like \"#FFFFFF\"");
	}

	/// Keys left out keep the colors the set had, so an entry of buttons can change only the hover color.
	void readColorSet(const Json& value, const std::string& path, std::optional<PG_Gui::ColorSet>& out) {
		PG_Gui::ColorSet colors = out.value_or(PG_Gui::ColorSet{});
		const bool isObject = readObject(value, path, [&colors](const std::string& key, const Json& entry, const std::string& keyPath) {
			glm::vec3* target = key == "color" ? &colors.color : key == "hover" ? &colors.hover
				: key == "pressed" ? &colors.pressed : key == "disabled" ? &colors.disabled : nullptr;
			if (!target) return false;
			std::optional<glm::vec3> color;
			readColor(entry, keyPath, color);
			if (color) *target = *color;
			return true;
		});
		if (isObject) out = colors;
	}

	void readJointStyle(const Json& value, const std::string& path, std::optional<PG_GuiRadialMenu::EJointStyle>& out) {
		if (value == "rough") out = PG_GuiRadialMenu::Rough;
		else if (value == "gauged") out = PG_GuiRadialMenu::Gauged;
		else warn(path, "is neither \"rough\" nor \"gauged\"");
	}

	std::optional<std::pair<unsigned int, unsigned int>> readSlots(const Json& value, const std::string& path) {
		if (value.is_array() && value.size() == 2 && value[0].is_number_unsigned() && value[1].is_number_unsigned()) {
			const auto first = value[0].get<unsigned int>();
			const auto last = value[1].get<unsigned int>();
			if (first <= last) return std::pair{ first, last };
		}
		warn(path, "is not [first, last] with first up to last");
		return std::nullopt;
	}

	bool readLook(const std::string& key, const Json& value, const std::string& path, DisaRadialMenuStyle::Look& look) {
		if (key == "background") read(value, path, look.background);
		else if (key == "backgroundColors") readColorSet(value, path, look.backgroundColors);
		else if (key == "iconSize") read(value, path, look.iconSize);
		else if (key == "iconGap") read(value, path, look.iconGap);
		else if (key == "iconMaterial") read(value, path, look.iconMaterial);
		else if (key == "font") read(value, path, look.font);
		else if (key == "fontSize") read(value, path, look.fontSize);
		else if (key == "textColor") readColor(value, path, look.textColor);
		else if (key == "textMaterial") read(value, path, look.textMaterial);
		else return false;
		return true;
	}

	bool readContent(const std::string& key, const Json& value, const std::string& path, DisaRadialMenuStyle::Content& content) {
		if (key == "icon") read(value, path, content.icon);
		else if (key == "text") read(value, path, content.text);
		else return false;
		return true;
	}

	void readHoverText(const Json& value, const std::string& path, DisaRadialMenuStyle::HoverText& hoverText) {
		readObject(value, path, [&hoverText](const std::string& key, const Json& entry, const std::string& keyPath) {
			if (key == "enabled") read(entry, keyPath, hoverText.enabled);
			else if (key == "margin") read(entry, keyPath, hoverText.margin);
			else if (key == "font") read(entry, keyPath, hoverText.font);
			else if (key == "fontSize") read(entry, keyPath, hoverText.fontSize);
			else if (key == "color") readColor(entry, keyPath, hoverText.color);
			else if (key == "borderColor") readColor(entry, keyPath, hoverText.borderColor);
			else if (key == "material") read(entry, keyPath, hoverText.material);
			else return false;
			return true;
		});
	}

	void readButtons(const Json& value, const std::string& path, DisaRadialMenuStyle& style) {
		if (!value.is_array()) {
			warn(path, "is not a list");
			return;
		}
		for (size_t i = 0; i < value.size(); ++i) {
			const std::string entryPath = path + "[" + std::to_string(i) + "]";
			DisaRadialMenuStyle::Button button{ .look = style.buttonLook };
			std::optional<std::pair<unsigned int, unsigned int>> slots;
			readObject(value[i], entryPath, [&](const std::string& key, const Json& entry, const std::string& keyPath) {
				if (key == "name") read(entry, keyPath, button.name);
				else if (key == "slots") slots = readSlots(entry, keyPath);
				else if (key == "menuText") read(entry, keyPath, button.menuText);
				else return readContent(key, entry, keyPath, button.content) || readLook(key, entry, keyPath, button.look);
				return true;
			});
			if (!slots) {
				warn(entryPath, "has no \"slots\": [first, last]");
				continue;
			}
			button.firstSlot = slots->first;
			button.lastSlot = slots->second;
			style.buttons.push_back(std::move(button));
		}
	}
}

std::optional<DisaRadialMenuStyle> DisaRadialMenuStyle::load(const std::string& path) {
	std::ifstream file(path);
	if (!file) {
		OUT_ERROR("Radial menu style: can't read " << path);
		return std::nullopt;
	}
	const Json root = Json::parse(file, nullptr, false, true);
	if (root.is_discarded() || !root.is_object()) {
		OUT_ERROR("Radial menu style: " << path << " holds no valid JSON object");
		return std::nullopt;
	}

	DisaRadialMenuStyle style;
	//the buttons start from the look of the menu, so they are read after everything else
	const Json* buttons = nullptr;
	readObject(root, "", [&](const std::string& key, const Json& value, const std::string& keyPath) {
		if (key == "slotCount") read(value, keyPath, style.slotCount);
		else if (key == "angleOffset") read(value, keyPath, style.angleOffset);
		else if (key == "innerRadius") read(value, keyPath, style.innerRadius);
		else if (key == "outerRadius") read(value, keyPath, style.outerRadius);
		else if (key == "gap") read(value, keyPath, style.gap);
		else if (key == "jointStyle") readJointStyle(value, keyPath, style.jointStyle);
		else if (key == "hoverText") readHoverText(value, keyPath, style.hoverText);
		else if (key == "button") {
			readObject(value, keyPath, [&style](const std::string& lookKey, const Json& entry, const std::string& lookPath) {
				return readLook(lookKey, entry, lookPath, style.buttonLook);
			});
		}
		else if (key == "deadZone") {
			readObject(value, keyPath, [&style](const std::string& zoneKey, const Json& entry, const std::string& zonePath) {
				return readContent(zoneKey, entry, zonePath, style.deadZoneContent) || readLook(zoneKey, entry, zonePath, style.deadZoneLook);
			});
		}
		else if (key == "buttons") buttons = &value;
		else return false;
		return true;
	});
	if (buttons) readButtons(*buttons, "buttons", style);
	return style;
}

PG_GuiRadialMenu* DisaRadialMenuStyle::create(const std::string& name, PG_GuiItem* parent) const {
	//the menu has no defaults for these two, the fallbacks are the ones of the sandbox
	auto menu = PG_new<PG_GuiRadialMenu>(name, slotCount.value_or(4u), angleOffset.value_or(45.f), parent);
	applyLayout(menu);
	applyLook(menu);
	for (const Button& button : buttons) button.addTo(menu);
	return menu;
}

void DisaRadialMenuStyle::applyLayout(PG_GuiRadialMenu* menu) const {
	if (innerRadius) menu->setInnerRadius(*innerRadius);
	if (outerRadius) menu->setOuterRadius(*outerRadius);
	if (gap) menu->setGap(*gap);
	if (jointStyle) menu->setJointStyle(*jointStyle);
	if (hoverText.enabled) menu->setHoverTextEnabled(*hoverText.enabled);
}

void DisaRadialMenuStyle::applyLook(PG_GuiRadialMenu* menu) const {
	if (hoverText.margin) menu->setMenuTextMargin(*hoverText.margin);
	if (PG_GuiText* text = menu->getMenuText()) {
		if (hoverText.font) text->setFont(*hoverText.font);
		if (hoverText.fontSize) text->setFontSize(*hoverText.fontSize);
		if (hoverText.color) text->setColor(*hoverText.color);
		if (hoverText.borderColor) text->setBorderColor(*hoverText.borderColor);
		if (hoverText.material) text->setMaterial(PG_Material(*hoverText.material));
	}
	deadZoneContent.apply(menu->getDeadZone());
	deadZoneLook.apply(menu->getDeadZone());
}

PG_GuiRadialMenuButton* DisaRadialMenuStyle::Button::addTo(PG_GuiRadialMenu* menu) const {
	PG_GuiRadialMenuButton* button = menu->addButton(name.value_or("radialButton" + std::to_string(firstSlot)), firstSlot, lastSlot);
	if (!button) {
		OUT_WARNING("Radial menu style: slots " << firstSlot << "-" << lastSlot << " don't fit a menu of " << menu->getSlotCount()
			<< " slots, the button is skipped");
		return nullptr;
	}
	content.apply(button);
	if (menuText) button->setPropertyValue("menuText", PG_Any::from(*menuText));
	look.apply(button);
	return button;
}

void DisaRadialMenuStyle::Content::apply(PG_GuiButton* button) const {
	//PG_GuiButton would size it by the rect, far too big for a slice of a ring, Look::iconSize sets it later
	constexpr float iconSize = 32.f;
	if (icon) button->setIcon(*icon, glm::vec2(iconSize));
	if (text && !text->empty()) button->setText(*text);
}

void DisaRadialMenuStyle::Look::apply(PG_GuiButton* button) const {
	if (background) button->setBackground(*background);
	if (backgroundColors) button->setBackgroundColor(*backgroundColors);
	if (iconGap && dynamic_cast<PG_GuiRadialMenuButton*>(button)) button->setPropertyValue("iconGap", PG_Any::from(*iconGap));
	if (PG_GuiItem* icon = button->getIcon()) {
		if (iconSize) icon->setSize(glm::vec2(*iconSize));
		auto image = dynamic_cast<PG_GuiRectangle*>(icon);
		if (iconMaterial && image) image->setMaterial(*iconMaterial);
	}
	if (auto text = dynamic_cast<PG_GuiText*>(button->getText())) {
		if (font) text->setFont(*font);
		if (fontSize) text->setFontSize(*fontSize);
		if (textColor) text->setColor(*textColor);
		if (textMaterial) text->setMaterial(PG_Material(*textMaterial));
	}
}
