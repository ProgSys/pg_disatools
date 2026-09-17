#pragma once
#include <algorithm>
#include <array>
#include <string>
#include <variant>

#include <Scene/PG_ActorNode.h>
#include <Scene/Model/PG_ListModelLocation.h>
#include <Scene/Model/PG_ListModelIdentity.h>

class PG_DisaMapModel;
class PG_DisaMapTileListModel;

/**
* @brief Where an actor's sprite sits relative to its model, and how fast it is going there.
* @details The state a force drives, kept apart from the force itself so that swapping one for the
* other does not throw the movement away. A sprite the sway threw upwards keeps rising for a moment
* after gravity takes over, the way it would if it were really let go.
*/
struct PG_DisaActorMotion {
	glm::vec3 offset = glm::vec3(0.f);
	glm::vec3 velocity = glm::vec3(0.f);
};

/**
* @brief Bobs a dragged sprite above where its model sits.
* @details One of the two forces a PG_DisaActorSpriteNode can carry, see PG_DisaActorForce. It knows
* nothing about the model, the swing it drives is its own around the target.
*/
class SwaySpring {
public:
	void setTargetPos(const glm::vec3& value) { m_targetPosition = value; }
	const glm::vec3& getTargetPos() const { return m_targetPosition; }

	void update(PG_DisaActorMotion& motion, float deltaTime, const glm::vec3&) const {
		if (deltaTime <= 0.f) return;

		const glm::vec3 springForce = m_springConstant * (m_targetPosition - motion.offset);
		const glm::vec3 dampingForce = m_dampingFactor * motion.velocity;
		motion.velocity += (springForce - dampingForce) * deltaTime;
		motion.offset += motion.velocity * deltaTime;

		if (motion.offset.y < 0.f) {
			motion.offset.y = 0.f;
			motion.velocity.y = 0.f;
		}
	}

private:
	glm::vec3 m_targetPosition = glm::vec3(0.f);
	float m_springConstant = 0.002f;
	float m_dampingFactor = 0.01f;
};

/**
* @brief Drops a sprite onto the floor under its model.
* @details The other force, see PG_DisaActorForce. A script may leave an actor standing in the air,
* and this is what falls the rest of the way, as an offset, so the position the script set stays
* the position the script set. A gravity of zero holds the sprite exactly where the model is.
*/
class GravityForce {
public:
	float getGravity() const { return m_gravity; }
	/// Tiles per tick squared.
	void setGravity(float value) { m_gravity = value; }

	float getFloorHeight() const { return m_floorHeight; }
	/// Height the sprite comes to rest at, in the same space as the model's position.
	void setFloorHeight(float value) { m_floorHeight = value; }

	void update(PG_DisaActorMotion& motion, float deltaTime, const glm::vec3& position) const {
		if (m_gravity <= 0.f) {
			motion.offset = glm::vec3(0.f);
			motion.velocity = glm::vec3(0.f);
			return;
		}

		//how far the floor sits under the model, so a script holding the actor up gives a negative one
		const float drop = m_floorHeight - position.y;
		motion.velocity.y -= m_gravity * deltaTime;
		motion.offset += motion.velocity * deltaTime;

		if (motion.offset.y <= drop) {
			motion.offset = glm::vec3(0.f, drop, 0.f); //landed, and an actor under the floor is put on top
			motion.velocity = glm::vec3(0.f);
		}
	}

private:
	float m_gravity = 0.f;
	float m_floorHeight = 0.f;
};

using PG_DisaActorForce = std::variant<SwaySpring, GravityForce>;

/**
* @brief Visual representation of disa map tile
* Needs a PG_DisaMapModel.
* @see PG_DisaMapModel
*/
class PG_DisaActorSpriteNode : public PG_ActorNode {
	PG_OBJECT(PG_DisaActorSpriteNode);
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

	const PG_DisaActorForce& getForce() const { return m_force; }
	void setForce(const PG_DisaActorForce& value) { m_force = value; }

	/// Handing this over with the force is what lets a sprite carry its movement to another node.
	const PG_DisaActorMotion& getMotion() const { return m_motion; }
	void setMotion(const PG_DisaActorMotion& value) { m_motion = value; }

	void updateActor();

	static glm::vec3 getRotationDir(PG_Scene* scene, const glm::vec3& originWorld);

private:
	void loadSprite() { loadSprite(getCharacterId()); }
	void loadSprite(unsigned short id);
	void updateTransformation();
	/**
	* @brief Reads one identity off the model, without throwing when the item does not answer it.
	* @details A slot may hold something that is not a sprite, and the drag indicator has no model
	* behind its location at all, so an unanswered identity has to come back as a default.
	*/
	template<typename T>
	T readData(Identity identity) const { return m_location.getData(identity).convert<T>(); }

	void updateGroundShadow();
	void updateColorOffset();
	/// Feeds the force the model it is pulling on, then lets it run for a tick.
	void updateForce(float deltaTime);
	void updateColorMult();
	/// Hides the actor when the model says so, or when its colour faded it out to nothing.
	void updateVisible();

	PG_ListModelLocation m_location;
	bool m_dragged = false;
	PG_DisaActorForce m_force = GravityForce();
	PG_DisaActorMotion m_motion;
};

