#include "PG_DisaAnimationComponent.h"

#include <Disa/map/PG_DisaSpriteModel.h>

PG_DisaSpriteModel* PG_DisaAnimationComponent::getActorModel() const {
	return getParent<PG_DisaSpriteModel>();
}

PG_DisaAnimationComponent::~PG_DisaAnimationComponent() {
	//the actor is already gone when it is the one taking this down, so the end has to cope with null
	if (m_endFunction) m_endFunction(getActorModel());
}

void PG_DisaAnimationComponent::setTickFunction(const TickFunction& value) {
	m_tickFunction = value;
	m_finished = !m_tickFunction; //a takeover revives one that had already run out
}

bool PG_DisaAnimationComponent::tick(float deltaTicks) {
	if (m_finished) return true;

	PG_DisaSpriteModel* actor = getActorModel();
	if (!actor || !m_tickFunction) {
		m_finished = true;
		return true;
	}
	m_finished = m_tickFunction(actor, deltaTicks);
	return m_finished;
}
