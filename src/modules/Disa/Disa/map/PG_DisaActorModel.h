#pragma once

#include <Scene/Model/PG_ListSelectionInteractableModel.h>
#include <Disa/fileformats/PG_MPD.h>
#include <Disa/PG_DisaColorEnum.h>
#include <Disa/PG_DisaIdentifier.h>
#include <resources/PG_Texture.h>

class PG_DisaActorModel : public PG_Object {
	PG_OBJECT(PG_DisaActorModel);
	PG_PARENT(PG_Object);

	PG_PROPERTY(unsigned short characterId GET getCharacterId SET setCharacterId CHANGED characterIdChanged);
	PG_PROPERTY(std::string characterName GET getCharacterName CHANGED characterIdChanged);
	PG_PROPERTY(std::string characterTitle GET getCharacterTitle CHANGED characterIdChanged);
	PG_PROPERTY(PG_Texture characterPortrait GET getCharacterPortrait CHANGED characterIdChanged);
	PG_PROPERTY(unsigned short characterLevel GET getCharacterLevel SET setCharacterLevel CHANGED characterLevelChanged);

	PG_PROPERTY(char characterRotation GET getCharacterRotation SET setCharacterRotation CHANGED characterRotationChanged);
	PG_PROPERTY(unsigned char characterAlliance GET getCharacterAlliance SET setCharacterAlliance CHANGED characterAllianceChanged);
	PG_PROPERTY(unsigned char characterAi GET getCharacterAi SET setCharacterAi CHANGED characterAiChanged);
	PG_PROPERTY(unsigned short characterWeapon GET getCharacterWeapon SET setCharacterWeapon CHANGED characterWeaponChanged);
	PG_PROPERTY(unsigned short characterItemSlot0 GET getCharacterItemSlot0 SET setCharacterItemSlot0 CHANGED characterItemSlot0Changed);
	PG_PROPERTY(unsigned short characterItemSlot1 GET getCharacterItemSlot1 SET setCharacterItemSlot1 CHANGED characterItemSlot1Changed);
	PG_PROPERTY(unsigned short characterItemSlot2 GET getCharacterItemSlot2 SET setCharacterItemSlot2 CHANGED characterItemSlot2Changed);

	//only for geo constructs (2800)
	PG_PROPERTY(int characterGeoColor GET getGeoColor SET setGeoColor CHANGED geoColorChanged)
	PG_PROPERTY(unsigned short characterGeoEffect GET getGeoEffect SET setGeoEffect CHANGED geoEffectChanged)
signals:
	void translationChanged();
	void characterIdChanged();
	void characterLevelChanged();
	void characterRotationChanged();
	void characterAiChanged();
	void characterAllianceChanged();
	void characterWeaponChanged();
	void characterItemSlot0Changed();
	void characterItemSlot1Changed();
	void characterItemSlot2Changed();
	void geoColorChanged();
	void geoEffectChanged();
public:
	using PG_Object::PG_Object;

	enum ECustomIdentity : Identity {
		Identity_Translation = Identity_Custom,
		Identity_TileGround,
		Identity_Id,
		Identity_Rotation,
		Identity_Level,
		Identity_Alliance,
		Identity_Ai,
		Identity_Weapon,
		Identity_ItemSlot0,
		Identity_ItemSlot1,
		Identity_ItemSlot2,
		Identity_GeoColor,
		Identity_GeoEffect,
	};

	void init();

	int getLocation() const { return m_location; }
	void setLocation(int index) { m_location = index; updateHeight(); }

	glm::vec3 getObjectTranslation() const;

	glm::ivec2 getTilePosition() const;
	void setTilePosition(glm::ivec2 pos);

	PG_DisaIdentifier getCharacterIdentifier() const;

	unsigned short getCharacterId() const { return m_data.id; }
	void setCharacterId(unsigned short id);

	const std::string& getCharacterName() const;
	const std::string& getCharacterTitle() const;
	PG_Texture getCharacterPortrait() const;

	unsigned short getCharacterLevel() const { return m_data.level; }
	void setCharacterLevel(unsigned short level);

	char getCharacterRotation() const { return m_data.rotation; }
	void setCharacterRotation(char value);

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

	const mpdfileActor& getData() const { return m_data; }
	void setData(const mpdfileActor& data) { m_data = data; }
	void setData(mpdfileActor&& data) { m_data = std::move(data); }

	PG_Variety getData(Identity identity) const;
	bool setData(const PG_Variety& value, Identity identity);
private:
	float calcHeight();
	void updateHeight();
	void updateStatMods();

	int m_location = -1;
	float m_height = 0.f;
	mpdfileActor m_data;
};

class PG_DisaActorListModel : public PG_ListModel<PG_DisaActorModel*> {
	PG_OBJECT(PG_DisaActorListModel);
	PG_PARENT(PG_ListModel<PG_DisaActorModel*>);
public:
	using PG_ListModel<PG_DisaActorModel*>::PG_ListModel;
};