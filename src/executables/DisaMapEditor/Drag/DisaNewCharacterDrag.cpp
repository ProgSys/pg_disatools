#include "DisaNewCharacterDrag.h"
#include <Base/Events/PG_KeyEvent.h>
#include <Base/PG_Application.h>
#include <Base/Events/PG_MouseButtonEvent.h>
#include <Base/Events/PG_MouseMotionEvent.h>
#include <Scene/PG_CameraNode.h>
#include <Scene/Gui/PG_GuiItem.h>
#include <Scene/PG_FootlooseViewNode.h>
#include <Controllers/PG_DisaEditorActorController.h>


#include <Disa/PG_DisaCharactersModel.h>
#include <Disa/map/PG_DisaActorViewNode.h>
#include <Disa/map/PG_DisaTileViewNode.h>
#include <Disa/map/PG_DisaActorModel.h>
#include <Disa/PG_DisaMapModel.h>
#include <Disa/PG_DisaMapViewNode.h>
#include <RootScene.h>
#include <Views/PG_DisaMapSceneView.h>
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
		case PG_DisaActorModel::Identity_Translation:
			return p_position;
		case PG_DisaActorModel::Identity_TileGround:{
			if (!p_tile) return std::array<glm::vec3, 4>{glm::vec3(0.f)};
			auto out = p_tile->getTop(glm::vec3(0.f));
			for (glm::vec3& p : out) {
				p.y -= p_position.y;
			}
			return out;
		}
		case PG_DisaActorModel::Identity_Id:
			return p_characterId;
		case PG_DisaActorModel::Identity_Rotation:
			return p_rotation;
		case PG_DisaActorModel::Identity_GeoColor:
			return EDisaGeoColor::DisaGeoColor_None;
		}

		return PG_Variety();
	}

	PG_DisaMapTileModel* p_tile = nullptr;
	unsigned short p_characterId = 0;
	glm::vec3 p_position = glm::vec3(0.f);
	char p_rotation = 0;
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
	PG_DisaMapSceneView* gameScene = render->findFirstInTree<PG_DisaMapSceneView>();
	if (!gameScene) return;

	if (!m_indicator) m_indicator = PG_new<PG_DisaActorViewNode>("Indicator", gameScene->getRoot());
	m_indicator->setDragged(true);
	m_indicator->setVisible(false);
	if (!m_modelMoc) m_modelMoc = PG_new<TileMocModel>("TileMocModel", this);
	m_modelMoc->p_characterId = characterId;
	m_indicator->setLocation(PG_ListModelLocation(m_modelMoc, 0));

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
		case Event_MouseMotion:
		{
			auto me = (PG_MouseMotionEvent*)e;
			//if (me->getButton() == SDL_BUTTON_LEFT) 
			mouseMoved((PG_MouseMotionEvent*)e);
			e->accept();
			return true;
		}break;
		case Event_MouseButton:
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
		case Event_Key: {
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
	PG_DisaMapSceneView* gameScene = render->findFirstInTree<PG_DisaMapSceneView>();
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
	glm::vec2 pos = m_modelMoc->p_tile->getTilePosition();
	float height = m_modelMoc->p_tile->getTopHeight();
	m_modelMoc->p_position = glm::vec3(pos.x, height, pos.y);

	if (updateRotation) {
		const glm::vec3 camDir = cam->getForward();
		const float angle = PG_Math::angle(glm::normalize(glm::vec2(camDir.x, camDir.z)), glm::vec2(1.f, 0.f));

		if (angle < 0.f) {
			if (angle < glm::radians(-90.f)) {
				m_modelMoc->p_rotation = 2;
			}
			else {
				m_modelMoc->p_rotation = 1;
			}
		}
		else {
			if (angle < glm::radians(90.f)) {
				m_modelMoc->p_rotation = 0;
			}
			else {
				m_modelMoc->p_rotation = 0;
			}
		}
	}
	

	m_indicator->update(m_indicator->getLocation(), { PG_DisaActorModel::Identity_Translation , PG_DisaActorModel::Identity_TileGround });
	m_indicator->setVisible(true);
	m_indicator->updateModelMatrix();
}

void DisaNewCharacterDrag::released() {
	if (m_indicator->isVisible()) {
		auto* scene = dynamic_cast<PG_DisaMapSceneView*>(m_indicator->getScene());
		auto actors = scene->getMapModel()->getActors();



		mpdfileActor data;
		data.id = m_modelMoc->p_characterId;
		data.x = -static_cast<unsigned char>(m_modelMoc->p_position.x);
		data.z = static_cast<unsigned char>(m_modelMoc->p_position.z);

		unsigned int level = 0;
		for (const auto& actor : *actors) {
			if (actor->getData().x == data.x && actor->getData().z == data.z) {
				actor->setCharacterId(data.id);
				reset();
				return;
			}
			level += actor->getData().level;
		}
		level = std::round(((float)level) / std::max(actors->size(), (std::size_t)1));
		level = std::max(level, 1u);

		data.level = static_cast<unsigned short>(level);
		data.alliance = 1;
		data.rotation = m_modelMoc->p_rotation;
		data.visible = 0;
		data.ai = 1;
		data.unk5 = 0;
		data.unk6 = 0;
		std::memset(&data.items, 0, sizeof(unsigned short) * 4);
		data.appearance = 0;
		data.unk7 = 0;
		data.geo_effect = 0;
		data.unk8 = 0;
		std::memset(&data.magic, 0, sizeof(unsigned short) * 4);
		std::memset(&data.unk, 0, sizeof(char) * 30);
		data.unk[0] = 64;
		data.unk[1] = 32;
		data.unk[2] = 96;
		data.unk[3] = 96;

		auto model = PG_new<PG_DisaActorModel>("PG_DisaActorModel", actors);
		model->setData(data);
		actors->push_back(model);

		auto* view = scene->getMapNode()->getActorView();
		if (view->getItems().size() == actors->size()) {
			auto item = dynamic_cast<PG_DisaActorViewNode*>(view->getItems().back());
			item->setSpring(m_indicator->getSpring());
			auto copy = m_indicator->getChildren();
			for (auto* child : copy | PG_CastTo<PG_ModelNode*>) {
				if (auto found = item->findFirstChild(child->getName())) {
					child->setParent(item);
					delete found;
				}
			}
			item->updateModelMatrix();
			item->updateModelMatrixWorld();
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
