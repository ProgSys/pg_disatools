#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "DisaCustomWindow.h"

#include <Base/PG_EventLoop.h>
#include <Base/Events/PG_WindowResizeEvent.h>
#include <algorithm>

#include <Windows.h>

DisaCustomWindow::DisaCustomWindow(const std::string& name, HWND__* nativeWindow) : PG_Window(name), m_nativeWindow(nativeWindow) {
	m_window = std::make_shared<DisaCustomWindowData>();
	updateSize();
}

std::string DisaCustomWindow::getTitle() const {
	std::string title(256, '\0');
	const int length = GetWindowTextA(m_nativeWindow, title.data(), static_cast<int>(title.size()));
	title.resize(std::max(length, 0));
	return title;
}

bool DisaCustomWindow::isShown() const {
	return IsWindowVisible(m_nativeWindow) && !IsIconic(m_nativeWindow);
}

void DisaCustomWindow::setSize(glm::ivec2 size) {
	if (size.x <= 0 || size.y <= 0 || size == m_size) return;
	m_size = size;
	PG_EventLoop::postEvent(new PG_WindowResizeEvent(getWindowId(), m_size, PG_Event_Behaviour_Singleton), this);
}

void DisaCustomWindow::updateSize() {
	RECT rect;
	if (!GetClientRect(m_nativeWindow, &rect)) return;
	setSize(glm::ivec2(rect.right - rect.left, rect.bottom - rect.top));
}
