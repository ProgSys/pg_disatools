#pragma once

#include <Scene/Model/PG_ListSelectionInteractableModel.h>
#include <Disa/fileformats/PG_MPD.h>

/**
* @brief Anything a slot of the map can hold, whatever it is drawn as.
* @details Only what every kind of item shares: where it stands, which way it looks, whether it is
* drawn and what pulls it down. A sprite adds the character behind it, see PG_DisaSpriteModel, and
* a mesh adds the resource it loads, so that one list can hold both and one view can follow either.
*/
class PG_DisaActorModel : public PG_Object {
	PG_OBJECT(PG_DisaActorModel);
	PG_PARENT(PG_Object);

	PG_PROPERTY(char characterFacing GET getCharacterFacing SET setCharacterFacing CHANGED characterFacingChanged);
	PG_PROPERTY(glm::vec3 characterTranslation GET getObjectTranslation SET setObjectTranslation CHANGED translationChanged);
	PG_PROPERTY(bool characterVisible GET isVisible SET setVisible CHANGED visibleChanged);
	PG_PROPERTY(glm::vec4 characterColorMult GET getColorMult SET setColorMult CHANGED colorMultChanged);
	PG_PROPERTY(float characterGravity GET getGravity SET setGravity CHANGED gravityChanged);
signals:
	void translationChanged();
	void characterFacingChanged();
	void rotationChanged();
	void visibleChanged();
	void colorMultChanged();
	void gravityChanged();
public:
	using PG_Object::PG_Object;
	~PG_DisaActorModel() override = default;

	enum ECustomIdentity : Identity {
		Identity_Translation = PG_Identity_Custom,
		Identity_TileGround,
		Identity_Facing,
		Identity_Rotation,
		Identity_Visible,
		Identity_ColorMult,
		Identity_Gravity,
		Identity_FloorHeight,

		Identity_ActorLast_ /// Where a deriving model starts its own, never an identity itself
	};

	/// Tiles per tick squared, the pull the game drops an actor with. 120 script units are a tile.
	static constexpr float defaultGravity = 0.1633f / 120.f;

	void init();

	int getLocation() const { return m_location; }
	void setLocation(int index) { m_location = index; updateHeight(); }

	/// Where the actor really stands, in tile units, y is the height.
	const glm::vec3& getObjectTranslation() const { return m_translation; }
	/**
	* @brief Moves the actor, and the tile it is counted as standing on with it.
	* @details The map stores a whole tile, so moving an actor rounds that out of the position and
	* takes the ground height of the new tile. A script may leave it in the air, the height is its
	* own.
	*/
	void setObjectTranslation(const glm::vec3& value);

	glm::ivec2 getTilePosition() const;
	/// Puts the actor onto a tile, which sets the position to the top of it. @see setObjectTranslation
	void setTilePosition(glm::ivec2 pos);

	/// The quarter turn the map stores, which is the rotation snapped. @see getRotation
	EDisaFacing getCharacterFacing() const;
	/// Turns the actor onto a quarter, which sets the rotation with it.
	void setCharacterFacing(EDisaFacing value);
	void setCharacterFacing(char value) { setCharacterFacing(static_cast<EDisaFacing>(value)); }

	/**
	* @brief Which way the actor looks, in degree around y.
	* North +X Axis: -90 degree
	* East +Z Axis: 0/360 degree
	* South -X Axis: 90 degree
	* West -Z Axis: 180 degree
	*/
	float getRotation() const { return m_rotation; }
	/// Turns the actor, the facing follows it snapped to the nearest quarter. @see getRotation
	void setRotation(float value);

	/// Whether the actor is drawn at all. A script hides one it only keeps as a position.
	bool isVisible() const { return m_visible; }
	void setVisible(bool value);

	/// Top of the tile the actor stands on, which is what it falls onto.
	float getFloorHeight() const { return m_height; }

	/// How fast the actor falls, in tiles per tick squared. Zero leaves it where it is.
	float getGravity() const { return m_gravity; }
	void setGravity(float value);

	/**
	* @brief Colour the sprite is multiplied with, 1 leaves it as it is.
	* @details What SetMultRGBA writes, as a fraction rather than the percent the script gives.
	*/
	const glm::vec4& getColorMult() const { return m_colorMult; }
	void setColorMult(const glm::vec4& value);

	/**
	* @brief Will remove this model from list
	*/
	void remove();

	/* Virtual because the list holds this base: without it a deriving model's identities would be
	 * invisible to every view that reads through the list. */
	virtual PG_Variety getData(Identity identity) const;
	virtual bool setData(const PG_Variety& value, Identity identity);
protected:
	/// Drops the placement onto the model without telling anyone, for a load. @see setObjectTranslation
	void resetPlacement(glm::ivec2 tile, float rotation, bool visible);

	float calcHeight();
	void updateHeight();

	int m_location = -1;
	float m_height = 0.f;
	float m_rotation = 0.f;
	glm::vec3 m_translation = glm::vec3(0.f);
	glm::vec4 m_colorMult = glm::vec4(1.f);
	float m_gravity = defaultGravity;
	bool m_visible = true;
};
