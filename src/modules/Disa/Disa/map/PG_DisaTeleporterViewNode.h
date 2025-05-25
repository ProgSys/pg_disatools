#pragma once
#include <Disa/PG_DisaModelNode.h>
#include <Scene/Model/PG_ListModelLocation.h>
#include <Scene/Model/PG_ListModelIdentity.h>
#include <unordered_set>
#include <Util/PG_iVec2Hasher.h>

class PG_DisaEventTileListModel;
/**
* @brief Visual representation of the base teleporter
* The teleporter is located at the very first event tile with the ID 1
* Needs a PG_DisaMapModel.
* @see PG_DisaMapModel
*/
class PG_DisaTeleporterViewNode : public PG_DisaModelNode {
	PG_OBJECT(PG_DisaTeleporterViewNode);
	PG_PARENT(PG_DisaModelNode);
public:
	using PG_DisaModelNode::PG_DisaModelNode;

	inline PG_DisaEventTileListModel* getModel() const { return m_model; }
	void setModel(PG_DisaEventTileListModel* model);

	void update();
private:
	void tileChanged(const std::unordered_set<glm::ivec2, PG_iVec2Hasher>& tiles);
	void doUpdate();

	PG_DisaEventTileListModel* m_model = nullptr;
};
