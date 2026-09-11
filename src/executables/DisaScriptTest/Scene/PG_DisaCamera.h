#pragma once
#include <Scene/PG_FlyCameraNode.h>


class PG_DisaCamera : public PG_FlyCameraNode {
	PG_OBJECT(PG_DisaCamera);
	PG_PARENT(PG_FlyCameraNode);
public:
	using PG_FlyCameraNode::PG_FlyCameraNode;

	void init() override;

protected:
	void onMove(float deltaTime) override;
	bool onMouseMotion(PG_MouseMotionEvent* e) override;
	bool onMouseWheel(PG_MouseWheelEvent* e)  override;
	bool onKey(PG_KeyEvent* key) override;
};
