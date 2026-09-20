#include "PG_DisaCameraAnimations.h"

#include <algorithm>
#include <cmath>
#include <memory>

#include <Scene/PG_DisaCamera.h>
#include <Script/PG_DisaCameraAnimationComponent.h>
#include <Script/PG_DisaContext.h>

namespace {

/// The way round from one angle to another that is at most half a turn.
float shortestDelta(float fromDegree, float toDegree) {
	float delta = std::fmod(toDegree - fromDegree, 360.f);
	if (delta > 180.f) delta -= 360.f;
	if (delta < -180.f) delta += 360.f;
	return delta;
}

/// Runs a value from where it is to a target over the ticks, or sets it when there are none.
template<typename Apply>
void animate(PG_DisaContext& context, PG_DisaCameraAnimationComponent::EChannel channel,
	const char* name, float durationTicks, Apply apply) {
	if (!context.getGameCamera()) return;

	if (durationTicks <= 0.f) {
		context.clearCameraAnimation(channel);
		apply(context.getGameCamera(), 1.f);
		return;
	}

	auto elapsed = std::make_shared<float>(0.f);
	context.takeCameraAnimation(channel, name)
		->setTickFunction([apply, durationTicks, elapsed](PG_DisaCamera* camera, float deltaTicks) {
			*elapsed += deltaTicks;
			const float progress = std::min(1.f, *elapsed / durationTicks);
			apply(camera, progress);
			return progress >= 1.f;
			});
}

} //namespace

void PG_DisaCameraAnimations::moveTo(PG_DisaContext& context, const glm::vec3& target, float durationTicks) {
	auto* camera = context.getGameCamera();
	if (!camera) return;

	animate(context, PG_DisaCameraAnimationComponent::Channel_Move, "PG_DisaCameraMoveAnimation",
		durationTicks, [start = camera->getOrbitPoint(), target](PG_DisaCamera* self, float progress) {
			self->setOrbitPoint(glm::mix(start, target, progress));
		});
}

void PG_DisaCameraAnimations::zoomTo(PG_DisaContext& context, float distance, float durationTicks) {
	auto* camera = context.getGameCamera();
	if (!camera) return;

	animate(context, PG_DisaCameraAnimationComponent::Channel_Zoom, "PG_DisaCameraZoomAnimation",
		durationTicks, [start = camera->getOrbitDistance(), distance](PG_DisaCamera* self, float progress) {
			self->setOrbitDistance(glm::mix(start, distance, progress));
		});
}

void PG_DisaCameraAnimations::rotateTo(PG_DisaContext& context, float pitchDegree, float orbitDegree, float durationTicks) {
	auto* camera = context.getGameCamera();
	if (!camera) return;

	//the game wraps every step into half a turn before it divides it up, so a turn never goes the long way
	rotateBy(context, shortestDelta(camera->getOrbitPitch(), pitchDegree),
		shortestDelta(camera->getCameraOrbit(), orbitDegree), durationTicks);
}

void PG_DisaCameraAnimations::rotateBy(PG_DisaContext& context, float pitchDegree, float orbitDegree, float durationTicks) {
	auto* camera = context.getGameCamera();
	if (!camera) return;

	const float pitchStart = camera->getOrbitPitch();
	const float orbitStart = camera->getCameraOrbit();
	animate(context, PG_DisaCameraAnimationComponent::Channel_Rotation, "PG_DisaCameraRotateAnimation",
		durationTicks, [pitchStart, orbitStart, pitchDegree, orbitDegree](PG_DisaCamera* self, float progress) {
			self->setOrbitAngles(pitchStart + pitchDegree * progress, orbitStart + orbitDegree * progress);
		});
}

void PG_DisaCameraAnimations::fovTo(PG_DisaContext& context, float fovDegree, float durationTicks) {
	auto* camera = context.getGameCamera();
	if (!camera) return;

	animate(context, PG_DisaCameraAnimationComponent::Channel_Fov, "PG_DisaCameraFovAnimation",
		durationTicks, [start = glm::degrees(camera->getFov()), fovDegree](PG_DisaCamera* self, float progress) {
			self->setFov(glm::radians(glm::mix(start, fovDegree, progress)));
		});
}

float PG_DisaCameraAnimations::fovOfLensValue(float value) {
	/* The game rebuilds the frustum from the value, see FUN_004f8050: the half width at the near
	 * plane is (320 / value) * 0.075 and the height is 0.5666667 of that. So the value reads like a
	 * focal length, bigger means narrower, and the default of 320 is a very long lens. */
	constexpr float defaultValue = 320.f;
	constexpr float aspect = 0.5666667f;
	const float half = (defaultValue / (value > 0.f ? value : defaultValue)) * 0.075f;
	return glm::degrees(2.f * std::atan(half * aspect));
}
