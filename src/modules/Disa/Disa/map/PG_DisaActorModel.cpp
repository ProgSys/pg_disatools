#include "PG_DisaActorModel.h"

#include <Disa/map/PG_DisaActorListModel.h>
#include <Disa/PG_DisaMapModel.h>
#include <cmath>
#include <unordered_set>

namespace {

constexpr float degreePerQuarter = 90.f;

/// The quarter an angle rounds to, the map only stores those four.
EDisaFacing facingOf(float degree) {
	degree = std::fmod(degree + 180.f, 360.f);
	if (degree <= 0.f) degree += 360.f;

	//the wrapped angle runs over (-180, 180], so a quarter of -2 is the same west as 2
	long quarter = std::lround((degree - 180.f) / degreePerQuarter);
	if (quarter < DisaFacing_North) quarter += 4;
	return static_cast<EDisaFacing>(quarter);
}

void notifyParent(PG_DisaActorModel* self, const PG_IdentitySet& identities) {
	if (auto* base = self->getParent<PG_ListModelBase>(); base && self->getLocation() >= 0) {
		emit base->itemsChanged(self->getLocation(), self->getLocation(), identities);
	}
}

} //namespace

void PG_DisaActorModel::init() {
	connect(this, &PG_DisaActorModel::parentChanged, this, [this] {
		if (auto* model = findParent<PG_DisaMapModel>(5)) {
			connect(model->getTiles(), &PG_DisaMapTileListModel::tilesGeometryChanged, this, [this](const std::unordered_set<glm::ivec2, PG_iVec2Hasher>& tiles) {
				if (tiles.contains(getTilePosition())) {
					updateHeight();
				}
			}, PG_ConnectionType_Single);
			updateHeight();
		}
	});
}

float PG_DisaActorModel::calcHeight() {
	if (auto* model = findParent<PG_DisaMapModel>(5)) {
		if (auto location = model->getTiles()->getTileLocation(getTilePosition())) {
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
		changes.insert(Identity_FloorHeight);
		//the ground moved, so the actor moves with it and keeps whatever a script lifted it by
		m_translation.y += newHeight - m_height;
		m_height = newHeight;
		changes.insert(Identity_Translation);
		emit translationChanged();
	}

	emit getParent<PG_DisaActorListModel>()->itemsChanged(m_location, m_location, changes);
}

glm::ivec2 PG_DisaActorModel::getTilePosition() const {
	//a position between two tiles belongs to the one it stands on, so it is floored not rounded
	return glm::ivec2(static_cast<int>(std::floor(m_translation.x)), static_cast<int>(std::floor(m_translation.z)));
}

void PG_DisaActorModel::setTilePosition(glm::ivec2 pos) {
	if (getTilePosition() == pos) return;

	//the height comes off the new tile, so the move has to happen before it is asked for
	const glm::vec3 center = tileCenter(pos);
	m_translation = glm::vec3(center.x, m_translation.y, center.z);
	m_height = calcHeight();
	m_translation.y = m_height;

	emit translationChanged();
	notifyParent(this, { Identity_Translation, Identity_TileGround, Identity_FloorHeight });
}

void PG_DisaActorModel::setObjectTranslation(const glm::vec3& value) {
	if (m_translation == value) return;

	const glm::ivec2 tile = getTilePosition();
	m_translation = value;

	PG_IdentitySet changes{ Identity_Translation };
	if (getTilePosition() != tile) {
		m_height = calcHeight();
		changes.insert(Identity_TileGround);
		changes.insert(Identity_FloorHeight);
	}

	emit translationChanged();
	notifyParent(this, changes);
}

void PG_DisaActorModel::setVisible(bool value) {
	if (m_visible == value) return;
	m_visible = value;
	emit visibleChanged();
	notifyParent(this, { Identity_Visible });
}

void PG_DisaActorModel::setGravity(float value) {
	if (m_gravity == value) return;
	m_gravity = value;
	emit gravityChanged();
	notifyParent(this, { Identity_Gravity });
}

void PG_DisaActorModel::setColorMult(const glm::vec4& value) {
	if (m_colorMult == value) return;
	m_colorMult = value;
	emit colorMultChanged();
	notifyParent(this, { Identity_ColorMult });
}

void PG_DisaActorModel::setRotation(float value) {
	if (m_rotation == value) return;

	const EDisaFacing before = getCharacterFacing();
	m_rotation = value;

	if (getCharacterFacing() != before) {
		emit characterFacingChanged();
		emit rotationChanged();
		notifyParent(this, { Identity_Facing, Identity_Rotation });
		return;
	}
	emit rotationChanged();
	notifyParent(this, { Identity_Rotation });
}

void PG_DisaActorModel::setCharacterFacing(EDisaFacing value) {
	setRotation(static_cast<float>(value) * degreePerQuarter);
}

EDisaFacing PG_DisaActorModel::getCharacterFacing() const {
	return facingOf(m_rotation);
}

void PG_DisaActorModel::resetPlacement(glm::ivec2 tile, float rotation, bool visible) {
	m_rotation = rotation;
	m_visible = visible;
	m_translation = tileCenter(tile); //calcHeight reads the tile off the position
	m_height = calcHeight();
	m_translation.y = m_height;
}

void PG_DisaActorModel::remove() {
	auto list = getParent<PG_DisaActorListModel>();
	if (list) list->remove(m_location);
}

PG_Variety PG_DisaActorModel::getData(Identity identity) const {
	switch (identity) {
	case Identity_Translation: return m_translation;
	case Identity_TileGround: {
		if (auto* model = findParent<PG_DisaMapModel>(5)) {
			if (auto location = model->getTiles()->getTileLocation(getTilePosition())) {
				auto* item = location.getDataT<PG_DisaMapTileModel*>(PG_Identity_Item);
				auto top = item->getTop(glm::vec3(0.f, item->getTranslation().y, 0.f));
				for (glm::vec3& v : top) v.y -= m_height;
				return top;
			}
		}
		return std::array<glm::vec3, 4>{glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.f)};
	} break;
	case Identity_Facing: return static_cast<char>(getCharacterFacing());
	case Identity_Rotation: return m_rotation;
	case Identity_Visible: return m_visible;
	case Identity_ColorMult: return m_colorMult;
	case Identity_Gravity: return m_gravity;
	case Identity_FloorHeight: return m_height;
	}
	return PG_Variety();
}

bool PG_DisaActorModel::setData(const PG_Variety& value, Identity identity) {
	switch (identity) {
	case Identity_Translation: {
		if (auto v = value.get<glm::vec3>(); v != m_translation) {
			setObjectTranslation(v);
			return true;
		}
	}break;
	case Identity_Facing: {
		if (auto v = static_cast<EDisaFacing>(glm::clamp(value.convert<char>(), (char)-1, (char)2)); v != getCharacterFacing()) {
			setCharacterFacing(v);
			return true;
		}
	}break;
	case Identity_Rotation: {
		if (auto v = value.convert<float>(); v != m_rotation) {
			setRotation(v);
			return true;
		}
	}break;
	case Identity_Visible: {
		if (auto v = value.convert<bool>(); v != m_visible) {
			setVisible(v);
			return true;
		}
	}break;
	case Identity_Gravity: {
		if (auto v = value.convert<float>(); v != m_gravity) {
			setGravity(v);
			return true;
		}
	}break;
	case Identity_ColorMult: {
		if (auto v = value.get<glm::vec4>(); v != m_colorMult) {
			setColorMult(v);
			return true;
		}
	}break;
	}
	return false;
}
