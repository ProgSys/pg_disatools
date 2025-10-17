#pragma once

#include <Util/PG_File.h>

class zip;
using zipUPtr = std::unique_ptr<zip, void(*)(zip*)>;

struct PG_DisaModInfo {
	std::string id;
	std::string name;
	std::string namespaceMod;

	PG_File zipFile;
	zipUPtr archive = { nullptr, [](zip* a) {} }; //is valid if archive is curretly open

	bool operator==(const PG_DisaModInfo& other) const { return id == other.id; }
};
