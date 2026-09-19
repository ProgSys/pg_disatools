#include "DisaNewCharacterDrag.h"
#include <Base/Events/PG_KeyEvent.h>
#include <Base/PG_Application.h>
#include <Base/Events/PG_MouseButtonEvent.h>
#include <Base/Events/PG_MouseMotionEvent.h>
#include <Scene/PG_CameraNode.h>
#include <Scene/Gui/PG_GuiItem.h>
#include <Scene/PG_FootlooseViewNode.h>
#include <Disa/map/controllers/PG_DisaEditorActorController.h>


#include <Disa/PG_DisaCharactersModel.h>
#include <Disa/map/PG_DisaActorsViewNode.h>
#include <Disa/map/PG_DisaActorSpriteNode.h>
#include <Disa/map/PG_DisaTileViewNode.h>
#include <Disa/map/PG_DisaActorListModel.h>
#include <Disa/map/PG_DisaSpriteModel.h>
#include <Disa/PG_DisaMapModel.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <RootScene.h>
#include <Disa/PG_DisaMapScene.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_Math.h>
#include <Util/PG_RangesTransform.h>

//@todo dublicate code
static inline void simulateMouseRelease() {
	if (auto target = PG_Application::get()->grabbedMouse()) {
		//simulate key release
		auto e = PG_new<PG_MouseButtonEvent>(0, true, PG_BUTTON_LEFT, 0, glm::ivec2(-1), glm::ivec2(-1), false, 0);
		target->event(e);//pretend like mouse was released
		delete e;
	}
}

class TileMocModel : public PG_ListModelBase {
public:
	using PG_ListModelBase::PG_ListModelBase;

	bool empty() const override { return false; };
	std::size_t size() const override { return 1; };

	PG_Variety getData(PG_Index, Identity identity) const override {
		switch (identity)
		{
		case PG_DisaSpriteModel::Identity_Translation:
			return p_position;
		case PG_DisaSpriteModel::Identity_TileGround:{
			if (!p_tile) return std::array<glm::vec3, 4>{glm::vec3(0.f)};
			//read against the indicator, which stands in the middle of the tile top
			auto out = p_tile->getTop(glm::vec3(-0.5f, p_tile->getTranslation().y, -0.5f));
			for (glm::vec3& p : out) {
				p.y -= p_position.y;
			}
			return out;
		}
		case PG_DisaSpriteModel::Identity_Id:
			return p_characterId;
		case PG_DisaSpriteModel::Identity_Facing:
			return static_cast<char>(p_facing);
		case PG_DisaSpriteModel::Identity_Rotation:
			return p_facing * 90.f;
		case PG_DisaSpriteModel::Identity_Visible:
			return true;
		case PG_DisaSpriteModel::Identity_ColorMult:
			return glm::vec4(1.f);
		case PG_DisaSpriteModel::Identity_GeoColor:
			return EDisaGeoColor::DisaGeoColor_None;
		case PG_DisaSpriteModel::Identity_Gravity:
			return 0.f; //the indicator hangs where the cursor puts it, it does not fall
		case PG_DisaSpriteModel::Identity_FloorHeight:
			return p_position.y;
		case PG_DisaSpriteModel::Identity_AnimationMode:
			return static_cast<int>(PG_DisaSpriteModel::Mode_Combat);
		case PG_DisaSpriteModel::Identity_AnimationSet:
			return static_cast<int>(p_aniSet);
		case PG_DisaSpriteModel::Identity_Animation:
			return 0;
		case PG_Identity_Item:
			return static_cast<PG_DisaActorModel*>(nullptr); //there is no model, only these answers
		}

		return PG_Variety();
	}

	PG_DisaMapTileModel* p_tile = nullptr;
	unsigned short p_characterId = 0;
	glm::vec3 p_position = glm::vec3(0.f);
	EDisaFacing p_facing = DisaFacing_East;
	PG_DisaSpriteModel::EAnimationSet p_aniSet = PG_DisaSpriteModel::IdleStanding;
};


DisaNewCharacterDrag::~DisaNewCharacterDrag() {
	if (m_indicator) delete m_indicator;
	if (m_modelMoc) delete m_modelMoc;
}

void DisaNewCharacterDrag::start(unsigned short characterId) {

	auto root = findParent<RootScene>();
	if (!root) return;
	auto render = root->getRoot()->findFirstInTree(PG_Search::byName{ "GridRenderItem" });
	if (!render) return;
	PG_DisaMapScene* gameScene = render->findFirstInTree<PG_DisaMapScene>();
	if (!gameScene) return;

	if (!m_indicator) m_indicator = PG_new<PG_DisaActorSpriteNode>("Indicator", gameScene->getRoot());
	m_indicator->setDragged(true);
	if (!m_modelMoc) m_modelMoc = PG_new<TileMocModel>("TileMocModel", this);
	m_modelMoc->p_characterId = characterId;
	m_indicator->setLocation(PG_ListModelLocation(m_modelMoc, 0));
	m_indicator->setVisible(false); //stays hidden until the first mouse move places it

	simulateMouseRelease();

	//take over mouse
	PG_Application::get()->grabMouse(this);
	m_dragging = true;
}

/**
* If returns true, then event will not be propagate farther in tree.
*/
bool DisaNewCharacterDrag::event(PG_Event* e) {
	if (m_dragging) {
		switch (e->getType()) {
		case PG_Event_MouseMotion:
		{
			auto me = (PG_MouseMotionEvent*)e;
			//if (me->getButton() == SDL_BUTTON_LEFT) 
			mouseMoved((PG_MouseMotionEvent*)e);
			e->accept();
			return true;
		}break;
		case PG_Event_MouseButton:
		{
			auto key = (PG_MouseButtonEvent*)e;
			if (key->getButton() == PG_BUTTON_LEFT) {
				m_dragging = false;
				postCallEvent(&DisaNewCharacterDrag::released);
			}
			else if (key->getButton() == PG_BUTTON_RIGHT) {
				//cancel
				m_dragging = false;
				postCallEvent(&DisaNewCharacterDrag::reset);
			}
			e->accept();
			return true;
		}break;
		case PG_Event_Key: {
			auto key = (PG_KeyEvent*)e;
			e->accept();
			return true;
		}break;

		default:break;
		}
	}
	return PG_Object::event(e);
}

void DisaNewCharacterDrag::cleanUpMoved() {
	if (m_indicator) delete m_indicator;
	m_indicator = nullptr;
	if (m_modelMoc) delete m_modelMoc;
	m_modelMoc = nullptr;
}

void DisaNewCharacterDrag::mouseMoved(PG_MouseMotionEvent* e) {
	auto root = findParent<RootScene>();
	if (!root) return;
	auto render = dynamic_cast<PG_GuiItem*>(root->getRoot()->findFirstInTree(PG_Search::byName{ "GridRenderItem" }));
	if (!render) return;
	PG_DisaMapScene* gameScene = render->findFirstInTree<PG_DisaMapScene>();
	if (!gameScene) return;
	auto* cam = gameScene->getActiveCamera();

	auto ray = cam->getRay(render->toLocalTranslation(e->getPosition()), render->getSize());

	auto tile = PG_DisaEditorActorController::findMinTile(gameScene, ray, true);
	if (!tile) {
		m_indicator->setVisible(false);
		return;
	}
	if (m_modelMoc->p_tile == tile->getTileModel()) return;
	const bool updateRotation = !m_modelMoc->p_tile;

	m_modelMoc->p_tile = dynamic_cast<PG_DisaMapTileModel*>(tile->getTileModel());
	const glm::vec3 center = PG_DisaActorModel::tileCenter(m_modelMoc->p_tile->getTilePosition());
	m_modelMoc->p_position = glm::vec3(center.x, m_modelMoc->p_tile->getTopHeight(), center.z);
	m_modelMoc->p_aniSet = PG_DisaSpriteModel::Hurt;

	if (updateRotation) {
		const glm::vec3 camDir = cam->getForward();
		const float angle = PG_Math::angle(glm::normalize(glm::vec2(camDir.x, camDir.z)), glm::vec2(1.f, 0.f));

		if (angle < 0.f) {
			if (angle < glm::radians(-90.f)) {
				m_modelMoc->p_facing = DisaFacing_West;
			}
			else {
				m_modelMoc->p_facing = DisaFacing_South;
			}
		}
		else {
			if (angle < glm::radians(90.f)) {
				m_modelMoc->p_facing = DisaFacing_East;
			}
			else {
				m_modelMoc->p_facing = DisaFacing_East;
			}
		}
	}
	

	m_indicator->update(m_indicator->getLocation(), { PG_DisaSpriteModel::Identity_Translation , PG_DisaSpriteModel::Identity_TileGround });
	m_indicator->setVisible(true);
	m_indicator->updateModelMatrix();
}

void DisaNewCharacterDrag::released() {
	if (m_indicator->isVisible()) {
		auto* scene = dynamic_cast<PG_DisaMapScene*>(m_indicator->getScene());
		auto actors = scene->getMapModel()->getActors();



		//the position is the middle of the tile now, so the file entry comes off the tile itself
		const glm::ivec2 tile = m_modelMoc->p_tile->getTilePosition();

		mpdfileActor data;
		data.id = m_modelMoc->p_characterId;
		data.x = static_cast<char>(-tile.x);
		data.z = static_cast<char>(tile.y);

		unsigned int level = 0;
		std::size_t counted = 0;
		for (auto* item : *actors) {
			//the list holds the base, only a unit can be replaced by another one
			auto* actor = dynamic_cast<PG_DisaSpriteModel*>(item);
			if (!actor) continue;

			if (actor->getTilePosition() == tile) {
				actor->setCharacterId(data.id);
				reset();
				return;
			}
			level += actor->getCharacterLevel();
			++counted;
		}
		//a new unit starts at the average level of the ones already placed
		level = std::round(((float)level) / std::max(counted, (std::size_t)1));
		level = std::max(level, 1u);

		data.level = static_cast<unsigned short>(level);
		data.alliance = 1;
		data.facing = m_modelMoc->p_facing;
		data.visible = 0;
		data.ai = 1;
		data.unk5 = 0;
		data.unk6 = 0;
		std::memset(&data.items, 0, sizeof(unsigned short) * 4);
		data.difficulty_spawn_rule = 0;
		data.geo_effect = 0;
		data.unk8 = 0;
		std::memset(&data.magic, 0, sizeof(unsigned short) * 4);
		std::memset(&data.magic_level, 0, sizeof(unsigned char) * 4);
		std::memset(&data.unk, 0, sizeof(char) * 26);

		/* The game puts a new unit in the first free entry of its actor array, and that array is
		 * the slot table, so a placed unit takes the smallest slot nobody holds. */
		const int freeSlot = actors->findFreeMapSlot();
		if (freeSlot < 0) {
			OUT_WARNING("Every one of the " << PG_DisaActorListModel::lastMapSlot + 1 << " unit slots is taken, the map can hold no more.");
			reset();
			return;
		}

		auto model = PG_new<PG_DisaSpriteModel>("PG_DisaSpriteModel", actors);
		model->setData(data);
		actors->setSlot(freeSlot, model);

		auto* view = scene->getMapNode()->getActorView();
		view->flush();
		if (view->getItems().size() == actors->size()) {
			auto item = dynamic_cast<PG_DisaActorSpriteNode*>(view->getItems().back());
			item->setForce(GravityForce()); //we released so it should fall to ground
			item->setMotion(m_indicator->getMotion());
			auto copy = m_indicator->getChildren();
			for (auto* child : copy | PG_CastTo<PG_ModelNode*>) {
				if (auto found = item->findFirstChild(child->getName())) {
					child->setParent(item);
					delete found;
				}
			}
			item->updateActor(); //make transition smooth
		}
	}

	reset();
}
void DisaNewCharacterDrag::reset() {
	//clean up
	cleanUpMoved();
	m_dragging = false;
	PG_Application::get()->grabMouse(nullptr);
}
