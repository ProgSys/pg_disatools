#pragma once
#include <Scene/PG_ModelNode.h>
#include <Disa/PG_DisaTransformation.h>



class PG_DisaActorNode : public PG_ActorNode {
	PG_OBJECT(PG_DisaActorNode);
	PG_PARENT(PG_ActorNode);

	PG_PROPERTY(short actorId GET getActorId);
public:
	PG_DisaActorNode(const std::string& name);
	PG_DisaActorNode(const std::string& name, short actorId);

	void init();

	inline short getActorId() const { return m_actorId; }

private:
	void onParentChanged(PG_Object* oldManager, PG_Object* newManager);

	short m_actorId = 0;
	
};