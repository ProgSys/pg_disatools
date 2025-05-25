#include "PG_DisaMapChunkModel.h"


glm::vec3 PG_DisaMapChunkModel::getTileTranslation() const {
	glm::vec3 translation = getTranslationPtr();
	translation = (glm::vec3(-translation.x + 6.f, translation.y, translation.z - 6.f)) / 12.f;
	return translation;
}

