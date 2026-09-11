#include "PG_DisaLooseFileContainer.h"

#include <Stream/PG_StreamInByteFile.h>
#include <Util/Debug/PG_DebugOut.h>

PG_DisaFileEntry PG_DisaLooseFileContainer::add(const PG_File& file) {
	PG_DisaFileInfo info(file);
	if (!info.isValid()) {
		OUT_ERROR("File \"" << file << "\" is missing or empty!");
		return PG_DisaFileEntry();
	}

	push_back(PG_DisaFileEntry(std::move(info), this));
	return get(static_cast<int>(size()) - 1);
}

std::shared_ptr<PG::STREAM::In> PG_DisaLooseFileContainer::createEntryReaderOriginal(const PG_DisaFileEntry& entry) const {
	const PG_File& file = entry.getInfo().getName();
	auto reader = std::make_shared<PG::STREAM::InByteFile>(file);
	if (!reader->isOpen()) {
		OUT_ERROR("Failed to open \"" << file << "\"!");
		return nullptr;
	}
	return reader;
}
