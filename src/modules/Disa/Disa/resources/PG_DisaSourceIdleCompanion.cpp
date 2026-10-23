#include <Disa/resources/PG_DisaSourceIdleCompanion.h>

#include <Base/PG_Timer.h>
#include <Resources/PG_ResourceManager.h>

PG_DisaSourceIdleCompanion::PG_DisaSourceIdleCompanion(PG_ResourceManager* manager, PG_ResourceSource_WPtr source,
	std::shared_ptr<std::atomic_bool> released, Check check) :
	PG_Object("DisaSourceIdleCompanion"), m_manager(manager), m_source(std::move(source)), m_released(std::move(released)), m_check(std::move(check)) {
	setThreadId(manager->getThreadId());
	PG_Connector::postCallEvent(PG_Event_Behaviour_Normal, this, &PG_DisaSourceIdleCompanion::attach);
}

void PG_DisaSourceIdleCompanion::attach() {
	if (*m_released) {
		deleteLater();
		return;
	}
	setParent(m_manager);

	auto* timer = PG_newWithParent<PG_Timer>("SourceIdleTimer", this);
	timer->setRepeat(true);
	timer->setDuration(std::chrono::seconds(1));
	connect(timer, &PG_Timer::passed, this, [this] { check(); });
	timer->start();
}

void PG_DisaSourceIdleCompanion::check() {
	auto source = m_source.lock();
	if (!source || *m_released) {
		deleteLater();
		return;
	}
	if (m_check) m_check(*source);
}
