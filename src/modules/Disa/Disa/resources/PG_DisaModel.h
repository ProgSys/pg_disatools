#pragma once

#include <resources/PG_Model.h>
#include <Disa/fileformats/PG_GEO.h>

class PG_DisaModel : public PG_Model{
public:
	using PG_Model::PG_Model;

	void load(const std::string& modelName, const PG_GEO& geo);

private:
	static PG_Model::BasicDynamicModel parse(const std::string& modelName, const PG_GEO& geo);
};