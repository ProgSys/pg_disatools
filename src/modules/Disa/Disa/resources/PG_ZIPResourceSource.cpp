#include <Disa/resources/PG_ZIPResourceSource.h>

#include <zip.h>
#include <vector>
#include <Base/PG_Object.h>
#include <Base/PG_Timer.h>
#include <Resources/PG_ResourceManager.h>
#include <Stream/PG_StreamInOutByteVector.h>
#include <Disa/fileformats/PG_ZIPUtil.h>
#include <Util/Debug/PG_DebugOut.h>

namespace {

	/**
	* @brief Lives under the resource manager beside a registered zip source and closes its handle
	* once it was idle long enough.
	* @details Created on whatever thread registered the source, so it moves itself to the managers
	* thread before it touches the object tree. Deletes itself once the source releases it or is gone.
	*/
	class PG_ZIPResourceSourceCompanion : public PG_Object {
	public:
		PG_ZIPResourceSourceCompanion(PG_ResourceManager* manager, PG_ResourceSource_WPtr source, std::shared_ptr<std::atomic_bool> released) :
			PG_Object("ZIPResourceSourceCompanion"), m_manager(manager), m_source(std::move(source)), m_released(std::move(released)) {
			setThreadId(manager->getThreadId());
			PG_Connector::postCallEvent(PG_Event_Behaviour_Normal, this, &PG_ZIPResourceSourceCompanion::attach);
		}

	private:
		void attach() {
			if (*m_released) {
				deleteLater();
				return;
			}
			setParent(m_manager);

			auto* timer = PG_newWithParent<PG_Timer>("ZIPIdleTimer", this);
			timer->setRepeat(true);
			timer->setDuration(std::chrono::seconds(1));
			connect(timer, &PG_Timer::passed, this, [this] { check(); });
			timer->start();
		}

		void check() {
			auto source = std::static_pointer_cast<PG_ZIPResourceSource>(m_source.lock());
			if (!source || *m_released) {
				deleteLater();
				return;
			}
			source->closeIfIdle(PG_ZIPResourceSource::idleTimeout);
		}

		PG_ResourceManager* m_manager;
		PG_ResourceSource_WPtr m_source;
		std::shared_ptr<std::atomic_bool> m_released;
	};

} //namespace


PG_ZIPResourceSource::PG_ZIPResourceSource(const PG_File& zipFile, std::string nameSpace) :
	PG_ResourceSource(std::move(nameSpace)), m_file(zipFile), m_archive(nullptr, [](zip* archive) { zip_discard(archive); }) {
	{
		std::lock_guard g(m_mutex);
		if (!indexArchive()) return;
	}
	buildIndex();
	//nothing asks for more until it is registered, and an unregistered source should not lock the file
	closeNow();
}

PG_ZIPResourceSource::~PG_ZIPResourceSource() {
	if (m_companionReleased) *m_companionReleased = true;
}

void PG_ZIPResourceSource::traverseEntries(const std::function<void(const std::string&)>& unary) const {
	if (!unary) return;
	//copied, so unary may read from this source without waiting for itself
	std::vector<std::string> paths;
	{
		std::lock_guard g(m_mutex);
		paths.reserve(m_entries.size());
		for (const auto& [path, entry] : m_entries) paths.push_back(path);
	}
	for (const std::string& path : paths) unary(path);
}

void PG_ZIPResourceSource::traversePGR(const std::function<void(const std::string&)>& unary) const {
	traverseEntries(unary);
}

void PG_ZIPResourceSource::onRegistered(PG_ResourceManager* manager) {
	if (!m_isValid) return;

	std::lock_guard g(m_mutex);
	if (m_companionReleased) *m_companionReleased = true;
	m_companionReleased = std::make_shared<std::atomic_bool>(false);
	//owned by the manager once it attached, and it deletes itself
	new PG_ZIPResourceSourceCompanion(manager, weak_from_this(), m_companionReleased);
}

void PG_ZIPResourceSource::onDeregistered(PG_ResourceManager* manager) {
	std::lock_guard g(m_mutex);
	if (m_companionReleased) *m_companionReleased = true;
	m_companionReleased.reset();
	m_archive.reset();
}

void PG_ZIPResourceSource::onModificationBegin() {
	closeNow();
}

void PG_ZIPResourceSource::onModificationEnd() {
	std::lock_guard g(m_mutex);
	m_entries.clear();
	m_isChangeReported = false;
	m_isValid = indexArchive();
	if (!m_companionReleased) m_archive.reset();
	//the base reads the .pgr files next, through openStream like any other read
}

bool PG_ZIPResourceSource::isOpen() const {
	std::lock_guard g(m_mutex);
	return m_archive != nullptr;
}

void PG_ZIPResourceSource::closeIfIdle(std::chrono::steady_clock::duration timeout) {
	std::lock_guard g(m_mutex);
	if (m_archive && std::chrono::steady_clock::now() - m_lastRead >= timeout) m_archive.reset();
}

void PG_ZIPResourceSource::closeNow() {
	std::lock_guard g(m_mutex);
	m_archive.reset();
}

std::shared_ptr<PG::STREAM::In> PG_ZIPResourceSource::openStream(const std::string& path) const {
	std::lock_guard g(m_mutex);
	auto findIt = m_entries.find(path);
	if (findIt == m_entries.end()) return nullptr;
	const Entry& entry = findIt->second;

	if (!ensureOpen()) return nullptr;
	m_lastRead = std::chrono::steady_clock::now();

	zip_file* file = zip_fopen_index(m_archive.get(), entry.index, 0);
	if (!file) {
		OUT_ERROR("Failed to open \"" << path << "\" in archive \"" << m_file << "\"!");
		return nullptr;
	}

	std::vector<char> data(static_cast<std::size_t>(entry.size));
	std::size_t offset = 0;
	while (offset < data.size()) {
		const zip_int64_t bytesRead = zip_fread(file, data.data() + offset, data.size() - offset);
		if (bytesRead <= 0) break;
		offset += static_cast<std::size_t>(bytesRead);
	}
	zip_fclose(file);

	//without a companion nobody would close it when idle
	if (!m_companionReleased) m_archive.reset();

	if (offset != data.size()) {
		OUT_ERROR("Failed to read \"" << path << "\" in archive \"" << m_file << "\"!");
		return nullptr;
	}
	return std::make_shared<PG::STREAM::InOutByteVector>(std::move(data));
}

bool PG_ZIPResourceSource::indexArchive() {
	m_archive.reset();
	const auto stamp = m_file.readStamp();
	if (!stamp) {
		OUT_WARNING("Archive \"" << m_file << "\" can't be read, it carries nothing!");
		return false;
	}
	m_stamp = *stamp;

	m_archive = PG_ZIPUtil::openZip(m_file.getPath());
	if (!m_archive) return false;
	m_isValid = true;

	const zip_int64_t count = zip_get_num_entries(m_archive.get(), 0);
	struct zip_stat st;
	for (zip_int64_t i = 0; i < count; ++i) {
		if (zip_stat_index(m_archive.get(), i, 0, &st) != 0 || !(st.valid & ZIP_STAT_NAME)) continue;
		std::string path(st.name);
		if (path.empty() || path.back() == '/') continue; //a folder
		const Entry entry{ static_cast<std::uint64_t>(i), (st.valid & ZIP_STAT_SIZE) ? st.size : 0 };
		if (!m_entries.emplace(path, entry).second)
			OUT_WARNING("Archive \"" << m_file << "\" carries \"" << path << "\" twice, one of them is ignored!");
	}
	m_lastRead = std::chrono::steady_clock::now();
	return true;
}

bool PG_ZIPResourceSource::ensureOpen() const {
	if (m_archive) return true;
	if (!m_isValid) return false;

	//the indexes describe the file as it was, a replaced one could hand out the wrong entries
	const auto stamp = m_file.readStamp();
	if (!stamp || *stamp != m_stamp) {
		if (!m_isChangeReported)
			OUT_WARNING("Archive \"" << m_file << "\" changed or vanished outside a modification, build its source again to read from it!");
		m_isChangeReported = true;
		return false;
	}

	m_archive = PG_ZIPUtil::openZip(m_file.getPath());
	return m_archive != nullptr;
}
