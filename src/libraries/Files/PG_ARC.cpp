#include "PG_ARC.h"

#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamOutByteFile.h>
#include <Files/PG_FileTests.h>
#include <Util/PG_StringUtil.h>
#include <array>

using namespace PG::FILE;

struct ArcFileEntry {
	std::array<char, 40> name;
	unsigned int size;
	unsigned int offset;
};

bool PG::FILE::ARC::open(const PG::UTIL::File& file, PercentIndicator* percent) {
	clear();

	m_file = file;
	m_file_buffer = PG::UTIL::File(file.getPath() + "_buffer");

	if (!m_file.exists()) {
		PG_ERROR_STREAM("Target file does not exist!");
		m_file.clear();
		return true;
	}

	try {
		PG::STREAM::InByteFile reader(m_file);
		if (!reader.isopen()) return true;

		if (reader.readString(8) != "DSARCIDX") {
			throw PG::UTIL::Exception("Invalid magic!");
		}

		const unsigned int numberOfFiles = reader.readUnsignedInt();
		if (numberOfFiles > 90001) {
			throw PG::UTIL::Exception("Too many files!");
		}
		reader.skip(4); //resevers
		//indeces for some reason
		for (int i = 0; i < numberOfFiles; ++i) {
			reader.readShort();
		}
		if (auto value = reader.readShort(); value != -1) {
			throw PG::UTIL::Exception("Invalid index terminator! Expected -1 but got " + std::to_string(value)+".");
		}

		std::vector<ArcFileEntry> fileEntries(numberOfFiles);
		reader.read((char*)fileEntries.data(), fileEntries.size() * sizeof(ArcFileEntry));
		m_fileInfos.reserve(numberOfFiles);

		int i = 0;
		fileInfo info;
		for (const ArcFileEntry& entry: fileEntries) {
			if (percent) percent->percent = (i++) / (float)numberOfFiles;

			auto findIt = std::find(entry.name.begin(), entry.name.end(), char(0x00));
			if (findIt == entry.name.begin()) {
				throw PG::UTIL::Exception("Empty file name!");
			}

			info.name = std::string(entry.name.begin(), findIt);
			info.size = entry.size;
			info.offset = entry.offset;
			const unsigned int currentPos = reader.pos();
			reader.seek(info.offset);
			if (isIMY(reader)) {
				info.fileType = fileInfo::IMY;
			}
			else {
				info.setTypeByFileExtension(info.getFileExtension());
			}
			reader.seek(currentPos);

			m_fileInfos.push_back(info);
		}

		reader.close();
		m_originalFileSize = m_fileInfos.size();

	}
	catch (PG::UTIL::Exception& e) {
		PG_ERROR_STREAM("Couldn't read given ARC file '" << file << "': " << e.what());
		m_file.clear();
		return true;
	}
	catch (...) {
		PG_ERROR_STREAM("Couldn't read given ARC file '" << file << "'!");
		m_file.clear();
		return true;
	}

	return false;
}

constexpr unsigned nextWithLowbyte00(unsigned n) {
	// Add one block (0x100) then clear the low byte.
	// Example: 0x2CB0 -> (0x2CB0 + 0x100) & ~0xFF = 0x2D00
	return (n + 0x100u) & ~0xFFu;
}

bool PG::FILE::ARC::save(const PG::UTIL::File& targetfile, PercentIndicator* percent ) {
	if (m_file.isEmpty()) {
		PG_ERROR_STREAM("No ARC file opened.");
		return FAILURE;
	}

	//do we overwrtie the original or do we save as?
	bool overwrite = false;
	PG::UTIL::File target;
	if (targetfile == m_file) {
		target = PG::UTIL::File(m_file.getPath() + "_buffer");
		overwrite = true;
	}
	else {
		target = targetfile;
	}

	PG::STREAM::InByteFile readerOriginal(m_file);
	if (!readerOriginal.isopen()) return FAILURE;

	std::vector<fileInfo> newFileInfos;
	try {
		PG::STREAM::OutByteFile writer(target);
		if (!writer.isOpen()) {
			throw PG::UTIL::Exception("Failed to open/create "+ target.getPath());
		}

		writer.writeString("DSARCIDX"); //write magic number
		writer.writeInt(m_fileInfos.size()); //number of files
		writer.writeInt(0); // mystery int

		for (int i = 0; i < m_fileInfos.size(); ++i) {
			writer.writeShort(i);
		}
		writer.writeShort(-1);

		if (m_fileInfos.empty()) return SUCCESS;

		const auto headerStart = writer.pos();
		std::vector<ArcFileEntry> fileEntries(m_fileInfos.size() + 3);
		writer.write((char*)fileEntries.data(), fileEntries.size() * sizeof(ArcFileEntry));
		writer.writeLongLong(0); //empty space
		writer.writeLongLong(0);

		
		newFileInfos.reserve(m_fileInfos.size());

		auto headerIt = fileEntries.begin();
		std::vector<char> fileData;

		for (auto& entry : m_fileInfos) {
			auto& fileInfo = newFileInfos.emplace_back();
			if (entry.isExternalFile()) {
				PG::STREAM::InByteFile readerFile(entry.externalFile);

				const unsigned int fileSize = readerFile.size();
				if (fileSize == 0) {
					PG_WARN_STREAM("File '" << entry.externalFile << "' has a size of zero!");
				}

				readerFile.readChars(fileData, fileSize);
				auto extension = PG::UTIL::toUpperCopy(entry.getFileExtension());
				fileInfo.setTypeByFileExtension(fileInfo.getFileExtension());
			}
			else {
				readerOriginal.seek(entry.offset);
				readerOriginal.readChars(fileData, entry.size);
				
				fileInfo.fileType = entry.fileType;
			}

			fileInfo.name = entry.getName();
			fileInfo.offset = writer.pos();
			fileInfo.size = fileData.size();
			
			std::memcpy(headerIt->name.data(), fileInfo.name.getFile().c_str(), std::min(headerIt->name.size(), fileInfo.name.getFile().size()));
			headerIt->offset = fileInfo.offset;
			headerIt->size = fileInfo.size;

			writer.write(fileData.data(), fileData.size());
			//add buffer to each file
			auto pos = writer.pos();
			auto fill = nextWithLowbyte00(pos);
			for (int i = pos; i < fill; ++i) {
				writer.writeChar(0);
			}
			
			++headerIt;
		}

		writer.seek(headerStart);
		writer.write((char*)fileEntries.data(), fileEntries.size() * sizeof(ArcFileEntry));

	}
	catch (PG::UTIL::Exception& e) {
		PG_ERROR_STREAM("Couldn't save ARC archive! : " << e.what());
		target.remove();
		return FAILURE;
	}
	catch (...) {
		PG_ERROR_STREAM("Couldn't save ARC archive!");
		target.remove();
		return FAILURE;
	}

	if (overwrite) {
		m_file.remove();
		target.rename(m_file.getPath());
		m_fileInfos = std::move(newFileInfos);
		m_changed = false;
	}

	return SUCCESS;
}

char const* PG::FILE::ARC::getType() const { return "ARC"; }