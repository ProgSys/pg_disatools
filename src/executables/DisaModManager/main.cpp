
// SDL2 Hello, World!
// This should display a white screen for 2 seconds
// compile with: clang++ main.cpp -o hello_sdl2 -lSDL2
// run with: ./hello_sdl2

#include <stdio.h>

#include <SDLApp/PG_SDLApplication.h>
#include <SDLApp/PG_SDLWindow.h>

#include <RootScene.h>
#include <Renderer/PG_SceneRenderer.h>

#define APP_NAME "Disa Mod Manager v0.3.6"

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
