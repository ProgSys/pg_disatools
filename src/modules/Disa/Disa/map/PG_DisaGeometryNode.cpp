#include "PG_DisaGeometryNode.h"

#include <glm/gtc/quaternion.hpp>

#include <Disa/map/PG_DisaGeometryModel.h>

void PG_DisaGeometryNode::setLocation(PG_ListModelLocation loc) {
	m_location = loc;
	updateResource();
	updateTransformation();
	updateModelMatrix();
	updateModelMatrixWorld();
}

void PG_DisaGeometryNode::update(const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
	const bool hasItem = identities.contains(PG_Identity_Item) || m_location != location;

	if (hasItem || identities.contains(PG_Identity_Location)) {
		m_location = location;
		updateResource();
		updateTransformation();
		return;
	}

	if (identities.contains(PG_DisaGeometryModel::Identity_Resource)) updateResource();

	if (contains<PG_DisaGeometryModel::Identity_Translation, PG_DisaGeometryModel::Identity_Rotation,
		PG_DisaGeometryModel::Identity_Scale, PG_DisaGeometryModel::Identity_Visible>(identities)) {
		updateTransformation();
	}
}

void PG_DisaGeometryNode::updateResource() {
	if (!m_location) return;

	const std::string resource = readData<std::string>(PG_DisaGeometryModel::Identity_Resource);
	if (resource == m_loaded) return;

	m_loaded = resource;
	if (!resource.empty()) loadGlobal(resource);
}

void PG_DisaGeometryNode::updateTransformation() {
	if (!m_location) return;

	setTranslation(readData<glm::vec3>(PG_DisaGeometryModel::Identity_Translation));
	setScale(glm::vec3(readData<float>(PG_DisaGeometryModel::Identity_Scale)));

	//the same half turn the sprites carry, so a mesh and a unit facing east look the same way
	const float degree = readData<float>(PG_DisaGeometryModel::Identity_Rotation) + 90.f;
	setRotation(glm::angleAxis(glm::radians(-degree), glm::vec3(0.f, 1.f, 0.f)));

	setVisible(readData<bool>(PG_DisaGeometryModel::Identity_Visible));
}
