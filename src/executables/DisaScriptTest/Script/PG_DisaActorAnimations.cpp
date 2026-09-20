#include "PG_DisaActorAnimations.h"

#include <algorithm>
#include <cmath>
#include <memory>

#include <glm/gtc/constants.hpp>

#include <Disa/map/PG_DisaSpriteModel.h>

#include "PG_DisaAnimationComponent.h"
#include "PG_DisaContext.h"

namespace {

/// Wraps an angle into (-180, 180], the range the game keeps a facing in.
float normalizeDegree(float degree) {
	degree = std::fmod(degree + 180.f, 360.f);
	if (degree <= 0.f) degree += 360.f;
	return degree - 180.f;
}

} //namespace

void PG_DisaActorAnimations::move(PG_DisaContext& context, PG_DisaSpriteModel* actor, const glm::vec3& target,
	float durationTicks, EMoveMode mode) {
	if (!actor) return;

	const glm::vec3 start = actor->getObjectTranslation();

	float ticks = durationTicks;
	if (mode == Move_Speed) {
		//the duration is a speed here, so the travel time is what the distance makes of it
		const float distance = glm::length(glm::vec2(target.x - start.x, target.z - start.z));
		ticks = durationTicks > 0.f ? distance / durationTicks : 0.f;
	}

	if (ticks <= 0.f) {
		//nothing to walk, and an arrival puts back whatever a walk already running was holding
		context.clearAnimation(actor, PG_DisaAnimationComponent::Channel_Move);
		actor->setObjectTranslation(target);
		return;
	}

	/* A jump keeps the height of the target as the height of its arc and lands on the ground, a
	 * straight move simply runs to the target and drops from there. */
	const float arcHeight = mode == Move_Jump ? target.y - start.y : 0.f;
	const glm::vec3 end = mode == Move_Jump
		? glm::vec3(target.x, context.getGroundHeight(target), target.z)
		: target;

	auto* animation = context.takeAnimation(actor, PG_DisaAnimationComponent::Channel_Move, "PG_DisaMoveAnimation");

	/* The game walks an actor that is going somewhere. Only the first move remembers what it was
	 * doing, so a walk retargeted halfway does not come back as a walk. */
	if (!animation->hasEndFunction()) {
		const PG_DisaSpriteModel::EAnimationSet before = actor->getAnimationSet();
		animation->setEndFunction([before](PG_DisaSpriteModel* self) { if (self) self->setAnimationSet(before); });
	}
	actor->setAnimationSet(PG_DisaSpriteModel::Walking);

	auto elapsed = std::make_shared<float>(0.f);
	animation->setTickFunction([start, end, ticks, arcHeight, elapsed](PG_DisaSpriteModel* self, float deltaTicks) {
		*elapsed += deltaTicks;
		const float progress = std::min(1.f, *elapsed / ticks);

		glm::vec3 position = glm::mix(start, end, progress);
		if (arcHeight != 0.f) position.y += arcHeight * std::sin(progress * glm::pi<float>());
		self->setObjectTranslation(position);
		return progress >= 1.f; //the end function puts the animation set back
		});
}

void PG_DisaActorAnimations::rotateTo(PG_DisaContext& context, PG_DisaSpriteModel* actor, float degree, float durationTicks) {
	if (!actor) return;

	const float end = normalizeDegree(degree);
	if (durationTicks <= 0.f) {
		context.clearAnimation(actor, PG_DisaAnimationComponent::Channel_Rotation);
		actor->setRotation(end);
		return;
	}

	/* The game runs straight from the angle it started at to the target, so a turn that could go
	 * either way around takes the way the two numbers say and not the shorter one. */
	const float start = actor->getRotation();
	auto elapsed = std::make_shared<float>(0.f);
	context.takeAnimation(actor, PG_DisaAnimationComponent::Channel_Rotation, "PG_DisaRotateAnimation")
		->setTickFunction([start, end, durationTicks, elapsed](PG_DisaSpriteModel* self, float deltaTicks) {
			*elapsed += deltaTicks;
			const float progress = std::min(1.f, *elapsed / durationTicks);

			self->setRotation(normalizeDegree(glm::mix(start, end, progress)));
			return progress >= 1.f;
			});
}

void PG_DisaActorAnimations::rotateBy(PG_DisaContext& context, PG_DisaSpriteModel* actor, float degree, float durationTicks) {
	if (!actor) return;

	const float start = actor->getRotation();
	const float end = start + degree;

	if (durationTicks <= 0.f) {
		context.clearAnimation(actor, PG_DisaAnimationComponent::Channel_Rotation);
		actor->setRotation(normalizeDegree(end));
		return;
	}

	//the step is what the game keeps, so a turn of more than once around really goes around twice
	auto elapsed = std::make_shared<float>(0.f);
	context.takeAnimation(actor, PG_DisaAnimationComponent::Channel_Rotation, "PG_DisaRotateAnimation")
		->setTickFunction([start, end, durationTicks, elapsed](PG_DisaSpriteModel* self, float deltaTicks) {
			*elapsed += deltaTicks;
			const float progress = std::min(1.f, *elapsed / durationTicks);

			self->setRotation(normalizeDegree(glm::mix(start, end, progress)));
			return progress >= 1.f;
			});
}

void PG_DisaActorAnimations::play(PG_DisaContext& context, PG_DisaSpriteModel* actor, int animationSet, int loops) {
	if (!actor) return;

	actor->clearAnimation();
	actor->setAnimationSet(animationSet);

	//an animation that runs forever is not something a script can wait for, so none is registered
	if (loops <= 0) {
		context.clearAnimation(actor, PG_DisaAnimationComponent::Channel_Animation);
		return;
	}

	/* The sprite node owns the playback, all this does is hold the actor busy for as long as the
	 * animation would take. A frame count would be better than a guess, but the view only settles
	 * on a sprite once it has drawn, so the length is not known here yet. */
	constexpr float ticksPerLoop = 30.f;
	auto left = std::make_shared<float>(ticksPerLoop * static_cast<float>(loops));
	context.takeAnimation(actor, PG_DisaAnimationComponent::Channel_Animation, "PG_DisaPlayAnimation")
		->setTickFunction([left](PG_DisaSpriteModel*, float deltaTicks) {
			*left -= deltaTicks;
			return *left <= 0.f;
			});
}

void PG_DisaActorAnimations::tint(PG_DisaContext& context, PG_DisaSpriteModel* actor, const glm::vec4& color,
	float durationTicks) {
	if (!actor) return;

	if (durationTicks <= 0.f) {
		context.clearAnimation(actor, PG_DisaAnimationComponent::Channel_Color);
		actor->setColorMult(color);
		return;
	}

	const glm::vec4 start = actor->getColorMult();
	auto elapsed = std::make_shared<float>(0.f);
	context.takeAnimation(actor, PG_DisaAnimationComponent::Channel_Color, "PG_DisaTintAnimation")
		->setTickFunction([start, color, durationTicks, elapsed](PG_DisaSpriteModel* self, float deltaTicks) {
			*elapsed += deltaTicks;
			const float progress = std::min(1.f, *elapsed / durationTicks);

			self->setColorMult(glm::mix(start, color, progress));
			return progress >= 1.f;
			});
}

void PG_DisaActorAnimations::setAnimation(PG_DisaSpriteModel* actor, int animation) {
	if (!actor) return;
	//naming the sheet takes the choice away from the angle, the way the game drops the mode
	actor->setMode(PG_DisaSpriteModel::Mode_None);
	actor->setAnimation(animation);
}
