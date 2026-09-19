#include "PG_DisaFileIcons.h"

std::string_view PG_DisaFileIcon::get(PG_DisaFileInfo::Type type) {
	switch (type) {
	case PG_DisaFileInfo::SOLA:
	case PG_DisaFileInfo::KM3:
	case PG_DisaFileInfo::OLA: return "gui/icon_folder.svg";
	case PG_DisaFileInfo::COLA: 
	case PG_DisaFileInfo::PSPFS_V1:
	case PG_DisaFileInfo::ZIP:	return "gui/disa/icon_file_archive.svg";
	case PG_DisaFileInfo::TX2:	return "gui/disa/icon_file_tx2.svg";
	case PG_DisaFileInfo::PNG:	return "gui/disa/icon_file_img.svg";
	case PG_DisaFileInfo::MPD:	return "gui/disa/icon_file_terrain.svg";
	case PG_DisaFileInfo::MPP:	return "gui/disa/icon_file_terrain_texture.svg";
	case PG_DisaFileInfo::SH:
	case PG_DisaFileInfo::SH2:	return "gui/disa/icon_file_sprite.svg";
	case PG_DisaFileInfo::GEO:	return "gui/icon_model.svg";
	case PG_DisaFileInfo::OGG:	return "gui/disa/icon_file_ogg.svg";
	default:					return generic;
	}
}
