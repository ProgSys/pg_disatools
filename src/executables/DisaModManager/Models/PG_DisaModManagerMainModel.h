#pragma once
#include <Base/PG_Object.h>
#include <Disa/models/PG_DisaMainModel.h>
#include <Util/PG_Dir.h>

class PG_DisaModManagerMainModel : public PG_DisaMainModel {
	PG_OBJECT(PG_DisaModManagerMainModel);
	PG_PARENT(PG_DisaMainModel);
public:
	using PG_DisaMainModel::PG_DisaMainModel;

	void init() override;

	static PG_DisaModManagerMainModel* get();
};