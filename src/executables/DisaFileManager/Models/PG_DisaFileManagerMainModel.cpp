#include "PG_DisaFileManagerMainModel.h"
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
		delete dynamic_cast<PG_Object*>(m_modelStack.back());
		m_modelStack.pop_back();
	}
	emit centerModelChanged(nullptr);
}

void PG_DisaFileManagerMainModel::navBack() {
	if (m_modelStack.size() <= 1) return;
	popStack();
}

std::string PG_DisaFileManagerMainModel::getNavText() const {
	if (m_modelStack.empty()) return "No file is open.";
	std::stringstream ss;
	ss << m_modelStack.front()->getFileName();
	for (auto* item : m_modelStack | std::views::drop(1)) {
		ss << " > " << item->getFileName();
	}
	return ss.str();
}

bool PG_DisaFileManagerMainModel::open(const PG_File& file) {
	if (file.isEmpty()) {
		close();
		return false;
	}

	std::shared_ptr<PG::STREAM::In> reader(new PG::STREAM::InByteFile(file));
	if (!reader->isOpen()) {
		close();
		return false;
	}

	PG_UiStateBaseModel* newCenterModel = PG_DisaFileContainerModel::tryOpen(file, reader);

	while (!m_modelStack.empty()) {
		delete dynamic_cast<PG_Object*>(m_modelStack.back());
		m_modelStack.pop_back();
	}

	if (!newCenterModel) {
		emit centerModelChanged(newCenterModel);
		return false;
	}
	pushStack(newCenterModel);
	return true;
}

void PG_DisaFileManagerMainModel::pushStack(PG_UiStateBaseModel* model) {
	if (!model) return;
	m_modelStack.push_back(model);
	emit centerModelChanged(model);
}

void PG_DisaFileManagerMainModel::popStack() {
	if (m_modelStack.empty()) return;
	auto* top = m_modelStack.back();
	top->closeUi();
	m_modelStack.pop_back();
	delete dynamic_cast<PG_Object*>(top);
	emit centerModelChanged(getCenterModel());
}