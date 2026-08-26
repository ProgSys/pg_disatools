#pragma once

#include <Util/PG_File.h>
#include <Resources/PG_Texture.h>

namespace PG_FileDialog {
	PG_File save(const PG_Texture& texture, const std::string& fileName);
}