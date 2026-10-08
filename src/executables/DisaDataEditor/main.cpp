#include <SDLApp/PG_SDLApplication.h>
#include <SDLApp/PG_SDLWindow.h>
#include <Renderer/PG_SceneRenderer.h>
#ifdef DISA_DATA_EDITOR_MCP
#include <MCP/PG_MCPServer.h>
#endif

#include <RootScene.h>

#include <cstring>

#define APP_NAME "Disa Data Editor"

/// The first argument that isn't an option is the file to open on start
static std::string findFileArgument(int argc, char* args[]) {
	for (int i = 1; i < argc; ++i) {
		if (std::strncmp(args[i], "--", 2) != 0) return args[i];
		//skip the optional port behind --mcp
		if (std::strcmp(args[i], "--mcp") == 0 && i + 1 < argc && std::isdigit((unsigned char)args[i + 1][0])) ++i;
	}
	return {};
}

int main(int argc, char* args[]) {
	PG_SDLApplication app;
#ifdef DISA_DATA_EDITOR_MCP
	if (auto port = PG_MCPServer::portFromArgs(argc, args)) PG_new<PG_MCPServer>(*port, &app);
#endif

	app.setAlwaysRender(true);
	PG_SDLWindow* window = PG_new<PG_SDLWindow>(APP_NAME, glm::ivec2(1280, 800), &app);
	PG_SceneRenderer* renderer = PG_new<PG_SceneRenderer>("MainRenderer", window);
	RootScene* scene = PG_new<RootScene>("RootScene", renderer);
	PG_Object::connect(scene, &RootScene::titleChanged, window, [window](std::string title) { window->setTitle(title); });

	if (const std::string file = findFileArgument(argc, args); !file.empty()) scene->open(file);

	app.exec();
	delete renderer;

	return 0;
}
