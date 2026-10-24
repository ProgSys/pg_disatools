/*
 * GNU Lesser General Public License (LGPL-3.0)
 *
 * Copyright (c) 2026 ProgSys
 *
 * This code is part of a free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 * This code is distributed in the hope that it will be useful, without any
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 * or https://www.gnu.org/licenses/lgpl-3.0.md
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once
#include <Scene/Gui/PG_GuiRadialMenu.h>
#include <Scene/Gui/PG_GuiUtil.h>
#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <utility>
#include <vector>

class PG_GuiButton;
class PG_GuiItem;

/**
* @brief A PG_GuiRadialMenu described by a JSON file, its layout, its buttons, their sizes, colors and materials.
* @details Every key is optional. A missing key leaves its setter uncalled, so the default of the menu or the button
* stays. Colors are "#RRGGBB", comments are allowed. A copy of the RadialMenuStyle of the SceneApp sandbox, which
* also gives every button a "name" to tell them apart by.
*/
struct DisaRadialMenuStyle {
	/**
	* @brief How a button looks. The one of the menu goes for every button, an entry of buttons can override any key.
	*/
	struct Look {
		std::optional<std::string> background; /// material
		std::optional<PG_Gui::ColorSet> backgroundColors;
		std::optional<float> iconSize; /// pixels, the icon is square
		std::optional<float> iconGap; /// pixels between icon and text, only a PG_GuiRadialMenuButton has it
		std::optional<std::string> iconMaterial;
		std::optional<std::string> font; /// a .ttf in the fonts folder, like "Disa/DisaFont-Bold.ttf"
		std::optional<float> fontSize;
		std::optional<glm::vec3> textColor;
		std::optional<std::string> textMaterial; /// on a PG_GuiRadialMenuButton it has to bend, like M_GuiTextSDFArc

		/**
		* @brief Sets the keys that are present. The icon and text keys skip a button without icon or text.
		*/
		void apply(PG_GuiButton* button) const;
	};

	/// What a button shows, empty for nothing.
	struct Content {
		std::optional<std::string> icon; /// image name or path
		std::optional<std::string> text; /// inside the button

		void apply(PG_GuiButton* button) const;
	};

	struct Button {
		std::optional<std::string> name; /// the name of the button item, "radialButton" + firstSlot without one
		unsigned int firstSlot = 0;
		unsigned int lastSlot = 0; /// included
		Content content;
		std::optional<std::string> menuText;
		Look look; /// the look of the menu with the keys of this entry over it

		/**
		* @brief Adds it to the menu.
		* @returns nullptr when the slots don't fit the menu, which is logged.
		*/
		PG_GuiRadialMenuButton* addTo(PG_GuiRadialMenu* menu) const;
	};

	struct HoverText {
		std::optional<bool> enabled;
		std::optional<float> margin;
		std::optional<std::string> font; /// a .ttf in the fonts folder, like "Disa/DisaFont-Bold.ttf"
		std::optional<float> fontSize;
		std::optional<glm::vec3> color;
		std::optional<glm::vec3> borderColor; /// only a border material shows it, like M_GuiTextBorderSDFArc
		std::optional<std::string> material; /// it has to bend, like M_GuiTextSDFArc
	};

	std::optional<unsigned int> slotCount;
	std::optional<float> angleOffset; /// degrees
	std::optional<float> innerRadius;
	std::optional<float> outerRadius;
	std::optional<float> gap; /// pixels
	std::optional<PG_GuiRadialMenu::EJointStyle> jointStyle;
	/// Window height in pixels the sizes are meant for, the menu is scaled by the window height over it
	std::optional<float> referenceHeight;
	HoverText hoverText;
	Look buttonLook;
	Content deadZoneContent;
	Look deadZoneLook;
	std::vector<Button> buttons;

	/**
	* @brief Reads a style file. Unknown keys and wrong types are warned about with their path and skipped.
	* @returns std::nullopt when the file can't be read or holds no valid JSON, the error is logged.
	*/
	static std::optional<DisaRadialMenuStyle> load(const std::string& path);
	/**
	* @brief Reads a style file through the resource manager, like "mod::scripts/style.json" out of a mod.
	* @returns std::nullopt when nobody carries the file or it holds no valid JSON, the error is logged.
	*/
	static std::optional<DisaRadialMenuStyle> loadResource(const std::string& name);
	/**
	* @brief A plain style made of the materials of the engine, one slot for each button and its label as text.
	* @param buttons The name of each button and its label, clockwise from the top.
	*/
	static DisaRadialMenuStyle createDefault(const std::vector<std::pair<std::string, std::string>>& buttons);

	/**
	* @brief A closed menu with everything of the style, slot count and angle offset are fixed on creation.
	*/
	PG_GuiRadialMenu* create(const std::string& name, PG_GuiItem* parent) const;
	/**
	* @brief The radii, the gap, the joint style and whether there is a hover text.
	*/
	void applyLayout(PG_GuiRadialMenu* menu) const;
	/**
	* @brief The look of the hover text, if there is one, and the dead zone.
	*/
	void applyLook(PG_GuiRadialMenu* menu) const;
};
