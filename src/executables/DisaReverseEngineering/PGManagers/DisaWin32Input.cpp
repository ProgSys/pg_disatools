#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "DisaWin32Input.h"

#include <Windows.h>
#include <Xinput.h>
#include <array>

namespace DisaWin32Input {

	static inline unsigned int getScancodeByte(std::intptr_t lParam) {
		return static_cast<unsigned int>((lParam >> 16) & 0xFF);
	}

	static inline bool isExtended(std::intptr_t lParam) {
		return (lParam >> 24) & 0x1;
	}

	PG_EKeyCode toKeyCode(unsigned int virtualKey, std::intptr_t lParam) {
		if (virtualKey >= 'A' && virtualKey <= 'Z') return static_cast<PG_EKeyCode>(PG_Key_a + (virtualKey - 'A'));
		if (virtualKey >= '0' && virtualKey <= '9') return static_cast<PG_EKeyCode>(PG_Key_0 + (virtualKey - '0'));
		if (virtualKey >= VK_F1 && virtualKey <= VK_F12) return static_cast<PG_EKeyCode>(PG_Key_F1 + (virtualKey - VK_F1));
		if (virtualKey >= VK_F13 && virtualKey <= VK_F24) return static_cast<PG_EKeyCode>(PG_Key_F13 + (virtualKey - VK_F13));
		if (virtualKey >= VK_NUMPAD1 && virtualKey <= VK_NUMPAD9) return static_cast<PG_EKeyCode>(PG_KP_1 + (virtualKey - VK_NUMPAD1));

		const bool extended = isExtended(lParam);
		switch (virtualKey) {
		case VK_RETURN: return extended ? PG_KP_Enter : PG_Key_Return;
		case VK_ESCAPE: return PG_Key_Escape;
		case VK_BACK: return PG_Key_Backspace;
		case VK_TAB: return PG_Key_Tab;
		case VK_SPACE: return PG_Key_Space;
		case VK_OEM_MINUS: return PG_Key_Minus;
		case VK_OEM_PLUS: return PG_Key_Equals;
		case VK_OEM_COMMA: return PG_Key_Comma;
		case VK_OEM_PERIOD: return PG_Key_Period;
		case VK_OEM_1: return PG_Key_Semicolon;
		case VK_OEM_2: return PG_Key_Slash;
		case VK_OEM_3: return PG_Key_Backquote;
		case VK_OEM_4: return PG_Key_LeftBracket;
		case VK_OEM_5: return PG_Key_Backslash;
		case VK_OEM_6: return PG_Key_RightBracket;
		case VK_OEM_7: return PG_Key_Quote;
		case VK_OEM_102: return PG_Key_Backslash;
		case VK_CAPITAL: return PG_Key_CapsLock;
		case VK_SNAPSHOT: return PG_Key_PrintScreen;
		case VK_SCROLL: return PG_Key_ScrollLock;
		case VK_PAUSE: return PG_Key_Pause;
		case VK_INSERT: return PG_Key_Insert;
		case VK_HOME: return PG_Key_Home;
		case VK_PRIOR: return PG_Key_PageUp;
		case VK_DELETE: return PG_Key_Delete;
		case VK_END: return PG_Key_End;
		case VK_NEXT: return PG_Key_PageDown;
		case VK_RIGHT: return PG_Key_Right;
		case VK_LEFT: return PG_Key_Left;
		case VK_DOWN: return PG_Key_Down;
		case VK_UP: return PG_Key_Up;
		case VK_NUMLOCK: return PG_Key_NumLockClear;
		case VK_DIVIDE: return PG_KP_Divide;
		case VK_MULTIPLY: return PG_KP_Multiply;
		case VK_SUBTRACT: return PG_KP_Minus;
		case VK_ADD: return PG_KP_Plus;
		case VK_NUMPAD0: return PG_KP_0;
		case VK_DECIMAL: return PG_KP_Period;
		case VK_APPS: return PG_Key_Application;
		//WM_KEYDOWN only reports the generic modifier keys, the scancode tells left from right
		case VK_SHIFT: return getScancodeByte(lParam) == 0x36 ? PG_Key_RShift : PG_Key_LShift;
		case VK_CONTROL: return extended ? PG_Key_RCtrl : PG_Key_LCtrl;
		case VK_MENU: return extended ? PG_Key_RAlt : PG_Key_LAlt;
		case VK_LSHIFT: return PG_Key_LShift;
		case VK_RSHIFT: return PG_Key_RShift;
		case VK_LCONTROL: return PG_Key_LCtrl;
		case VK_RCONTROL: return PG_Key_RCtrl;
		case VK_LMENU: return PG_Key_LAlt;
		case VK_RMENU: return PG_Key_RAlt;
		case VK_LWIN: return PG_Key_LGUI;
		case VK_RWIN: return PG_Key_RGUI;
		case VK_VOLUME_MUTE: return PG_Key_Mute;
		case VK_VOLUME_UP: return PG_Key_VolumeUp;
		case VK_VOLUME_DOWN: return PG_Key_VolumeDown;
		case VK_MEDIA_NEXT_TRACK: return PG_Key_AudioNext;
		case VK_MEDIA_PREV_TRACK: return PG_Key_AudioPrev;
		case VK_MEDIA_STOP: return PG_Key_AudioStop;
		case VK_MEDIA_PLAY_PAUSE: return PG_Key_AudioPlay;
		default: return PG_Key_Unknown;
		}
	}

	PG_EScancode toScancode(unsigned int virtualKey, std::intptr_t lParam) {
		//Windows reports both with scancode 0x45, only the extended bit is swapped between them
		if (virtualKey == VK_PAUSE) return static_cast<PG_EScancode>(72);
		if (virtualKey == VK_NUMLOCK) return static_cast<PG_EScancode>(83);

		const unsigned int scancode = getScancodeByte(lParam);
		if (isExtended(lParam)) {
			switch (scancode) {
			case 0x1C: return static_cast<PG_EScancode>(88);  //keypad enter
			case 0x1D: return static_cast<PG_EScancode>(228); //right ctrl
			case 0x35: return static_cast<PG_EScancode>(84);  //keypad divide
			case 0x37: return static_cast<PG_EScancode>(70);  //print screen
			case 0x38: return static_cast<PG_EScancode>(230); //right alt
			case 0x47: return static_cast<PG_EScancode>(74);  //home
			case 0x48: return static_cast<PG_EScancode>(82);  //up
			case 0x49: return static_cast<PG_EScancode>(75);  //page up
			case 0x4B: return static_cast<PG_EScancode>(80);  //left
			case 0x4D: return static_cast<PG_EScancode>(79);  //right
			case 0x4F: return static_cast<PG_EScancode>(77);  //end
			case 0x50: return static_cast<PG_EScancode>(81);  //down
			case 0x51: return static_cast<PG_EScancode>(78);  //page down
			case 0x52: return static_cast<PG_EScancode>(73);  //insert
			case 0x53: return static_cast<PG_EScancode>(76);  //delete
			case 0x5B: return static_cast<PG_EScancode>(227); //left windows
			case 0x5C: return static_cast<PG_EScancode>(231); //right windows
			case 0x5D: return static_cast<PG_EScancode>(101); //application
			default: return static_cast<PG_EScancode>(0);
			}
		}

		//PC scancode set 1 to USB HID usage ids, which PG_EScancode uses
		constexpr std::array<unsigned short, 0x59> setOneToUsb = {
			0,   41,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  45,  46,  42,  43,  //0x00
			20,  26,  8,   21,  23,  28,  24,  12,  18,  19,  47,  48,  40,  224, 4,   22,  //0x10
			7,   9,   10,  11,  13,  14,  15,  51,  52,  53,  225, 49,  29,  27,  6,   25,  //0x20
			5,   17,  16,  54,  55,  56,  229, 85,  226, 44,  57,  58,  59,  60,  61,  62,  //0x30
			63,  64,  65,  66,  67,  83,  71,  95,  96,  97,  86,  92,  93,  94,  87,  89,  //0x40
			90,  91,  98,  99,  0,   0,   100, 68,  69                                      //0x50
		};
		return static_cast<PG_EScancode>(scancode < setOneToUsb.size() ? setOneToUsb[scancode] : 0);
	}

	PG_KeymodValue getKeymod() {
		const auto isDown = [](int virtualKey) { return (GetKeyState(virtualKey) & 0x8000) != 0; };
		const auto isToggled = [](int virtualKey) { return (GetKeyState(virtualKey) & 0x1) != 0; };

		PG_KeymodValue mod = PG_KMod_None;
		if (isDown(VK_LSHIFT)) mod |= PG_KMod_LShift;
		if (isDown(VK_RSHIFT)) mod |= PG_KMod_RShift;
		if (isDown(VK_LCONTROL)) mod |= PG_KMod_LCtrl;
		if (isDown(VK_RCONTROL)) mod |= PG_KMod_RCtrl;
		if (isDown(VK_LMENU)) mod |= PG_KMod_LAlt;
		if (isDown(VK_RMENU)) mod |= PG_KMod_RAlt;
		if (isDown(VK_LWIN)) mod |= PG_KMod_LGUI;
		if (isDown(VK_RWIN)) mod |= PG_KMod_RGUI;
		if (isToggled(VK_NUMLOCK)) mod |= PG_KMod_Num;
		if (isToggled(VK_CAPITAL)) mod |= PG_KMod_Caps;
		if (isToggled(VK_SCROLL)) mod |= PG_KMod_Scroll;
		return mod;
	}

	int getMouseButtons() {
		const auto isDown = [](int virtualKey) { return (GetKeyState(virtualKey) & 0x8000) != 0; };
		int buttons = 0;
		if (isDown(VK_LBUTTON)) buttons |= PG_BUTTON_LMASK;
		if (isDown(VK_MBUTTON)) buttons |= PG_BUTTON_MMASK;
		if (isDown(VK_RBUTTON)) buttons |= PG_BUTTON_RMASK;
		if (isDown(VK_XBUTTON1)) buttons |= PG_BUTTON_X1MASK;
		if (isDown(VK_XBUTTON2)) buttons |= PG_BUTTON_X2MASK;
		return buttons;
	}

	int toMouseButton(unsigned int message, std::uintptr_t wParam) {
		switch (message) {
		case WM_LBUTTONDOWN: case WM_LBUTTONUP: return PG_BUTTON_LEFT;
		case WM_MBUTTONDOWN: case WM_MBUTTONUP: return PG_BUTTON_MIDDLE;
		case WM_RBUTTONDOWN: case WM_RBUTTONUP: return PG_BUTTON_RIGHT;
		case WM_XBUTTONDOWN: case WM_XBUTTONUP: return GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? PG_BUTTON_X1 : PG_BUTTON_X2;
		default: return 0;
		}
	}

	glm::ivec2 toMousePosition(std::intptr_t lParam) {
		//signed, the position is negative left or above the client area while the mouse is captured
		return glm::ivec2(static_cast<short>(LOWORD(lParam)), static_cast<short>(HIWORD(lParam)));
	}

	PG_EControllerButton toControllerButton(unsigned short xinputButton) {
		switch (xinputButton) {
		case XINPUT_GAMEPAD_A: return PG_ControllerButton_A;
		case XINPUT_GAMEPAD_B: return PG_ControllerButton_B;
		case XINPUT_GAMEPAD_X: return PG_ControllerButton_X;
		case XINPUT_GAMEPAD_Y: return PG_ControllerButton_Y;
		case XINPUT_GAMEPAD_BACK: return PG_ControllerButton_Back;
		case XINPUT_GAMEPAD_START: return PG_ControllerButton_Start;
		case XINPUT_GAMEPAD_LEFT_THUMB: return PG_ControllerButton_LeftStick;
		case XINPUT_GAMEPAD_RIGHT_THUMB: return PG_ControllerButton_RightStick;
		case XINPUT_GAMEPAD_LEFT_SHOULDER: return PG_ControllerButton_LeftShoulder;
		case XINPUT_GAMEPAD_RIGHT_SHOULDER: return PG_ControllerButton_RightShoulder;
		case XINPUT_GAMEPAD_DPAD_UP: return PG_ControllerButton_DPadUp;
		case XINPUT_GAMEPAD_DPAD_DOWN: return PG_ControllerButton_DPadDown;
		case XINPUT_GAMEPAD_DPAD_LEFT: return PG_ControllerButton_DPadLeft;
		case XINPUT_GAMEPAD_DPAD_RIGHT: return PG_ControllerButton_DPadRight;
		default: return PG_ControllerButton_Unknown;
		}
	}

	glm::vec2 toStickDirection(short xAxis, short yAxis) {
		constexpr float maxAxis = 32767.f;
		//the same dead zone PG_SDLApplication uses
		constexpr float deadZone = 8000.f / maxAxis;

		glm::vec2 direction(static_cast<float>(xAxis) / maxAxis, static_cast<float>(yAxis) / maxAxis);
		const float length = glm::length(direction);
		if (length < deadZone) return glm::vec2(0.f);
		if (length > 1.f) direction /= length;
		return direction;
	}

	float toTriggerValue(unsigned char value) {
		if (value < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) return 0.f;
		return static_cast<float>(value) / 255.f;
	}

	std::string toUtf8(char32_t codePoint) {
		std::string out;
		if (codePoint < 0x80) {
			out += static_cast<char>(codePoint);
		}
		else if (codePoint < 0x800) {
			out += static_cast<char>(0xC0 | (codePoint >> 6));
			out += static_cast<char>(0x80 | (codePoint & 0x3F));
		}
		else if (codePoint < 0x10000) {
			if (codePoint >= 0xD800 && codePoint <= 0xDFFF) return {}; //a lone surrogate
			out += static_cast<char>(0xE0 | (codePoint >> 12));
			out += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
			out += static_cast<char>(0x80 | (codePoint & 0x3F));
		}
		else if (codePoint < 0x110000) {
			out += static_cast<char>(0xF0 | (codePoint >> 18));
			out += static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F));
			out += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
			out += static_cast<char>(0x80 | (codePoint & 0x3F));
		}
		return out;
	}
}
