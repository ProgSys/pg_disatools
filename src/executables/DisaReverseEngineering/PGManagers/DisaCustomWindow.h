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

#include <Base/PG_Window.h>
#include <Resources/PG_ContextData.h>
#include <glm/glm.hpp>

struct HWND__;

/**
* @brief Window data for the game's own GL context.
* @details The game creates, binds and swaps its context itself, so every call here does nothing. It exists so
* BasicRenderData::window is set like in any pg application.
*/
class DisaCustomWindowData : public PG_WindowData {
public:
	PG_ContextData_SPtr createContext() override { return std::make_shared<PG_ContextData>(); }
	bool makeCurrent(PG_ContextData*) override { return true; }
	void swapWindow(PG_ContextData*) override {}
};

/**
* @brief Stands in for the game window.
* @details The size is the client area in pixels. DisaCustomApplication keeps it up to date and the window
* posts a PG_WindowResizeEvent down its tree when it changes, which resizes the gui root.
*/
class DisaCustomWindow : public PG_Window {
	PG_OBJECT(DisaCustomWindow);
	PG_PARENT(PG_Window);
public:
	DisaCustomWindow(const std::string& name, HWND__* nativeWindow);

	glm::ivec2 getSize() const override { return m_size; }
	std::string getTitle() const override;
	bool isShown() const override;

	inline HWND__* getNativeWindow() const { return m_nativeWindow; }
	/**
	* @brief Takes the new client size and posts a PG_WindowResizeEvent to itself when it differs.
	* @details A zero size, like the one of a minimized window, is ignored so the gui keeps its layout.
	*/
	void setSize(glm::ivec2 size);
	/// Reads the client size from the native window, @see setSize
	void updateSize();
private:
	HWND__* m_nativeWindow = nullptr;
	glm::ivec2 m_size = glm::ivec2(0);
};
