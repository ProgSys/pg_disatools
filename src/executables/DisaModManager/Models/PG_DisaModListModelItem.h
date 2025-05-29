#pragma once

#include <functional>
#include <string>
#include <Resources/PG_Texture.h>
#include <Scene/Model/PG_ListModelBase.h>

struct PG_DisaModListModelItem {
	std::function<std::string()> title;
	std::function<std::string()> description;
	std::function<std::string()> author;
	std::function<PG_Texture()> previewImage;
	std::size_t order;
	std::shared_ptr<PG_ListModelBase> model;

	std::string toString() const { return title(); }
};
