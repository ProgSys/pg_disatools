#pragma once

#include <Util/PG_File.h>
class PG_Object;

PG_File openDialog(const PG_Object* owner, const std::string& fileFormat);

PG_File saveDialog(const PG_Object* owner, const std::string& fileFormat, const std::string& fileName);