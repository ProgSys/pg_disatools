#include "PG_FileDialog.h"
#include <nfd.h>
#include <Base/PG_Application.h>
#include <Util/PG_StringUtil.h>


PG_File PG_FileDialog::open(const PG_Object* owner, const std::string& fileFormat) {
	PG_Application::grabMouse(const_cast<PG_Object*>(owner));
	nfdchar_t* outPath = nullptr;
	nfdresult_t result = NFD_OpenDialog(fileFormat.c_str(), nullptr, &outPath);
	std::string path;
	if (outPath) {
		path = outPath;
		delete[] outPath;
	}

	PG_EventLoop::doRun();
	PG_Application::grabMouse(nullptr);

	if (result == NFD_OKAY) {
		return PG_File(path);
	}
	else if (result != NFD_CANCEL) {
		OUT_ERROR("OpenDialog error: " << NFD_GetError());
	}
	return PG_File();
}

PG_File PG_FileDialog::save(const PG_Object* owner, const std::string& fileFormat, const std::string& fileName) {
	PG_Application::grabMouse(const_cast<PG_Object*>(owner));
	nfdchar_t* outPath = nullptr;
	nfduint_t fileTypeIndex = 0;
	nfdresult_t result = NFD_SaveDialog(fileFormat.c_str(), nullptr, fileName.c_str(), &outPath, &fileTypeIndex);
	std::string path;
	if (outPath) {
		path = outPath;
		delete[] outPath;
	}

	PG_EventLoop::doRun();
	PG_Application::grabMouse(nullptr);

	if (result == NFD_OKAY) {
		PG_File file = path;
		if (fileTypeIndex > 0 && file.getFileExtension().empty()) {
			--fileTypeIndex;
			auto formats = PG_StringUtil::split(fileFormat, ';');
			if (fileTypeIndex < formats.size()) {
				file += "." + formats[fileTypeIndex];
			}
		}
		return file;
	}
	else if (result != NFD_CANCEL) {
		OUT_ERROR("SaveDialog error: " << NFD_GetError());
	}
	return PG_File();
}