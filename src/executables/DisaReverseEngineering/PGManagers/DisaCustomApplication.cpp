#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "DisaCustomApplication.h"
#include "DisaCustomWindow.h"
#include "DisaCustomSceneRenderer.h"
#include "DisaWin32Input.h"

#include <gl/glew.h>
#include <Base/PG_EventLoop.h>
#include <Base/Events/PG_TickEvent.h>
#include <Base/Events/PG_RenderEvent.h>
#include <Base/Events/PG_KeyEvent.h>
#include <Base/Events/PG_TextInputEvent.h>
#include <Base/Events/PG_MouseButtonEvent.h>
#include <Base/Events/PG_MouseMotionEvent.h>
#include <Base/Events/PG_MouseWheelEvent.h>
#include <Base/Events/PG_ControllerButtonEvent.h>
#include <Base/Events/PG_ControllerMotionEvent.h>
#include <Resources/PG_Context.h>
#include <Resources/PG_ResourceManager.h>
#include <Scene/PG_Scene.h>
#include <Scene/Gui/PG_GuiMain.h>
#include <Scene/Gui/PG_GuiItem.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Managers/DisaCustomBaseModManager.h>
#include <FunctionPatcher.h>

#include <mutex>
#include <sstream>

#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "xinput9_1_0.lib")

static DisaCustomApplication* g_customApplication = nullptr;
static WNDPROC g_originalWindowProc = nullptr;

static std::mutex g_nativeHandlersMutex;
static std::vector<DisaCustomApplication::NativeMessageHandler> g_nativeHandlers;

static LRESULT CALLBACK customWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (g_customApplication && g_customApplication->handleMessage(message, wParam, lParam)) return 0;
	if (g_originalWindowProc) return CallWindowProcA(g_originalWindowProc, hwnd, message, wParam, lParam);
	return DefWindowProcA(hwnd, message, wParam, lParam);
}

static std::string describeInputEvent(PG_Event* e) {
	std::stringstream out;
	switch (e->getType()) {
	case PG_Event_MouseButton: {
		auto* button = static_cast<PG_MouseButtonEvent*>(e);
		out << "MouseButton " << button->getButton() << (button->isDown() ? " down" : " up") << " at " << button->getPosition().x << ", " << button->getPosition().y;
		if (button->isDoubleClick()) out << " double";
	} break;
	case PG_Event_MouseMotion: {
		auto* motion = static_cast<PG_MouseMotionEvent*>(e);
		out << "MouseMotion " << motion->getPosition().x << ", " << motion->getPosition().y;
	} break;
	case PG_Event_MouseWheel: {
		auto* wheel = static_cast<PG_MouseWheelEvent*>(e);
		out << "MouseWheel " << wheel->getMove().x << ", " << wheel->getMove().y;
	} break;
	case PG_Event_Key: {
		auto* key = static_cast<PG_KeyEvent*>(e);
		out << "Key " << key->getKey() << " scancode " << key->getScanecode() << (key->isDown() ? " down" : " up") << (key->isRepeat() ? " repeat" : "") << " mod " << key->getKeyMod();
	} break;
	case PG_Event_TextInput:
		out << "TextInput '" << static_cast<PG_TextInputEvent*>(e)->getText() << "'";
		break;
	case PG_Event_ControllerButton: {
		auto* button = static_cast<PG_ControllerButtonEvent*>(e);
		out << "ControllerButton " << button->getButton() << (button->isDown() ? " down" : " up");
	} break;
	case PG_Event_ControllerMotion: {
		auto* motion = static_cast<PG_ControllerMotionEvent*>(e);
		out << "ControllerMotion " << motion->getMotionType() << " " << motion->getMotion().x << ", " << motion->getMotion().y;
	} break;
	default:
		out << "Event " << e->getType();
		break;
	}
	return out.str();
}

DisaCustomApplication* DisaCustomApplication::create() {
	if (g_customApplication) return g_customApplication;

	if (const GLenum error = glewInit(); error != GLEW_OK) {
		OUT_ERROR("glewInit failed, something is seriously wrong: " << glewGetErrorString(error));
		exit(1);
	}

	HWND nativeWindow = FP::getData<HWND>(FP::getData<uint32_t>(0x005bae70) + 8);
	g_customApplication = new DisaCustomApplication(nativeWindow); //PG_Application starts the event loop
	PG_ResourceManager::create(DisaCustomBaseModManager::get()->exePath().getPath() + "/resources");
	PG_Context::create();
	g_customApplication->createTree();

	g_originalWindowProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(nativeWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&customWindowProc)));
	return g_customApplication;
}

DisaCustomApplication* DisaCustomApplication::get() {
	return g_customApplication;
}

void DisaCustomApplication::addNativeMessageHandler(NativeMessageHandler handler) {
	std::lock_guard lock(g_nativeHandlersMutex);
	g_nativeHandlers.push_back(std::move(handler));
}

DisaCustomApplication::DisaCustomApplication(HWND__* nativeWindow) : m_nativeWindow(nativeWindow), m_startTicks(GetTickCount64()) {
	m_controllerMotion.fill(glm::vec2(0.f));
}

void DisaCustomApplication::createTree() {
	m_window = PG_new<DisaCustomWindow>("DisaWindow", m_nativeWindow, this);
	m_renderer = PG_new<DisaCustomSceneRenderer>("DisaSceneRenderer", m_window);
	m_scene = PG_new<PG_Scene>("DisaCustomScene", m_renderer);
	m_guiMain = PG_new<PG_GuiMain>("DisaGuiMain", m_scene->getRoot());
	m_guiRoot = PG_new<PG_GuiItem>("DisaGuiRoot", m_guiMain);
	m_renderer->setGuiMain(m_guiMain);
}

void DisaCustomApplication::processFrame() {
	m_deltaTimer.tick();
	pollController();

	const double deltaSeconds = m_deltaTimer.getDeltaTime();
	//the scene animates on ticks, which are in milliseconds
	PG_EventLoop::postEvent(new PG_TickEvent(deltaSeconds * 1000.0), this);
	PG_EventLoop::postEvent(new PG_RenderEvent(deltaSeconds), this);
	PG_EventLoop::doRun();

	PG_Context::runCreate();
	PG_Context::destroyGarbage();
}

glm::ivec2 DisaCustomApplication::getMousePosition() const {
	POINT cursor;
	if (!GetCursorPos(&cursor) || !ScreenToClient(m_nativeWindow, &cursor)) return glm::ivec2(-1);
	return glm::ivec2(cursor.x, cursor.y);
}

glm::ivec2 DisaCustomApplication::getMouseGlobalPosition() const {
	POINT cursor;
	if (!GetCursorPos(&cursor)) return glm::ivec2(-1);
	return glm::ivec2(cursor.x, cursor.y);
}

std::vector<PG_Window*> DisaCustomApplication::getWindows() const {
	if (!m_window) return {};
	return { m_window };
}

std::uint64_t DisaCustomApplication::getTicks() const {
	return GetTickCount64() - m_startTicks;
}

void DisaCustomApplication::postInputEvent(PG_Event* e, PG_Window*) {
	assert(e);
	switch (e->getType()) {
	case PG_Event_MouseButton:
	case PG_Event_MouseMotion:
	case PG_Event_MouseWheel: postMouseEvent(e); break;
	case PG_Event_TextInput: postTextEvent(e); break;
	default: postKeyEvent(e); break;
	}
}

bool DisaCustomApplication::event(PG_Event* e) {
	if (filterEventProcess(e)) return true;
	switch (e->getType()) {
	case PG_Event_Quit: e->accept(); return true;
	case PG_Event_Render: return false;
	default: return PG_Object::event(e);
	}
}

bool DisaCustomApplication::handleMessage(unsigned int message, std::uintptr_t wParam, std::intptr_t lParam) {
	{
		std::lock_guard lock(g_nativeHandlersMutex);
		for (const NativeMessageHandler& handler : g_nativeHandlers) {
			if (handler(message, wParam, lParam)) return true;
		}
	}
	translateMessage(message, wParam, lParam);
	return false;
}

void DisaCustomApplication::translateMessage(unsigned int message, std::uintptr_t wParam, std::intptr_t lParam) {
	switch (message) {
	case WM_MOUSEMOVE: {
		if (!m_isMouseTracked) {
			TRACKMOUSEEVENT track{ sizeof(TRACKMOUSEEVENT), TME_LEAVE, m_nativeWindow, 0 };
			m_isMouseTracked = TrackMouseEvent(&track);
		}
		const glm::ivec2 position = DisaWin32Input::toMousePosition(lParam);
		const glm::ivec2 relative = m_lastMousePosition.x < 0 ? glm::ivec2(0) : position - m_lastMousePosition;
		m_lastMousePosition = position;
		postMouseEvent(new PG_MouseMotionEvent(m_window->getWindowId(), DisaWin32Input::getMouseButtons(), position, relative, m_window->getSize(), DisaWin32Input::getKeymod()));
	} break;
	case WM_MOUSELEAVE: {
		m_isMouseTracked = false;
		m_lastMousePosition = glm::ivec2(-1);
		postMouseEvent(new PG_MouseMotionEvent(m_window->getWindowId(), 0, glm::ivec2(-1), glm::ivec2(-1), m_window->getSize(), PG_KMod_None));
	} break;
	case WM_LBUTTONDOWN: case WM_MBUTTONDOWN: case WM_RBUTTONDOWN: case WM_XBUTTONDOWN:
	case WM_LBUTTONUP: case WM_MBUTTONUP: case WM_RBUTTONUP: case WM_XBUTTONUP:
		translateMouseButton(message, wParam, lParam);
		break;
	case WM_MOUSEWHEEL: translateMouseWheel(false, wParam, lParam); break;
	case WM_MOUSEHWHEEL: translateMouseWheel(true, wParam, lParam); break;
	case WM_KEYDOWN: case WM_SYSKEYDOWN: translateKey(false, wParam, lParam); break;
	case WM_KEYUP: case WM_SYSKEYUP: translateKey(true, wParam, lParam); break;
	case WM_CHAR: translateChar(wParam); break;
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED) m_window->setSize(glm::ivec2(LOWORD(lParam), HIWORD(lParam)));
		break;
	case WM_KILLFOCUS: releaseInput(); break;
	default: break;
	}
}

void DisaCustomApplication::translateMouseButton(unsigned int message, std::uintptr_t wParam, std::intptr_t lParam) {
	const int button = DisaWin32Input::toMouseButton(message, wParam);
	if (!button) return;
	const bool isUp = message == WM_LBUTTONUP || message == WM_MBUTTONUP || message == WM_RBUTTONUP || message == WM_XBUTTONUP;
	const glm::ivec2 position = DisaWin32Input::toMousePosition(lParam);
	const int buttons = DisaWin32Input::getMouseButtons();

	//keeps the up message of a press that leaves the window
	if (!isUp && GetCapture() != m_nativeWindow) SetCapture(m_nativeWindow);
	if (isUp && buttons == 0 && GetCapture() == m_nativeWindow) ReleaseCapture();

	//the window class has no CS_DBLCLKS, so double clicks are detected here
	bool isDoubleClick = false;
	if (!isUp) {
		const std::uint64_t time = getTicks();
		const glm::ivec2 maxDistance(GetSystemMetrics(SM_CXDOUBLECLK) / 2, GetSystemMetrics(SM_CYDOUBLECLK) / 2);
		const glm::ivec2 distance = glm::abs(position - m_lastClick.position);
		isDoubleClick = m_lastClick.button == button && time - m_lastClick.time <= GetDoubleClickTime() && distance.x <= maxDistance.x && distance.y <= maxDistance.y;
		//a third click starts over instead of being another double click
		m_lastClick = isDoubleClick ? Click() : Click{ button, position, time };
	}

	postMouseEvent(new PG_MouseButtonEvent(m_window->getWindowId(), isUp, button, buttons, position, m_window->getSize(), isDoubleClick, DisaWin32Input::getKeymod()));
}

void DisaCustomApplication::translateMouseWheel(bool isHorizontal, std::uintptr_t wParam, std::intptr_t lParam) {
	//the wheel position comes in screen coordinates
	POINT cursor{ static_cast<short>(LOWORD(lParam)), static_cast<short>(HIWORD(lParam)) };
	ScreenToClient(m_nativeWindow, &cursor);

	const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
	int& remainder = isHorizontal ? m_wheelRemainder.x : m_wheelRemainder.y;
	remainder += delta;
	const int ticks = remainder / WHEEL_DELTA;
	if (ticks == 0) return; //a fine scrolling device, wait for a whole tick
	remainder -= ticks * WHEEL_DELTA;

	const glm::ivec2 move = isHorizontal ? glm::ivec2(ticks, 0) : glm::ivec2(0, ticks);
	postMouseEvent(new PG_MouseWheelEvent(m_window->getWindowId(), 0, move, glm::ivec2(cursor.x, cursor.y), DisaWin32Input::getKeymod()));
}

void DisaCustomApplication::translateKey(bool isUp, std::uintptr_t wParam, std::intptr_t lParam) {
	const unsigned int virtualKey = static_cast<unsigned int>(wParam);
	const bool isRepeat = !isUp && ((lParam >> 30) & 0x1);
	const PG_KeyGroup key{ DisaWin32Input::toKeyCode(virtualKey, lParam), DisaWin32Input::getKeymod(), DisaWin32Input::toScancode(virtualKey, lParam) };
	postKeyEvent(new PG_KeyEvent(isUp, isRepeat, key));
}

void DisaCustomApplication::translateChar(std::uintptr_t wParam) {
	//the game window is an ANSI window, so WM_CHAR carries the active code page, a double byte character in two messages
	const char byte = static_cast<char>(wParam & 0xFF);
	char bytes[2] = { byte, 0 };
	int byteCount = 1;
	if (m_dbcsLeadByte) {
		bytes[0] = m_dbcsLeadByte;
		bytes[1] = byte;
		byteCount = 2;
		m_dbcsLeadByte = 0;
	}
	else if (IsDBCSLeadByte(static_cast<BYTE>(byte))) {
		m_dbcsLeadByte = byte;
		return;
	}

	wchar_t wide[2] = {};
	const int wideCount = MultiByteToWideChar(CP_ACP, 0, bytes, byteCount, wide, 2);
	if (wideCount <= 0) return;

	char32_t codePoint = wide[0];
	if (wideCount == 2 && wide[0] >= 0xD800 && wide[0] <= 0xDBFF) codePoint = 0x10000 + ((wide[0] - 0xD800) << 10) + (wide[1] - 0xDC00);
	//control characters like backspace or return are already key events
	if (codePoint < 0x20 || codePoint == 0x7F) return;

	const std::string text = DisaWin32Input::toUtf8(codePoint);
	if (!text.empty()) postTextEvent(new PG_TextInputEvent(text));
}

void DisaCustomApplication::releaseInput() {
	const glm::ivec2 outside(-1);
	for (const int button : { PG_BUTTON_LEFT, PG_BUTTON_MIDDLE, PG_BUTTON_RIGHT, PG_BUTTON_X1, PG_BUTTON_X2 }) {
		postMouseEvent(new PG_MouseButtonEvent(m_window->getWindowId(), true, button, 0, outside, m_window->getSize(), false, PG_KMod_None));
	}
	if (GetCapture() == m_nativeWindow) ReleaseCapture();
	setGrabMouse(nullptr);
	m_lastClick = Click();
	m_dbcsLeadByte = 0;
}

void DisaCustomApplication::pollController() {
	const auto postMotion = [this](PG_EControllerMotion type, glm::vec2 value) {
		glm::vec2& last = m_controllerMotion[type];
		if (value == last) return;
		last = value;
		postKeyEvent(new PG_ControllerMotionEvent(type, value));
	};
	const auto releaseAll = [&]() {
		for (unsigned short bit = 1; bit != 0; bit <<= 1) {
			const PG_EControllerButton button = DisaWin32Input::toControllerButton(bit);
			if ((m_controllerButtons & bit) && button != PG_ControllerButton_Unknown) postKeyEvent(new PG_ControllerButtonEvent(true, button));
		}
		m_controllerButtons = 0;
		for (const PG_EControllerMotion type : { PG_ControllerMotion_LeftStick, PG_ControllerMotion_RightStick, PG_ControllerMotion_LeftTrigger, PG_ControllerMotion_RightTrigger }) {
			postMotion(type, glm::vec2(0.f));
		}
	};

	//the same test the game makes before it reads its pad
	if (GetActiveWindow() != m_nativeWindow || IsIconic(m_nativeWindow)) {
		releaseAll();
		return;
	}

	XINPUT_STATE state{};
	if (m_controllerIndex < 0) {
		const std::uint64_t now = getTicks();
		//XInputGetState on an empty slot is slow, so look for a new pad only once a second
		if (now < m_nextControllerScan) return;
		m_nextControllerScan = now + 1000;
		for (DWORD index = 0; index < XUSER_MAX_COUNT; ++index) {
			if (XInputGetState(index, &state) == ERROR_SUCCESS) {
				m_controllerIndex = static_cast<int>(index);
				break;
			}
		}
		if (m_controllerIndex < 0) return;
	}
	else if (XInputGetState(static_cast<DWORD>(m_controllerIndex), &state) != ERROR_SUCCESS) {
		m_controllerIndex = -1;
		releaseAll();
		return;
	}

	const XINPUT_GAMEPAD& pad = state.Gamepad;
	const unsigned short changed = pad.wButtons ^ m_controllerButtons;
	for (unsigned short bit = 1; bit != 0; bit <<= 1) {
		if (!(changed & bit)) continue;
		const PG_EControllerButton button = DisaWin32Input::toControllerButton(bit);
		if (button != PG_ControllerButton_Unknown) postKeyEvent(new PG_ControllerButtonEvent(!(pad.wButtons & bit), button));
	}
	m_controllerButtons = pad.wButtons;

	postMotion(PG_ControllerMotion_LeftStick, DisaWin32Input::toStickDirection(pad.sThumbLX, pad.sThumbLY));
	postMotion(PG_ControllerMotion_RightStick, DisaWin32Input::toStickDirection(pad.sThumbRX, pad.sThumbRY));
	postMotion(PG_ControllerMotion_LeftTrigger, glm::vec2(DisaWin32Input::toTriggerValue(pad.bLeftTrigger), 0.f));
	postMotion(PG_ControllerMotion_RightTrigger, glm::vec2(DisaWin32Input::toTriggerValue(pad.bRightTrigger), 0.f));
}

void DisaCustomApplication::postMouseEvent(PG_Event* e) {
	routeEvent(e, getGrabMouse() ? getGrabMouse() : m_window);
}

void DisaCustomApplication::postKeyEvent(PG_Event* e) {
	routeEvent(e, getGrabKeys() ? getGrabKeys() : this);
}

void DisaCustomApplication::postTextEvent(PG_Event* e) {
	routeEvent(e, getGrabKeys() && isTextImputEnabled() ? getGrabKeys() : this);
}

void DisaCustomApplication::routeEvent(PG_Event* e, PG_Object* target) {
	if (m_inputLogging) OUT_INFO("[Input] " << describeInputEvent(e) << " -> " << target->getName());
	if (target != this && filterEventProcess(e)) {
		delete e;
		return;
	}
	PG_EventLoop::postEvent(e, target);
}
