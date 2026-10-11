#include "PG_DisaFileIcons.h"

std::string_view PG_DisaFileIcon::get(PG_DisaFileInfo::Type type) {
	switch (type) {
	case PG_DisaFileInfo::SOLA:
	case PG_DisaFileInfo::KM3:
	case PG_DisaFileInfo::OLA: return "I_icon_folder";
	case PG_DisaFileInfo::COLA: 
	case PG_DisaFileInfo::PSPFS_V1:
	case PG_DisaFileInfo::ZIP:	return "I_disa_icon_file_archive";
	case PG_DisaFileInfo::TX2:	return "I_disa_icon_file_tx2";
	case PG_DisaFileInfo::PNG:	return "I_disa_icon_file_img";
	case PG_DisaFileInfo::MPD:	return "I_disa_icon_file_terrain";
	case PG_DisaFileInfo::MPP:	return "I_disa_icon_file_terrain_texture";
	case PG_DisaFileInfo::SH:
	case PG_DisaFileInfo::SH2:	return "I_disa_icon_file_sprite";
	case PG_DisaFileInfo::GEO:	return "I_icon_model";
	case PG_DisaFileInfo::OGG:	return "I_disa_icon_file_ogg";
	default:					return generic;
	}
}
