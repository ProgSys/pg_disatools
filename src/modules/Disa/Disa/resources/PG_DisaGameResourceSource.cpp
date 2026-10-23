#include <Disa/resources/PG_DisaGameResourceSource.h>

#include <filesystem>
#include <system_error>
#include <Disa/resources/PG_DisaSourceIdleCompanion.h>
#include <Resources/PG_ResourceManager.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamInOutByteVector.h>
#include <Util/PG_StringUtil.h>
#include <Util/Debug/PG_DebugOut.h>

namespace {

	/// Without trailing separators, the exe path of the game ends with one.
	PG_Dir trimmed(const PG_Dir& dir) {
		std::string path = dir.getDir();
		while (path.size() > 1 && (path.back() == '/' || path.back() == '\\')) path.pop_back();
		return PG_Dir(std::move(path));
	}

} //namespace

PG_DisaGameResourceSource::PG_DisaGameResourceSource(const PG_Dir& gameDir) :
	PG_ResourceSource(std::string(nameSpace)), m_gameDir(trimmed(gameDir)), m_walker([this](std::string_view root) -> std::shared_ptr<PG::STREAM::In> {
		const PG_File file = toGameFile(root);
		if (file.isEmpty()) return nullptr;
		auto reader = std::make_shared<PG::STREAM::InByteFile>(file);
		if (!reader->isOpen()) return nullptr;
		return reader;
	}) {
	//the game ships no .pgr files, the index stays empty
	buildIndex();
}

PG_DisaGameResourceSource::~PG_DisaGameResourceSource() {
	if (m_companionReleased) *m_companionReleased = true;
}

void PG_DisaGameResourceSource::traverseEntries(const std::function<void(const std::string&)>& unary) const {
	if (!unary) return;
	m_gameDir.traverseFiles([&unary](const PG_File& file) { unary(file.getFile()); }, 0);
}

std::string PG_DisaGameResourceSource::toEntryPath(PG_EResourceFolder folder, std::string_view name) const {
	if (folder != PG_ResourceFolder_Materials) return {};
	return std::string(name);
}

void PG_DisaGameResourceSource::onRegistered(PG_ResourceManager* manager) {
	if (m_companionReleased) *m_companionReleased = true;
	m_companionReleased = std::make_shared<std::atomic_bool>(false);
	//owned by the manager once it attached, and it deletes itself
	new PG_DisaSourceIdleCompanion(manager, weak_from_this(), m_companionReleased, [](PG_ResourceSource& source) {
		static_cast<PG_DisaGameResourceSource&>(source).m_walker.releaseIdle(idleTimeout);
	});
}

void PG_DisaGameResourceSource::onDeregistered(PG_ResourceManager* manager) {
	if (m_companionReleased) *m_companionReleased = true;
	m_companionReleased.reset();
	m_walker.clear();
}

PG_ResourceSource::ModificationGuard PG_DisaGameResourceSource::guardFile(const PG_File& file) {
	if (!PG_ResourceManager::exists() || file.isEmpty()) return {};

	const std::filesystem::path fileDir(file.getDir());
	for (const auto& source : PG_ResourceManager::get()->getSources(nameSpace)) {
		auto gameSource = std::dynamic_pointer_cast<PG_DisaGameResourceSource>(source);
		if (!gameSource) continue;
		std::error_code error;
		if (std::filesystem::equivalent(fileDir, std::filesystem::path(gameSource->m_gameDir.getDir()), error))
			return gameSource->guardModification();
	}
	return {};
}

std::shared_ptr<PG::STREAM::In> PG_DisaGameResourceSource::openStream(const std::string& path) const {
	const std::string_view root = std::string_view(path).substr(0, path.find('/'));
	checkStamp(root);

	std::optional<std::vector<char>> data = m_walker.read(path);
	if (!data) return nullptr;
	return std::make_shared<PG::STREAM::InOutByteVector>(std::move(*data));
}

void PG_DisaGameResourceSource::onModificationBegin() {
	m_walker.clear();
}

void PG_DisaGameResourceSource::onModificationEnd() {
	std::lock_guard g(m_stampMutex);
	m_stamps.clear();
}

PG_File PG_DisaGameResourceSource::toGameFile(std::string_view name) const {
	if (name.empty() || name == "." || name == ".." || name.find_first_of("/\\:") != std::string_view::npos) return {};
	return PG_File(m_gameDir.toFilePath(std::string(name)));
}

void PG_DisaGameResourceSource::checkStamp(std::string_view root) const {
	const PG_File file = toGameFile(root);
	if (file.isEmpty()) return;
	const std::optional<PG_File::Stamp> stamp = file.readStamp();
	const std::string key = PG_StringUtil::toLowerCopy(std::string(root));

	std::lock_guard g(m_stampMutex);
	auto findIt = m_stamps.find(key);
	if (findIt == m_stamps.end()) {
		if (stamp) m_stamps.emplace(key, *stamp);
		return;
	}
	if (stamp && *stamp == findIt->second) return;

	OUT_WARNING("Game file \"" << file << "\" changed outside a modification, its archives are read again!");
	m_walker.clear();
	if (stamp) findIt->second = *stamp;
	else m_stamps.erase(findIt);
}

PG_ResourceSourceScope<PG_DisaGameResourceSource> registerDisaGameSource(const PG_Dir& gameDir) {
	if (gameDir.isEmpty() || !PG_File(gameDir.toFilePath("SUBDATA.DAT")).exists()) return {};
	return PG_ResourceManager::registerSource<PG_DisaGameResourceSource>(gameDir);
}
