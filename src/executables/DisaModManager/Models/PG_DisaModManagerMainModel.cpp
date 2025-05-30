#include "PG_DisaModManagerMainModel.h"

static PG_DisaModManagerMainModel* g_mainModel = nullptr;

void PG_DisaModManagerMainModel::init() {
	PG_DisaMainModel::init();
	g_mainModel = this;
}

PG_DisaModManagerMainModel* PG_DisaModManagerMainModel::get() {
	return g_mainModel;
}