#include "PG_DisaActorsViewNode.h"

#include <Disa/map/PG_DisaActorSpriteNode.h>
#include <Disa/map/PG_DisaGeometryModel.h>
#include <Disa/map/PG_DisaGeometryNode.h>
#include <Disa/map/PG_DisaSpriteModel.h>

namespace {

/// What the row holds, null when the model behind it does not hand its item out.
PG_DisaActorModel* itemOf(const PG_ListModelLocation& location) {
	return location.getData(PG_Identity_Item).convert<PG_DisaActorModel*>();
}

} //namespace

void PG_DisaActorsViewNode::init() {
	PG_FootlooseViewNode::init(); //without it the node never reaches its scene

	setItem([](PG_ActorNode* parent, const PG_ListModelLocation& location) -> PG_ActorNode* {
		/* One row, one item, but not always the same kind of item. A mesh gets a model node, and
		 * everything else a sprite, which is also what a row that hands out no item at all gets. */
		if (dynamic_cast<PG_DisaGeometryModel*>(itemOf(location))) {
			auto* mesh = PG_new<PG_DisaGeometryNode>("ViewItem", parent);
			mesh->setLocation(location);
			return mesh;
		}

		auto* sprite = PG_new<PG_DisaActorSpriteNode>("ViewItem", parent);
		sprite->setLocation(location);
		return sprite;
		});

	setItemUpdate([](PG_ActorNode* item, const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
		if (auto* sprite = dynamic_cast<PG_DisaActorSpriteNode*>(item)) sprite->update(location, identities);
		else if (auto* mesh = dynamic_cast<PG_DisaGeometryNode*>(item)) mesh->update(location, identities);
		});
}
