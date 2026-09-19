#include "PG_DisaCameraAnimationComponent.h"

#include <Scene/PG_DisaCamera.h>

PG_DisaCamera* PG_DisaCameraAnimationComponent::getCamera() const {
	return getParent<PG_DisaCamera>();
}

void PG_DisaCameraAnimationComponent::setTickFunction(const TickFunction& value) {
	m_tickFunction = value;
	m_finished = !m_tickFunction; //a takeover revives one that had already run out
}

bool PG_DisaCameraAnimationComponent::tick(float deltaTicks) {
	if (m_finished) return true;

	PG_DisaCamera* camera = getCamera();
	if (!camera || !m_tickFunction) {
		m_finished = true;
		return true;
	}
	m_finished = m_tickFunction(camera, deltaTicks);
	return m_finished;
}
