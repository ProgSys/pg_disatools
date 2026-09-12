#include "PG_DisaAnimationComponent.h"

#include "PG_DisaContext.h"

PG_DisaAnimationComponent::PG_DisaAnimationComponent(PG_DisaContext* context, PG_DisaActorModel* actor,
	EChannel channel, const TickFunction& tickFunction, const std::string& name)
	: PG_Object(name), m_actor(actor), m_channel(channel), m_tickFunction(tickFunction) {
	if (context) context->addAnimation(this);
}

bool PG_DisaAnimationComponent::tick(float deltaTicks) {
	if (m_finished) return true;
	if (!m_actor || !m_tickFunction) {
		m_finished = true;
		return true;
	}
	m_finished = m_tickFunction(m_actor, deltaTicks);
	return m_finished;
}
