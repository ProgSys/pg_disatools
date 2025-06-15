#include "PG_DisaFileManagerMainModel.h"
#include <Disa/fileformats/PG_ZIP.h>
#include <Disa/fileformats/PG_PSPFS.h>
#include <Disa/fileformats/PG_OLA.h>
#include <Disa/fileformats/PG_DisaFileTests.h>
#include <Models/PG_DisaFileContainerModel.h>
#include <Stream/PG_StreamInByteFile.h>

static PG_DisaFileManagerMainModel* g_mainModel = nullptr;

void PG_DisaFileManagerMainModel::init() {
	PG_DisaMainModel::init();
	g_mainModel = this;
}

PG_DisaFileManagerMainModel* PG_DisaFileManagerMainModel::get() {
	return g_mainModel;
}

void PG_DisaFileManagerMainModel::close() {
	while (!m_modelStack.empty()) {
		delete dynamic_cast<PG_Object*>(m_modelStack.top());
		m_modelStack.pop();
	}
	emit centerModelChanged(nullptr);
}

bool PG_DisaFileManagerMainModel::open(const PG_File& file) {
	if (file.isEmpty()) {
		close();
		return false;
	}

	std::shared_ptr<PG::STREAM::InByteFile> reader(new PG::STREAM::InByteFile(file));
	if (!reader->isOpen()) {
		close();
		return false;
	}

	PG_UiStateBaseModel* newCenterModel = nullptr;
	if (PG_Disa::isPSPFS(file)) {
		auto model = std::shared_ptr<PG_PSPFS>(PG_new<PG_PSPFS>("PG_PSPFS"));
		if (!model->open(reader)) {
			close();
			return false;
		}
		model->setFile(file);
		auto container = PG_new<PG_DisaFileContainerModel>("PG_DisaFileContainerModel", this);
		container->open(model.get());
		container->addContainer(std::move(model));
		newCenterModel = container;
	}
	else if (PG_Disa::isOLA(file)) {
		auto model = std::shared_ptr<PG_OLA>(PG_new<PG_OLA>("PG_OLA"));
		if (!model->open(reader)) {
			close();
			return false;
		}
		model->setFile(file);
		auto container = PG_new<PG_DisaFileContainerModel>("PG_DisaFileContainerModel", this);
		container->open(model.get());
		container->addContainer(std::move(model));
		newCenterModel = container;
	}

	while (!m_modelStack.empty()) {
		delete dynamic_cast<PG_Object*>(m_modelStack.top());
		m_modelStack.pop();
	}

	if (!newCenterModel) {
		emit centerModelChanged(newCenterModel);
		return false;
	}
	m_modelStack.push(newCenterModel);
	emit centerModelChanged(newCenterModel);
	return true;
}

void PG_DisaFileManagerMainModel::pushStack(PG_UiStateBaseModel* model) {
	if (!model) return;
	m_modelStack.push(model);
	emit centerModelChanged(model);
}