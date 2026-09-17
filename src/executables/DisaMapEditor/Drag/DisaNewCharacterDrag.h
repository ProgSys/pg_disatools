#pragma once
#include <Base/PG_Object.h>
#include <Scene/Model/PG_ListModelLocation.h>


class PG_MouseMotionEvent;
class PG_DisaActorSpriteNode;
class TileMocModel;

class DisaNewCharacterDrag : public PG_Object {
	PG_OBJECT(DisaNewCharacterDrag);
public:
	using PG_Object::PG_Object;
	~DisaNewCharacterDrag() override;

	void start(unsigned short characterId);

	/**
	* If returns true, then event will not be propagate farther in tree.
	*/
	bool event(PG_Event* e);

private:
	void cleanUpMoved();
	void mouseMoved(PG_MouseMotionEvent* e);
	void released();
	void reset();

	PG_DisaActorSpriteNode* m_indicator = nullptr;
	TileMocModel* m_modelMoc = nullptr;
	bool m_dragging = false;
};
