#include "PG_DisaScriptTestModel.h"

#include <Disa/PG_DisaCharactersModel.h>
#include <Disa/fileformats/PG_DisaLooseFileContainer.h>
#include <Disa/fileformats/PG_OLA.h>
#include <Disa/fileformats/PG_PSPFS.h>
#include <Util/Debug/PG_DebugOut.h>

void PG_DisaScriptTestModel::init() {
	PG_DisaMainModel::init();
	m_mapModel = PG_new<PG_DisaMapModel>("PG_DisaMapModel", this);
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
	return SUCCESS;
}

bool PG_DisaScriptTestModel::openMap(const std::string& mapName) {
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
	return m_mapModel->open(mpd, mppSD, mppHD);
}

bool PG_DisaScriptTestModel::openMap(int id) {
	if (id < 0) return false;

	std::stringstream stream;
	stream << "MP" << std::setfill('0') << std::setw(5) << id;
	return openMap(stream.str());
}

bool PG_DisaScriptTestModel::openMap(const PG_File& file) {
	PG_File given = file;
	if (file.getFileExtensionFormated() == "mpp") {
		std::string name = file.getName();
		if (file.toUpper().getFile().starts_with("NEW_")) name.erase(0, 4); //the HD textures
		given = file.replaceFileNameAndExtension(name + ".mpd");
	}
	const PG_File mpdFile = std::move(given);

	auto files = std::make_shared<PG_DisaLooseFileContainer>();
	auto mpd = files->add(mpdFile);
	if (!mpd) return FAILURE;

	const PG_File mppFile = mpdFile.replaceFileExtension("mpp");
	auto mppSD = mppFile.exists() ? files->add(mppFile) : PG_DisaFileEntry();
	if (!mppSD) OUT_WARNING("Failed to find the mpp of \"" << mpdFile << "\", the map stays untextured!");

	const PG_File mppHDFile = mppFile.replaceFileNameAndExtension("NEW_" + mppFile.getFile());
	auto mppHD = mppHDFile.exists() ? files->add(mppHDFile) : PG_DisaFileEntry();

	auto previousFiles = std::move(m_looseFiles);
	m_looseFiles = std::move(files);
	return m_mapModel->open(mpd, mppSD, mppHD);
}
