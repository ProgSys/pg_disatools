#include "PG_DisaScriptTestModel.h"

#include <Disa/PG_DisaCharactersModel.h>
#include <Disa/fileformats/PG_DisaLooseFileContainer.h>
#include <Disa/fileformats/PG_OLA.h>
#include <Disa/fileformats/PG_PSPFS.h>
#include <Disa/fileformats/PG_Script.h>
#include <Disa/fileformats/PG_TalkScript.h>
#include <Disa/PG_DisaMapScene.h>
#include <Scene/PG_DisaCamera.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_StringUtil.h>

#include <Script/PG_DisaContext.h>
#include <Script/PG_DisaScriptExecutor.h>

constexpr auto gGameCameraName = "PG_DisaGameCameraNode";
constexpr auto gEditorCameraName = "PG_EditorCameraNode";

void PG_DisaScriptTestModel::init() {
	PG_DisaMainModel::init();
	m_mapModel = PG_new<PG_DisaMapModel>("PG_DisaMapModel", this);
	m_context = PG_new<PG_DisaContext>("PG_DisaContext", this);
	m_context->setMapModel(m_mapModel);
	m_scriptExecutor = PG_new<PG_DisaScriptExecutor>("PG_DisaScriptExecutor", m_context);

	connect(m_context, &PG_DisaContext::dungeonChanged, this, &PG_DisaScriptTestModel::openMapIntern);
	postCallEvent([this] { openMapIntern(m_context->getDungeonId()); });
}

bool PG_DisaScriptTestModel::openGameFiles() {
	const PG_File exePath = getExePath();
	if (exePath.isEmpty() || !exePath.exists()) {
		OUT_ERROR("No game executable! Set \"disaExe\" in config.ini to your Disgaea exe.");
		return FAILURE;
	}

	const std::string dir = exePath.getDir();

	m_dataDat = std::make_shared<PG_PSPFS>();
	if (!m_dataDat->PG_DisaFileContainer::open(PG_File(dir + "/DATA.DAT"))) {
		m_dataDat.reset();
		return FAILURE;
	}

	m_subdata = std::make_shared<PG_PSPFS>();
	if (!m_subdata->PG_DisaFileContainer::open(PG_File(dir + "/SUBDATA.DAT"))) {
		m_subdata.reset();
		return SUCCESS; //the maps are in DATA.DAT, only the actor sprites are lost
	}

	auto startDat = std::dynamic_pointer_cast<PG_OLA>(getReader(m_subdata, {
		VisitPair{"START.DAT", [](PG_DisaFileEntry& entry) ->std::shared_ptr<PG_DisaFileContainer> { return entry.open<PG_OLA>(); } },
		}));

	if (!startDat) {
		OUT_WARNING("Failed to find START.DAT, actors will show as cubes!");
		return SUCCESS;
	}

	/* DisaMapEditor decompresses this one and writes SUBDATA.DAT back out. A viewer has no business
	 * rewriting game files, so it goes without the character data instead. */
	if (startDat->isCompressed()) {
		OUT_WARNING("START.DAT is compressed, actors will show as cubes! Open the game once in DisaMapEditor.");
		return SUCCESS;
	}

	m_charactersModel = PG_new<PG_DisaCharactersModel>("PG_DisaCharactersModel", this);
	m_charactersModel->open(exePath, m_subdata, startDat);

	cachDisaGeometry(startDat.get(), "BASE.KM2", "R_DisaTeleporterBase");
	openScripts(startDat);
	return SUCCESS;
}

void PG_DisaScriptTestModel::openScripts(const std::shared_ptr<PG_OLA>& startDat) {
	if (auto entry = startDat->find(PG_File("SCRIPT.DAT"), false)) m_scripts = entry.open<PG_Script>();
	if (!m_scripts) OUT_WARNING("Failed to open SCRIPT.DAT, no script can be run!");

	if (auto entry = startDat->find(PG_File("TALK.DAT"), false)) m_talkScripts = entry.open<PG_TalkScript>();
	if (!m_talkScripts) OUT_WARNING("Failed to open TALK.DAT, no talk script can be run!");

	m_scriptExecutor->setContainers(m_scripts, m_talkScripts);
}

bool PG_DisaScriptTestModel::runScript(const PG_DisaIdentifier& scriptId) {
	if (!scriptId.isValid()) return FAILURE;
	return m_scriptExecutor->runScriptById(scriptId);
}

bool PG_DisaScriptTestModel::runTalkScript(const PG_DisaIdentifier& scriptId) {
	if (!scriptId.isValid()) return FAILURE;
	return m_scriptExecutor->runTalkScriptById(scriptId);
}

void PG_DisaScriptTestModel::useGameCamera(bool value) {
	assert(m_scene);
	PG_CameraNode* cam = m_scene->getRoot()->findFirstChild<PG_CameraNode>(value? gGameCameraName : gEditorCameraName);
	if (cam) m_scene->setActiveCamera(cam);
}

bool PG_DisaScriptTestModel::iaGameCamera() const {
	return m_scene && m_scene->getActiveCamera()->getName() == gGameCameraName;
}

void PG_DisaScriptTestModel::setScene(PG_DisaMapScene* scene) {
	m_scene = scene;
	if (!scene) return;
	auto disaCam = PG_new<PG_DisaCamera>(gGameCameraName, PG_CameraNode::Type_Perspective, scene->getRoot());
	scene->setActiveCamera(disaCam);
}

bool PG_DisaScriptTestModel::openMapFile(const std::string& mapName) {
	if (!m_dataDat) {
		OUT_ERROR("No game files are open, \"" << mapName << "\" can't be found!");
		return FAILURE;
	}

	PG_File given(mapName);
	if (given.getFileExtensionFormated() != "mpd") given += ".MPD";
	const PG_File mpdName = std::move(given);

	auto mpd = m_dataDat->find(mpdName, false);
	if (!mpd) {
		OUT_ERROR("Failed to find \"" << mpdName << "\" in \"" << m_dataDat->getFile() << "\"!");
		return FAILURE;
	}

	const PG_File mppName = mpdName.replaceFileExtension("mpp");
	auto mppSD = m_dataDat->find(mppName, false);
	if (!mppSD) OUT_WARNING("Failed to find the mpp of \"" << mpdName << "\", the map stays untextured!");
	auto mppHD = m_dataDat->find(PG_File("NEW_" + mppName.getFile()), false);

	//the model drops the textures of the last map inside open(), the old files may only go after that
	auto previousFiles = std::move(m_looseFiles);
	m_context->reset(); //the actors of the last map have nowhere to stand any more
	if (!m_mapModel->open(mpd, mppSD, mppHD)) return FAILURE;
	return SUCCESS;
}

bool PG_DisaScriptTestModel::openMapIntern(int id) {
	if (id < 0) return false;

	std::stringstream stream;
	stream << "MP" << std::setfill('0') << std::setw(5) << id;
	return openMapFile(stream.str());
}
