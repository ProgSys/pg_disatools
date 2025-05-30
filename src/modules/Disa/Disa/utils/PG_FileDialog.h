#pragma once

#include <Util/PG_File.h>
class PG_Object;

namespace PG_FileDialog {
	PG_File open(const PG_Object* owner, const std::string& fileFormat);

	PG_File save(const PG_Object* owner, const std::string& fileFormat, const std::string& fileName);
}