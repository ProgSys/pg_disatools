#include "PG_DisaActorAnimations.h"

#include <algorithm>
#include <cmath>
#include <memory>

#include <glm/gtc/constants.hpp>

#include <Disa/map/PG_DisaActorModel.h>

#include "PG_DisaAnimationComponent.h"
#include "PG_DisaContext.h"

namespace {

/// The override of the actor with its translation filled in, so callers never see an empty one.
PG_DisaActorScriptOverride overrideOf(PG_DisaActorModel* actor) {
	PG_DisaActorScriptOverride out = actor->getScriptOverride();
	if (!out.translation) out.translation = actor->getObjectTranslation();
	return out;
}

void setTranslation(PG_DisaActorModel* actor, const glm::vec3& position) {
	PG_DisaActorScriptOverride out = overrideOf(actor);
	out.translation = position;
	actor->setScriptOverride(out);
}

} //namespace

void PG_DisaActorAnimations::fall(PG_DisaContext& context, PG_DisaActorModel* actor, float gravity) {
	if (!actor) return;

	const glm::vec3 start = overrideOf(actor).translation.value();
	const float ground = context.getGroundHeight(start);
	if (start.y <= ground) {
		//already standing on it, or under it, in which case the game puts it back on top
		if (start.y < ground) setTranslation(actor, glm::vec3(start.x, ground, start.z));
		return;
	}

	auto speed = std::make_shared<float>(0.f);
	PG_DisaContext* world = &context;
	new PG_DisaAnimationComponent(&context, actor, PG_DisaAnimationComponent::Channel_Move,
		[world, gravity, speed](PG_DisaActorModel* self, float deltaTicks) {
			glm::vec3 position = overrideOf(self).translation.value();
			*speed += self->getScriptOverride().gravity.value_or(gravity) * deltaTicks;
			position.y -= *speed * deltaTicks;

			const float floorHeight = world->getGroundHeight(position);
			const bool landed = position.y <= floorHeight;
			if (landed) position.y = floorHeight;
			setTranslation(self, position);
			return landed;
		}, "PG_DisaFallAnimation");
}

void PG_DisaActorAnimations::move(PG_DisaContext& context, PG_DisaActorModel* actor, const glm::vec3& target,
	float durationTicks, EMoveMode mode) {
	if (!actor) return;

	const glm::vec3 start = overrideOf(actor).translation.value();

	float ticks = durationTicks;
	if (mode == Move_Speed) {
		//the duration is a speed here, so the travel time is what the distance makes of it
		const float distance = glm::length(glm::vec2(target.x - start.x, target.z - start.z));
		ticks = durationTicks > 0.f ? distance / durationTicks : 0.f;
	}

	if (ticks <= 0.f) {
		setTranslation(actor, target);
		fall(context, actor, PG_DisaContext::defaultGravity);
		return;
	}

	/* A jump keeps the height of the target as the height of its arc and lands on the ground, a
	 * straight move simply runs to the target and drops from there. */
	const float arcHeight = mode == Move_Jump ? target.y - start.y : 0.f;
	const glm::vec3 end = mode == Move_Jump
		? glm::vec3(target.x, context.getGroundHeight(target), target.z)
		: target;

	auto elapsed = std::make_shared<float>(0.f);
	PG_DisaContext* world = &context;
	new PG_DisaAnimationComponent(&context, actor, PG_DisaAnimationComponent::Channel_Move,
		[world, start, end, ticks, arcHeight, elapsed](PG_DisaActorModel* self, float deltaTicks) {
			*elapsed += deltaTicks;
			const float progress = std::min(1.f, *elapsed / ticks);

			glm::vec3 position = glm::mix(start, end, progress);
			if (arcHeight != 0.f) position.y += arcHeight * std::sin(progress * glm::pi<float>());
			setTranslation(self, position);

			if (progress < 1.f) return false;
			fall(*world, self, PG_DisaContext::defaultGravity);
			return true;
		}, "PG_DisaMoveAnimation");
}

void PG_DisaActorAnimations::rotate(PG_DisaContext& context, PG_DisaActorModel* actor, float degree, float durationTicks) {
	if (!actor) return;

	PG_DisaActorScriptOverride scriptOverride = overrideOf(actor);
	const float start = scriptOverride.rotation.value_or(0.f);
	const float end = start + degree; //the game adds the angle, it does not set it

	if (durationTicks <= 0.f) {
		scriptOverride.rotation = end;
		actor->setScriptOverride(scriptOverride);
		return;
	}

	auto elapsed = std::make_shared<float>(0.f);
	new PG_DisaAnimationComponent(&context, actor, PG_DisaAnimationComponent::Channel_Rotation,
		[start, end, durationTicks, elapsed](PG_DisaActorModel* self, float deltaTicks) {
			*elapsed += deltaTicks;
			const float progress = std::min(1.f, *elapsed / durationTicks);

			PG_DisaActorScriptOverride out = self->getScriptOverride();
			out.rotation = glm::mix(start, end, progress);
			self->setScriptOverride(out);
			return progress >= 1.f;
		}, "PG_DisaRotateAnimation");
}

void PG_DisaActorAnimations::play(PG_DisaContext& context, PG_DisaActorModel* actor, const std::string& animation, int loops) {
	if (!actor) return;

	PG_DisaActorScriptOverride scriptOverride = actor->getScriptOverride();
	if (animation.empty()) {
		/* Clearing it hands the actor back to the idle set the view picks by angle, which is what
		 * a script means when it asks for animation 0. */
		scriptOverride.animation.reset();
		actor->setScriptOverride(scriptOverride);
		return;
	}

	scriptOverride.animation = animation;
	actor->setScriptOverride(scriptOverride);

	//an animation that runs forever is not something a script can wait for, so none is registered
	if (loops <= 0) return;

	/* The sprite node owns the playback, all this does is hold the actor busy for as long as the
	 * animation would take. A frame count would be better than a guess, but the view only settles
	 * on a sprite once it has drawn, so the length is not known here yet. */
	constexpr float ticksPerLoop = 30.f;
	auto left = std::make_shared<float>(ticksPerLoop * static_cast<float>(loops));
	new PG_DisaAnimationComponent(&context, actor, PG_DisaAnimationComponent::Channel_Animation,
		[left](PG_DisaActorModel*, float deltaTicks) {
			*left -= deltaTicks;
			return *left <= 0.f;
		}, "PG_DisaPlayAnimation");
}
