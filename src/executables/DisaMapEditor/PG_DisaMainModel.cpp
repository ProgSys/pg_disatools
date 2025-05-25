#include "PG_DisaMainModel.h"

#include <Base/Thread/PG_Thread.h>
#include <PG_DisaMapListModel.h>

#include <Disa/PG_DisaCharactersModel.h>
#include <Disa/PG_DisaItemModel.h>
#include <Disa/PG_DisaGeoEffectsModel.h>
#include <Disa/fileformats/PG_PSPFS.h>
#include <Disa/fileformats/PG_OLA.h>
#include <Disa/fileformats/PG_SOLA.h>
#include <Disa/fileformats/PG_SH.h>
#include <Disa/fileformats/PG_GEO.h>
#include <Disa/fileformats/PG_ShiftJisEncoder.h>
#include <Disa/resources/PG_DisaModel.h>
#include <PG_DisaDefines.h>


#include <Util/PG_Exception.h>
#include <Models/PG_OverviewListModel.h>
#include <Stream/PG_StreamInOutByteVector.h>


static PG_DisaMainModel* g_mainModel = nullptr;

/**
* @brief Holds some globaly cashed resourses
*/
struct PG_DisaMainModelResources: public PG_Object {
	using PG_Object::PG_Object;

	PG_DisaModel teleporterModel;
};

void PG_DisaMainModel::init() {
	try {
		m_config.open("config.ini");
	}
	catch (const PG_Exception& ext) {
		OUT_ERROR(ext.what());
	}
	catch (...) {
		OUT_ERROR("Failed to read or find \"config.ini\"!");
	}
	g_mainModel = this;
}

PG_DisaMainModel* PG_DisaMainModel::get() {
	return g_mainModel;
}

PG_File PG_DisaMainModel::getExePath() const {
	return m_config.getValue<std::string>("disaExe", "");
}
void PG_DisaMainModel::setExePath(const PG_File& path) {
	m_config.setValue<std::string>("disaExeVersion", APP_VERSION);
	m_config.setValue<std::string>("disaExe", path);
}

PG_Dir PG_DisaMainModel::getModDir() const {
	auto exePath = getExePath();
	return exePath.getDir() + "/mod";
}
PG_Dir PG_DisaMainModel::getCustomCampaignsDir() const {
	auto exePath = getExePath();
	return exePath.getDir() + "/mod/CustomCampaigns";
}

void PG_DisaMainModel::back() {
	//TODO check save state
	popStateStack();
}

void PG_DisaMainModel::openExe() {
	auto file = getExePath();
	auto dir = file.getDir();
	PG_File dataFile(dir + "/DATA.DAT");
	if (!dataFile.exists()) return;

	if (!m_charactersModel) {
		m_charactersModel = PG_new<PG_DisaCharactersModel>("PG_DisaCharactersModel", this);
	}
	if (!m_itemsModel) {
		m_itemsModel = PG_new<PG_DisaItemsModel>("PG_DisaItemsModel", this);
	}
	if (!m_geoEffectsModel) {
		m_geoEffectsModel = PG_new<PG_DisaGeoEffectsModel>("PG_DisaGeoEffectsModel", this);
	}

	PG_File subdataFile(dir + "/SUBDATA.DAT");
	OUT_INFO("Reading subdata: " << subdataFile);
	if (!subdataFile.exists()) return;

	if (m_PSPFS) {
		m_PSPFS->clear();
		m_PSPFS.reset();
	}
	m_PSPFS = std::make_shared<PG_PSPFS>();
	m_PSPFS->PG_DisaFileContainer::open(subdataFile);

	auto startDat = std::dynamic_pointer_cast<PG_OLA>(getReader(m_PSPFS, {
		VisitPair{"START.DAT", [](PG_DisaFileEntry& entry) ->std::shared_ptr<PG_DisaFileContainer> { return  entry.open<PG_OLA>(); } },
	}));

	if (startDat && startDat->isCompressed()) {
		OUT_INFO("START.DAT is compressed. Starting decompression!");
		class DecompressClass : public PG_Object {
		public:
			using PG_Object::PG_Object;
			void run() {
				assert(startDat);
				PG::STREAM::InOutByteVector out;
				if (!startDat->uncompress(&out) || out.getData().empty()) {
					startDat.reset();
					exit(1);
					return;
				}
				startDat.reset();//close current

				auto fileEntry = PSPFS->find(PG_File("START.DAT"), false);
				if (!fileEntry) {
					OUT_ERROR("START.DAT failed to refind?!");
					return;
				}
				fileEntry.getInfo().setDecompressedSize(0);
				auto container = fileEntry.open<PG_DisaChangedDataContainer>(std::move(out.getData()));
				PSPFS->set(fileEntry.getLocation(), fileEntry);
				PSPFS->save();
				PSPFS->PG_DisaFileContainer::open(PSPFS->getFile());
			}
			float progress = 0.0F;
			PG_DisaMainModel* mainModel;
			std::shared_ptr<PG_OLA> startDat;
			std::shared_ptr<PG_PSPFS> PSPFS;
		};



		emit displayProgress("Decompressing START.DAT", -1.f);

		auto decompressThread = PG_new<PG_Thread>("DecompressThread", this);
		DecompressClass* decompressClass = PG_new<DecompressClass>("DecompressClass", decompressThread);


		decompressClass->mainModel = this;
		decompressClass->startDat = std::move(startDat);
		decompressClass->PSPFS = m_PSPFS;
		decompressThread->create();
		decompressThread->giveToThread(decompressClass->startDat.get());
		decompressThread->giveToThread(decompressClass);

		connect(decompressClass->startDat.get(), &PG_OLA::decompresProgress, this, [this](float progress) {
			emit displayProgress("Update", progress);
			}, ConnectionType_Event);
		connect(decompressThread, &PG_Thread::done, this, [this, decompressThread, file]() {
			emit displayProgress("", -1.f);

			auto startDat = m_PSPFS->find(PG_File("START.DAT"), false).open<PG_OLA>();
			m_charactersModel->open(file, m_PSPFS, startDat);
			m_itemsModel->open(startDat);
			m_geoEffectsModel->open(startDat);
			loadDisaMainModelResources(startDat.get());

			//main Data.dat
			auto model = PG_new<PG_OverviewListModel>("PG_OverviewListModel", this);
			model->reload();
			pushStateStack(std::shared_ptr<PG_UiStateBaseModel>(model, [](PG_UiStateBaseModel*) {}));
			decompressThread->deleteLater();
			}, ConnectionType_Event);
		
		decompressThread->start();
		decompressClass->postCallEvent(&DecompressClass::run);
		std::this_thread::sleep_for(std::chrono::microseconds{100});
		decompressThread->postCallEvent(&PG_Thread::stop);

		return;
	}

	
	m_charactersModel->open(file, m_PSPFS, startDat);
	m_itemsModel->open(startDat);
	m_geoEffectsModel->open(startDat);
	loadDisaMainModelResources(startDat.get());

	//main Data.dat
	auto model = PG_new<PG_OverviewListModel>("PG_OverviewListModel", this);
	model->reload();
	pushStateStack(std::shared_ptr<PG_UiStateBaseModel>(model, [](PG_UiStateBaseModel*) {}));
}

PG_UiStateBaseModel::UiState PG_DisaMainModel::getUiState() const {
	if (m_modelStack.empty()) return PG_UiStateBaseModel::UiState::Empty;
	return m_modelStack.back()->getUiState();
}

void PG_DisaMainModel::setGridModel(PG_ListModelBase* model) {
	if (m_gridModel == model) return;
	m_gridModel = model;
	emit gridModelChanged(m_gridModel);
}

void PG_DisaMainModel::loadDisaMainModelResources(PG_OLA* startDat) {
	assert(startDat);
	if (!m_disaMainModelResources) {
		m_disaMainModelResources = PG_new<PG_DisaMainModelResources>("PG_DisaMainModelResources", this);
	}

	if (auto entry = startDat->find(PG_File("BASE.KM2"), false)) {
		auto reader = entry.createReader();
		PG_GEO geo;
		geo.open(reader.get());
		m_disaMainModelResources->teleporterModel.load("R_DisaTeleporterBase", geo);
	}

}

constexpr auto uiStateChnagedStr = "void uiStateChanged()";

void PG_DisaMainModel::pushStateStack(std::shared_ptr<PG_UiStateBaseModel> model) {
	assert(model);
	if (!m_modelStack.empty()) { //disconnect from uiStateChanged
		auto obj = getCurrentObject();
		if (auto func = obj->getFunction(uiStateChnagedStr)) {
			disconnect(obj.get(), func, this, nullptr);
		}
	}

	const auto uiState = getUiState();
	m_modelStack.push_back(model);
	model->openUi();
	setGridModel(model->getGridModel());
	const auto uiStateNew = getUiState();
	if (uiState != uiStateNew) {
		emit uiStateChanged(uiStateNew);
	}

	{ //connect to uiStateChanged
		auto obj = getCurrentObject();
		if (auto func = obj->getFunction(uiStateChnagedStr)) {
			connect(obj.get(), func, this, [this] {
				emit uiStateChanged(getUiState());
				}, ConnectionType_Event);
		}
	}
}

void PG_DisaMainModel::popStateStack() {
	if (m_modelStack.empty()) return;
	const auto uiState = getUiState();
	auto old = std::move(m_modelStack.back());
	old->closeUi();
	m_modelStack.pop_back();
	if (m_modelStack.empty()) return;
	setGridModel(m_modelStack.back()->getGridModel());
	const auto uiStateNew = getUiState();
	old.reset();
	if (uiState != uiStateNew) {
		emit uiStateChanged(uiStateNew);
	}

	{ //connect to uiStateChanged
		auto obj = getCurrentObject();
		if (auto func = obj->getFunction(uiStateChnagedStr)) {
			connect(obj.get(), func, this, [this] {
				emit uiStateChanged(getUiState());
				}, ConnectionType_Event);
		}
	}
}