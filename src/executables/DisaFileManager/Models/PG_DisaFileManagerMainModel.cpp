#include "PG_DisaFileManagerMainModel.h"
#include <Disa/fileformats/PG_ZIP.h>
#include <Disa/fileformats/PG_PSPFS.h>
#include <Scene/Model/PG_ListSelectionWrapperModel.h>

static PG_DisaFileManagerMainModel* g_mainModel = nullptr;

void PG_DisaFileManagerMainModel::init() {
	PG_DisaMainModel::init();
	g_mainModel = this;
}

PG_DisaFileManagerMainModel* PG_DisaFileManagerMainModel::get() {
	return g_mainModel;
}

bool PG_DisaFileManagerMainModel::open(const PG_File& file) {
	if (m_centerModel) return false; //TODO

	auto model = PG_new<PG_PSPFS>("PG_PSPFS", this);
	if (!model->PG_DisaFileContainer::open(file)) {
		delete model;
		return false;
	}
	m_centerModel = model;
	if (!m_selectionModel) {
		m_selectionModel = PG_new<PG_ListSelectionWrapperModel>("PG_ListSelectionWrapperModel", this);
	}
	reinterpret_cast<PG_ListSelectionWrapperModel*>(m_selectionModel)->setModel(m_centerModel);
	emit centerModelChanged(m_selectionModel);
	return true;
}