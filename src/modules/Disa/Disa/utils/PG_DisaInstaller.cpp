#include "PG_DisaInstaller.h"

#include <Base/PG_Application.h>
#include <Resources/PG_ResourceManager.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_StringUtil.h>
#include <Util/PG_MD5.h>
#include <Util/PG_Dir.h>
#include <Stream/PG_StreamInByteFile.h>

#include <Disa/customGui/PG_PupUpDialog.h>
#include <Disa/fileformats/PG_Exe.h>
#include <Disa/fileformats/PG_ZIP.h>
#include <Script/CScript/PG_CScript.h>

void PG_DisaInstaller::runInstall(std::string version, PG_File exePath) {
	if (isDeleteLater()) return;
	connect(this, &PG_DisaInstaller::done, this, &PG_ActorNode::deleteLater, ConnectionType_Event);

	if (exePath.isEmpty() || !exePath.exists()) {
		runFreshInstall();
		return;
	}

	if (version != DISA_ISNTALL_APP_VERSION) {
		OUT_INFO("Config version number dosn't match, running install!");
	}

	const std::string errorMsg = execInstall(exePath, false);
	if (!errorMsg.empty()) {
		//handle error
		OUT_ERROR(errorMsg);
		PG_PopUpDialog* errorPopup = PG_PopUpDialog::createError(this,
			"Error",
			"Installation of version " DISA_ISNTALL_APP_VERSION " failed!\nReason: " + errorMsg
		);
		errorPopup->open();
		connect(errorPopup, &PG_PopUpDialog::confirmClicked, errorPopup, [] {
			PG_EventLoop::postEvent(new PG_Event(Event_Quit), PG_Application::get());
			});
		return;
	}

	emit done(DISA_ISNTALL_APP_VERSION, exePath);
}

void PG_DisaInstaller::runFreshInstall() {
	PG_PopUpDialog* installPopup = PG_PopUpDialog::createSelectFile(this,
		"Please select Disa exe",
		"The exe will be modefied with DLL injection!\nThe editor requeres a valid path to the Disa exe to proceed.\nIn Steam you can find it under \"Mange -> Browse local files\".",
		"Modify exe",
		"Close editor"
	);

	connect(installPopup, &PG_PopUpDialog::cancelClicked, installPopup, [] {
		PG_EventLoop::postEvent(new PG_Event(Event_Quit), PG_Application::get());
		});

	connect(installPopup, &PG_PopUpDialog::confirmClickedPath, this, [this, installPopup](PG_File inputFile) {
		OUT_INFO("Path defined, running install!");
		const std::string errorMsg = execInstall(inputFile, true);
		if (!errorMsg.empty()) {
			//handle error
			OUT_ERROR(errorMsg);
			installPopup->close();
			PG_PopUpDialog* errorPopup = PG_PopUpDialog::createError(this,
				"Error",
				"Installation failed!\nReason: " + errorMsg
			);
			errorPopup->open();
			connect(errorPopup, &PG_PopUpDialog::confirmClicked, errorPopup, [] {
				PG_EventLoop::postEvent(new PG_Event(Event_Quit), PG_Application::get());
				});

			return;
		}

		installPopup->close();
		PG_PopUpDialog* okPopup = PG_PopUpDialog::createSuccess(getParent<PG_GuiItem>(),
			"Success",
			"Installation successful!"
		);

		okPopup->open();
		emit done(DISA_ISNTALL_APP_VERSION, inputFile);
		});

	installPopup->open();
}

static inline PG_MD5 readCheckSum(PG::STREAM::InByteFile& reader) {
	reader.seek(0);

	PG_MD5 md5;
	unsigned int totalSize = reader.size();
	std::vector<char> data;
	data.resize(md5.blocksize);

	while (true) {
		reader.readBytes(data, std::min(totalSize, md5.blocksize));
		md5.add(data);
		if (totalSize > md5.blocksize) totalSize -= md5.blocksize;
		else break;
	}
	md5.finish();
	reader.seek(0);
	return md5;
}

std::string PG_DisaInstaller::execInstall(PG_File exePath, bool doUnzip) {
	if (exePath.isEmpty()) {
		return "Path empty!";
	}

	if (!exePath.exists()) {
		return "File dosn't exist!";
	}

	if (exePath.getFileExtensionFormated() != "exe") {
		return "File has wrong extenstion! Must be 'exe'!";
	}

	PG_File outputFile = exePath;
	outputFile.replaceFileName(outputFile.getName() + "_instalmod");
	if (outputFile.exists()) {
		outputFile.remove();
	}

	//looks kinda critical
	if (PG_StringUtil::contains(outputFile.getPath(), "windows", false)) {
		return "Path contains 'windows'?!";
	}

	if (PG_StringUtil::contains(outputFile.getPath(), "system32", false)) {
		return "Path contains 'system32'?!";
	}

	PG::STREAM::InByteFile reader(exePath);
	if (!reader.isOpen() || reader.isEmpty()) {
		return "Failed to read file! Maybe read/write protected?";
	}
	if (reader.readString(2) != "MZ") {
		return "Invalid magic number! Must be 'MZ'!";
	}
	reader.seek(0);

	PG_MD5 md5 = readCheckSum(reader);
	OUT_INFO("Input MD5 checksum: " << md5.getHash());

	PG_Exe exe;
	bool isPerformUnzip = false;

	std::string checkSumsBefore = "1235eb61e0f31ad211b1fc441bef121d";
	std::string checkSumsAfter = "c4ebae3bc277fe4198274a70dbb1fe6d";
	std::string error;

	PG_CScript installScript;
	installScript.getContext().add("md5checkhash", [&installScript, &checkSumsBefore, &checkSumsAfter, &md5](const std::string& before, const std::string& after) {
		checkSumsBefore = before;
		checkSumsAfter = after;
		if (checkSumsAfter == md5.getHash()) {
			installScript.forceStop(); //already installed
		}
		});

	installScript.getContext().add("stopwhennochangeneeded", [&installScript, &checkSumsAfter, &md5]() {
		if (checkSumsAfter == md5.getHash()) {
			installScript.forceStop();
		}
		});

	installScript.getContext().add("addsection", [&exe](const std::string& name, int misc,
		int virtualAddress, int sizeOfRawData,
		int pointerToRawData, int characteristics) {
			if (exe.isEmpty()) return;
			if (name.empty() || name.front() != '.') return;
			SectionHeader section{
				.misc = (unsigned int)misc,
				.virtualAddress = (unsigned int)virtualAddress,
				.sizeOfRawData = (unsigned int)sizeOfRawData,
				.pointerToRawData = (unsigned int)pointerToRawData,
				.pointerToRelocations = 0,
				.pointerToLineNumbers = 0,
				.numberOfRelocations = 0,
				.numberOfLineNumbers = 0,
				.characteristics = (unsigned int)characteristics,
			};
			section.name.fill(0);
			memcpy(section.name.data(), name.data(), std::min(name.size(), section.name.size()));

			exe.addSectionHeader(section);
		});

	installScript.getContext().add("override", [&exe](int addressU, const std::string& hex) {
		if (exe.isEmpty()) return;
		unsigned int address = addressU;
		auto data = PG_StringUtil::hexToData(hex);
		if (data.empty()) return;

		unsigned int headerAddress = exe.dosHeader.size() + exe.dosStub.size() + sizeof(NtHeaderStup);
		if (headerAddress <= address && (address + data.size()) < headerAddress + exe.ntHeaderRest.size()) {
			memcpy(exe.ntHeaderRest.data() + (address - headerAddress), data.data(), data.size());
			return;
		}

		if (exe.inbetweenAddress <= address && (address + data.size()) < exe.inbetweenAddress + exe.inbetween.size()) {
			memcpy(exe.inbetween.data() + (address - exe.inbetweenAddress), data.data(), data.size());
			return;
		}
		});

	installScript.getContext().add("addtoreloc", [&exe](int physicalAddress) {
		if (exe.isEmpty()) return;
		unsigned int address = physicalAddress;
		exe.addToRelocTable(address);
		});

	installScript.getContext().add("addtoend", [&exe](const std::string& hex) {
		if (exe.isEmpty()) return;
		auto data = PG_StringUtil::hexToData(hex);
		if (data.empty()) return;

		exe.dataAfter.insert(exe.dataAfter.end(), data.begin(), data.end());
		});

	installScript.getContext().add("addzerotoend", [&exe](int size) {
		if (exe.isEmpty() || size <= 0 || size > 10000) return;
		exe.dataAfter.resize(exe.dataAfter.size() + size, 0);
		});

	installScript.getContext().add("removejdll", [&isPerformUnzip, &exe, &exePath]() {
		if (!isPerformUnzip || !exe.isEmpty()) return;
		PG_File jDll(exePath.getDir() + "/j.dll");
		jDll.remove();
		});

	installScript.getContext().add("unzipfiles", [&isPerformUnzip, &installScript, &error, &exePath]() {
		if (!isPerformUnzip) return;
		OUT_INFO("Unzipping files");
		PG_File zipFile(PG_ResourceManager::resourceLocation() + "/script/disa/disaInstall.zip");
		if (!zipFile.exists()) {
			OUT_ERROR("Failed to find installation files at \"" << zipFile.getPath() << "\"!");
			error = "Failed to find installation files at\n\"" + zipFile.getPath() + "\"!";
			installScript.forceStop();
			return;
		}

		PG_Dir::createDir(exePath.getDir() + "/mod");
		if (!PG_ZIP::unizipToFolder(zipFile, exePath.getDir())) {
			OUT_ERROR("Failed to unzip \"" << zipFile << "\"!");
			installScript.forceStop();
			return;
		}
		});

	installScript.parse(PG_File(PG_ResourceManager::resourceLocation() + "/script/disa/disaInstall.cscript"));
	installScript.execute();

	if (!error.empty()) {
		return error;
	}

	if (md5.getHash() != checkSumsBefore) {
		if (md5.getHash() == checkSumsAfter) {
			OUT_INFO("The checksum matches the target, indicating the mod is already installed!");
			if (!doUnzip) return "";
			OUT_INFO("Running install script! (" << installScript.size() << ")");

			isPerformUnzip = true;
			installScript.execute();
			return "";
		}
		return "Invalid input checksum! The executable does not match the expected file before installation.";
	}

	if (!exe.open(&reader)) {
		return "Failed to parse exe!";
	}
	reader.close();

	isPerformUnzip = doUnzip;
	OUT_INFO("Running install script! (" << installScript.size() << ")");
	installScript.execute();

	if (!error.empty()) {
		return error;
	}

	if (!exe.isEmpty() && !exe.save(outputFile)) {
		return "Failed to save output file!";
	}
	exe.clear();
	if (!reader.open(outputFile)) {
		return "Failed to open output file!";
	}
	if (reader.readString(2) != "MZ") {
		outputFile.remove();
		return "Invalid magic number in output! Must be 'MZ'!";
	}

	//check output
	md5 = readCheckSum(reader);
	reader.close();
	OUT_INFO("Output MD5 checksum: " << md5.getHash());

	if (md5.getHash() != checkSumsAfter) {
		outputFile.remove();
		return "Invalid output checksum!";
	}

	//rename old
	PG_File renamedOrgFile = exePath;
	renamedOrgFile.replaceFileName(renamedOrgFile.getName() + "_original");
	if (!exePath.rename(renamedOrgFile)) {
		outputFile.remove();
		return "Failed to rename original file!";
	}
	//replace exe
	if (!outputFile.rename(exePath)) {
		outputFile.remove();
		return "Failed to rename output file!";
	}

	return std::string();
}