#include "PG_DisaGeoPanelViewNode.h"
#include <Scene/Model/PG_ListModelBase.h>
#include <Disa/PG_DisaMapModel.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/map/PG_DisaTileViewNode.h>

void PG_DisaGeoPanelViewNode::setModel(PG_DisaMapTileListModel* model) {
	if (m_model == model) return;

	if (m_model) {
		disconnect(m_model, nullptr, this, nullptr);
	}

	m_model = model;

	if (m_model) {
		connect(m_model, &PG_ListModelBase::itemsAdded, this, &PG_DisaGeoPanelViewNode::update);
		connect(m_model, &PG_ListModelBase::itemsRemoved, this, &PG_DisaGeoPanelViewNode::update);
		connect(m_model, &PG_ListModelBase::itemsChanged, this, [this](unsigned int startIndex, unsigned int endIndex , const PG_IdentitySet& identities) {
			if (!m_model->empty() && contains<
				PG_DisaMapTileModel::Identity_GeoColor, PG_DisaMapTileModel::Identity_Geometry,
				PG_DisaMapTileModel::Identity_Translation, PG_Identity_Location, PG_Identity_Item>(identities)) {
				if (std::ranges::any_of(std::next(m_model->begin(), startIndex), std::next(m_model->begin(), endIndex + 1), &PG_DisaMapTileModel::hasGeoColor)) {
					update();
				}
				else if (PG_ModelNode::getModel().isValid()) {
					update(); //to clear the geometry if there is now no geo panels present
				}
			}
		});
		connect(m_model, &PG_ListModelBase::onDeleted, this, [this] { m_model = nullptr; update(); });
	}

	update();
}

void PG_DisaGeoPanelViewNode::update() {
	postCallEvent(PG_Event_Behaviour_Singleton, &PG_DisaGeoPanelViewNode::doUpdate);
}

void PG_DisaGeoPanelViewNode::doUpdate() {
	if (!m_model) {
		PG_ModelNode::getModel().clear();
		return;
	}

	PG_DisaTileViewNode::updateFloorGeoGeometry(m_model, this);

}