#pragma once
#include <Scene/PG_ModelNode.h>
#include <Scene/Model/PG_ListModelLocation.h>
#include <Scene/Model/PG_ListModelIdentity.h>

class PG_DisaMapTileListModel;
/**
* @brief Visual representation of disa map geo panels
* Needs a PG_DisaMapModel.
* @see PG_DisaMapModel
*/
class PG_DisaGeoPanelViewNode : public PG_ModelNode {
	PG_OBJECT(PG_DisaGeoPanelViewNode);
	PG_PARENT(PG_ModelNode);
public:
	using PG_ModelNode::PG_ModelNode;

	inline PG_DisaMapTileListModel* getModel() const { return m_model; }
	void setModel(PG_DisaMapTileListModel* model);

	void update();
private:
	void doUpdate();

	PG_DisaMapTileListModel* m_model = nullptr;
};
