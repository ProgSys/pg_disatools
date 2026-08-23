#include "PG_DisaActorEditSelectionNode.h"
#include <Disa/PG_DisaMapViewNode.h>
#include <Disa/map/PG_DisaTileViewNode.h>
#include <Disa/map/PG_DisaActorViewNode.h>
#include <Disa/map/PG_DisaActorModel.h>
#include <Disa/PG_DisaCharactersModel.h>
#include <Disa/PG_DisaSpriteNode.h>
#include <Disa/resources/PG_DisaSprite.h>
#include <Base/Events/PG_TickEvent.h>
#define _USE_MATH_DEFINES
#include <math.h>

static inline PG_DisaActorEditSelectionNode* createNode(PG_DisaActorEditSelectionHandlerNode* parent, PG_DisaActorModel* model, float lastTime) {
	auto* node = PG_new<PG_DisaActorEditSelectionNode>("PG_DisaActorEditSelectionNode", parent);
	node->setTime(lastTime);

	PG_Object::connect(model, &PG_DisaActorModel::translationChanged, node, [node, model]() {
		node->setTranslation(model->getObjectTranslation());
		}, PG_ConnectionType_Event);

	return node;
}

static inline float getBaseHeight(PG_DisaActorModel* model) {
	return model->getCharacterId() == 2800 ? 0.7 : 1.4;
}

void PG_DisaActorEditSelectionHandlerNode::setHovered(PG_DisaActorModel* model, bool hovered) {
	auto findIt = m_selection.find(model);

	PG_DisaActorEditSelectionNode* node = nullptr;
	if (findIt != m_selection.end()) {
		node = findIt->second;
	}

	if (!hovered) {
		if (node) {
			if (!node->isSelected()) {
				m_lastTime = findIt->second->getTime();
				delete findIt->second;
				m_selection.erase(findIt);
			}
			else {
				node->setHovered(false);
			}
		}
		return;
	}

	if (!node) {
		float lastTime = m_selection.empty() ? m_lastTime : m_selection.begin()->second->getTime();
		node = createNode(this, model, lastTime);
		connect(model, &PG_Object::onDeleted, this, [this, model]() {
			auto findIt = m_selection.find(model);
			if (findIt != m_selection.end()) {
				delete findIt->second;
				m_selection.erase(findIt);
			}
			});
	}

	node->setHovered(true);
	node->setBaseHeight(getBaseHeight(model));
	node->setTranslation(model->getObjectTranslation());
	node->updateModelMatrix();
	node->updateModelMatrixWorld();
	node->rotateTowardsCamera();
	m_selection[model] = node;
}

void PG_DisaActorEditSelectionHandlerNode::setSelected(PG_DisaActorModel* model, bool selected) {
	auto findIt = m_selection.find(model);

	PG_DisaActorEditSelectionNode* node = nullptr;
	if (findIt != m_selection.end()) {
		node = findIt->second;
	}

	if (!selected) {
		if (node){
			if (!node->isHovered()) {
				m_lastTime = findIt->second->getTime();
				delete findIt->second;
				m_selection.erase(findIt);
			}
			else {
				node->setSelected(false);
			}
		}
		return;
	}

	if (!node) {
		float lastTime = m_selection.empty() ? m_lastTime : m_selection.begin()->second->getTime();
		node = createNode(this, model, lastTime);
		connect(model, &PG_Object::onDeleted, this, [this, model]() {
			auto findIt = m_selection.find(model);
			if (findIt != m_selection.end()) {
				delete findIt->second;
				m_selection.erase(findIt);
			}
			});
	}

	node->setSelected(true);
	node->setBaseHeight(getBaseHeight(model));
	node->setTranslation(model->getObjectTranslation());
	node->updateModelMatrix();
	node->updateModelMatrixWorld();
	node->rotateTowardsCamera();
	m_selection[model] = node;
}

void PG_DisaActorEditSelectionHandlerNode::update(PG_DisaActorModel* model) {

}

void PG_DisaActorEditSelectionHandlerNode::cleanUp() {

}


void PG_DisaActorEditSelectionNode::init() {
	PG_ActorNode::init();
	PG_DisaSprite sprite;
	if (auto* charModel = PG_DisaCharactersModel::get()) {
		auto pair = charModel->getSprite(1001);
		sprite = std::move(pair.first);
	}
	else return;

	if (!sprite.isValid()) return;

	PG_DisaSpriteNode* node = PG_new<PG_DisaSpriteNode>("N_Sprite", this);

	sprite.setAnimationSpeed(PG_Sprite::defaultTimeStep / 2.f);
	node->setSprite(std::move(sprite));
	node->setScale(glm::vec3(0.3f));
	node->setColorTableOffset(0);
}

constexpr glm::vec3 baseUp = glm::vec3(0.f, 1.f, 0.f);

bool PG_DisaActorEditSelectionNode::event(PG_Event* e) {
	if (e->getType() == PG_Event_Tick && isVisible()) {
		PG_TickEvent* t = static_cast<PG_TickEvent*>(e);
		rotateTowardsCamera(0.1f * t->getDeltaTime());
	}

	return PG_ActorNode::event(e);
}

void PG_DisaActorEditSelectionNode::rotateTowardsCamera(float dt) {
	auto* scene = getScene();
	if (!scene) return;
	dt *= 0.01f;

	const glm::vec3 dir = PG_DisaActorViewNode::getRotationDir(scene, getTranslationWorld());

	//set animation
	if (PG_SpriteNode* node = findFirstChild<PG_SpriteNode>("N_Sprite"); node && node->getSprite().isValid()) {

		//face sprite towards camera
		{
			glm::mat3 m;
			m[0] = glm::normalize(glm::cross(dir, baseUp));
			m[2] = -dir;
			m[1] = glm::normalize(glm::cross(m[2], m[0]));

			node->setAnimation(isSelected()? "4": "3");
			//std::cout << "Offset: "<< glm::normalize(m[1]) << std::endl;
			node->setTranslation(glm::vec3(0.5f, 0.f, 0.5f) + glm::normalize(m[1]) * (m_baseHeight + 0.2f * std::sin(m_time)) - m[2] * 0.5f);
			node->setRotation(glm::normalize(glm::quat_cast(m)));
			node->updateModelMatrix();
			node->updateModelMatrixWorld();
		}
	}

	m_time += dt;
	m_time = glm::mod(m_time, (float)M_PI);
}