#pragma once

#include <optional>
#include <string>

#include <Scene/Model/PG_ListSelectionInteractableModel.h>
#include <Disa/fileformats/PG_MPD.h>
#include <Disa/PG_DisaColorEnum.h>
#include <Disa/PG_DisaIdentifier.h>
#include <resources/PG_Texture.h>

/**
* @brief What a script takes over on an actor, each part only for as long as it is set.
* @details Everything a script sets that the actor has no place for of its own lives here. The
* things it shares with a placed actor, the position, the facing, the animation, are members of
* the model instead.
*/
struct PG_DisaActorScriptOverride {
	/// How fast it falls, in tiles per tick squared. Unset means the engine's own.
	std::optional<float> gravity;

	inline bool isEmpty() const { return !gravity; }
};

class PG_DisaActorModel : public PG_Object {
	PG_OBJECT(PG_DisaActorModel);
	PG_PARENT(PG_Object);

	PG_PROPERTY(unsigned short characterId GET getCharacterId SET setCharacterId CHANGED characterIdChanged);
	PG_PROPERTY(std::string characterName GET getCharacterName CHANGED characterIdChanged);
	PG_PROPERTY(std::string characterTitle GET getCharacterTitle CHANGED characterIdChanged);
	PG_PROPERTY(PG_Texture characterPortrait GET getCharacterPortrait CHANGED characterIdChanged);
	PG_PROPERTY(unsigned short characterLevel GET getCharacterLevel SET setCharacterLevel CHANGED characterLevelChanged);

	PG_PROPERTY(char characterFacing GET getCharacterFacing SET setCharacterFacing CHANGED characterFacingChanged);
	PG_PROPERTY(unsigned char characterAlliance GET getCharacterAlliance SET setCharacterAlliance CHANGED characterAllianceChanged);
	PG_PROPERTY(unsigned char characterAi GET getCharacterAi SET setCharacterAi CHANGED characterAiChanged);
	PG_PROPERTY(unsigned short characterWeapon GET getCharacterWeapon SET setCharacterWeapon CHANGED characterWeaponChanged);
	PG_PROPERTY(unsigned short characterItemSlot0 GET getCharacterItemSlot0 SET setCharacterItemSlot0 CHANGED characterItemSlot0Changed);
	PG_PROPERTY(unsigned short characterItemSlot1 GET getCharacterItemSlot1 SET setCharacterItemSlot1 CHANGED characterItemSlot1Changed);
	PG_PROPERTY(unsigned short characterItemSlot2 GET getCharacterItemSlot2 SET setCharacterItemSlot2 CHANGED characterItemSlot2Changed);

	PG_PROPERTY(unsigned short characterMagicSlot0 GET getCharacterMagicSlot0 SET setCharacterMagicSlot0 CHANGED characterMagicSlot0Changed);
	PG_PROPERTY(unsigned short characterMagicSlot1 GET getCharacterMagicSlot1 SET setCharacterMagicSlot1 CHANGED characterMagicSlot1Changed);
	PG_PROPERTY(unsigned short characterMagicSlot2 GET getCharacterMagicSlot2 SET setCharacterMagicSlot2 CHANGED characterMagicSlot2Changed);
	PG_PROPERTY(unsigned short characterMagicSlot3 GET getCharacterMagicSlot3 SET setCharacterMagicSlot3 CHANGED characterMagicSlot3Changed);

	PG_PROPERTY(glm::u8vec4 characterMagicLevels GET getCharacterMagicLevels SET setCharacterMagicLevels CHANGED characterMagicLevelsChanged);

	//only for geo constructs (2800)
	PG_PROPERTY(int characterGeoColor GET getGeoColor SET setGeoColor CHANGED geoColorChanged)
	PG_PROPERTY(unsigned short characterGeoEffect GET getGeoEffect SET setGeoEffect CHANGED geoEffectChanged)

	PG_PROPERTY(glm::vec3 characterTranslation GET getObjectTranslation SET setObjectTranslation CHANGED translationChanged);
	PG_PROPERTY(bool characterVisible GET isVisible SET setVisible CHANGED visibleChanged);
	PG_PROPERTY(glm::vec4 characterColorMult GET getColorMult SET setColorMult CHANGED colorMultChanged);

	PG_PROPERTY(int animationMode GET getMode SET setMode CHANGED animationModeChanged);
	PG_PROPERTY(int animationSet GET getAnimationSet SET setAnimationSet CHANGED animationSetChanged);
	PG_PROPERTY(int animation GET getAnimation SET setAnimation CHANGED animationChanged);
signals:
	void translationChanged();
	void characterIdChanged();
	void characterLevelChanged();
	void characterFacingChanged();
	void characterAiChanged();
	void characterAllianceChanged();
	void characterWeaponChanged();
	void characterItemSlot0Changed();
	void characterItemSlot1Changed();
	void characterItemSlot2Changed();

	void characterMagicSlot0Changed();
	void characterMagicSlot1Changed();
	void characterMagicSlot2Changed();
	void characterMagicSlot3Changed();
	void characterMagicLevelsChanged();

	void geoColorChanged();
	void geoEffectChanged();
	void rotationChanged();
	void visibleChanged();
	void colorMultChanged();
	void animationModeChanged();
	void animationSetChanged();
	void animationChanged();
public:
	using PG_Object::PG_Object;

	enum ECustomIdentity : Identity {
		Identity_Translation = PG_Identity_Custom,
		Identity_TileGround,
		Identity_Id,
		Identity_Facing,
		Identity_Level,
		Identity_Alliance,
		Identity_Ai,
		Identity_Weapon,
		Identity_ItemSlot0,
		Identity_ItemSlot1,
		Identity_ItemSlot2,

		Identity_MagicSlot0,
		Identity_MagicSlot1,
		Identity_MagicSlot2,
		Identity_MagicSlot3,
		Identity_MagicLevels,

		Identity_GeoColor,
		Identity_GeoEffect,
		Identity_ScriptOverride,
		Identity_Unpause,
		Identity_Next, //will force play next animation segment
		Identity_Rotation,
		Identity_AnimationMode,
		Identity_AnimationSet,
		Identity_Animation,
		Identity_Visible,
		Identity_ColorMult,
	};

	/**
	* @brief Which numbering and how many side views the actor's animations are picked from.
	* @details A sprite always faces the camera, what changes is which of its side views is shown.
	* In battle only two exist and the turn is covered by mirroring them, a cutscene actor has five.
	* The game calls this the animation mode, 1 and 2 of SetAnimationMode.
	*/
	enum EMode : unsigned char {
		Mode_None,    /// Only the animation a pose named, the angle has no say
		Mode_Combat,  /// Two side views and the 1000 numbering
		Mode_Cutscene /// Five side views and the cutscene numbering
	};

	/**
	* @brief What the actor is doing, which is a row of the animation table and not a sheet number.
	* @details Both modes hold twelve of these and only the first three differ between them. What a
	* sprite really owns varies, a monster has entries a humanoid does not and the other way around.
	*/
	enum EAnimationSet : unsigned char {
		IdleStanding,
		Walking,
		FastWalking,      /// In combat this is the frozen frame of an attack in mid air instead
		Jump,
		PunchAttack,
		Hurt,
		VictoryPose,
		Skill,            /// Monster only, many sprites do not have it
		Throw,
		VictoryJump,      /// Usually continues with VictoryPose
		PunchFreezeFrame, /// Monster only
		SkillAlt          /// Monster only, many sprites do not have it
	};

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

	/// Whether the actor is drawn at all. A script hides one it only keeps as a position.
	bool isVisible() const { return m_visible; }
	void setVisible(bool value);

	/**
	* @brief Colour the sprite is multiplied with, 1 leaves it as it is.
	* @details What SetMultRGBA writes, as a fraction rather than the percent the script gives.
	* The sprite has no alpha of its own yet, so a fully clear actor is simply not drawn.
	*/
	const glm::vec4& getColorMult() const { return m_colorMult; }
	void setColorMult(const glm::vec4& value);

	PG_DisaIdentifier getCharacterIdentifier() const;

	unsigned short getCharacterId() const { return m_data.id; }
	void setCharacterId(unsigned short id);

	const std::string& getCharacterName() const;
	const std::string& getCharacterTitle() const;
	PG_Texture getCharacterPortrait() const;

	unsigned short getCharacterLevel() const { return m_data.level; }
	void setCharacterLevel(unsigned short level);

	/// The quarter turn the map stores, which is the rotation snapped. @see getRotation
	EDisaFacing getCharacterFacing() const { return m_data.facing; }
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

	unsigned char getCharacterAlliance() const { return m_data.alliance; }
	void setCharacterAlliance(unsigned char value);

	unsigned char getCharacterAi() const { return m_data.ai; }
	void setCharacterAi(unsigned char value);

	unsigned short getCharacterWeapon() const { return m_data.items[0]; }
	void setCharacterWeapon(unsigned short value);

	unsigned short getCharacterItemSlot0() const { return m_data.items[1]; }
	void setCharacterItemSlot0(unsigned short value);

	unsigned short getCharacterItemSlot1() const { return m_data.items[2]; }
	void setCharacterItemSlot1(unsigned short value);

	unsigned short getCharacterItemSlot2() const { return m_data.items[3]; }
	void setCharacterItemSlot2(unsigned short value);

	unsigned short getCharacterMagicSlot0() const { return m_data.magic[0]; }
	void setCharacterMagicSlot0(unsigned short value);

	unsigned short getCharacterMagicSlot1() const { return m_data.magic[1]; }
	void setCharacterMagicSlot1(unsigned short value);

	unsigned short getCharacterMagicSlot2() const { return m_data.magic[2]; }
	void setCharacterMagicSlot2(unsigned short value);

	unsigned short getCharacterMagicSlot3() const { return m_data.magic[3]; }
	void setCharacterMagicSlot3(unsigned short value);

	glm::u8vec4 getCharacterMagicLevels() const { return { m_data.magic_level[0], m_data.magic_level[1], m_data.magic_level[2], m_data.magic_level[3] }; };
	void setCharacterMagicLevels(glm::u8vec4 levels);

	/**
	* Only for geo constructs (2800)
	*/
	bool hasGeoColor() const { return static_cast<bool>(m_data.geo_effect % 10); }
	EDisaGeoColor getGeoColor() const { return static_cast<EDisaGeoColor>(m_data.geo_effect % 10); }
	void setGeoColor(EDisaGeoColor color);
	void setGeoColor(int color) { setGeoColor(static_cast<EDisaGeoColor>(color)); }

	unsigned short getGeoEffect() const { return static_cast<unsigned short>(m_data.geo_effect/10); }
	void setGeoEffect(unsigned short value);
	/**
	* @brief Will set all geo effets to zero
	*/
	void resetGeo();
	void fixStats();

	/**
	* @brief Will remove this model from list
	*/
	void remove();
	/**
	* @brief will play next animation segemnt
	*/
	void nextAnimationSegment();
	void unpauseAnimation();

	EMode getMode() const { return m_mode; }
	void setMode(EMode value);
	void setMode(int value) { setMode(static_cast<EMode>(value)); }

	EAnimationSet getAnimationSet() const { return m_animationSet; }
	void setAnimationSet(EAnimationSet value);
	void setAnimationSet(int value) { setAnimationSet(static_cast<EAnimationSet>(value)); }

	/**
	* @brief Sprite animation to play instead of the one the set and the angle would give.
	* @details This is what SetActorPose names, a sheet number spelled out. The actor then stops
	* following the camera, a negative number is the same sheet mirrored.
	* @returns 0 while the set picks the animation, which is the normal case.
	*/
	int getAnimation() const { return m_animation; }
	void setAnimation(int value);
	/// Hands the choice back to the animation set. @see setAnimation
	void clearAnimation() { setAnimation(0); }

	/**
	* @brief What a script has taken over on this actor, see PG_DisaActorScriptOverride.
	* @details Set through setScriptOverride, which tells the view about it. An actor nothing has
	* touched has an empty one and behaves like any placed actor.
	*/
	const PG_DisaActorScriptOverride& getScriptOverride() const { return m_scriptOverride; }
	PG_DisaActorScriptOverride& getScriptOverride() { return m_scriptOverride; }
	void setScriptOverride(const PG_DisaActorScriptOverride& value);
	/// Drops every override at once, the actor falls back to its map data.
	void clearScriptOverride();

	const mpdfileActor& getData() const { return m_data; }
	void setData(const mpdfileActor& data) { m_data = data; updateFromData(); }
	void setData(mpdfileActor&& data) { m_data = std::move(data); updateFromData(); }

	PG_Variety getData(Identity identity) const;
	bool setData(const PG_Variety& value, Identity identity);
private:
	float calcHeight();
	void updateHeight();
	/// Puts the rotation back onto the facing.
	void updateRotation();
	/// Puts the position and the rotation back onto the file data, when that is swapped underneath.
	void updateFromData();
	void updateStatMods();

	int m_location = -1;
	float m_height = 0.f;
	float m_rotation = 0.f;
	glm::vec3 m_translation = glm::vec3(0.f);
	glm::vec4 m_colorMult = glm::vec4(1.f);
	bool m_visible = true;
	mpdfileActor m_data;
	PG_DisaActorScriptOverride m_scriptOverride;
	EMode m_mode = Mode_Combat;
	EAnimationSet m_animationSet = IdleStanding;
	int m_animation = 0;
};

class PG_DisaActorListModel : public PG_ListModel<PG_DisaActorModel*> {
	PG_OBJECT(PG_DisaActorListModel);
	PG_PARENT(PG_ListModel<PG_DisaActorModel*>);
public:
	using PG_ListModel<PG_DisaActorModel*>::PG_ListModel;
};