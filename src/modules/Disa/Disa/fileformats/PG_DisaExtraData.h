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

#include <algorithm>
#include <functional>
#include <limits>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include <Base/PG_Variety.h>

/// A number, shown as a spin box
struct PG_DisaExtraNumber {
	double min = 0.0;
	double max = 0.0;
	double step = 1.0;
	bool isInteger = true;
};

/// A line of text, shown as a text field
struct PG_DisaExtraText {
	std::string placeholder;
};

/// A flag, shown as a toggle button
struct PG_DisaExtraFlag {};

using PG_DisaExtraWidget = std::variant<PG_DisaExtraNumber, PG_DisaExtraText, PG_DisaExtraFlag>;

/**
* @brief One value inside a entry's extra data, and everything a editor needs to show it
* @details Built by the container that owns the entry, so a editor can put a widget on a value
* whose type it never hears about.
*/
struct PG_DisaExtraField {
	std::string title;
	std::string tooltip;
	PG_DisaExtraWidget widget;
	bool readOnly = false;
	/// The entry's name is built from this value, so a change to it renames the file
	bool nameRelevant = false;

	/// Reads this one value out of a entry's whole extra data
	std::function<PG_Variety(const PG_Variety&)> getValue;
	/// Writes this one value back, leaving the rest of the extra data alone
	std::function<void(PG_Variety&, const PG_Variety&)> setValue;
};

using PG_DisaExtraFields = std::vector<PG_DisaExtraField>;

/// True when a value the entry's name is built from differs between the two
bool nameRelevantChanged(const PG_DisaExtraFields& fields, const PG_Variety& before, const PG_Variety& after);

/**
* @brief A description for a container that stores one plain number per entry, its ID
* @details Archives of that kind store no names, the ID is what the entry is named after, so the
* field is name relevant.
* @param title What the number is called, like 'Script ID'
*/
template<typename T>
PG_DisaExtraFields makeIdDescription(std::string title, std::string tooltip = {}) {
	static_assert(std::is_integral_v<T>, "A ID has to be a whole number!");

	//PG_GuiSpinBox counts in double and its int setup takes int bounds, so a wider type is capped
	constexpr double intMax = (double)std::numeric_limits<int>::max();
	constexpr double typeMax = (double)std::numeric_limits<T>::max();
	constexpr double maxValue = typeMax < intMax ? typeMax : intMax;

	PG_DisaExtraField field;
	field.title = std::move(title);
	field.tooltip = std::move(tooltip);
	field.widget = PG_DisaExtraNumber{ (double)std::numeric_limits<T>::min(), maxValue };
	field.nameRelevant = true;
	field.getValue = [](const PG_Variety& extraData) { return PG_Variety(extraData.convert<T>()); };
	field.setValue = [](PG_Variety& extraData, const PG_Variety& value) { extraData = (T)value.convert<double>(); };

	PG_DisaExtraFields fields;
	fields.push_back(std::move(field));
	return fields;
}
