#include "PG_DisaObjectViewNode.h"
#include <Disa/PG_DisaMapModel.h>
#include <Disa/map/PG_DisaObjectModel.h>
#include <Disa/PG_DisaModelNode.h>
#include <Disa/PG_DisaAssetsModel.h>


PG_Object* PG_DisaObjectViewNode::getObjectModel() const {
	if (!m_location) return nullptr;
	return m_location.getDataT<PG_DisaObjectModel*>(Identity_Item);
}

void PG_DisaObjectViewNode::setLocation(PG_ListModelLocation loc) {
	m_location = loc;
	loadGeometry();
}

void PG_DisaObjectViewNode::update(const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
	const bool hasItem = identities.contains(Identity_Item) || m_location != location;

	if (hasItem || identities.contains(Identity_Location)) {
		m_location = location;
		loadGeometry();
		emit translationUserChanged();
		emit rotationUserChanged();
		emit scaleUserChanged();
		return;
	}

	if (identities.contains(PG_DisaObjectModel::Identity_Translation)) {
		setTranslation(getTranslationUser());
		emit translationUserChanged();
	}
	if (identities.contains(PG_DisaObjectModel::Identity_Rotation)) {
		updateTransformation();
		emit rotationUserChanged();
	}
	if (identities.contains(PG_DisaObjectModel::Identity_Scale)) {
		updateTransformation();
		emit scaleUserChanged();
	}
}

PG_DisaModelNode* PG_DisaObjectViewNode::getModelNode() const {
	return findFirstChild<PG_DisaModelNode>("PG_DisaModelNode");
}

void PG_DisaObjectViewNode::loadGeometry() {
	static int i = -1;
	if (!m_location || (i > 0 && m_location.getIndex() != i)) {
		clearChildren();
		return;
	}

	auto model = m_location.getDataT<PG_DisaObjectModel*>(Identity_Item);
	const auto& object = model->getData();
	auto mapModel = model->findParent<PG_DisaMapModel>(3);

	if (!mapModel || object.model_id >= mapModel->getAssets()->getGeometries()->size()) {
		clearChildren();
		return;
	}

	setVisible(static_cast<bool>(object.visible));
	setTranslation(model->getObjectTranslation());
	setScale(glm::vec3(PG_DisaObjectModel::getBaseScale()));

	PG_DisaModelNode* disaModel = findFirstChild<PG_DisaModelNode>("PG_DisaModelNode");

	if (!disaModel) {
		disaModel = PG_new<PG_DisaModelNode>("PG_DisaModelNode", this);
	}

	const glm::vec2 scroll = glm::vec2(object.textureScrollX, object.textureScrollY) / 10000.0f;
	disaModel->setTextureScroll(scroll);

	{
		PG_Model geoModel;
		const auto& items = mapModel->getAssets()->getGeometries()->getItems();
		if (object.model_id < items.size()) {
			const auto& item = items[object.model_id];
			const bool isHdMode = mapModel->getAssets()->isHDViewEnabled();
			geoModel = isHdMode? item.getHDModel() : item.getSDModel();
		}
		disaModel->setModel(std::move(geoModel));
	}

	//euler YXZ rotation in degree radians
	disaModel->setRootTransformation(
		PG_Transformation(
			glm::vec3(0.f), 
			model->getObjectRotation(),
			model->getObjectScale()
		)
	);
	
}

void PG_DisaObjectViewNode::updateTransformation() {
	if (!m_location) return;
	auto model = m_location.getDataT<PG_DisaObjectModel*>(Identity_Item);

	PG_DisaModelNode* disaModel = findFirstChild<PG_DisaModelNode>("PG_DisaModelNode");
	if (!disaModel) return;
	setTranslation(model->getObjectTranslation());
	//euler YXZ rotation in degree radians
	disaModel->setRootTransformation(
		PG_Transformation(
			glm::vec3(0.f), 
			model->getObjectRotation(),
			model->getObjectScale()
		)
	);
}

glm::vec3 PG_DisaObjectViewNode::getTranslationUser() const {
	return m_location.getDataT<glm::vec3>(PG_DisaObjectModel::Identity_Translation);
}

void PG_DisaObjectViewNode::setTranslationUser(const glm::vec3& translation) {
	m_location.setData(translation, PG_DisaObjectModel::Identity_Translation);
}

glm::quat PG_DisaObjectViewNode::getRotationUser() const {
	return m_location.getDataT<glm::quat>(PG_DisaObjectModel::Identity_Rotation);
}

void PG_DisaObjectViewNode::setRotationUser(const glm::quat& rotation) {
	m_location.setData(rotation, PG_DisaObjectModel::Identity_Rotation);
}

glm::vec3 PG_DisaObjectViewNode::getScaleUser() const {
	return m_location.getDataT<glm::vec3>(PG_DisaObjectModel::Identity_Scale);
}

void PG_DisaObjectViewNode::setScaleUser(const glm::vec3& scale) {
	m_location.setData(scale, PG_DisaObjectModel::Identity_Scale);
}