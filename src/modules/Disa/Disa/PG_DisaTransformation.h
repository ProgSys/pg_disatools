#pragma once
#include <Util/PG_Transformation.h>

inline glm::vec3 fromDisaPosition(const glm::vec3& position) { return glm::vec3(-position.x, -position.y, position.z) / 10.0f; }
glm::vec3 fromDisaRotationEuler(const glm::vec3& rotation); //return value in degree
glm::quat fromDisaRotation(const glm::vec3& eulerRotation); //use fromDisaRotationEuler to get euler
inline glm::vec3 fromDisaScale(const glm::vec3& scale) { return scale / 100.0f; }

class PG_DisaTransformation : public PG_Transformation {

public:
	PG_DisaTransformation(const glm::vec3& translation, const glm::vec3& rotationEuler, const glm::vec3& scale);

	inline const glm::vec3& getRotationDisaEuler() const { return m_rotationEuler; }
	inline void getRotationDisaEuler(glm::vec3& delta) { m_rotationEuler = delta; }

	PG_Transformation mix(const PG_DisaTransformation& target, float value) const;
private:
	glm::vec3 m_rotationEuler = glm::vec3(0.0f);
};