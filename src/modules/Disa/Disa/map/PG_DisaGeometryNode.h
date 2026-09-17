#pragma once

#include <string>

#include <Disa/PG_DisaModelNode.h>
#include <Scene/Model/PG_ListModelLocation.h>
#include <Scene/Model/PG_ListModelIdentity.h>

/**
* @brief Visual representation of a slot item that is a mesh rather than a sprite.
* @details The counterpart of PG_DisaActorSpriteNode, made by the same view off the same list. It
* follows the identities every slot item answers, so where it stands and whether it is drawn come
* from the model exactly like they do for a unit.
* @see PG_DisaGeometryModel
*/
class PG_DisaGeometryNode : public PG_DisaModelNode {
	PG_OBJECT(PG_DisaGeometryNode);
	PG_PARENT(PG_DisaModelNode);
public:
	using PG_DisaModelNode::PG_DisaModelNode;

	PG_ListModelLocation getLocation() const { return m_location; }
	void setLocation(PG_ListModelLocation loc);
	void update(const PG_ListModelLocation& location, const PG_IdentitySet& identities);

private:
	/// Reads one identity, without throwing when the item does not answer it.
	template<typename T>
	T readData(Identity identity) const { return m_location.getData(identity).convert<T>(); }

	void updateResource();
	void updateTransformation();

	PG_ListModelLocation m_location;
	std::string m_loaded;
};
