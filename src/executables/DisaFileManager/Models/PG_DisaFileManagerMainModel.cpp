#include "PG_DisaFileManagerMainModel.h"
#include <Models/PG_DisaFileContainerModel.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamOutByteFile.h>
#include <Util/PG_RangesTransform.h>

static PG_DisaFileManagerMainModel* g_mainModel = nullptr;

void PG_DisaFileManagerMainModel::init() {
	PG_DisaMainModel::init();
	g_mainModel = this;
}

PG_DisaFileManagerMainModel* PG_DisaFileManagerMainModel::get() {
	return g_mainModel;
}

void PG_DisaFileManagerMainModel::close() {
	emit centerModelChanged(getCenterModel(), nullptr);
	while (!m_modelStack.empty()) {
		delete dynamic_cast<PG_Object*>(m_modelStack.back());
		m_modelStack.pop_back();
	}
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

bool PG_DisaFileManagerMainModel::isModified() const {
	if (m_modelStack.empty()) return false;
	auto* asObject = dynamic_cast<PG_Object*>(m_modelStack.front());
	if (auto prop = asObject->getProperty("isModified")) {
		return asObject->getPropertyValue(prop).toBool();
	}
	return false;
}

bool PG_DisaFileManagerMainModel::open(const PG_File& file) {
	close(); //could be optimized, as centerModelChanged is emited twice
	if (file.isEmpty()) return false;
	
	std::shared_ptr<PG::STREAM::In> reader(new PG::STREAM::InByteFile(file));
	if (!reader->isOpen()) return false;
	
	PG_UiStateBaseModel* newCenterModel = PG_DisaFileContainerModel::tryOpen(file, reader);
	if (!newCenterModel) return false;
	auto* asObject = dynamic_cast<PG_Object*>(newCenterModel);
	if (auto prop = asObject->getProperty("isModified")) {
		connect(asObject, prop, this, &PG_DisaFileManagerMainModel::isModifiedChanged);
	}
	emit isModifiedChanged();

	pushStack(newCenterModel);
	return true;
}

void PG_DisaFileManagerMainModel::pushStack(PG_UiStateBaseModel* model) {
	if (!model) return;
	auto old = getCenterModel();
	m_modelStack.push_back(model);
	model->openUi();
	emit centerModelChanged(old, model);
}

void PG_DisaFileManagerMainModel::popStack() {
	if (m_modelStack.empty()) return;
	auto* top = m_modelStack.back();
	top->closeUi();
	m_modelStack.pop_back();
	emit centerModelChanged(top, getCenterModel());
	delete dynamic_cast<PG_Object*>(top);
}


void PG_DisaFileManagerMainModel::save() {
	if (m_modelStack.empty()) return;
	auto* fileContainer = dynamic_cast<PG_DisaFileContainerModel*>(m_modelStack.front());
	if (!fileContainer || !fileContainer->isModified()) return;
	saveAs(fileContainer->getOriginFile());
}
void PG_DisaFileManagerMainModel::saveAs(const PG_File& file) {
	if (m_modelStack.empty() || file.isEmpty()) return;
	auto* fileContainer = dynamic_cast<PG_DisaFileContainerModel*>(m_modelStack.front());
	if (!fileContainer) return;

	auto saveFile = file.replaceFileName(file.getName() + "_tempSave");
	auto writer = std::make_shared<PG::STREAM::OutByteFile>(saveFile);
	if (!writer->isOpen()) {
		OUT_ERROR("Failed to open file " << file << "!");
		return;
	}
	assert(fileContainer->getDisaFileContainer());
	if (!fileContainer->getDisaFileContainer()->save(writer)) {
		OUT_ERROR("Failed to save file " << file << "!");
		saveFile.remove();
		return;
	}
	writer.reset();

	if (file.exists()) {
		if (!file.remove()) {
			OUT_ERROR("Failed to remove original file " << file << "!");
			saveFile.remove();
			return;
		}
	}

	if (!saveFile.move(file)) {
		saveFile.remove();
		return;
	}

	if (file == fileContainer->getOriginFile()) {
		m_changedFiles.clear();
	}
	fileContainer->getDisaFileContainer()->refresh();
}