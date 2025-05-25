#pragma once
#include <Scene/PG_ModelNode.h>
#include <Scene/Model/PG_ListModelLocation.h>
#include <Scene/Model/PG_ListModelIdentity.h>
#include <Disa/map/PG_DisaIntersection.h>
#include <Util/PG_iVec2Hasher.h>

class PG_DisaMapModel;
class PG_FootlooseViewNode;


class PG_DisaMaterialEditSelectionNode;

class PG_DisaMaterialEditSelectionHandlerNode : public PG_ActorNode {
	PG_OBJECT(PG_DisaMaterialEditSelectionHandlerNode);
	PG_PARENT(PG_ActorNode);
public:
	using PG_ActorNode::PG_ActorNode;

	void setHovered(glm::ivec2 pos, bool hovered, PG_DisaFaceHit hitInfo);
	void setSelected(glm::ivec2 pos, bool selected, PG_DisaFaceHit hitInfo);
	void update(glm::ivec2 pos);

private:
	using FaceMap = std::unordered_map<PG_DisaFaceHit::Face, PG_DisaMaterialEditSelectionNode*>;
	std::unordered_map<glm::ivec2, FaceMap, PG_iVec2Hasher> m_selectionNodes;
};

class PG_DisaMaterialEditSelectionNode : public PG_ModelNode {
	PG_OBJECT(PG_DisaMaterialEditSelectionNode);
	PG_PARENT(PG_ModelNode);
	PG_PROPERTY(int displayMode GET getDisplayMode CHANGED disaFaceHitChanged)
	PG_PROPERTY(bool isHovered GET isHovered CHANGED hoveredChanged)
	PG_PROPERTY(bool isSelected GET isSelected CHANGED selectedChanged)
signals:
	void disaFaceHitChanged();
	void hoveredChanged();
	void selectedChanged();
public:
	using PG_ModelNode::PG_ModelNode;


	bool isHovered() const { return mHovered; }
	void setHovered(bool hoverd) {
		if (mHovered == hoverd) return;
		mHovered = hoverd;
		emit hoveredChanged();
	}
	bool isSelected() const { return mSelected; }
	void setSelected(bool selected) {
		if (mSelected == selected) return;
		mSelected = selected;
		emit selectedChanged();
	}

	PG_DisaFaceHit getHitInfo() const { return m_hitInfo; }
	int getDisplayMode() const { return m_hitInfo.faceSection; }
	void setDisaFaceHit(PG_DisaFaceHit hit) {
		m_hitInfo = hit;
		emit disaFaceHitChanged();
	}
	bool isValid() const { return isSelected() || isHovered(); }

	PG_DisaFaceHit m_hitInfo;
	bool mHovered = false;
	bool mSelected = false;
};

