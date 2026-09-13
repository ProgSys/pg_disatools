#include "PG_DisaActorModel.h"
#include <Disa/PG_DisaMapModel.h>
#include <Disa/PG_DisaCharactersModel.h>
#include <unordered_set>

void PG_DisaActorModel::init() {
	connect(this, &PG_DisaActorModel::parentChanged, this, [this] {
		if (auto* model = findParent<PG_DisaMapModel>(5)) {
			connect(model->getTiles(), &PG_DisaMapTileListModel::tilesGeometryChanged, this, [this](const std::unordered_set<glm::ivec2, PG_iVec2Hasher>& tiles) {
				if (tiles.contains(glm::ivec2(-m_data.x, m_data.z))) {
					updateHeight();
				}
			}, PG_ConnectionType_Single);
			updateHeight();
		}
	});
}

float PG_DisaActorModel::calcHeight() {
	if (auto* model = findParent<PG_DisaMapModel>(5)) {
		if (auto location = model->getTiles()->getTileLocation(glm::ivec2(-m_data.x, m_data.z))) {
			return location.getDataT<float>(PG_DisaMapTileModel::Identity_TopHeight);
		}
	}
	return 0.f;
}

void PG_DisaActorModel::updateHeight() {
	if (m_location < 0) return;

	const float newHeight = calcHeight();
	PG_IdentitySet changes;
	changes.insert(Identity_TileGround);
	if (newHeight != m_height) {
		m_height = newHeight;
		changes.insert(Identity_Translation);
		emit translationChanged();
	}

	emit getParent<PG_DisaActorListModel>()->itemsChanged(m_location, m_location, changes);
}

glm::vec3 PG_DisaActorModel::getObjectTranslation() const {
	return glm::vec3(-m_data.x, m_height, m_data.z) ;
}

glm::ivec2 PG_DisaActorModel::getTilePosition() const {
	return glm::ivec2(-m_data.x, m_data.z);
}

static inline void notifyParent(PG_DisaActorModel* self, const PG_IdentitySet& identities) {
	if (auto* base = self->getParent<PG_ListModelBase>(); base && self->getLocation() >= 0) {
		emit base->itemsChanged(self->getLocation(), self->getLocation(), identities);
	}
}

void PG_DisaActorModel::setTilePosition(glm::ivec2 pos) {
	if (getTilePosition() == pos) return;
	m_data.x = -pos.x;
	m_data.z = pos.y;
	m_height = calcHeight();

	emit translationChanged();
	notifyParent(this, { Identity_Translation, Identity_TileGround });
}

PG_DisaIdentifier PG_DisaActorModel::getCharacterIdentifier() const {
	auto* model = PG_DisaCharactersModel::get();
	if (!model) PG_DisaIdentifier();
	return model->getCharacterIdentifier(getCharacterId());
}

void PG_DisaActorModel::setCharacterId(unsigned short id) {
	if (getCharacterId() == id) return;
	m_data.id = id;

	emit characterIdChanged();
	notifyParent(this, { Identity_Id });
}

void PG_DisaActorModel::setCharacterLevel(unsigned short level) {
	if (getCharacterLevel() == level) return;
	m_data.level = level;

	emit characterLevelChanged();
	notifyParent(this, { Identity_Level });
}

void PG_DisaActorModel::setCharacterRotation(char value) {
	if (getCharacterRotation() == value) return;
	m_data.rotation = value;

	emit characterRotationChanged();
	notifyParent(this, { Identity_Rotation });
}

void PG_DisaActorModel::setCharacterAlliance(unsigned char value) {
	if (getCharacterAlliance() == value) return;
	m_data.alliance = value;

	emit characterAllianceChanged();
	notifyParent(this, { Identity_Alliance });
}

void PG_DisaActorModel::setCharacterAi(unsigned char value) {
	if (getCharacterAi() == value) return;
	m_data.ai = value;

	emit characterAiChanged();
	notifyParent(this, { Identity_Ai });
}

void PG_DisaActorModel::setCharacterWeapon(unsigned short value) {
	if (getCharacterWeapon() == value) return;
	m_data.items[0] = value;

	emit characterWeaponChanged();
	notifyParent(this, { Identity_Weapon });
}

void PG_DisaActorModel::setCharacterItemSlot0(unsigned short value) {
	if (getCharacterItemSlot0() == value) return;
	m_data.items[1] = value;

	emit characterItemSlot0Changed();
	notifyParent(this, { Identity_ItemSlot0 });
}

void PG_DisaActorModel::setCharacterItemSlot1(unsigned short value) {
	if (getCharacterItemSlot1() == value) return;
	m_data.items[2] = value;

	emit characterItemSlot1Changed();
	notifyParent(this, { Identity_ItemSlot1 });
}

void PG_DisaActorModel::setCharacterItemSlot2(unsigned short value) {
	if (getCharacterItemSlot2() == value) return;
	m_data.items[3] = value;

	emit characterItemSlot2Changed();
	notifyParent(this, { Identity_ItemSlot2 });
}

void PG_DisaActorModel::setCharacterMagicSlot0(unsigned short value) {
	if (getCharacterMagicSlot0() == value) return;
	m_data.magic[0] = value;

	emit characterMagicSlot0Changed();
	notifyParent(this, { Identity_MagicSlot0 });
}

void PG_DisaActorModel::setCharacterMagicSlot1(unsigned short value) {
	if (getCharacterMagicSlot1() == value) return;
	m_data.magic[1] = value;

	emit characterMagicSlot1Changed();
	notifyParent(this, { Identity_MagicSlot1 });
}

void PG_DisaActorModel::setCharacterMagicSlot2(unsigned short value) {
	if (getCharacterMagicSlot2() == value) return;
	m_data.magic[2] = value;

	emit characterMagicSlot2Changed();
	notifyParent(this, { Identity_MagicSlot2 });
}

void PG_DisaActorModel::setCharacterMagicSlot3(unsigned short value) {
	if (getCharacterMagicSlot3() == value) return;
	m_data.magic[3] = value;

	emit characterMagicSlot3Changed();
	notifyParent(this, { Identity_MagicSlot3 });
}

void PG_DisaActorModel::setCharacterMagicLevels(glm::u8vec4 levels) {
	if (getCharacterMagicLevels() == levels) return;
	std::memcpy(&m_data.magic_level[0], &levels.x, sizeof(unsigned char) * 4);

	emit characterMagicLevelsChanged();
	notifyParent(this, { Identity_MagicLevels });
}

void PG_DisaActorModel::updateStatMods() {
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

void PG_DisaActorModel::setGeoColor(EDisaGeoColor color) {
	if (getGeoColor() == color) return;

	m_data.geo_effect -= m_data.geo_effect % 10;
	m_data.geo_effect += glm::clamp<unsigned short>(color, DisaGeoColor_None, DisaGeoColor_White);
	updateStatMods();
	setCharacterAlliance(2);

	emit geoColorChanged();
	notifyParent(this, { Identity_GeoColor });
}


void PG_DisaActorModel::setGeoEffect(unsigned short value) {
	if (getGeoEffect() == value) return;

	unsigned short color = m_data.geo_effect % 10;
	m_data.geo_effect = value * 10 + color;
	updateStatMods();
	setCharacterAlliance(2);

	emit geoEffectChanged();
	notifyParent(this, { Identity_GeoEffect });
}

void PG_DisaActorModel::resetGeo() {
	if (m_data.geo_effect == 0) return;
	m_data.geo_effect = 0;
	updateStatMods();

	emit geoColorChanged();
	emit geoEffectChanged();
	notifyParent(this, { Identity_GeoColor, Identity_GeoEffect });
}	

void PG_DisaActorModel::fixStats() {
	if (m_data.geo_effect != 0) {
		m_data.unk[0] = 1;
		m_data.unk[1] = 1;
		m_data.unk[2] = 1;
		m_data.unk[3] = 1;
		m_data.alliance = 2;
	}
}

void PG_DisaActorModel::remove() {
	auto list = getParent<PG_DisaActorListModel>();
	if (list) list->remove(m_location);
}

void PG_DisaActorModel::nextAnimationSegment() {
	notifyParent(this, { Identity_Next });
}

void PG_DisaActorModel::unpauseAnimation() {
	notifyParent(this, { Identity_Unpause });
}

const std::string& PG_DisaActorModel::getCharacterName() const {
	const auto& character = PG_DisaCharactersModel::get()->getCharacterModel(getCharacterId());
	return character.nameNormelized;
}

const std::string& PG_DisaActorModel::getCharacterTitle() const {
	const auto& character = PG_DisaCharactersModel::get()->getCharacterModel(getCharacterId());
	return character.titleNormelized;
}

PG_Texture PG_DisaActorModel::getCharacterPortrait() const {
	return PG_DisaCharactersModel::get()->getPortrait(getCharacterId());
}

void PG_DisaActorModel::setScriptOverride(const PG_DisaActorScriptOverride& value) {
	const bool moved = m_scriptOverride.translation != value.translation;
	m_scriptOverride = value;

	PG_IdentitySet changes{ Identity_ScriptOverride, Identity_Unpause };
	if (moved) {
		changes.insert(Identity_Translation);
		emit translationChanged();
	}
	notifyParent(this, changes);
}

void PG_DisaActorModel::clearScriptOverride() {
	setScriptOverride(PG_DisaActorScriptOverride());
}

glm::vec3 PG_DisaActorModel::getTranslation() const {
	return m_scriptOverride.translation.value_or(getObjectTranslation());
}

PG_Variety PG_DisaActorModel::getData(Identity identity) const {
	switch (identity) {
	case Identity_Translation: return getTranslation();
	case Identity_TileGround: {
		if (auto* model = findParent<PG_DisaMapModel>(5)) {
			if (auto location = model->getTiles()->getTileLocation(glm::ivec2(-m_data.x, m_data.z))) {
				auto* item = location.getDataT<PG_DisaMapTileModel*>(PG_Identity_Item);
				auto top = item->getTop(glm::vec3(0.f, item->getTranslation().y, 0.f));
				for (glm::vec3& v: top) v.y -= m_height;
				return top;
			}
		}
		return std::array<glm::vec3, 4>{glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.f)};
	} break;
	case Identity_Id: return m_data.id;
	case Identity_Rotation: return getCharacterRotation();
	case Identity_Alliance: return getCharacterAlliance();
	case Identity_GeoColor: return getGeoColor();
	case Identity_GeoEffect: return getGeoEffect();
	}
	return PG_Variety();
}

bool PG_DisaActorModel::setData(const PG_Variety& value, Identity identity) {
	switch (identity) {
	case Identity_Id: {
		if (auto id = value.convert<unsigned short>(); id != m_data.id){
			m_data.id = id;
			emit characterIdChanged();
			return true;
		}
	}break;
	case Identity_Rotation: {
		if (auto v = glm::clamp(value.convert<char>(), (char) -1, (char)3); v != m_data.rotation) {
			m_data.rotation = v;
			emit characterRotationChanged();
			return true;
		}
	}break;
	case Identity_Alliance: {
		if (auto v = value.convert<unsigned char>(); v != m_data.alliance) {
			m_data.alliance = v;
			emit characterAllianceChanged();
			return true;
		}
	}break;
	}
	return false;
}