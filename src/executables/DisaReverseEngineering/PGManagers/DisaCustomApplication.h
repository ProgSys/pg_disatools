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

#include <Base/PG_Application.h>
#include <Util/PG_DeltaTimer.h>
#include <Resources/PG_ResourceSourceScope.h>
#include <Disa/resources/PG_DisaGameResourceSource.h>
#include <glm/glm.hpp>
#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <vector>

struct HWND__;
class DisaCustomWindow;
class DisaCustomSceneRenderer;
class PG_Scene;
class PG_GuiMain;
class PG_GuiItem;

/**
* @brief The pg_engine application inside the game.
* @details Holds the tree every pg application has, so pg content and the gui work as usual:
*
*     DisaCustomApplication
*       + DisaCustomWindow "DisaWindow"
*         + DisaCustomSceneRenderer "DisaSceneRenderer"
*           + PG_Scene "DisaCustomScene"
*             + root
*               + PG_GuiMain "DisaGuiMain"
*                 + PG_GuiItem "DisaGuiRoot" (client size)
*
* It replaces the window proc of the game window. Mouse, key and text messages and the XInput controller are
* translated into pg events and routed like PG_SDLApplication does: to the grabbing item, otherwise to the
* window (mouse) or the application (keys, text, controller). The game still receives every message.
*
* Lives on the game thread from the first frame until the game ends.
*/
class DisaCustomApplication : public PG_Application
{
	PG_OBJECT(DisaCustomApplication);
	PG_PARENT(PG_Application);
public:
	/// Sees a game window message first. Returning true ends the message with 0, neither pg nor the game get it.
	using NativeMessageHandler = std::function<bool(unsigned int message, std::uintptr_t wParam, std::intptr_t lParam)>;

	/**
	* @brief Creates the application on the first call, every later call only returns it.
	* @details Needs the game window and its GL context, so call it from the game thread once the game loop runs.
	* Starts the engine: event loop, glew, resource manager and context.
	*/
	static DisaCustomApplication* create();
	/// @returns The singleton, nullptr before create
	static DisaCustomApplication* get();
	/**
	* @brief Adds a handler for the raw messages of the game window.
	* @details Can be called from any thread and before create. Handlers run on the game thread in the order
	* they were added.
	*/
	static void addNativeMessageHandler(NativeMessageHandler handler);

	/**
	* @brief Runs the engine for one frame.
	* @details Polls the controller, posts the tick and render events and runs the event loop, so input, layout
	* and the gui render job are all done before the game polls its own input. Called by FUN_00424ae0_FrameBegin.
	*/
	void processFrame();

	/// The cursor in client pixels, worked out from the current cursor position. Can be outside the client area.
	glm::ivec2 getMousePosition() const override;
	glm::ivec2 getMouseGlobalPosition() const override;
	std::vector<PG_Window*> getWindows() const override;
	/// Milliseconds since create
	std::uint64_t getTicks() const override;
	void postInputEvent(PG_Event* e, PG_Window* window) override;
	/// Never quits, the game decides when it ends
	void foceQuit() override {}

	bool event(PG_Event* e) override;

	inline DisaCustomWindow* getWindow() const { return m_window; }
	inline DisaCustomSceneRenderer* getRenderer() const { return m_renderer; }
	inline PG_Scene* getScene() const { return m_scene; }
	inline PG_GuiMain* getGuiMain() const { return m_guiMain; }
	/// Parent of all gui items, it covers the client area
	inline PG_GuiItem* getGuiRoot() const { return m_guiRoot; }
	inline HWND__* getNativeWindow() const { return m_nativeWindow; }
	inline const PG_DeltaTimer& getDeltaTimer() const { return m_deltaTimer; }

	/// If true every routed input event is printed with its target. Meant to check the input bridge.
	inline bool isInputLogging() const { return m_inputLogging; }
	inline void setInputLogging(bool value) { m_inputLogging = value; }

	/**
	* @brief Called by the window proc for every message of the game window, before the game's own proc.
	* @details Runs the native handlers, then translates the message into pg events.
	* @returns true when a native handler took the message, the game then never sees it
	*/
	bool handleMessage(unsigned int message, std::uintptr_t wParam, std::intptr_t lParam);
private:
	explicit DisaCustomApplication(HWND__* nativeWindow);
	void createTree();

	void translateMessage(unsigned int message, std::uintptr_t wParam, std::intptr_t lParam);
	void translateMouseButton(unsigned int message, std::uintptr_t wParam, std::intptr_t lParam);
	void translateMouseWheel(bool isHorizontal, std::uintptr_t wParam, std::intptr_t lParam);
	void translateKey(bool isUp, std::uintptr_t wParam, std::intptr_t lParam);
	void translateChar(std::uintptr_t wParam);
	void releaseInput();
	void pollController();

	void postMouseEvent(PG_Event* e);
	void postKeyEvent(PG_Event* e);
	void postTextEvent(PG_Event* e);
	/// Runs the application filters for events that do not target the application, then posts it
	void routeEvent(PG_Event* e, PG_Object* target);

	HWND__* m_nativeWindow = nullptr;
	PG_ResourceSourceScope<PG_DisaGameResourceSource> m_gameResourceScope; /// the game folder under "disa"

	DisaCustomWindow* m_window = nullptr;
	DisaCustomSceneRenderer* m_renderer = nullptr;
	PG_Scene* m_scene = nullptr;
	PG_GuiMain* m_guiMain = nullptr;
	PG_GuiItem* m_guiRoot = nullptr;

	PG_DeltaTimer m_deltaTimer;
	std::uint64_t m_startTicks = 0;
	std::atomic_bool m_inputLogging = false; /// toggled from the command terminal thread

	//mouse
	glm::ivec2 m_lastMousePosition = glm::ivec2(-1);
	bool m_isMouseTracked = false;
	glm::ivec2 m_wheelRemainder = glm::ivec2(0); /// in WHEEL_DELTA fractions
	struct Click {
		int button = 0;
		glm::ivec2 position = glm::ivec2(0);
		std::uint64_t time = 0;
	};
	Click m_lastClick;

	//text
	char m_dbcsLeadByte = 0; /// first byte of a double byte character, WM_CHAR sends those in two messages

	//controller
	int m_controllerIndex = -1; /// the XInput user index, -1 while none is connected
	std::uint64_t m_nextControllerScan = 0;
	unsigned short m_controllerButtons = 0;
	std::array<glm::vec2, 5> m_controllerMotion = {}; /// last posted value per PG_EControllerMotion
};
