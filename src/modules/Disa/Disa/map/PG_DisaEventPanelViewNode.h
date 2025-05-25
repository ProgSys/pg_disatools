#pragma once
#include <Scene/PG_ModelNode.h>
#include <Scene/Model/PG_ListModelLocation.h>
#include <Scene/Model/PG_ListModelIdentity.h>

class PG_DisaEventTileListModel;
/**
* @brief Visual representation of disa map event panels
* Needs a PG_DisaMapModel.
* @see PG_DisaMapModel
*/
class PG_DisaEventPanelViewNode : public PG_ModelNode {
	PG_OBJECT(PG_DisaEventPanelViewNode);
	PG_PARENT(PG_ModelNode);
public:
	using PG_ModelNode::PG_ModelNode;

	inline PG_DisaEventTileListModel* getModel() const { return m_model; }
	void setModel(PG_DisaEventTileListModel* model);

	void update();
private:
	void doUpdate();

	PG_DisaEventTileListModel* m_model = nullptr;
};
