#include "PG_DisaActorViewNode.h"
#include <cmath>
#include <stdexcept>
#include <utility>

#include <Disa/map/PG_DisaActorModel.h>
#include <Disa/map/PG_DisaTileViewNode.h>
#include <Disa/PG_DisaSpriteNode.h>
#include <Disa/resources/PG_DisaSprite.h>
#include <Disa/PG_DisaCharactersModel.h>
#include <Scene/PG_ModelNode.h>

#include <Scene/PG_Scene.h>
#include <Scene/PG_CameraNode.h>
#include <Base/Events/PG_TickEvent.h>
#include <Util/PG_Math.h>
#include <Util/PG_StringUtil.h>

PG_Object* PG_DisaActorViewNode::getObjectModel() const {
	if (!m_location) return nullptr;
	return m_location.getDataT<PG_DisaActorModel*>(PG_Identity_Item);
}

const PG_DisaActorScriptOverride& PG_DisaActorViewNode::getScriptOverride() const {
	static const PG_DisaActorScriptOverride empty;
	if (m_mode != Mode_Script || !m_location) return empty;
	auto* model = m_location.getDataT<PG_DisaActorModel*>(PG_Identity_Item);
	return model ? model->getScriptOverride() : empty;
}


void PG_DisaActorViewNode::setLocation(PG_ListModelLocation loc) {
	m_location = loc;
	loadSprite();
	updateTransformation();
	updateGroundShadow();
	setVolumeCollision(PG_Volume(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.3f, 0.5f, 0.3f), glm::mat3(1.0f)));
	updateModelMatrix();
	updateModelMatrixWorld();
}

void PG_DisaActorViewNode::update(const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
	const bool hasItem = identities.contains(PG_Identity_Item) || m_location != location;

	if (hasItem || identities.contains(PG_Identity_Location)) {
		m_location = location;
		loadSprite();
		updateTransformation();
		updateGroundShadow();
		updateColorOffset();
		emit translationUserChanged();
		return;
	}

	if (identities.contains(PG_DisaActorModel::Identity_Translation)) {
		setTranslation(getTranslationUser());
		emit translationUserChanged();
	}

	if (identities.contains(PG_DisaActorModel::Identity_TileGround)) {
		updateGroundShadow();
	}

	if (identities.contains(PG_DisaActorModel::Identity_Id)) {
		loadSprite();
	}

	if (identities.contains(PG_DisaActorModel::Identity_GeoColor)) {
		updateColorOffset();
	}
	if (identities.contains(PG_DisaActorModel::Identity_Next)) {
		if (PG_SpriteNode* node = findFirstChild<PG_SpriteNode>("N_Sprite")) {
			node->getSprite().next(true);
		}
	}
	else if (identities.contains(PG_DisaActorModel::Identity_Unpause)) {
		if (PG_SpriteNode* node = findFirstChild<PG_SpriteNode>("N_Sprite")) {
			node->getSprite().setPaused(false);
		}
	}

	if (identities.contains(PG_DisaActorModel::Identity_ScriptOverride)) {
		if (const auto& visible = getScriptOverride().visible) setVisible(*visible);
	}
}

void PG_DisaActorViewNode::loadSprite(unsigned short id) {
	std::cout << "PG_DisaActorViewNode::loadSprite " << id << std::endl;
	if (id == 0) {
		PG_ModelNode* model = findFirstChild<PG_ModelNode>("N_Prev");
		if (model) delete model;
		PG_DisaSpriteNode* node = findFirstChild<PG_DisaSpriteNode>("N_Sprite");
		if (node) delete node;
		return;
	}
	PG_DisaSprite sprite;
	int colorOffset = 0;
	if (auto* charModel = PG_DisaCharactersModel::get()) {
		auto pair = charModel->getSprite(id);
		sprite = std::move(pair.first);
		colorOffset = pair.second;
		if (id == 2800 && m_location) { //is geo symbol
			colorOffset = static_cast<int>(m_location.getDataT<EDisaGeoColor>(PG_DisaActorModel::Identity_GeoColor));
		}
	}else{
		sprite.loadExisting("DisaSprite" + std::to_string(id));
	}

	PG_ModelNode* model = findFirstChild<PG_ModelNode>("N_Prev");
	if (!sprite.isValid()) {
		if (!model) {
			model = PG_new<PG_ModelNode>("N_Prev", this);
			model->setModel(PG_Model("D_MyCube", PG_Material("M_Handle"), PG_Geometry::cube()));
			model->setScale(glm::vec3(0.5f));
		}
	}
	else {
		if (model) delete model;
		PG_DisaSpriteNode* node = findFirstChild<PG_DisaSpriteNode>("N_Sprite");
		if (!node) {
			node = PG_new<PG_DisaSpriteNode>("N_Sprite", this);
		}
		sprite.setAnimationSpeed(PG_Sprite::defaultTimeStep / 2.f);
		node->setSprite(std::move(sprite));
		node->setScale(glm::vec3(1.3f));
		node->setTranslation(glm::vec3(0.5f, 0.1f, 0.5f));
		node->setColorTableOffset(colorOffset);
	}
}

bool PG_DisaActorViewNode::isDragged() const {
	return m_dragged;
}

void PG_DisaActorViewNode::setDragged(bool value) {
	if (m_dragged == value) return;
	m_dragged = value;
	m_spring.setTargetPos(glm::vec3(0.f, value ? 0.5f : 0.0f, 0.f));
}

// Idle animation, has more angles then all other animations. Used mainly for cutscenes.
constexpr std::array<std::string_view, 5> g_idleStand = { "1" /*side-fornt (m)*/, "2" /*fornt*/, "3" /*side (m)*/, "4" /*side-back (m)*/, "5" /*back*/ };
// Basic walk. Used mainly for cutscenes. Most sprites don't have a walk fornt (12), side (14), back (18). They either dublicate 11 or 17, or its fully missing. 11 and 17 are expected to be present.
constexpr std::array<std::string_view, 5> g_idleWalk = { "11" /*side-fornt (m)*/, "12" /*fornt*/, "14" /*side (m)*/, "17" /*side-back (m)*/, "18" /*back*/ };

//Combat animations. Only have two faces that are mirroed based on angle.
constexpr std::array<std::string_view, 2> g_idleCombatStand = { "1001" , "1007"  };
constexpr std::array<std::string_view, 2> g_hurtStand = { "1061" , "1067"  };

constexpr auto g_mirrorX = "mirrorX";

/**
* @brief The side view of a five angle set that fits the angle, and whether it has to be mirrored.
* @details The sheet only holds one half of the turn, the other half is the same frames mirrored.
* Front and back look the same from either side, so those two never are.
*/
std::pair<std::string_view, bool> pickSideView(const std::array<std::string_view, 5>& animations, float angle) {
	constexpr float step = 45.f; //five views over half a turn, so each covers 45 degree
	const float degree = glm::degrees(std::abs(angle));
	const bool mirrored = angle < 0.f;

	if (degree < step * 0.5f) return { animations[1], false };           //front
	if (degree < step * 1.5f) return { animations[0], mirrored };        //side front
	if (degree < step * 2.5f) return { animations[2], mirrored };        //side
	if (degree < step * 3.5f) return { animations[3], mirrored };        //side back
	return { animations[4], false };                                     //back
}
constexpr glm::vec3 baseDir = glm::vec3(1.f, 0.f, 0.f);
constexpr glm::vec3 baseUp = glm::vec3(0.f, 1.f, 0.f);

glm::vec3 PG_DisaActorViewNode::getRotationDir(PG_Scene* scene, const glm::vec3& originWorld) {
	glm::vec3 dir = ((scene->getActiveCamera()->getTranslationWorld() - originWorld) - scene->getActiveCamera()->getViewDir()) / 2.f;
	dir = glm::normalize(dir);
	dir.y = glm::clamp(dir.y, -0.3f, 0.3f);
	dir = glm::normalize(dir);

	if (glm::any(glm::isnan(dir))) dir = baseDir;

	return dir;
}

bool PG_DisaActorViewNode::event(PG_Event* e) {
	if (e->getType() == PG_Event_Tick && isVisible()) {
		auto* et = static_cast<PG_TickEvent*>(e);
		m_spring.update(0.1f * et->getDeltaTime());

		auto* scene = getScene();
		const glm::vec3 dir = getRotationDir(scene, getTranslationWorld()); 
		//set animation
		PG_SpriteNode* node = findFirstChild<PG_SpriteNode>("N_Sprite");
		if (!node || !node->getSprite().isValid()) return PG_ActorNode::event(e);

		auto& sprite = node->getSprite();
		auto name = sprite.getAnimationName(); //debug
		auto spriteName = sprite.getName(); //debug

		node->setTranslationY(0.1f + m_spring.getPosition().y);
		//face sprite towards camera
		{
			glm::mat3 m;
			m[0] = glm::normalize(glm::cross(dir, baseUp));
			m[2] = -dir;
			m[1] = glm::normalize(glm::cross(m[2], m[0]));

			node->setRotation(glm::normalize(glm::quat_cast(m)));
		}
		node->updateModelMatrix();
		node->updateModelMatrixWorld();

		//choose animation
		const PG_DisaActorScriptOverride& scriptOverride = getScriptOverride();

		glm::vec2 baseRotation = glm::vec2(1.f, 0.f);
		if (scriptOverride.rotation) {
			//the map rotations run clockwise, so the angle does too
			const float radians = glm::radians(-*scriptOverride.rotation);
			baseRotation = glm::vec2(std::cos(radians), std::sin(radians));
		} else if (m_location) {
			switch (m_location.getDataT<char>(PG_DisaActorModel::Identity_Rotation)) {
				case -1: baseRotation = glm::vec2(0.f, 1.f); break;
				case 0: baseRotation = glm::vec2(-1.f, 0.f); break;
				case 1: baseRotation = glm::vec2(0.f, -1.f); break;
				case 2: baseRotation = glm::vec2(1.f, 0.f); break;
			};
		}


		const float angle = PG_Math::angle(glm::normalize(glm::vec2(dir.x, dir.z)), baseRotation);
		const auto setMirrored = [&sprite](bool mirrored) {
			if (mirrored) sprite.addFrameModefier(g_mirrorX, {});
			else sprite.removeFrameModefier(g_mirrorX);
			};


		if (scriptOverride.animation) {
			//the script named one animation, so there is no side view to pick between
			node->setAnimation(*scriptOverride.animation, false);
			setMirrored(false); //@todo is wrong can be mirred too
		}
		else if (m_mode == Mode_Script) {
			//a cutscene actor uses the sets that cover five angles, not the two of a battle
			const auto [animation, mirrored] = pickSideView(m_idleSet == Idle_Walk ? g_idleWalk : g_idleStand, angle);
			node->setAnimation(std::string(animation), false);
			setMirrored(mirrored);
		}
		else {
			const std::array<std::string_view, 2>& animationSet = m_dragged ? g_hurtStand : g_idleCombatStand;
			if (angle < 0.f) {
				node->setAnimation(std::string(animationSet[1]), false);
				setMirrored(angle >= glm::radians(-90.f));
			}
			else {
				node->setAnimation(std::string(animationSet[0]), false);
				setMirrored(angle < glm::radians(90.f));
			}
		}
	}


	return PG_ActorNode::event(e);
}

void PG_DisaActorViewNode::updateActor() {
	updateModelMatrix();
	updateModelMatrixWorld();
	PG_TickEvent e(0.f);
	event(&e);
}

void PG_DisaActorViewNode::updateTransformation() {
	if (!m_location) return;
	setTranslation(getTranslationUser());
}

void PG_DisaActorViewNode::updateGroundShadow() {
	if (!m_location) return;
	PG_ModelNode* model = findFirstChild<PG_ModelNode>("N_Shadow");
	const auto top = m_location.getDataT<std::array<glm::vec3, 4>>(PG_DisaActorModel::Identity_TileGround);

	if (top.front() == top.back()) {
		if (model) delete model;
		return;
	}

	if (!model) {
		model = PG_new<PG_ModelNode>("N_Shadow", this);
	}
	model->setTranslationY(0.01f);
	PG_DisaTileViewNode::updateFloorShadowGeometry(top, model);
}

glm::vec3 PG_DisaActorViewNode::getTranslationUser() const {
	return m_location.getDataT<glm::vec3>(PG_DisaActorModel::Identity_Translation);
}

void PG_DisaActorViewNode::setTranslationUser(const glm::vec3& translation) {
	m_location.setData(translation, PG_DisaActorModel::Identity_Translation);
}


unsigned short PG_DisaActorViewNode::getCharacterId() const {
	return m_location.getDataT<unsigned short>(PG_DisaActorModel::Identity_Id);
}
void PG_DisaActorViewNode::setCharacterId(unsigned short id) {
	m_location.setData(id, PG_DisaActorModel::Identity_Id);
}

void PG_DisaActorViewNode::updateColorOffset() {
	if (!m_location && getCharacterId() != 2800) return;//must be geo symbol

	if (PG_DisaSpriteNode* node = findFirstChild<PG_DisaSpriteNode>("N_Sprite"); node && node->getSprite().isValid()) {
		node->setColorTableOffset(static_cast<int>(m_location.getDataT<EDisaGeoColor>(PG_DisaActorModel::Identity_GeoColor)));
	}
}