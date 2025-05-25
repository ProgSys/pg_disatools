#include "PG_DisaTransformation.h"
#include <glm/gtx/euler_angles.hpp>

glm::vec3 fromDisaRotationEuler(const glm::vec3& rotation) {
	return glm::vec3(
		-rotation.x / 10.f,
		-rotation.y / 10.f,
		rotation.z / 10.f
	);
}
glm::quat fromDisaRotation(const glm::vec3& eulerRotation) {
	return glm::quat(glm::eulerAngleYXZ(
		glm::radians(eulerRotation.y), glm::radians(eulerRotation.x), glm::radians(eulerRotation.z)
	));
}

PG_DisaTransformation::PG_DisaTransformation(const glm::vec3& translation, const glm::vec3& rotationEuler, const glm::vec3& scale) :
	PG_Transformation(translation, fromDisaRotation(rotationEuler), scale), m_rotationEuler(rotationEuler){
}


PG_Transformation PG_DisaTransformation::mix(const PG_DisaTransformation& target, float value) const {
	return PG_Transformation(
		glm::mix(m_translation, target.getTranslation(), value),
		fromDisaRotation(glm::mix(m_rotationEuler, target.getRotationDisaEuler(), value)),
		glm::mix(m_scale, target.getScale(), value)
	);
}

