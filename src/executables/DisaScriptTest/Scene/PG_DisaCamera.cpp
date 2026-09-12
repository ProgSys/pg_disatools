#include "PG_DisaCamera.h"
#include <Base/Events/PG_MouseMotionEvent.h>
#include <Base/Events/PG_MouseWheelEvent.h>
#include <Base/Events/PG_KeyEvent.h>
#include <Base/Events/PG_RenderEvent.h>
#include <Base/PG_Application.h>
#include <Scene/PG_TickComponent.h>
#include <Util/PG_Math.h>

constexpr glm::vec3 g_cameraUp(0.f, 1.f, 0.f);

void PG_DisaCamera::init() {
	PG_FlyCameraNode::init();
	setTranslation(glm::vec3(25.554f, 31.5181f, -22.3441f));
	setForward(glm::vec3(-1.f, -0.8f, 1.f));
	setFov(0.1f);
	setFar(300.f); //@todo will need to be set by zoom
	setNear(10.f); //@todo will need to be set by zoom
}

void PG_DisaCamera::onMove(float deltaTime) {
	const glm::vec3 forwardProjected = glm::normalize(PG_Math::setY(getForward(), 0.f));
	const glm::vec3 right = glm::normalize(glm::cross(g_cameraUp, forwardProjected));
	// @todo scale with zoom
	const glm::vec3 move = (forwardProjected * m_velocity.x + getUpVector() * m_velocity.y + right  * m_velocity.z) * 0.01f * deltaTime;
	translate(move);
}

float PG_DisaCamera::PG_DisaCamera::getCameraOrbit() const {
	//const the current orbit of the camera around y axis
	return 0.f;
}

void PG_DisaCamera::setCameraOrbit(float degree, float time) {
	//@todo rotate camera around by 90 degree times dir
	// time defines how long the orbit will take, zero or smaller equals instant
	//use PG_TickComponent for a linar animation
	//discrad old animation by new "findFirstChild<PG_TickComponent>("setCameraOrbitAni")"
}

bool PG_DisaCamera::onMouseMotion(PG_MouseMotionEvent* e) {
	if (PG_Application::grabbedMouse() != (PG_Object*)this)
		return true;

	if (!getScene() || getScene()->getActiveCamera() != this) return PG_ActorNode::event(e);

	if (e->getButton() & getSettings().panMouseKey) {
		const glm::vec3 forwardProjected = glm::normalize(PG_Math::setY(getForward(), 0.f));
		const glm::vec3 right = glm::normalize(glm::cross(g_cameraUp, forwardProjected));

		const glm::vec3 move = (right * (float)e->getPositionRelative().x + forwardProjected * (float)e->getPositionRelative().y) * 0.02f;
		translate(move);
	}

	return true;
}

bool PG_DisaCamera::onMouseWheel(PG_MouseWheelEvent* e) {
	if (!getScene() || getScene()->getActiveCamera() != this) return PG_ActorNode::event(e);

	translate(getForward() * (float)e->y());
	return true;
}

bool PG_DisaCamera::onKey(PG_KeyEvent* key)  {
	if (!getScene() || getScene()->getActiveCamera() != this) return PG_ActorNode::event(key);

	if (!key->isRepeat()) {
		switch (key->getKey()) {
			case PG_Key_Q: setCameraOrbit(PG_Math::roundTo(getCameraOrbit() - 90.f, 45.f), 10.f); break;
			case PG_Key_E: setCameraOrbit(PG_Math::roundTo(getCameraOrbit() + 90.f, 45.f), 10.f); break;
		}
	}

	return PG_FlyCameraNode::onKey(key);
}