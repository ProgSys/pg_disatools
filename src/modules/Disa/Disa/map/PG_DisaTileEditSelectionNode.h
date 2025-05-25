#pragma once
#include <Scene/PG_ModelNode.h>
#include <Scene/Model/PG_ListModelLocation.h>
#include <Scene/Model/PG_ListModelIdentity.h>
#include <Disa/map/PG_DisaIntersection.h>
#include <Util/PG_iVec2Hasher.h>

class PG_DisaMapModel;
class PG_FootlooseViewNode;


class PG_DisaTileEditSelectionNode;


class PG_DisaMapSelectionHandlerNode : public PG_ActorNode {
	PG_OBJECT(PG_DisaMapSelectionHandlerNode);
	PG_PARENT(PG_ActorNode);
public:
	using PG_ActorNode::PG_ActorNode;

	void setHovered(glm::ivec2 pos, bool hovered, PG_DisaFaceHit hitInfo);
	void setSelected(glm::ivec2 pos, bool selected);
	void update(glm::ivec2 pos);
	void cleanUp(); //removes all that not exist anymore

private:
	std::unordered_map<glm::ivec2, PG_DisaTileEditSelectionNode*, PG_iVec2Hasher> m_selectionNodes;
};

class PG_DisaTileEditSelectionNode : public PG_ModelNode {
	PG_OBJECT(PG_DisaTileEditSelectionNode);
	PG_PARENT(PG_ModelNode);
	PG_PROPERTY(int displayMode GET getDisplayMode CHANGED disaFaceHitChanged)
		PG_PROPERTY(bool isHovered GET isHovered CHANGED disaFaceHitChanged)
		PG_PROPERTY(bool isSelected GET isSelected CHANGED selectedChanged)
signals:
	void disaFaceHitChanged();
	void selectedChanged();
public:
	using PG_ModelNode::PG_ModelNode;


	bool isSelected() const { return mSelected; }
	void setSelected(bool selected) {
		if (mSelected == selected) return;
		mSelected = selected;
		emit selectedChanged();
	}

	int getDisplayMode() const { return m_hitInfo.faceSection; }
	void setDisaFaceHit(PG_DisaFaceHit hit) {
		m_hitInfo = hit;
		emit disaFaceHitChanged();
	}
	bool isHovered() const { return m_hitInfo.faceSection != PG_DisaFaceHit::None; }
	bool isValid() const { return isSelected() || isHovered(); }

	PG_DisaFaceHit m_hitInfo;
	bool mSelected = false;
};

