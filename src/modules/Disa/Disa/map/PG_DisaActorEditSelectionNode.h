#pragma once
#include <Scene/PG_ModelNode.h>
#include <Scene/Model/PG_ListModelLocation.h>
#include <Scene/Model/PG_ListModelIdentity.h>

class PG_DisaActorModel;
class PG_FootlooseViewNode;
class PG_DisaActorEditSelectionNode;


class PG_DisaActorEditSelectionHandlerNode : public PG_ActorNode {
	PG_OBJECT(PG_DisaActorEditSelectionHandlerNode);
	PG_PARENT(PG_ActorNode);
public:
	using PG_ActorNode::PG_ActorNode;

	void setHovered(PG_DisaActorModel* model, bool hovered);
	void setSelected(PG_DisaActorModel* model, bool selected);
	void update(PG_DisaActorModel* model);
	void cleanUp(); //removes all that not exist anymore

private:
	std::unordered_map<PG_DisaActorModel*, PG_DisaActorEditSelectionNode*> m_selection;
	float m_lastTime = 0.f;
};


class PG_DisaActorEditSelectionNode : public PG_ActorNode {
	PG_OBJECT(PG_DisaActorEditSelectionNode);
	PG_PARENT(PG_ActorNode);
public:
	using PG_ActorNode::PG_ActorNode;

	void init() override;
	bool event(PG_Event* e);
	void rotateTowardsCamera(float dt = 0);

	float getTime() const { return m_time; }
	void setTime(float t) { m_time = t; }

	float getBaseHeight() const { return m_baseHeight; }
	void setBaseHeight(float t) { m_baseHeight = t; }

	bool isHovered() const { return m_isHovered; }
	void setHovered(bool b) { m_isHovered = b; }

	bool isSelected() const { return m_isSelected; }
	void setSelected(bool b) { m_isSelected = b; }

private:
	float m_time = 0.f;
	float m_baseHeight = 1.3f;
	bool m_isHovered = false;
	bool m_isSelected = false;
};


