#pragma once

#include <Util/PG_File.h>
#include <nlohmann/json.hpp>

struct PG_DisaModLoadInfo {
	std::string id; //mod id
	PG_File file;
	nlohmann::json json;

	bool operator==(const PG_DisaModLoadInfo& other) const { return id == other.id; }
};