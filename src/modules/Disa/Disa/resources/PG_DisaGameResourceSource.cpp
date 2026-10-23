#include <Disa/resources/PG_DisaGameResourceSource.h>

#include <Resources/PG_ResourceManager.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamInOutByteVector.h>

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

void PG_DisaGameResourceSource::traverseEntries(const std::function<void(const std::string&)>& unary) const {
	if (!unary) return;
	m_gameDir.traverseFiles([&unary](const PG_File& file) { unary(file.getFile()); }, 0);
}

std::string PG_DisaGameResourceSource::toEntryPath(PG_EResourceFolder folder, std::string_view name) const {
	if (folder != PG_ResourceFolder_Materials) return {};
	return std::string(name);
}

void PG_DisaGameResourceSource::onDeregistered(PG_ResourceManager* manager) {
	m_walker.clear();
}

std::shared_ptr<PG::STREAM::In> PG_DisaGameResourceSource::openStream(const std::string& path) const {
	std::optional<std::vector<char>> data = m_walker.read(path);
	if (!data) return nullptr;
	return std::make_shared<PG::STREAM::InOutByteVector>(std::move(*data));
}

void PG_DisaGameResourceSource::onModificationBegin() {
	m_walker.clear();
}

PG_File PG_DisaGameResourceSource::toGameFile(std::string_view name) const {
	if (name.empty() || name == "." || name == ".." || name.find_first_of("/\\:") != std::string_view::npos) return {};
	return PG_File(m_gameDir.toFilePath(std::string(name)));
}

PG_ResourceSourceScope<PG_DisaGameResourceSource> registerDisaGameSource(const PG_Dir& gameDir) {
	if (gameDir.isEmpty() || !PG_File(gameDir.toFilePath("SUBDATA.DAT")).exists()) return {};
	return PG_ResourceManager::registerSource<PG_DisaGameResourceSource>(gameDir);
}
