#include <Disa/resources/PG_DisaArchiveWalker.h>

#include <algorithm>
#include <Disa/fileformats/PG_DisaFileContainer.h>
#include <Disa/fileformats/PG_DisaFileContainerFactory.h>
#include <Disa/fileformats/PG_DisaFileInfo.h>
#include <Stream/PG_StreamIn.h>
#include <Stream/PG_StreamInOutByteVector.h>
#include <Util/PG_StringUtil.h>
#include <Util/Debug/PG_DebugOut.h>

namespace {

	bool isInvalidSegment(std::string_view segment) {
		return segment.empty() || segment == "." || segment == ".." || segment.find_first_of("\\:") != std::string_view::npos;
	}

	std::string toKey(std::string_view segment) {
		return PG_StringUtil::toLowerCopy(std::string(segment));
	}

	std::optional<std::vector<char>> readWhole(PG::STREAM::In* reader) {
		if (!reader || !reader->isOpen()) return std::nullopt;
		reader->rewind();
		std::vector<char> data(reader->size());
		if (!data.empty()) reader->read(data.data(), static_cast<unsigned int>(data.size()));
		return data;
	}

} //namespace

PG_DisaArchiveWalker::PG_DisaArchiveWalker(RootOpener opener) : m_opener(std::move(opener)) {}

PG_DisaArchiveWalker::~PG_DisaArchiveWalker() = default;

std::optional<std::vector<char>> PG_DisaArchiveWalker::read(std::string_view path) const {
	const std::size_t split = path.find('/');
	const std::string_view root = path.substr(0, split);
	const std::string_view innerPath = split == std::string_view::npos ? std::string_view{} : path.substr(split + 1);
	if (!m_opener || root.empty()) return std::nullopt;
	std::vector<std::string_view> segments;
	if (!innerPath.empty()) {
		segments = PG_StringUtil::splitView(innerPath, '/');
		if (std::ranges::any_of(segments, isInvalidSegment)) return std::nullopt;
	}

	std::lock_guard g(m_mutex);

	if (segments.empty()) {
		auto reader = m_opener(root);
		return readWhole(reader.get());
	}

	std::string key = toKey(root);
	std::shared_ptr<PG_DisaFileContainer> archive;
	if (auto findIt = m_archives.find(key); findIt != m_archives.end()) archive = findIt->second;
	else archive = openArchive(key, m_opener(root));

	for (std::size_t i = 0; archive && i < segments.size(); ++i) {
		PG_DisaFileEntry entry = archive->find(PG_File(std::string(segments[i])), false);
		if (!entry) return std::nullopt;
		if (i + 1 == segments.size()) return readWhole(entry.createReader().get());

		key += '/';
		key += toKey(segments[i]);
		if (auto findIt = m_archives.find(key); findIt != m_archives.end()) archive = findIt->second;
		else archive = openArchive(key, entry.createReaderOriginal(), &entry);
	}
	return std::nullopt;
}

void PG_DisaArchiveWalker::clear() {
	//destroyed outside the lock, a big archive takes a moment to let go
	std::map<std::string, std::shared_ptr<PG_DisaFileContainer>> archives;
	{
		std::lock_guard g(m_mutex);
		archives.swap(m_archives);
	}
}

std::shared_ptr<PG_DisaFileContainer> PG_DisaArchiveWalker::openArchive(const std::string& key, std::shared_ptr<PG::STREAM::In> reader,
	PG_DisaFileEntry* entry) const {
	if (!reader || !reader->isOpen()) return nullptr;

	PG_DisaFileInfo::Type type = PG_DisaFileInfo::typeFromFileFormat(reader.get());
	if (type == PG_DisaFileInfo::COLA) type = PG_DisaFileInfo::OLA; //PG_OLA reads the compressed header too
	std::shared_ptr<PG_DisaFileContainer> archive = PG_Disa::createFileContainer(type);
	if (!archive) return nullptr;
	//a nested archive hangs in the tree before it opens, its entries window into the parent entry
	if (entry) entry->setContainer(archive);
	if (!archive->open(reader)) {
		OUT_ERROR("Failed to open \"" << key << "\" as an archive!");
		if (entry) entry->setContainer(nullptr);
		return nullptr;
	}
	//only the root has nothing to derive its windows from
	if (!entry) archive->setReader(std::move(reader));

	if (archive->isCompressed()) {
		OUT_INFO("\"" << key << "\" is compressed, decompressing it in memory.");
		auto decompressed = std::make_shared<PG::STREAM::InOutByteVector>();
		if (!archive->uncompress(decompressed.get()) || decompressed->getData().empty()) {
			OUT_ERROR("Failed to decompress \"" << key << "\"!");
			if (entry) entry->setContainer(nullptr);
			return nullptr;
		}
		decompressed->seek(0);
		archive = PG_Disa::createFileContainer(decompressed.get());
		if (!archive) {
			OUT_ERROR("Decompressed \"" << key << "\" is no archive!");
			if (entry) entry->setContainer(nullptr);
			return nullptr;
		}
		if (entry) entry->setContainer(archive);
		if (!archive->open(decompressed)) {
			OUT_ERROR("Decompressed \"" << key << "\" is no archive!");
			if (entry) entry->setContainer(nullptr);
			return nullptr;
		}
		//the windows have to go into the decompressed bytes, not the compressed ones of the parent
		archive->setReader(std::move(decompressed));
	}

	m_archives.emplace(key, archive);
	return archive;
}
