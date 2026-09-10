#include "PG_DisaFileIcons.h"

#include <Util/PG_File.h>
#include <unordered_map>

namespace {
	const std::unordered_map<std::string, std::string> g_icons = {
		{ "TX2", "gui/disa/icon_file_tx2.svg" },
		{ "PNG", "gui/disa/icon_file_img.svg" },
		{ "DAT", "gui/disa/icon_file_archive.svg" },
		{ "MPD", "gui/disa/icon_file_terrain.svg" },
		{ "MPP", "gui/disa/icon_file_terrain_texture.svg" },
		{ "GEO", "gui/icon_model.svg" },
		{ "OGG", "gui/disa/icon_file_ogg.svg" },
	};

	const std::string g_generic = PG_DisaFileIcon::generic;
}

const std::string& PG_DisaFileIcon::get(const std::string& extension) {
	const auto findIt = g_icons.find(extension);
	return findIt == g_icons.end() ? g_generic : findIt->second;
}

const std::string& PG_DisaFileIcon::getFromName(const std::string& fileName) {
	return get(PG_File::getFileExtensionFormated(fileName, false));
}
