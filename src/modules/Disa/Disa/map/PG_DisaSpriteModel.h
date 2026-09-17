#pragma once

#include <string>

#include <Disa/map/PG_DisaActorModel.h>
#include <Disa/PG_DisaColorEnum.h>
#include <Disa/PG_DisaIdentifier.h>
#include <resources/PG_Texture.h>

/**
* @brief A slot item drawn as a character sprite, and the map record behind it.
* @details Everything the file stores about a unit lives here, along with what a script sets on one:
* the animation set, the pose and the mode the side views are picked in. Where it stands and which
* way it looks belong to PG_DisaActorModel, because a mesh in a slot needs those just as much.
*/
class PG_DisaSpriteModel : public PG_DisaActorModel {
	PG_OBJECT(PG_DisaSpriteModel);
	PG_PARENT(PG_DisaActorModel);

	PG_PROPERTY(unsigned short characterId GET getCharacterId SET setCharacterId CHANGED characterIdChanged);
	PG_PROPERTY(std::string characterName GET getCharacterName CHANGED characterIdChanged);
	PG_PROPERTY(std::string characterTitle GET getCharacterTitle CHANGED characterIdChanged);
	PG_PROPERTY(PG_Texture characterPortrait GET getCharacterPortrait CHANGED characterIdChanged);
	PG_PROPERTY(unsigned short characterLevel GET getCharacterLevel SET setCharacterLevel CHANGED characterLevelChanged);

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

	PG_PROPERTY(int animationMode GET getMode SET setMode CHANGED animationModeChanged);
	PG_PROPERTY(int animationSet GET getAnimationSet SET setAnimationSet CHANGED animationSetChanged);
	PG_PROPERTY(int animation GET getAnimation SET setAnimation CHANGED animationChanged);
signals:
	void characterIdChanged();
	void characterLevelChanged();
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
	void animationModeChanged();
	void animationSetChanged();
	void animationChanged();
public:
	using PG_DisaActorModel::PG_DisaActorModel;

	enum ECustomIdentity : Identity {
		Identity_Id = Identity_ActorLast_,
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
		Identity_Unpause,
		Identity_Next, //will force play next animation segment
		Identity_AnimationMode,
		Identity_AnimationSet,
		Identity_Animation,
	};

	/**
	* @brief Which numbering and how many side views the animations are picked from.
	* @details A sprite always faces the camera, what changes is which of its side views is shown.
	* In battle only two exist and the turn is covered by mirroring them, a cutscene actor has five.
	* The game calls this the animation mode, 1 and 2 of SetAnimationModeSet.
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

	PG_DisaIdentifier getCharacterIdentifier() const;

	unsigned short getCharacterId() const { return m_data.id; }
	void setCharacterId(unsigned short id);

	const std::string& getCharacterName() const;
	const std::string& getCharacterTitle() const;
	PG_Texture getCharacterPortrait() const;

	unsigned short getCharacterLevel() const { return m_data.level; }
	void setCharacterLevel(unsigned short level);

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

	unsigned short getGeoEffect() const { return static_cast<unsigned short>(m_data.geo_effect / 10); }
	void setGeoEffect(unsigned short value);
	/**
	* @brief Will set all geo effets to zero
	*/
	void resetGeo();
	void fixStats();

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
	* @brief The file record, with the placement the model owns folded back into it.
	* @details The position, the facing and the visibility live on PG_DisaActorModel while the map
	* is open, because a mesh in a slot needs them too, so they are written back here rather than
	* kept in step on every move.
	*/
	mpdfileActor getData() const;
	void setData(const mpdfileActor& data) { m_data = data; updateFromData(); }
	void setData(mpdfileActor&& data) { m_data = std::move(data); updateFromData(); }

	PG_Variety getData(Identity identity) const override;
	bool setData(const PG_Variety& value, Identity identity) override;
private:
	/// Puts the placement of the file record onto the model, when that is swapped underneath.
	void updateFromData();
	void updateStatMods();

	mpdfileActor m_data;
	EMode m_mode = Mode_Combat;
	EAnimationSet m_animationSet = IdleStanding;
	int m_animation = 0;
};
