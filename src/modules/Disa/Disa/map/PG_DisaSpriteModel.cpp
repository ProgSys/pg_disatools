#include "PG_DisaSpriteModel.h"

#include <Disa/PG_DisaCharactersModel.h>
#include <Disa/modifications/PG_DisaCustomModificationsModel.h>
#include <Disa/modifications/PG_DisaModCharactersModel.h>

namespace {

void notifyParent(PG_DisaSpriteModel* self, const PG_IdentitySet& identities) {
	if (auto* base = self->getParent<PG_ListModelBase>(); base && self->getLocation() >= 0) {
		emit base->itemsChanged(self->getLocation(), self->getLocation(), identities);
	}
}

} //namespace

PG_DisaIdentifier PG_DisaSpriteModel::getCharacterIdentifier() const {
	auto* model = PG_DisaCharactersModel::get();
	if (!model || !model->getModificationsModel()) return PG_DisaIdentifier();
	return model->getModificationsModel()->getCharactersModel()->getCharacterIdentifier(getCharacterId());
}

void PG_DisaSpriteModel::setCharacterId(unsigned short id) {
	if (getCharacterId() == id) return;
	m_data.id = id;

	emit characterIdChanged();
	notifyParent(this, { Identity_Id });
}

void PG_DisaSpriteModel::setCharacterLevel(unsigned short level) {
	if (getCharacterLevel() == level) return;
	m_data.level = level;

	emit characterLevelChanged();
	notifyParent(this, { Identity_Level });
}

void PG_DisaSpriteModel::setCharacterAlliance(unsigned char value) {
	if (getCharacterAlliance() == value) return;
	m_data.alliance = value;

	emit characterAllianceChanged();
	notifyParent(this, { Identity_Alliance });
}

void PG_DisaSpriteModel::setCharacterAi(unsigned char value) {
	if (getCharacterAi() == value) return;
	m_data.ai = value;

	emit characterAiChanged();
	notifyParent(this, { Identity_Ai });
}

void PG_DisaSpriteModel::setCharacterWeapon(unsigned short value) {
	if (getCharacterWeapon() == value) return;
	m_data.items[0] = value;

	emit characterWeaponChanged();
	notifyParent(this, { Identity_Weapon });
}

void PG_DisaSpriteModel::setCharacterItemSlot0(unsigned short value) {
	if (getCharacterItemSlot0() == value) return;
	m_data.items[1] = value;

	emit characterItemSlot0Changed();
	notifyParent(this, { Identity_ItemSlot0 });
}

void PG_DisaSpriteModel::setCharacterItemSlot1(unsigned short value) {
	if (getCharacterItemSlot1() == value) return;
	m_data.items[2] = value;

	emit characterItemSlot1Changed();
	notifyParent(this, { Identity_ItemSlot1 });
}

void PG_DisaSpriteModel::setCharacterItemSlot2(unsigned short value) {
	if (getCharacterItemSlot2() == value) return;
	m_data.items[3] = value;

	emit characterItemSlot2Changed();
	notifyParent(this, { Identity_ItemSlot2 });
}

void PG_DisaSpriteModel::setCharacterMagicSlot0(unsigned short value) {
	if (getCharacterMagicSlot0() == value) return;
	m_data.magic[0] = value;

	emit characterMagicSlot0Changed();
	notifyParent(this, { Identity_MagicSlot0 });
}

void PG_DisaSpriteModel::setCharacterMagicSlot1(unsigned short value) {
	if (getCharacterMagicSlot1() == value) return;
	m_data.magic[1] = value;

	emit characterMagicSlot1Changed();
	notifyParent(this, { Identity_MagicSlot1 });
}

void PG_DisaSpriteModel::setCharacterMagicSlot2(unsigned short value) {
	if (getCharacterMagicSlot2() == value) return;
	m_data.magic[2] = value;

	emit characterMagicSlot2Changed();
	notifyParent(this, { Identity_MagicSlot2 });
}

void PG_DisaSpriteModel::setCharacterMagicSlot3(unsigned short value) {
	if (getCharacterMagicSlot3() == value) return;
	m_data.magic[3] = value;

	emit characterMagicSlot3Changed();
	notifyParent(this, { Identity_MagicSlot3 });
}

void PG_DisaSpriteModel::setCharacterMagicLevels(glm::u8vec4 levels) {
	if (getCharacterMagicLevels() == levels) return;
	std::memcpy(&m_data.magic_level[0], &levels.x, sizeof(unsigned char) * 4);

	emit characterMagicLevelsChanged();
	notifyParent(this, { Identity_MagicLevels });
}

void PG_DisaSpriteModel::updateStatMods() {
	//reset stat mods
	if (m_data.geo_effect != 0) {
		m_data.unk[0] = 1;
		m_data.unk[1] = 1;
		m_data.unk[2] = 1;
		m_data.unk[3] = 1;
	}
	else {
		m_data.unk[0] = 64;
		m_data.unk[1] = 32;
		m_data.unk[2] = 96;
		m_data.unk[3] = 96;
	}
}

void PG_DisaSpriteModel::setGeoColor(EDisaGeoColor color) {
	if (getGeoColor() == color) return;

	m_data.geo_effect -= m_data.geo_effect % 10;
	m_data.geo_effect += glm::clamp<unsigned short>(color, DisaGeoColor_None, DisaGeoColor_White);
	updateStatMods();
	setCharacterAlliance(2);

	emit geoColorChanged();
	notifyParent(this, { Identity_GeoColor });
}

void PG_DisaSpriteModel::setGeoEffect(unsigned short value) {
	if (getGeoEffect() == value) return;

	unsigned short color = m_data.geo_effect % 10;
	m_data.geo_effect = value * 10 + color;
	updateStatMods();
	setCharacterAlliance(2);

	emit geoEffectChanged();
	notifyParent(this, { Identity_GeoEffect });
}

void PG_DisaSpriteModel::resetGeo() {
	if (m_data.geo_effect == 0) return;
	m_data.geo_effect = 0;
	updateStatMods();

	emit geoColorChanged();
	emit geoEffectChanged();
	notifyParent(this, { Identity_GeoColor, Identity_GeoEffect });
}

void PG_DisaSpriteModel::fixStats() {
	if (m_data.geo_effect != 0) {
		m_data.unk[0] = 1;
		m_data.unk[1] = 1;
		m_data.unk[2] = 1;
		m_data.unk[3] = 1;
		m_data.alliance = 2;
	}
}

void PG_DisaSpriteModel::nextAnimationSegment() {
	notifyParent(this, { Identity_Next });
}

void PG_DisaSpriteModel::unpauseAnimation() {
	notifyParent(this, { Identity_Unpause });
}

void PG_DisaSpriteModel::setMode(EMode value) {
	if (m_mode == value) return;
	m_mode = value;
	emit animationModeChanged();
	notifyParent(this, { Identity_AnimationMode });
}

void PG_DisaSpriteModel::setAnimationSet(EAnimationSet value) {
	if (m_animationSet == value) return;
	m_animationSet = value;
	emit animationSetChanged();
	notifyParent(this, { Identity_AnimationSet });
}

void PG_DisaSpriteModel::setAnimation(int value) {
	if (m_animation == value) return;
	m_animation = value;
	emit animationChanged();
	notifyParent(this, { Identity_Animation });
}

const std::string& PG_DisaSpriteModel::getCharacterName() const {
	const auto& character = PG_DisaCharactersModel::get()->getCharacterModel(getCharacterId());
	return character.nameNormelized;
}

const std::string& PG_DisaSpriteModel::getCharacterTitle() const {
	const auto& character = PG_DisaCharactersModel::get()->getCharacterModel(getCharacterId());
	return character.titleNormelized;
}

PG_Texture PG_DisaSpriteModel::getCharacterPortrait() const {
	return PG_DisaCharactersModel::get()->getPortrait(getCharacterId());
}

void PG_DisaSpriteModel::updateFromData() {
	/* mpdfileActor::visible is not a draw flag, the game compares it against an effect index, so
	 * the model keeps its own and the byte is only carried through to the file. */
	resetPlacement(glm::ivec2(-m_data.x, m_data.z), static_cast<float>(m_data.facing) * 90.f, true);
}

mpdfileActor PG_DisaSpriteModel::getData() const {
	mpdfileActor out = m_data;

	//the placement lives on the base while the map is open, this is where it goes back into the file
	const glm::ivec2 tile = getTilePosition();
	out.x = static_cast<char>(-tile.x);
	out.z = static_cast<char>(tile.y);
	out.facing = getCharacterFacing();
	return out;
}

PG_Variety PG_DisaSpriteModel::getData(Identity identity) const {
	switch (identity) {
	case Identity_Id: return m_data.id;
	case Identity_Alliance: return getCharacterAlliance();
	case Identity_GeoColor: return getGeoColor();
	case Identity_GeoEffect: return getGeoEffect();
	case Identity_AnimationMode: return static_cast<int>(m_mode);
	case Identity_AnimationSet: return static_cast<int>(m_animationSet);
	case Identity_Animation: return m_animation;
	}
	return PG_DisaActorModel::getData(identity);
}

bool PG_DisaSpriteModel::setData(const PG_Variety& value, Identity identity) {
	switch (identity) {
	case Identity_Id: {
		if (auto id = value.convert<unsigned short>(); id != m_data.id) {
			setCharacterId(id);
			return true;
		}
	}break;
	case Identity_Alliance: {
		if (auto v = value.convert<unsigned char>(); v != m_data.alliance) {
			setCharacterAlliance(v);
			return true;
		}
	}break;
	case Identity_AnimationMode: {
		if (auto v = static_cast<EMode>(value.convert<unsigned char>()); v != m_mode) {
			setMode(v);
			return true;
		} else if (auto v = value.get<EMode>(); v != m_mode) {
			setMode(v);
			return true;
		}
	}break;
	case Identity_AnimationSet: {
		if (auto v = static_cast<EAnimationSet>(value.convert<unsigned char>()); v != m_animationSet) {
			setAnimationSet(v);
			return true;
		} else if (auto v = value.get<EAnimationSet>(); v != m_animationSet) {
			setAnimationSet(v);
			return true;
		}
	}break;
	case Identity_Animation: {
		if (auto v = value.convert<int>(); v != m_animation) {
			setAnimation(v);
			return true;
		}
	}break;
	}
	return PG_DisaActorModel::setData(value, identity);
}
