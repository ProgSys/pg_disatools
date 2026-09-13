#pragma once
#include <array>
#include <string>

#include <Scene/PG_ActorNode.h>
#include <Scene/Model/PG_ListModelLocation.h>
#include <Scene/Model/PG_ListModelIdentity.h>

class PG_DisaMapModel;
class PG_DisaMapTileListModel;
struct PG_DisaActorScriptOverride;

class SwaySpring {
private:
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 targetPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 velocity = glm::vec3(0.0f);
    float springConstant = 0.002f;
    float dampingFactor = 0.01f;

public:
	void copy(const SwaySpring& other) {
		position = other.position;
		velocity = other.velocity;
	}

    void applyForce(const glm::vec3& force) {
        glm::vec3 acceleration = force / 1.0f; // Assuming unit mass
        velocity += acceleration;
    }

	void setTargetPos(const glm::vec3& _targetPosition) {
		targetPosition = _targetPosition;
	}

    void update(float deltaTime) {
		if (deltaTime <= 0.f) return;
		glm::vec3 displacement = targetPosition - position;
		glm::vec3 springForce = springConstant * displacement;
		glm::vec3 dampingForce = dampingFactor * velocity;

		glm::vec3 totalForce = springForce - dampingForce;

		velocity += totalForce * deltaTime;
		position += velocity * deltaTime;
		if (position.y < 0.f) {
			position.y = 0.f;
			velocity.y = 0.f;
		}
    }

    glm::vec3 getPosition() const {
        return position;
    }
};

/**
* @brief Visual representation of disa map tile
* Needs a PG_DisaMapModel.
* @see PG_DisaMapModel
*/
class PG_DisaActorViewNode : public PG_ActorNode {
	PG_OBJECT(PG_DisaActorViewNode);
	PG_PARENT(PG_ActorNode);

	PG_PROPERTY(PG_Object* model GET getObjectModel);
	PG_PROPERTY(glm::vec3 translationUser GET getTranslationUser SET setTranslationUser CHANGED translationUserChanged);
	PG_PROPERTY(unsigned short characterId GET getCharacterId SET setCharacterId CHANGED characterIdChanged);
signals:
	void translationUserChanged();
	void characterIdChanged();
public:
	using PG_ActorNode::PG_ActorNode;

	PG_Object* getObjectModel() const;
	PG_ListModelLocation getLocation() const { return m_location; }
	void setLocation(PG_ListModelLocation loc);
	void update(const PG_ListModelLocation& location, const PG_IdentitySet& identities);


	glm::vec3 getTranslationUser() const;
	void setTranslationUser(const glm::vec3& translation);

	unsigned short getCharacterId() const;
	void setCharacterId(unsigned short id);
	/**
	* @brief Will play a funny animation when dragged
	*/
	bool isDragged() const;
	void setDragged(bool value);

	bool event(PG_Event* e) override;

	const SwaySpring& getSpring() const { return m_spring; }
	void setSpring(const SwaySpring& value) { m_spring.copy(value); }

	void updateActor();

	static glm::vec3 getRotationDir(PG_Scene* scene, const glm::vec3& originWorld);

private:
	void loadSprite() { loadSprite(getCharacterId()); }
	void loadSprite(unsigned short id);
	void updateTransformation();
	void updateGroundShadow();
	void updateColorOffset();

	/// The script override of the model, or an empty one when there is no model or no override.
	const PG_DisaActorScriptOverride& getScriptOverride() const;

	PG_ListModelLocation m_location;
	bool m_dragged = false;
    SwaySpring m_spring;
};

