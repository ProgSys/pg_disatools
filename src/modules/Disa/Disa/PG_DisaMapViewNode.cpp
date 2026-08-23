#include "PG_DisaMapViewNode.h"
#include <Scene/PG_FootlooseViewNode.h>
#include <Disa/PG_DisaMapModel.h>
#include <Disa/PG_DisaAssetsModel.h>
#include <Disa/map/PG_DisaActorModel.h>

#include <Disa/map/PG_DisaTileEditSelectionNode.h>
#include <Disa/map/PG_DisaMaterialEditSelectionNode.h>
#include <Disa/map/PG_DisaEventEditSelectionNode.h>
#include <Disa/map/PG_DisaActorEditSelectionNode.h>
#include <Disa/map/PG_DisaTeleporterViewNode.h>
#include <Disa/map/PG_DisaGeoPanelViewNode.h>
#include <Disa/map/PG_DisaEventPanelViewNode.h>
#include <Disa/map/PG_DisaTileViewNode.h>
#include <Disa/map/PG_DisaObjectViewNode.h>
#include <Disa/map/PG_DisaActorViewNode.h>
#include <Disa/PG_DisaModelNode.h>

#include <resources/PG_ResourceSynchronisator.h>
#include <Util/PG_RangesTransform.h>

void PG_DisaMapViewNode::init() {
	PG_ActorNode::init();

	connect(this, &PG_DisaMapViewNode::isGeoVisibleChanged, this, &PG_DisaMapViewNode::createGeoPanels);

	connect(this, &PG_DisaMapViewNode::isActorsVisibleChanged, this, [this] {
		if (isActorsVisible()) {
			if (!m_actorView) {
				createActorView();
				m_actorView->setModel(getMapModel()->getActors());
			}
			
		}
		else if(m_actorView) {
			delete m_actorView;
			m_actorView = nullptr;
		}
	});

	m_matBuffer = { PG_Material("M_DisaTileEditSelection") };
}

void PG_DisaMapViewNode::setModel(const std::shared_ptr<PG_DisaMPDContainer>& container) {
	if (m_mpdContainer == container) return;

	if (m_mpdContainer) {
		disconnect(m_mpdContainer->getModel()->getTiles(), nullptr, this, nullptr);
		disconnect(m_mpdContainer->getModel(), nullptr, this, nullptr);
	}
	m_mpdContainer = container;

	if (m_mpdContainer) {
		auto* model = m_mpdContainer->getModel();
		connect(model->getTiles(), &PG_ListModelBase::itemsChanged, this, [this](unsigned int startIndex, unsigned int endIndex, const PG_IdentitySet& identities) {
			auto* selectionHandler = findFirstChild<PG_DisaMapSelectionHandlerNode>();
			if (!selectionHandler) return;
			auto* tiles = m_mpdContainer->getModel()->getTiles();
			if (tiles->empty()) return;
			endIndex = std::min(endIndex, (unsigned int)tiles->getItems().size() - 1);
			for (int i = startIndex; i <= endIndex; ++i) {
				selectionHandler->update(tiles->get(i)->getTilePosition());
			}
		});
		connect(model, &PG_DisaMapModel::mapChanged, this, &PG_DisaMapViewNode::contentChanged, PG_ConnectionType_Event);
	}
	
	reload();
}

PG_DisaMapModel* PG_DisaMapViewNode::getMapModel() const {
	if (m_mpdContainer) return m_mpdContainer->getModel();
	return nullptr;
}

void PG_DisaMapViewNode::setMode(EMode mode) {
	auto* model = getMapModel();
	if (!model) mode = Mode_View;
	if (mode == m_mode) return;

	switch (m_mode)
	{
	case PG_DisaMapViewNode::Mode_Terrain: {
		clearChildren(PG_Search::byType<PG_DisaMapSelectionHandlerNode>());

		const auto func = [](PG_Object* obj) {
			if (auto model = dynamic_cast<PG_DisaModelNode*>(obj)) {
				model->reverseTransparent();
			}
			return true;
			};
		getObjectsView()->traverseTestWithoutRoot(func);
		func(getTeleporterView());

	}break;
	case PG_DisaMapViewNode::Mode_Material:
		clearChildren(PG_Search::byType<PG_DisaMaterialEditSelectionHandlerNode>());
		break;
	case PG_DisaMapViewNode::Mode_Object:
		clearChildren(PG_Search::byType<PG_DisaMapSelectionHandlerNode>());
		clearChildren(PG_Search::byType<PG_DisaMaterialEditSelectionHandlerNode>());
		break;
	case PG_DisaMapViewNode::Mode_Event:
		clearChildren(PG_Search::byType<PG_DisaEventPanelViewNode>());
		clearChildren(PG_Search::byType<PG_DisaEventEditSelectionHandlerNode>());
		break;
	case PG_DisaMapViewNode::Mode_Actor:
		clearChildren(PG_Search::byType<PG_DisaActorEditSelectionHandlerNode>());
		break;
	default:break;
	}

	m_mode = mode;

	switch (m_mode)
	{
	default: break;
	case PG_DisaMapViewNode::Mode_Terrain: {
		auto selectionHandler = PG_new<PG_DisaMapSelectionHandlerNode>("PG_DisaMapSelectionHandlerNode", this);
		selectionHandler->moveFront(getTileView());
		connect(model->getTiles(), &PG_ListModelBase::itemsChanged, this, [this](unsigned int startIndex, unsigned int endIndex, const PG_IdentitySet& identities) {
			auto* selectionHandler = findFirstChild<PG_DisaMapSelectionHandlerNode>();
			if (!selectionHandler) return;
			auto* tiles = getMapModel()->getTiles();
			if (tiles->empty()) return;
			endIndex = std::min(endIndex, (unsigned int)tiles->getItems().size() - 1);
			for (int i = startIndex; i <= endIndex; ++i) {
				selectionHandler->update(getMapModel()->getTiles()->get(i)->getTilePosition());
			}
		});

		auto func = [](PG_Object* obj) {
			if (auto model = dynamic_cast<PG_DisaModelNode*>(obj)) {
				model->makeTransparent();
			}
			return true;
			};
		getObjectsView()->traverseTestWithoutRoot(func);
		func(getTeleporterView());

	} break;
	case PG_DisaMapViewNode::Mode_Material: {
		PG_new<PG_DisaMaterialEditSelectionHandlerNode>("PG_DisaMaterialEditSelectionHandlerNode", this);
		connect(model->getTiles(), &PG_ListModelBase::itemsChanged, this, [this](unsigned int startIndex, unsigned int endIndex, const PG_IdentitySet& identities) {
			if (auto* selectionHandler = findFirstChild<PG_DisaMaterialEditSelectionHandlerNode>()) {
				for (int i = startIndex; i <= endIndex; ++i) {
					selectionHandler->update(getMapModel()->getTiles()->get(i)->getTilePosition());
				}
			}
			});
	}break;
	case PG_DisaMapViewNode::Mode_Event: {
		PG_new<PG_DisaEventEditSelectionHandlerNode>("PG_DisaEventEditSelectionHandlerNode", this);
		connect(model->getTiles(), &PG_ListModelBase::itemsChanged, this, [this](unsigned int startIndex, unsigned int endIndex, const PG_IdentitySet& identities) {
			if (auto* selectionHandler = findFirstChild<PG_DisaEventEditSelectionHandlerNode>()) {
				for (int i = startIndex; i <= endIndex; ++i) {
					selectionHandler->update(getMapModel()->getTiles()->get(i)->getTilePosition());
				}
			}
		});
		PG_new<PG_DisaEventPanelViewNode>("PG_DisaEventPanelViewNode", this)->setModel(model->getEvents());
	}break;
	case PG_DisaMapViewNode::Mode_Actor: {
		PG_new<PG_DisaActorEditSelectionHandlerNode>("PG_DisaActorEditSelectionHandlerNode", this);
		connect(model->getTiles(), &PG_ListModelBase::itemsChanged, this, [this](unsigned int startIndex, unsigned int endIndex, const PG_IdentitySet& identities) {
			if (auto* selectionHandler = findFirstChild<PG_DisaActorEditSelectionHandlerNode>()) {
				for (auto* actor : *getMapModel()->getActors()) {
					selectionHandler->update(actor);
				}
			}
			});
	}break;
	}
	emit modeChanged();
}

void PG_DisaMapViewNode::setHDMode(bool state) {
	if (m_HDMode != state) {
		m_HDMode = state;
		emit hqModeChanged();
		if (auto* model = getMapModel()) model->getAssets()->setHDViewEnabled(m_HDMode);
	}
}

void PG_DisaMapViewNode::setActorsVisible(bool value) {
	if (m_isActorsVisible != value) {
		m_isActorsVisible = value;
		emit isActorsVisibleChanged();
	}
}

void PG_DisaMapViewNode::setGeoVisible(bool value) {
	if (m_isGeoVisible != value) {
		m_isGeoVisible = value;
		emit isGeoVisibleChanged();
	}
}

void PG_DisaMapViewNode::createGeoPanels() {
	if (auto* model = getMapModel(); model && isGeoVisible() ) {
		PG_new<PG_DisaGeoPanelViewNode>("PG_DisaGeoPanelViewNode", this)->setModel(model->getTiles());
	}
	else {
		clearChildren(PG_Search::byType<PG_DisaGeoPanelViewNode>());
	}
}

void PG_DisaMapViewNode::createActorView() {
	m_actorView = PG_new<PG_FootlooseViewNode>("PG_DisaActorViewNode", this);
	m_actorView->setItem(PG_FootlooseViewNode::getDefaultCreateFunc<PG_DisaActorViewNode>());
	m_actorView->setItemUpdate(PG_FootlooseViewNode::getDefaultUpdateFunc<PG_DisaActorViewNode>());
}

void PG_DisaMapViewNode::reload() {
	auto* model = getMapModel();
	if (!model) {
		clearChildren([](const PG_Object* o) { return dynamic_cast<const PG_ActorNode*>(o); });
		m_tilesView = nullptr;
		m_objectsView = nullptr;
		m_actorView = nullptr;
		return;
	}

	if (!m_tilesView) {
		m_tilesView = PG_new<PG_FootlooseViewNode>("PG_TilesViewNode", this);
		m_tilesView->setItem(PG_FootlooseViewNode::getDefaultCreateFunc<PG_DisaTileViewNode>());
		m_tilesView->setItemUpdate(PG_FootlooseViewNode::getDefaultUpdateFunc<PG_DisaTileViewNode>());
	}
	if (!m_objectsView) {
		m_objectsView = PG_new<PG_FootlooseViewNode>("PG_ObjectsViewNode", this);
		m_objectsView->setItem(PG_FootlooseViewNode::getDefaultCreateFunc<PG_DisaObjectViewNode>());
		m_objectsView->setItemUpdate(PG_FootlooseViewNode::getDefaultUpdateFunc<PG_DisaObjectViewNode>());
	}
	if (isActorsVisible() && !m_actorView) {
		createActorView();
	}

	if (!m_teleporterView) {
		m_teleporterView = PG_new<PG_DisaTeleporterViewNode>("PG_DisaTeleporterViewNode", this);
	}

	m_tilesView->setModel(model->getTiles());
	m_objectsView->setModel(model->getObjects());
	m_teleporterView->setModel(model->getEvents());
	if(m_actorView) m_actorView->setModel(model->getActors());

	
	if (auto geoPanelView = findFirstChild<PG_DisaGeoPanelViewNode>("PG_DisaGeoPanelViewNode")) {
		geoPanelView->setModel(model->getTiles());
	}
	else {
		createGeoPanels();
	}
}

void PG_DisaMapViewNode::contentChanged() {
	auto* model = getMapModel();
	if (!model || !m_tilesView) return;
	setName("View_"+ model->getName());
	auto* sync = model->getAssets()->getResourceSynchronisator();
	//collect everything else
	for (auto* item : m_tilesView->getItems() | PG_CastTo<PG_DisaTileViewNode*>) {
		sync->add(item->getModel());
	}

	connect(sync, &PG_ResourceSynchronisator::loaded, this, &PG_DisaMapViewNode::updateTree);
	connect(sync, &PG_ResourceSynchronisator::loaded, this, &PG_DisaMapViewNode::loaded, PG_ConnectionType_Event);
	sync->start();
}