#include <SDLApp/PG_SDLApplication.h>
#include <SDLApp/PG_SDLWindow.h>

#include <RootScene.h>
#include <Renderer/PG_SceneRenderer.h>
#include <PG_DisaScriptTestDefines.h>

#include <glm/glm.hpp>

int main(int argc, char* args[]) {
	PG_SDLApplication app;
	app.setAlwaysRender(true);
	PG_SDLWindow* window = PG_new<PG_SDLWindow>(APP_NAME, glm::ivec2(740, 480), &app);
	PG_SceneRenderer* renderer = PG_new<PG_SceneRenderer>("MainRenderer", window);
	RootScene* scene = PG_new<RootScene>("RootScene", renderer);

	app.exec();
	delete renderer;

	return 0;
}
