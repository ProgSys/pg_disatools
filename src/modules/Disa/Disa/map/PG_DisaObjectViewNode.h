#pragma once
#include <Scene/PG_ActorNode.h>
#include <Scene/Model/PG_ListModelLocation.h>
#include <Scene/Model/PG_ListModelIdentity.h>

class PG_DisaMapModel;
class PG_DisaMapTileListModel;
class PG_DisaModelNode;
/**
* @brief Visual representation of disa map tile
* Needs a PG_DisaMapModel.
* @see PG_DisaMapModel
*/
class PG_DisaObjectViewNode : public PG_ActorNode {
	PG_OBJECT(PG_DisaObjectViewNode);
	PG_PARENT(PG_ActorNode);

	PG_PROPERTY(PG_Object* model GET getObjectModel);
	PG_PROPERTY(glm::vec3 translationUser GET getTranslationUser SET setTranslationUser CHANGED translationUserChanged);
	PG_PROPERTY(glm::quat rotationUser GET getRotationUser SET setRotationUser CHANGED rotationUserChanged);
	PG_PROPERTY(glm::vec3 scaleUser GET getScaleUser SET setScaleUser CHANGED scaleUserChanged);
signals:
	void translationUserChanged();
	void rotationUserChanged();
	void scaleUserChanged();
public:
	using PG_ActorNode::PG_ActorNode;

	PG_Object* getObjectModel() const;
	PG_ListModelLocation getLocation() const { return m_location; }
	void setLocation(PG_ListModelLocation loc);
	void update(const PG_ListModelLocation& location, const PG_IdentitySet& identities);
	PG_DisaModelNode* getModelNode() const;


	glm::vec3 getTranslationUser() const;
	void setTranslationUser(const glm::vec3& translation);

	glm::quat getRotationUser() const;
	void setRotationUser(const glm::quat& rotation);

	glm::vec3 getScaleUser() const;
	void setScaleUser(const glm::vec3& scale);
private:
	void loadGeometry();
	void updateTransformation();

	PG_ListModelLocation m_location;
};

