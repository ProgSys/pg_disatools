#include "PG_DisaObjectModel.h"
#include <glm/gtx/euler_angles.hpp>

constexpr glm::vec3 g_originPoint = glm::vec3(6.f, 0.f, 6.f);
constexpr float g_objectScaleFactor = 12.f;
constexpr float g_objectScale = 1.f / g_objectScaleFactor;


glm::ivec3 PG_DisaObjectModel::toObjectTranslation(glm::vec3 worldPos) {
	worldPos = (worldPos / g_objectScale) - g_originPoint;
	return glm::ivec3(
		-std::round(worldPos.x),
		-std::round(worldPos.y),
		std::round(worldPos.z)
		);
}

glm::vec3 PG_DisaObjectModel::getObjectTranslation() const {
	return getChunkTranslation() + (glm::vec3(-m_data.x, -m_data.y, m_data.z) + g_originPoint) * g_objectScale;
}

static inline void notifyParent(PG_DisaObjectModel* self, const PG_IdentitySet& identities) {
	if (auto* base = self->getParent<PG_ListModelBase>(); base && self->getLocation() >= 0) {
		emit base->itemsChanged(self->getLocation(), self->getLocation(), identities);
	}
}

void PG_DisaObjectModel::setObjectTranslation(glm::vec3 t) {
	t -= getChunkTranslation();
	t /= g_objectScale;
	t -= g_originPoint;
	
	short x = -std::round(t.x);
	short y = -std::round(t.y);
	short z = std::round(t.z);
	if (x == m_data.x && y == m_data.y && z == m_data.z) return;

	m_data.x = x;
	m_data.y = y;
	m_data.z = z;
	emit objectTranslationChanged();

	notifyParent(this, { Identity_Translation });
}

void PG_DisaObjectModel::setObjectDisaTranslation(glm::ivec3 t) {
	if (m_data.x == t.x && m_data.y == t.y && m_data.z == t.z) return;
	m_data.x = t.x;
	m_data.y = t.y;
	m_data.z = t.z;

	emit objectTranslationChanged();

	notifyParent(this, { Identity_Translation });
}

void PG_DisaObjectModel::setObjectDisaRotation(glm::ivec3 t) {
	if (m_data.xRot == t.x && m_data.yRot == t.y && m_data.xRot == t.z) return;
	m_data.xRot = t.x;
	m_data.yRot = t.y;
	m_data.zRot = t.z;

	emit objectRotationChanged();
	notifyParent(this, { Identity_Rotation });
}

void PG_DisaObjectModel::setObjectDisaScale(glm::ivec3 t) {
	if (m_data.xScale == t.x && m_data.yScale == t.y && m_data.zScale == t.z) return;
	m_data.xScale = t.x;
	m_data.yScale = t.y;
	m_data.zScale = t.z;

	emit objectScaleChanged();
	notifyParent(this, { Identity_Scale });
}

glm::quat PG_DisaObjectModel::getObjectRotation() const {
	const glm::vec3 eulerRotation = glm::radians(glm::vec3(-m_data.xRot, -m_data.yRot, m_data.zRot));
	const glm::quat rotation = glm::quat(glm::eulerAngleYXZ(eulerRotation.y, eulerRotation.x, eulerRotation.z));
	return rotation;
}

void PG_DisaObjectModel::setObjectRotation(glm::quat r) {
	glm::vec3 rotEuler(0.f);
	glm::extractEulerAngleYXZ(glm::mat4_cast(r), rotEuler.y, rotEuler.x, rotEuler.z);
	rotEuler = glm::round(glm::degrees(rotEuler));
	short xRot = static_cast<short>(-rotEuler.x);
	short yRot = static_cast<short>(-rotEuler.y);
	short zRot = static_cast<short>(rotEuler.z);

	if (xRot == m_data.xRot && yRot == m_data.yRot && zRot == m_data.zRot) return;

	m_data.xRot = xRot;
	m_data.yRot = yRot;
	m_data.zRot = zRot;
	emit objectRotationChanged();

	notifyParent(this, { Identity_Rotation });

}

void PG_DisaObjectModel::setChunk(PG_DisaMapChunkModel* model) {
	assert(model);
	if (m_chunk == model) return;
	m_chunk = model;
}

float PG_DisaObjectModel::getBaseScale() {
	return g_objectScale;
}

glm::vec3 PG_DisaObjectModel::getObjectScale() const {
	return (glm::vec3(m_data.xScale, m_data.yScale, m_data.zScale) / 100.f);
}

void PG_DisaObjectModel::setObjectScale(glm::vec3 v) {
	v *= 100.f;
	short xScale = static_cast<short>(std::round(v.x));
	short yScale = static_cast<short>(std::round(v.y));
	short zScale = static_cast<short>(std::round(v.z));

	if (xScale == m_data.xScale && yScale == m_data.yScale && zScale == m_data.zScale) return;

	m_data.xScale = xScale;
	m_data.yScale = yScale;
	m_data.zScale = zScale;

	emit objectScaleChanged();
	notifyParent(this, { Identity_Scale });
}

void PG_DisaObjectModel::remove() {
	auto list = getParent<PG_DisaObjectListModel>();
	if (list) list->remove(m_location);
}

PG_Variety PG_DisaObjectModel::getData(Identity identity) const {
	switch (identity) {
	case Identity_Translation: return getObjectTranslation();
	case Identity_Rotation: return getObjectRotation();
	case Identity_Scale: return getObjectScale();
	}
	return PG_Variety();
}

bool PG_DisaObjectModel::setData(const PG_Variety& value, Identity identity) {
	try {
		switch (identity) {
		case Identity_Translation: {
			if (glm::vec3 v = value.get<glm::vec3>(); v != getObjectTranslation()) {
				v -= getChunkTranslation();
				v *= g_objectScaleFactor;
				v -= g_originPoint;
				m_data.x = static_cast<short>(std::round(-v.x));
				m_data.y = static_cast<short>(std::round(-v.y));
				m_data.z = static_cast<short>(std::round(v.z));
				emit objectTranslationChanged();
				return true;
			}
		}break;
		case Identity_Rotation: {
			if (glm::quat v = value.get<glm::quat>(); v != getObjectRotation()) {
				glm::vec3 rotEuler(0.f);
				glm::extractEulerAngleYXZ(glm::mat4_cast(v), rotEuler.y, rotEuler.x, rotEuler.z);
				rotEuler = glm::round(glm::degrees(rotEuler));
				m_data.xRot = static_cast<short>(-rotEuler.x);
				m_data.yRot = static_cast<short>(-rotEuler.y);
				m_data.zRot = static_cast<short>(rotEuler.z);
				emit objectRotationChanged();
				return true;
			}
		}break;
		case Identity_Scale: {
			if (glm::vec3 v = value.get<glm::vec3>(); v != getObjectScale()) {
				v *= 100.f;
				m_data.xScale = static_cast<short>(std::round(v.x));
				m_data.yScale = static_cast<short>(std::round(v.y));
				m_data.zScale = static_cast<short>(std::round(v.z));
				emit objectScaleChanged();
				return true;
			}
		}break;
		}
	}
	catch (...) {}

	return false;
}