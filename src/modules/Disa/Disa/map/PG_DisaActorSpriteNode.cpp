#include "PG_DisaActorSpriteNode.h"
#include <algorithm>
#include <cmath>
#include <optional>
#include <stdexcept>
#include <utility>

#include <Disa/map/PG_DisaSpriteModel.h>
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

PG_Object* PG_DisaActorSpriteNode::getObjectModel() const {
	if (!m_location) return nullptr;
	return readData<PG_DisaActorModel*>(PG_Identity_Item);
}

void PG_DisaActorSpriteNode::setLocation(PG_ListModelLocation loc) {
	m_location = loc;
	loadSprite();
	updateTransformation();
	updateGroundShadow();
	updateColorMult();
	setVolumeCollision(PG_Volume(glm::vec3(0.f, 0.5f, 0.f), glm::vec3(0.3f, 0.5f, 0.3f), glm::mat3(1.0f)));
	updateModelMatrix();
	updateModelMatrixWorld();
}

void PG_DisaActorSpriteNode::update(const PG_ListModelLocation& location, const PG_IdentitySet& identities) {
	const bool hasItem = identities.contains(PG_Identity_Item) || m_location != location;

	if (hasItem || identities.contains(PG_Identity_Location)) {
		m_location = location;
		loadSprite();
		updateTransformation();
		updateGroundShadow();
		updateColorOffset();
		updateColorMult();
		emit translationUserChanged();
		return;
	}

	if (identities.contains(PG_DisaSpriteModel::Identity_Translation)) {
		setTranslation(getTranslationUser());
		emit translationUserChanged();
	}

	if (identities.contains(PG_DisaSpriteModel::Identity_TileGround)) {
		updateGroundShadow();
	}

	if (identities.contains(PG_DisaSpriteModel::Identity_Id)) {
		loadSprite();
	}

	if (identities.contains(PG_DisaSpriteModel::Identity_GeoColor)) {
		updateColorOffset();
	}
	if (identities.contains(PG_DisaSpriteModel::Identity_Next)) {
		if (PG_SpriteNode* node = findFirstChild<PG_SpriteNode>("N_Sprite")) {
			node->getSprite().next(true);
		}
	}
	else if (identities.contains(PG_DisaSpriteModel::Identity_Unpause)) {
		if (PG_SpriteNode* node = findFirstChild<PG_SpriteNode>("N_Sprite")) {
			node->getSprite().setPaused(false);
		}
	}

	if (identities.contains(PG_DisaSpriteModel::Identity_ColorMult)) {
		updateColorMult();
	}
	else if (identities.contains(PG_DisaSpriteModel::Identity_Visible)) {
		updateVisible();
	}
}

void PG_DisaActorSpriteNode::updateForce(float deltaTime) {
	if (!m_location) return;

	//the fall needs the model under it, the sway does not, so only one of them is fed
	if (auto* gravity = std::get_if<GravityForce>(&m_force)) {
		gravity->setGravity(readData<float>(PG_DisaSpriteModel::Identity_Gravity));
		gravity->setFloorHeight(readData<float>(PG_DisaSpriteModel::Identity_FloorHeight));
	}

	const glm::vec3 position = readData<glm::vec3>(PG_DisaSpriteModel::Identity_Translation);
	std::visit([this, deltaTime, &position](const auto& force) { force.update(m_motion, deltaTime, position); }, m_force);
}

void PG_DisaActorSpriteNode::updateColorMult() {
	if (!m_location) return;
	if (PG_DisaSpriteNode* node = findFirstChild<PG_DisaSpriteNode>("N_Sprite")) {
		node->setColorMult(readData<glm::vec4>(PG_DisaSpriteModel::Identity_ColorMult));
	}
	updateVisible();
}

void PG_DisaActorSpriteNode::updateVisible() {
	if (!m_location) return;
	//the sprite carries no alpha of its own, so an actor faded out to nothing is simply not drawn
	const bool clear = readData<glm::vec4>(PG_DisaSpriteModel::Identity_ColorMult).a <= 0.f;
	setVisible(readData<bool>(PG_DisaSpriteModel::Identity_Visible) && !clear);
}

void PG_DisaActorSpriteNode::loadSprite(unsigned short id) {
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
			colorOffset = static_cast<int>(readData<EDisaGeoColor>(PG_DisaSpriteModel::Identity_GeoColor));
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
		node->setTranslation(glm::vec3(0.f, 0.1f, 0.f)); //lifted a little so the feet clear the floor
		node->setColorTableOffset(colorOffset);
	}
}

bool PG_DisaActorSpriteNode::isDragged() const {
	return m_dragged;
}

void PG_DisaActorSpriteNode::setDragged(bool value) {
	if (m_dragged == value) return;
	m_dragged = value;

	//a dragged actor hangs off the cursor and swings, a dropped one falls back onto the floor
	if (value) {
		SwaySpring spring;
		spring.setTargetPos(glm::vec3(0.f, 0.5f, 0.f));
		m_force = spring;
	}
	else {
		m_force = GravityForce();
	}
	//through the identity, because the drag indicator has no model at all behind its location
	m_location.setData(value ? PG_DisaSpriteModel::Hurt : PG_DisaSpriteModel::IdleStanding, PG_DisaSpriteModel::Identity_AnimationSet);
}

constexpr auto g_mirrorX = "mirrorX";

/* Sheet base of every animation set, indexed by PG_DisaSpriteModel::EAnimationSet. A side view is
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
std::optional<AnimationChoice> pickAnimation(const PG_Sprite& sprite, PG_DisaSpriteModel::EMode mode,
	int animationSet, int pose, float angle) {
	if (mode == PG_DisaSpriteModel::Mode_None) {
		/* The script named the sheet itself and nothing here may overrule it. Without one there is
		 * nothing to pick either, the actor keeps whatever it was last drawn from. */
		if (pose == 0) return {};
		return AnimationChoice{ std::abs(pose), pose < 0 };
	}

	const std::size_t set = std::min<std::size_t>(animationSet, static_cast<int>(g_idleSets.size()) - 1);
	const bool idle = mode == PG_DisaSpriteModel::Mode_Cutscene;
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

glm::vec3 PG_DisaActorSpriteNode::getRotationDir(PG_Scene* scene, const glm::vec3& originWorld) {
	glm::vec3 dir = ((scene->getActiveCamera()->getTranslationWorld() - originWorld) - scene->getActiveCamera()->getViewDir()) / 2.f;
	dir = glm::normalize(dir);
	dir.y = glm::clamp(dir.y, -0.3f, 0.3f);
	dir = glm::normalize(dir);

	if (glm::any(glm::isnan(dir))) dir = baseDir;

	return dir;
}

bool PG_DisaActorSpriteNode::event(PG_Event* e) {
	if (e->getType() == PG_Event_Tick && isVisible()) {
		auto* et = static_cast<PG_TickEvent*>(e);
		updateForce(0.1f * et->getDeltaTime());

		auto* scene = getScene();
		assert(scene);
		const glm::vec3 dir = getRotationDir(scene, getTranslationWorld()); 
		//set animation
		PG_SpriteNode* node = findFirstChild<PG_SpriteNode>("N_Sprite");
		if (!node || !node->getSprite().isValid()) return PG_ActorNode::event(e);

		auto& sprite = node->getSprite();

		node->setTranslationY(0.1f + m_motion.offset.y);
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
		const float degree = readData<float>(PG_DisaSpriteModel::Identity_Rotation);
		const float radians = glm::radians(degree + 90.f);
		const glm::vec2 baseRotation(std::cos(radians), std::sin(radians));


		const float angle = PG_Math::angle(glm::normalize(glm::vec2(dir.x, dir.z)), baseRotation);
		const auto setMirrored = [&sprite](bool mirrored) {
			if (mirrored) sprite.addFrameModefier(g_mirrorX, {});
			else sprite.removeFrameModefier(g_mirrorX);
			};


		/* Read through the identities rather than off a model: the drag indicator has no model at
		 * all behind its location, and a slot may hold something that is not a sprite. */
		const auto mode = static_cast<PG_DisaSpriteModel::EMode>(readData<int>(PG_DisaSpriteModel::Identity_AnimationMode));
		const int animationSet = readData<int>(PG_DisaSpriteModel::Identity_AnimationSet);
		const int pose = readData<int>(PG_DisaSpriteModel::Identity_Animation);

		if (const auto choice = pickAnimation(sprite, mode, animationSet, pose, angle)) {
			std::string aniName = std::to_string(choice->animation);
			if (node->getAnimationName() != aniName) {
				node->setAnimation(aniName, true);
				node->setPaused(false);
			}
			setMirrored(choice->mirrored);
		}
	}


	return PG_ActorNode::event(e);
}

void PG_DisaActorSpriteNode::updateActor() {
	updateModelMatrix();
	updateModelMatrixWorld();
	PG_TickEvent e(0.f);
	event(&e);
}

void PG_DisaActorSpriteNode::updateTransformation() {
	if (!m_location) return;
	setTranslation(getTranslationUser());
}

void PG_DisaActorSpriteNode::updateGroundShadow() {
	if (!m_location) return;
	PG_ModelNode* model = findFirstChild<PG_ModelNode>("N_Shadow");
	const auto top = readData<std::array<glm::vec3, 4>>(PG_DisaSpriteModel::Identity_TileGround);

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

glm::vec3 PG_DisaActorSpriteNode::getTranslationUser() const {
	return readData<glm::vec3>(PG_DisaSpriteModel::Identity_Translation);
}

void PG_DisaActorSpriteNode::setTranslationUser(const glm::vec3& translation) {
	m_location.setData(translation, PG_DisaSpriteModel::Identity_Translation);
}

unsigned short PG_DisaActorSpriteNode::getCharacterId() const {
	return readData<unsigned short>(PG_DisaSpriteModel::Identity_Id);
}

void PG_DisaActorSpriteNode::setCharacterId(unsigned short id) {
	m_location.setData(id, PG_DisaSpriteModel::Identity_Id);
}

void PG_DisaActorSpriteNode::updateColorOffset() {
	if (!m_location && getCharacterId() != 2800) return;//must be geo symbol

	if (PG_DisaSpriteNode* node = findFirstChild<PG_DisaSpriteNode>("N_Sprite"); node && node->getSprite().isValid()) {
		node->setColorTableOffset(static_cast<int>(readData<EDisaGeoColor>(PG_DisaSpriteModel::Identity_GeoColor)));
	}
}