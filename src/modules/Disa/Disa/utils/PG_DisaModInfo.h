#pragma once

#include <Util/PG_File.h>

struct PG_DisaModInfo {
	std::string id;
	std::string name;
	std::string namespaceMod;

	PG_File zipFile;

	bool operator==(const PG_DisaModInfo& other) const { return id == other.id; }
};
