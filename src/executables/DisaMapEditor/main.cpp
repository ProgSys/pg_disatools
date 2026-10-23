
// SDL2 Hello, World!
// This should display a white screen for 2 seconds
// compile with: clang++ main.cpp -o hello_sdl2 -lSDL2
// run with: ./hello_sdl2

#include <stdio.h>

#include <SDLApp/PG_SDLApplication.h>
#include <Disa/resources/PG_DisaResourceLoaders.h>
#include <SDLApp/PG_SDLWindow.h>

#include <RootScene.h>
#include <Renderer/PG_SceneRenderer.h>

#include <glm/glm.hpp>
#include <string_view>
#include <Util/PG_Any.h>
#include <Base/PG_Variety.h>
#include <PG_DisaDefines.h>



int main(int argc, char* args[]) {
	PG_SDLApplication app;
	registerDisaResourceLoaders();
	app.setAlwaysRender(true);
	PG_SDLWindow* window = PG_new<PG_SDLWindow>(APP_NAME, glm::ivec2(740, 480), &app);
	PG_SceneRenderer* renderer = PG_new<PG_SceneRenderer>("MainRenderer", window);
	RootScene* scene = PG_new<RootScene>("RootScene", renderer);

	app.exec();
	delete renderer;

	return 0;
}

/*In case I want to start without console
#ifdef _WIN32
#include <windows.h>
int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	return main(__argc, __argv);
}
#endif
*/