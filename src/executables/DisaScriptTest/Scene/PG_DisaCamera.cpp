#include "PG_DisaCamera.h"

#include <algorithm>
#include <cmath>

#include <glm/gtx/rotate_vector.hpp>
#include <Base/Events/PG_MouseMotionEvent.h>
#include <Base/Events/PG_MouseWheelEvent.h>
#include <Base/Events/PG_KeyEvent.h>
#include <Base/Events/PG_TickEvent.h>
#include <Base/PG_Application.h>
#include <Scene/PG_TickComponent.h>
#include <Util/PG_Math.h>

constexpr float g_moveSpeed = 0.0012f; /// Orbit point movement per millisecond and move scale
constexpr float g_panSpeed = 0.0024f; /// Orbit point movement per dragged pixel and move scale
constexpr float g_zoomSpeed = 0.1f; /// Part of the orbit distance one wheel step takes away
constexpr float g_orbitTurnTime = 250.f;
/// A view dir below this much length on the ground plane leaves no direction to move along
constexpr float g_minGroundLength = 0.02f;

const std::string g_orbitPointAni = "setOrbitPointAni";
const std::string g_orbitDistanceAni = "setOrbitDistanceAni";
const std::string g_forwardOrbitAni = "setForwardOrbitAni";

void PG_DisaCamera::init() {
	PG_FlyCameraNode::init();
	setFov(0.1f);

	const glm::vec3 startPosition(25.554f, 31.5181f, -22.3441f);
	m_orbitForward = glm::normalize(glm::vec3(-1.f, -0.8f, 1.f));

	//the orbit point starts where the view dir hits the ground plane
	const PG_Ray startRay(startPosition, m_orbitForward);
	m_orbitPoint = startRay.getPointOnRay(startRay.intersectPlane(glm::vec3(0.f), getUpVector()).value_or(m_orbitDistance));
	m_orbitDistance = glm::clamp(glm::distance(startPosition, m_orbitPoint), m_minOrbitDistance, m_maxOrbitDistance);
	updateOrbitTransformation();
}

void PG_DisaCamera::setOrbitPoint(glm::vec3 point, float time) {
	if (time <= 0.f) {
		stopOrbitAnimation(g_orbitPointAni);
		m_orbitPoint = point;
		updateOrbitTransformation();
		return;
	}

	startOrbitAnimation(g_orbitPointAni, time, [this, start = m_orbitPoint, point](float progress) {
		m_orbitPoint = glm::mix(start, point, progress);
		updateOrbitTransformation();
	});
}

void PG_DisaCamera::setOrbitDistance(float distance, float time) {
	distance = glm::clamp(distance, m_minOrbitDistance, m_maxOrbitDistance);

	if (time <= 0.f) {
		stopOrbitAnimation(g_orbitDistanceAni);
		m_orbitDistance = distance;
		updateOrbitTransformation();
		return;
	}

	startOrbitAnimation(g_orbitDistanceAni, time, [this, start = m_orbitDistance, distance](float progress) {
		m_orbitDistance = glm::mix(start, distance, progress);
		updateOrbitTransformation();
	});
}

void PG_DisaCamera::setForwardOrbit(glm::vec3 viewDir, float time, std::optional<glm::vec3> rotationAxis) {
	const float length = glm::length(viewDir);
	if (length < 0.0001f || glm::length(PG_Math::setY(viewDir, 0.f)) / length < g_minGroundLength) return;
	viewDir /= length;

	if (time <= 0.f) {
		stopOrbitAnimation(g_forwardOrbitAni);
		m_orbitForward = viewDir;
		updateOrbitTransformation();
		return;
	}

	if (rotationAxis) {
		const glm::vec3 axis = glm::normalize(*rotationAxis);
		const glm::vec3 start = m_orbitForward;
		/* Both dirs are flattened onto the plane of the axis, so only the turn around the axis is
		 * animated and the tilt towards it stays where it is. */
		const glm::vec3 startFlat = start - axis * glm::dot(start, axis);
		const glm::vec3 targetFlat = viewDir - axis * glm::dot(viewDir, axis);
		const float angle = std::atan2(glm::dot(glm::cross(startFlat, targetFlat), axis), glm::dot(startFlat, targetFlat));

		startOrbitAnimation(g_forwardOrbitAni, time, [this, start, viewDir, axis, angle](float progress) {
			m_orbitForward = progress < 1.f ? glm::rotate(start, angle * progress, axis) : viewDir;
			updateOrbitTransformation();
		});
		return;
	}

	glm::vec3 start = m_orbitForward;
	//slerp has no defined arc between opposing dirs, so the start is nudged off the axis
	if (glm::dot(start, viewDir) < -0.9999f) start = glm::rotate(start, 0.01f, getUpVector());

	startOrbitAnimation(g_forwardOrbitAni, time, [this, start, viewDir](float progress) {
		m_orbitForward = glm::slerp(start, viewDir, progress);
		updateOrbitTransformation();
	});
}

float PG_DisaCamera::getCameraOrbit() const {
	return glm::degrees(std::atan2(-m_orbitForward.z, m_orbitForward.x));
}

void PG_DisaCamera::setCameraOrbit(float degree, float time) {
	setForwardOrbit(glm::rotate(m_orbitForward, glm::radians(degree - getCameraOrbit()), getUpVector()), time, getUpVector());
}

float PG_DisaCamera::getOrbitPitch() const {
	return glm::degrees(std::asin(glm::clamp(m_orbitForward.y, -1.f, 1.f)));
}

void PG_DisaCamera::setOrbitAngles(float pitchDegree, float orbitDegree) {
	constexpr float maxPitch = 89.f;
	const float pitch = glm::radians(glm::clamp(pitchDegree, -maxPitch, maxPitch));
	const float orbit = glm::radians(orbitDegree);
	const float ground = std::cos(pitch);
	setForwardOrbit(glm::vec3(ground * std::cos(orbit), std::sin(pitch), -ground * std::sin(orbit)));
}

glm::vec3 PG_DisaCamera::getGroundForward() const {
	return glm::normalize(PG_Math::setY(m_orbitForward, 0.f));
}

float PG_DisaCamera::getMoveScale() const {
	return std::sqrt(m_orbitDistance);
}

void PG_DisaCamera::updateOrbitTransformation() {
	setForward(m_orbitForward);
	setTranslation(m_orbitPoint - m_orbitForward * m_orbitDistance);

	//the depth range follows the zoom, so a close up does not clip and a far shot still reaches the map
	setNear(std::max(0.1f, m_orbitDistance * 0.1f));
	setFar(m_orbitDistance * 4.f + 100.f);
}

void PG_DisaCamera::startOrbitAnimation(const std::string& name, float time, std::function<void(float)> apply) {
	stopOrbitAnimation(name);

	auto component = PG_new<PG_TickComponent>(PG_TickComponent::TickFunction(), name, this);
	auto animation = [component, time, apply = std::move(apply), elapsed = 0.f](PG_ActorNode*, PG_TickEvent* tick) mutable {
		elapsed += (float)tick->getDeltaTime();
		const float progress = glm::clamp(elapsed / time, 0.f, 1.f);
		apply(progress);
		if (progress >= 1.f) component->deleteLater();
	};
	component->setTickFunction(std::move(animation));
}

void PG_DisaCamera::stopOrbitAnimation(const std::string& name) {
	auto isRunning = [&name](PG_TickComponent* component) {
		return !component->isDeleteLater() && component->getName() == name;
	};

	if (auto component = findFirstChild<PG_TickComponent>(isRunning)) {
		//the component still ticks until the event loop deletes it
		component->setTickFunction(PG_TickComponent::TickFunction());
		component->deleteLater();
	}
}

void PG_DisaCamera::onMove(float deltaTime) {
	if (m_velocity == glm::vec3(0.f)) return;

	const glm::vec3 forwardProjected = getGroundForward();
	const glm::vec3 right = glm::normalize(glm::cross(getUpVector(), forwardProjected));

	const glm::vec3 move = (forwardProjected * m_velocity.x + getUpVector() * m_velocity.y + right * m_velocity.z) * (getMoveScale() * g_moveSpeed * deltaTime);
	setOrbitPoint(m_orbitPoint + move);
}

bool PG_DisaCamera::onMouseMotion(PG_MouseMotionEvent* e) {
	if (PG_Application::grabbedMouse() != (PG_Object*)this)
		return true;

	if (!isEnabled() || !getScene() || getScene()->getActiveCamera() != this) {
		PG_Application::grabMouse(nullptr); //clear
		return PG_ActorNode::event(e);
	}

	if (e->getButton() & getSettings().panMouseKey) {
		const glm::vec3 forwardProjected = getGroundForward();
		const glm::vec3 right = glm::normalize(glm::cross(getUpVector(), forwardProjected));

		const glm::vec3 move = (right * (float)e->getPositionRelative().x + forwardProjected * (float)e->getPositionRelative().y) * (getMoveScale() * g_panSpeed);
		setOrbitPoint(m_orbitPoint + move);
	}

	return true;
}

bool PG_DisaCamera::onMouseWheel(PG_MouseWheelEvent* e) {
	if (!isEnabled() || !getScene() || getScene()->getActiveCamera() != this) return PG_ActorNode::event(e);

	setOrbitDistance(m_orbitDistance - m_orbitDistance * (float)e->y() * g_zoomSpeed);
	return true;
}

bool PG_DisaCamera::onKey(PG_KeyEvent* key)  {
	if (!isEnabled() || !getScene() || getScene()->getActiveCamera() != this) return PG_ActorNode::event(key);

	if (!key->isRepeat() && key->isUp()) {
		switch (key->getKey()) {
			case PG_Key_Q: setCameraOrbit((float)PG_Math::roundTo(getCameraOrbit() - 90.f, 45.f), g_orbitTurnTime); return false;
			case PG_Key_E: setCameraOrbit((float)PG_Math::roundTo(getCameraOrbit() + 90.f, 45.f), g_orbitTurnTime); return false;
		}
	}

	return PG_FlyCameraNode::onKey(key);
}
