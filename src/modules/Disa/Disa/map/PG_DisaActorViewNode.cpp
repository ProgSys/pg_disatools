#include "PG_DisaActorViewNode.h"
#include <algorithm>
#include <cmath>
#include <optional>
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
	if (!m_location) return empty;
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
	if (auto* model = m_location.getDataT<PG_DisaActorModel*>(PG_Identity_Item)) {
		model->setAnimationSet(value ? PG_DisaActorModel::Hurt : PG_DisaActorModel::IdleStanding);
	}
}

constexpr auto g_mirrorX = "mirrorX";

/* Sheet base of every animation set, indexed by PG_DisaActorModel::EAnimationSet. A side view is
 * added on top, so a base is never a sheet that exists, and 0 is the sprite's "no data" frame.
 * Only the first three rows differ, everything from Jump on is shared. */
constexpr std::array<int, 12> g_combatSets = { 1000, 1010, 1020, 1030, 1500, 1060, 1070, 1080, 1090, 1050, 1610, 1620 };
constexpr std::array<int, 12> g_idleSets = { 0, 10, 20, 1030, 1500, 1060, 1070, 1080, 1090, 1050, 1610, 1620 };

/**
* @brief The side view for an angle, negative when the sheet has to be mirrored.
* @details A sheet only holds one half of the turn, the other half is the same frames flipped.
* Front and back look the same from either side, so those two are never mirrored.
*/
int pickFiveViews(float angle) {
	constexpr float step = 45.f; //five views over half a turn, so each covers 45 degree
	const float degree = glm::degrees(std::abs(angle));
	const int side = angle < 0.f ? -1 : 1;

	if (degree < step * 0.5f) return 2;         //front
	if (degree < step * 1.5f) return side * 1;  //side front
	if (degree < step * 2.5f) return side * 4;  //side
	if (degree < step * 3.5f) return side * 7;  //side back
	return 8;                                   //back
}

/// The same for the two views a battle sprite has, where every quarter turn is a mirror. @see pickFiveViews
int pickTwoViews(float angle) {
	const int side = angle < 0.f ? -1 : 1;
	return side * (glm::degrees(std::abs(angle)) < 90.f ? 1 : 7);
}

struct AnimationChoice {
	int animation = 0;
	bool mirrored = false;
};

/**
* @brief Which sheet the actor shows, out of what it is doing and where it looks.
* @details A pose the script named outright wins, it already is a sheet number and the angle has no
* say in it. Otherwise the set gives the base and the angle the side view on top of it.
*/
std::optional<AnimationChoice> pickAnimation(const PG_Sprite& sprite, const PG_DisaActorModel& model, float angle) {
	const PG_DisaActorModel::EMode mode = model.getMode();
	if (mode == PG_DisaActorModel::Mode_None) {
		/* The script named the sheet itself and nothing here may overrule it. Without one there is
		 * nothing to pick either, the actor keeps whatever it was last drawn from. */
		const int pose = model.getAnimation();
		if (pose == 0) return {};
		return AnimationChoice{ std::abs(pose), pose < 0 };
	}

	const std::size_t set = std::min<std::size_t>(model.getAnimationSet(), g_idleSets.size() - 1);
	const bool idle = mode == PG_DisaActorModel::Mode_Cutscene;
	const int base = idle ? g_idleSets[set] : g_combatSets[set];

	/* Not every sprite carries all five views, outside the idle and the walk sets most carry none
	 * of the extra three. One that is missing falls back to the two a battle sprite has. */
	if (idle) {
		const int view = pickFiveViews(angle);
		if (sprite.getAnimationIndex(std::to_string(base + std::abs(view))) >= 0) {
			return AnimationChoice{ base + std::abs(view), view < 0 };
		}
	}
	const int view = pickTwoViews(angle);
	return AnimationChoice{ base + std::abs(view), view < 0 };
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


		/* A script gives whole degree, the map a quarter turn count, and the game stores both as
		 * the same angle. It turns clockwise around an x axis that points the other way than ours,
		 * which together come out as half a turn. */
		const float degree = m_location.getDataT<float>(PG_DisaActorModel::Identity_Rotation);
		const float radians = glm::radians(degree + 90.f);
		const glm::vec2 baseRotation(std::cos(radians), std::sin(radians));


		const float angle = PG_Math::angle(glm::normalize(glm::vec2(dir.x, dir.z)), baseRotation);
		const auto setMirrored = [&sprite](bool mirrored) {
			if (mirrored) sprite.addFrameModefier(g_mirrorX, {});
			else sprite.removeFrameModefier(g_mirrorX);
			};


		if (auto* model = m_location.getDataT<PG_DisaActorModel*>(PG_Identity_Item)) {
			if (const auto choice = pickAnimation(sprite, *model, angle)) {
				node->setAnimation(std::to_string(choice->animation), false);
				setMirrored(choice->mirrored);
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