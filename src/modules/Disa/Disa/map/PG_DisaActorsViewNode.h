#pragma once

#include <Scene/PG_FootlooseViewNode.h>
#include <Disa/map/PG_DisaActorListModel.h> //the lifeline needs the whole type

/**
* @brief Draws everything standing on the map, whatever each of them is drawn as.
* @details One view over one PG_DisaActorListModel. A slot can hold a unit or, later, a mesh like
* the teleporter panel, so the item is chosen per row from what the row actually is rather than
* fixed for the whole view.
*
* Every item follows the same identities off the same list, which is what lets the two kinds sit
* next to each other without the view having to keep them apart.
*/
class PG_DisaActorsViewNode : public PG_FootlooseViewNode {
	PG_OBJECT(PG_DisaActorsViewNode);
	PG_PARENT(PG_FootlooseViewNode);
public:
	using PG_FootlooseViewNode::PG_FootlooseViewNode;

	void init() override;
};
