#include "PG_DisaActorsViewNode.h"

#include <Disa/map/PG_DisaActorSpriteNode.h>
#include <Disa/map/PG_DisaSpriteModel.h>

void PG_DisaActorsViewNode::init() {
	PG_FootlooseViewNode::init(); //without it the node never reaches its scene

	setItem([](PG_ActorNode* parent, const PG_ListModelLocation& location) -> PG_ActorNode* {
		/* One row, one item, but not always the same kind of item. A unit is drawn as a sprite,
		 * and the other kinds pick their own node as they arrive. */
		auto* item = PG_new<PG_DisaActorSpriteNode>("ViewItem", parent);
		item->setLocation(location);
		return item;
		});

	setItemUpdate([](PG_ActorNode* item, const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
		if (auto* sprite = dynamic_cast<PG_DisaActorSpriteNode*>(item)) sprite->update(location, identities);
		});
}
