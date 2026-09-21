#include <Disa/resources/PG_ZIPResourceSource.h>

#include <Util/Debug/PG_DebugOut.h>

PG_ZIPResourceSource::PG_ZIPResourceSource(const std::shared_ptr<PG_ZIP>& zip, std::string nameSpace) :
	PG_ResourceSource(std::move(nameSpace)), m_zip(zip) {
	if (!zip) return;

	m_description = zip->getFile().getPath();
	for (const PG_DisaFileEntry& entry : *zip) {
		const std::string& path = entry.getInfo().getName().getPath();
		if (path.empty()) continue;
		if (!m_entries.emplace(path, entry).second)
			OUT_WARNING("Archive \"" << m_description << "\" carries \"" << path << "\" twice, one of them is ignored!");
	}

	buildIndex();
}

std::shared_ptr<PG::STREAM::In> PG_ZIPResourceSource::openStream(const std::string& path) const {
	auto findIt = m_entries.find(path);
	if (findIt == m_entries.end()) return nullptr;

	//whoever opened the archive may have closed it, which is a read that fails and nothing worse
	auto zip = m_zip.lock();
	if (!zip) return nullptr;

	std::lock_guard g(m_mutex);
	return findIt->second.createReader();
}

void PG_ZIPResourceSource::traverseEntries(const std::function<void(const std::string&)>& unary) const {
	if (!unary) return;
	for (const auto& [path, entry] : m_entries) unary(path);
}

void PG_ZIPResourceSource::traversePGR(const std::function<void(const std::string&)>& unary) const {
	traverseEntries(unary);
}
