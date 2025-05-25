#include "PG_DisaEventPanelViewNode.h"
#include <Scene/Model/PG_ListModelBase.h>
#include <Disa/PG_DisaMapModel.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/map/PG_DisaTileViewNode.h>
#include <Disa/map/PG_DisaEventTileModel.h>
#include <ranges>

void PG_DisaEventPanelViewNode::setModel(PG_DisaEventTileListModel* model) {
	if (m_model == model) return;

	if (m_model) {
		disconnect(m_model, nullptr, this, nullptr);
	}

	m_model = model;

	if (m_model) {
		connect(m_model, &PG_ListModelBase::itemsAdded, this, &PG_DisaEventPanelViewNode::update);
		connect(m_model, &PG_ListModelBase::itemsRemoved, this, &PG_DisaEventPanelViewNode::update);
		connect(m_model, &PG_ListModelBase::itemsChanged, this, [this](unsigned int startIndex, unsigned int endIndex , const PG_IdentitySet& identities) {
			if (!m_model->empty() && contains<Identity_Location, Identity_Item>(identities)) {
				update(); 
			}
		});
		connect(m_model, &PG_ListModelBase::onDeleted, this, [this] { m_model = nullptr; update(); });
	}

	update();
}

void PG_DisaEventPanelViewNode::update() {
	postCallEvent(EventBehaviour_Singleton, &PG_DisaEventPanelViewNode::doUpdate);
}

void PG_DisaEventPanelViewNode::doUpdate() {
	if (!m_model) {
		PG_ModelNode::getModel().clear();
		return;
	}

	std::vector<glm::ivec2> eventTiles;
	eventTiles.reserve(m_model->size());
	for (auto* item : *m_model) {
		eventTiles.push_back(item->getTilePosition());
	}
	std::ranges::sort(eventTiles, [](glm::ivec2 a, glm::ivec2 b) { return (a.x == b.x && a.y < b.y) || a.x < b.y; });
	const auto ret = std::ranges::unique(eventTiles);
	eventTiles.erase(ret.begin(), ret.end());

	PG_DisaTileViewNode::updateFloorEventGeometry(m_model->getParent<PG_DisaMapModel>()->getTiles(), eventTiles, this);

}