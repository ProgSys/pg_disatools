#include "PG_DisaTeleporterViewNode.h"
#include <Scene/Model/PG_ListModelBase.h>
#include <Disa/PG_DisaMapModel.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/map/PG_DisaEventTileModel.h>

void PG_DisaTeleporterViewNode::setModel(PG_DisaEventTileListModel* model) {
	if (m_model == model) return;

	if (m_model) {
		disconnect(m_model, nullptr, this, nullptr);
		if (auto* mapModel = m_model->findParent<PG_DisaMapModel>(7)) {
			disconnect(mapModel->getTiles(), nullptr, this, nullptr);
		}
	}

	m_model = model;

	if (m_model) {
		connect(m_model, &PG_ListModelBase::itemsAdded, this, &PG_DisaTeleporterViewNode::update);
		connect(m_model, &PG_ListModelBase::itemsRemoved, this, &PG_DisaTeleporterViewNode::update);
		connect(m_model, &PG_ListModelBase::itemsChanged, this, [this](unsigned int startIndex, unsigned int endIndex, const PG_IdentitySet& identities) {
			if (!m_model->empty() && contains<
				PG_DisaEventTileModel::Identity_Value, PG_DisaEventTileModel::Identity_TilePosition, PG_Identity_Location, PG_Identity_Item>(identities)) {
				update();
			}
			});
		connect(m_model, &PG_ListModelBase::onDeleted, this, [this] { m_model = nullptr; update(); });

		auto* mapModel = m_model->findParent<PG_DisaMapModel>(7);
		assert(mapModel);

		connect(mapModel->getTiles(), &PG_DisaMapTileListModel::tilesGeometryChanged, this, &PG_DisaTeleporterViewNode::tileChanged, PG_ConnectionType_Single);

		connect(mapModel->getTiles(), &PG_DisaMapTileListModel::onDeleted, this, [this]() {
			auto i = 0;
			});
	}

	loadGlobal("R_DisaTeleporterBase");
	setScale(glm::vec3(1.f / 12.f));

	update();
}

void PG_DisaTeleporterViewNode::update() {
	postCallEvent(PG_Event_Behaviour_Singleton, &PG_DisaTeleporterViewNode::doUpdate);
}

void PG_DisaTeleporterViewNode::tileChanged(const std::unordered_set<glm::ivec2, PG_iVec2Hasher>& tiles) {
	if (tiles.contains(glm::ivec2(getTranslation().x, getTranslation().z) - glm::ivec2(1,0))) {
		update();
	}
}

void PG_DisaTeleporterViewNode::doUpdate() {
	if (!m_model) {
		setVisible(false);
		return;
	}

	auto findIt = std::ranges::find(*m_model, 1, &PG_DisaEventTileModel::getValue);
	
	if (findIt == m_model->end()) {
		setVisible(false);
		return;
	}

	auto* mapModel = m_model->findParent<PG_DisaMapModel>(7);
	assert(mapModel);
	if (!mapModel) {
		setVisible(false);
		return;
	}

	const glm::ivec2 pos = (*findIt)->getTilePosition();
	auto tileLoc = mapModel->getTiles()->getTileLocation(pos);

	float height = 0.0F;
	if (tileLoc) {
		height = tileLoc.getDataT<float>(PG_DisaMapTileModel::Identity_RefHeight);
	}
	
	setTranslation(glm::vec3(pos.x + 1, height, pos.y));
	setVisible(true);

}