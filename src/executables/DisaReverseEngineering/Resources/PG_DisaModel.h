#pragma once

#include <Resources/PG_Model.h>


class PG_DisaModel : public PG_Model {
public:
	using PG_Model::PG_Model;
	PG_DisaModel(const PG_Model& m) : PG_Model(m) {}
	PG_DisaModel(PG_DisaModel&& m) noexcept: PG_Model(m), additionalProperties(std::move(m.additionalProperties)){}

	std::map<std::string, PG_Any> additionalProperties;
};