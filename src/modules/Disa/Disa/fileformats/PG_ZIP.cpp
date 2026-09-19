#include "PG_ZIP.h"
#include <Util/Typedefs.h>
#include <Util/Debug/PG_DebugOut.h>
#include <istream>
#include <Stream/PG_StreamInByteArray.h>
#include <Stream/PG_StreamInOutByteVector.h>
#include <Stream/PG_StreamInOutByteFile.h>
#include <Util/PG_Math.h>
#include <Util/PG_Dir.h>

#include <zip.h>

struct StreamData {
    virtual ~StreamData() = default;
    PG::STREAM::In* stream;
    unsigned int size;
};

struct StreamDataMem: public StreamData {
    ~StreamDataMem() override {
        hold.reset();
    }
    std::shared_ptr<PG::STREAM::In> hold;
};

static inline zip_int64_t readCallback(void* state, void* data, zip_uint64_t len, zip_source_cmd_t cmd) {
    StreamData* streamData = static_cast<StreamData*>(state);
    switch (cmd) {
    case ZIP_SOURCE_OPEN:
        //OUT_INFO("ZIP_SOURCE_OPEN");
        streamData->stream->seek(0);
        return 0;
    case ZIP_SOURCE_READ:
        //OUT_INFO("ZIP_SOURCE_READ "<< streamData->stream->pos() << " s: "<< streamData->size);
        if (streamData->size <= streamData->stream->pos()) return 0;
        len = std::min(zip_uint64_t(streamData->size - streamData->stream->pos()), len);
        streamData->stream->read(static_cast<char*>(data), len);
        return len; 
    case ZIP_SOURCE_CLOSE:
        //OUT_INFO("ZIP_SOURCE_CLOSE");
        return 0;
    case ZIP_SOURCE_STAT: {
        //OUT_INFO("ZIP_SOURCE_STAT");
        zip_stat_t* st = static_cast<zip_stat_t*>(data);
        zip_stat_init(st);
        st->valid |= (ZIP_STAT_SIZE | ZIP_STAT_INDEX | ZIP_STAT_COMP_METHOD | ZIP_STAT_CRC | ZIP_STAT_ENCRYPTION_METHOD);
        st->index = streamData->stream->pos();
        st->size = streamData->size;
        st->comp_method = ZIP_CM_STORE; // Compression method (none)
        st->encryption_method = ZIP_EM_NONE; // No encryption
        st->crc = 0; // No check
        return sizeof(zip_stat_t);
    }
    case ZIP_SOURCE_ERROR:
        //OUT_INFO("ZIP_SOURCE_ERROR");
        return -1;
    case ZIP_SOURCE_FREE:
        //OUT_INFO("ZIP_SOURCE_FREE");
        delete streamData;
        return 0;
    case ZIP_SOURCE_SEEK: {
        //OUT_INFO("ZIP_SOURCE_SEEK");
        zip_source_args_seek_t* args = static_cast<zip_source_args_seek_t*>(data);
        unsigned int target = 0;
        switch (args->whence)
        {
        default:
        case std::ios_base::beg: target = args->offset; break;
        case std::ios_base::cur: target = (streamData->stream->pos() + args->offset); break;
        case std::ios_base::end: target = (streamData->size + args->offset); break;
        break;
        }
        if (target >= streamData->size) return -1;

        streamData->stream->seek(target);

        return 0;
    }
    case ZIP_SOURCE_TELL:
        //OUT_INFO("ZIP_SOURCE_TELL");
        return streamData->stream->pos();
    case ZIP_SOURCE_SUPPORTS:
        //OUT_INFO("ZIP_SOURCE_SUPPORTS");
        return ZIP_SOURCE_SUPPORTS_SEEKABLE;
    default:
        return -1;
    }
}

struct StreamWriteData {
    PG::STREAM::Out* stream;
    unsigned int size;
};


static inline zip_int64_t writeCallback(void* state, void* data, zip_uint64_t len, zip_source_cmd_t cmd) {
    StreamWriteData* streamData = static_cast<StreamWriteData*>(state);
    switch (cmd) {
    case ZIP_SOURCE_OPEN:
        return 0;
    case ZIP_SOURCE_CLOSE:
        return 0;
    case ZIP_SOURCE_STAT: {
        zip_stat_t* st = static_cast<zip_stat_t*>(data);
        st->valid |= (ZIP_STAT_SIZE | ZIP_STAT_INDEX | ZIP_STAT_COMP_METHOD | ZIP_STAT_CRC | ZIP_STAT_ENCRYPTION_METHOD);
        st->index = streamData->stream->pos();
        st->size = streamData->size;
        st->comp_method = ZIP_CM_STORE; // Compression method (none)
        st->encryption_method = ZIP_EM_NONE; // No encryption
        st->crc = 0; // No check
        return sizeof(zip_stat_t);
    }
    case ZIP_SOURCE_ERROR:
        return -1;
    case ZIP_SOURCE_FREE:
        delete streamData;
        return 0;
    case ZIP_SOURCE_SEEK_WRITE:
    case ZIP_SOURCE_SEEK: {
        zip_source_args_seek_t* args = static_cast<zip_source_args_seek_t*>(data);
        unsigned int target = 0;
        switch (args->whence)
        {
        default:
        case std::ios_base::beg: target = args->offset; break;
        case std::ios_base::cur: target = (streamData->stream->pos() + args->offset); break;
        case std::ios_base::end: target = (streamData->size + args->offset); break;
            break;
        }
        if (target >= streamData->size) return -1;

        streamData->stream->seek(target);
        return 0;
    }
    case ZIP_SOURCE_TELL:
    case ZIP_SOURCE_TELL_WRITE:
        return streamData->stream->pos();
    case ZIP_SOURCE_BEGIN_WRITE:
        return 0;
    case ZIP_SOURCE_WRITE: {
        streamData->stream->write((char*)data, len);
        streamData->size += len;
        return len;
    }
    case ZIP_SOURCE_COMMIT_WRITE:
        return 0;
    case ZIP_SOURCE_ROLLBACK_WRITE:
        return -1;
    case ZIP_SOURCE_SUPPORTS:
        return ZIP_SOURCE_SUPPORTS_WRITABLE;
    default:
        return -1;
    }
}

struct StreamWriteDataRollback {
    PG::STREAM::InOut* stream;
    PG::STREAM::InOutByteVector out;
    std::size_t sizeOriginal;

    void reset() {
        stream->seek(0);
        out.truncate(0);
        sizeOriginal = 0;
    }

    void write(void* data, zip_uint64_t len) {
        out.write((char*)data, len);
    }

    void rollback() {
        out.truncate(0);
    }

    void submit() {
        if (out.getData().empty()) return;
        auto posBefor = stream->pos();
        stream->seek(0);
        stream->write(out.getData().data(), out.getData().size());
        if (sizeOriginal > out.getData().size()) {
            stream->truncate(out.getData().size());
            sizeOriginal = out.getData().size();
        }
        out.truncate(0);
    }

    bool seek(int offset, int whence) {
        unsigned int target = 0;
        switch (whence)
        {
        default:
        case std::ios_base::beg: target = offset; break;
        case std::ios_base::cur: target = (out.pos() + offset); break;
        case std::ios_base::end: target = (out.size() + offset); break;
            break;
        }

        if (target > out.size()) return FAILURE;
        out.seek(target);

        return SUCCESS;
    }

};

static inline zip_int64_t writeRollbackableCallback(void* state, void* data, zip_uint64_t len, zip_source_cmd_t cmd) {
    StreamWriteDataRollback* streamData = static_cast<StreamWriteDataRollback*>(state);

    switch (cmd) {
    case ZIP_SOURCE_OPEN:
        //OUT_INFO("ZIP_SOURCE_OPEN");
        streamData->stream->seek(0);
        streamData->sizeOriginal = streamData->stream->size();
        return 0;
    case ZIP_SOURCE_CLOSE:
        //OUT_INFO("ZIP_SOURCE_CLOSE");
        return 0;
    case ZIP_SOURCE_STAT: {
        //OUT_INFO("ZIP_SOURCE_STAT");
        zip_stat_t* st = static_cast<zip_stat_t*>(data);
        zip_stat_init(st);
        st->valid |= (ZIP_STAT_SIZE | ZIP_STAT_INDEX | ZIP_STAT_COMP_METHOD | ZIP_STAT_CRC | ZIP_STAT_ENCRYPTION_METHOD);
        st->index = streamData->stream->pos();
        st->size = streamData->sizeOriginal;
        st->comp_method = ZIP_CM_STORE; // Compression method (none)
        st->encryption_method = ZIP_EM_NONE; // No encryption
        st->crc = 0; // No check
        return sizeof(zip_stat_t);
    }
    case ZIP_SOURCE_ERROR:
        //OUT_INFO("ZIP_SOURCE_ERROR");
        return -1;
    case ZIP_SOURCE_FREE:
        //OUT_INFO("ZIP_SOURCE_FREE");
        delete streamData;
        return 0;
    case ZIP_SOURCE_READ:
        //OUT_INFO("ZIP_SOURCE_READ");
        if (streamData->stream->pos() >= streamData->sizeOriginal) return -1;
        len = std::min(zip_uint64_t(streamData->sizeOriginal - streamData->stream->pos()), len);
        streamData->stream->read(static_cast<char*>(data), len);
        return len;
    case ZIP_SOURCE_SEEK_WRITE: {
        //OUT_INFO("ZIP_SOURCE_SEEK_WRITE");
        zip_source_args_seek_t* args = static_cast<zip_source_args_seek_t*>(data);
        if (!streamData->seek(args->offset, args->whence)) return -1;
        return 0;
    }
    case ZIP_SOURCE_SEEK: {
        //OUT_INFO("ZIP_SOURCE_SEEK");
        zip_source_args_seek_t* args = static_cast<zip_source_args_seek_t*>(data);
        unsigned int target = 0;
        switch (args->whence)
        {
        default:
        case std::ios_base::beg: target = args->offset; break;
        case std::ios_base::cur: target = (streamData->stream->pos() + args->offset); break;
        case std::ios_base::end: target = (streamData->sizeOriginal + args->offset); break;
            break;
        }
        if (target >= streamData->sizeOriginal) return -1;

        streamData->stream->seek(target);
        return 0;
    }
    case ZIP_SOURCE_TELL:
        //OUT_INFO("ZIP_SOURCE_TELL");
        return streamData->stream->pos();
    case ZIP_SOURCE_TELL_WRITE:
        //OUT_INFO("ZIP_SOURCE_TELL_WRITE");
        return streamData->out.pos();
    case ZIP_SOURCE_BEGIN_WRITE:
        //OUT_INFO("ZIP_SOURCE_BEGIN_WRITE");
        return 0;
    case ZIP_SOURCE_WRITE:{
        //OUT_INFO("ZIP_SOURCE_WRITE");
        streamData->write((char*)data, len);
        return len;
    }
    case ZIP_SOURCE_COMMIT_WRITE:
        //OUT_INFO("ZIP_SOURCE_COMMIT_WRITE");
        streamData->submit();
        return 0;
    case ZIP_SOURCE_ROLLBACK_WRITE:
        //OUT_INFO("ZIP_SOURCE_ROLLBACK_WRITE");
        streamData->rollback();
        return 0;
    case ZIP_SOURCE_SUPPORTS:
        //OUT_INFO("ZIP_SOURCE_SUPPORTS");
        return ZIP_SOURCE_SUPPORTS_WRITABLE;
    case ZIP_SOURCE_BEGIN_WRITE_CLONING:
        //OUT_INFO("ZIP_SOURCE_BEGIN_WRITE_CLONING");
    default:
        return -1;
    }
    
}


struct ZipWrapper : public zipUPtr {
    using zipUPtr::zipUPtr;
    void discard() {
        if(get()) zip_discard(get());
    }

    static void detroyFunc(zip* a) {
        if (!a) return;
        if (zip_close(a) != 0) {
            OUT_ERROR("Failed to save zip: " << zip_strerror(a));
            zip_discard(a);
        }
    }

    static ZipWrapper open(const std::string& file) {
        int error = 0;
        zip* ptr = zip_open(file.c_str(), 0, &error);
        if (ptr == nullptr) {
            OUT_ERROR("Failed to open the zip '" << file << "': error code " << error);
            return ZipWrapper(nullptr, &ZipWrapper::detroyFunc);
        }

        return ZipWrapper(ptr, &ZipWrapper::detroyFunc);
    }

    static ZipWrapper open(const std::shared_ptr<PG::STREAM::In>& reader) {
        zip_error_t error;
        error.str = nullptr;
        StreamData* streamData = new StreamData;
        streamData->stream = reader.get();
        streamData->size = reader->size();

        zip_source_t* src = zip_source_function_create(readCallback, streamData, &error);
        if (!src) {
            OUT_ERROR("Failed to create ZIP source: " << zip_error_strerror(&error));
            delete streamData;
            return ZipWrapper(nullptr, &ZipWrapper::detroyFunc);
        }

        zip* ptr = zip_open_from_source(src, 0, &error);
        if (!ptr) {
            OUT_ERROR("Failed to open ZIP archive: " << zip_error_strerror(&error));
            zip_source_free(src);
            return ZipWrapper(nullptr, &ZipWrapper::detroyFunc);
        }
        return ZipWrapper(ptr, &ZipWrapper::detroyFunc);
    }

    static ZipWrapper open(const std::shared_ptr<PG::STREAM::Out>& writer) {
        zip_error_t error;
        error.str = nullptr;
        StreamWriteData* streamData = new StreamWriteData;
        streamData->stream = writer.get();
        streamData->size = 0;

        zip_source_t* src = zip_source_function_create(writeCallback, streamData, &error);
        if (!src) {
            OUT_ERROR("Failed to create ZIP source: " << zip_error_strerror(&error));
            delete streamData;
            return ZipWrapper(nullptr, &ZipWrapper::detroyFunc);
        }

        zip* ptr = zip_open_from_source(src, ZIP_CREATE, &error);
        if (!ptr) {
            OUT_ERROR("Failed to open ZIP archive: " << zip_error_strerror(&error));
            zip_source_free(src);
            return ZipWrapper(nullptr, &ZipWrapper::detroyFunc);
        }
        return ZipWrapper(ptr, &ZipWrapper::detroyFunc);
    }

    static ZipWrapper open(const std::shared_ptr<PG::STREAM::InOut>& reader) {
        zip_error_t error;
        error.str = nullptr;

        StreamWriteDataRollback* streamData = new StreamWriteDataRollback;
        streamData->stream = reader.get();
        streamData->sizeOriginal = reader->size();

        zip_source_t* src = zip_source_function_create(writeRollbackableCallback, streamData, &error);
        if (!src) {
            OUT_ERROR("Failed to create ZIP source: " << zip_error_strerror(&error));
            delete streamData;
            return ZipWrapper(nullptr, &ZipWrapper::detroyFunc);
        }

        zip* ptr = zip_open_from_source(src, ZIP_CHECKCONS, &error);
        if (!ptr) {
            OUT_ERROR("Failed to open ZIP archive: " << zip_error_strerror(&error));
            zip_source_free(src);
            return ZipWrapper(nullptr, &ZipWrapper::detroyFunc);
        }
        return ZipWrapper(ptr, &ZipWrapper::detroyFunc);
    }

};



PG_DisaFileInfo toFileInfo(struct zip_stat& st) {
    PG_DisaFileInfo info;
    info.setName(std::string(st.name));
    info.setOffset(st.index);
    info.setSize(st.comp_size);
    info.setDecompressedSize(st.size);
    info.determinTypeFromFileExtention();
    return info;
}

bool PG_ZIP::open(const PG_File& file) {
    if (!file.exists()) {
        clear();
        if (!PG_Dir(file.getDir()).exists()) {
            OUT_ERROR("The target folder \"" << file.getDir() << "\" doesn't exist!");
            return FAILURE;
        }
        if (!file.isValidFileName()) {
            OUT_ERROR("Invalid file name format in \"" << file << "\"!");
            return FAILURE;
        }
        m_file = file;
        return SUCCESS;
    }
    return PG_DisaFileContainer::open(file);
}

bool PG_ZIP::open(std::shared_ptr<PG::STREAM::In> reader, const std::function<void(PG_DisaFileEntry&)>& visitor) {
    if (!reader->isOpen()) return FAILURE;
    auto zipU = ZipWrapper::open(reader);
    if (!zipU) return FAILURE;

    zip_int64_t numEntries = zip_get_num_entries(zipU.get(), 0);
    std::vector<PG_DisaFileEntry> entries;
    entries.reserve(numEntries);
    struct zip_stat st;
    for (zip_int64_t i = 0; i < numEntries; ++i) {
        zip_stat_index(zipU.get(), i, 0, &st);
        if (st.size == 0) continue; //is folder
        assert(st.index == i);
        PG_DisaFileInfo info = toFileInfo(st);
        if (info.getName().isEmpty() || info.getName().getPath().back() == '/') continue; //is folder

        PG_DisaFileEntry entry = PG_DisaFileEntry(this);
        entry.setInfo(std::move(info));
        if (visitor) visitor(entry);
        entries.push_back(std::move(entry));
    }
    set(std::move(entries));
    m_originalReader = reader;
    m_originalZip = zipU.get();
    afterOpen(zipU);
    m_originalReader.reset();
    m_originalZip = nullptr;

    return SUCCESS;
}

bool PG_ZIP::refresh(std::shared_ptr<PG::STREAM::In> reader)  {
    if (!reader->isOpen()) return FAILURE;
    auto zipU = ZipWrapper::open(reader);
    if (!zipU) return FAILURE;
    zip_int64_t numEntries = zip_get_num_entries(zipU.get(), 0);
    m_originalReader = reader;
    m_originalZip = zipU.get();

    //invalidate all
    for (PG_DisaFileEntry& e : *this) {
        e.getInfo().setSize(0);
    }

    std::vector<PG_DisaFileEntry> entries;
    struct zip_stat st;
    for (zip_int64_t i = 0; i < numEntries; ++i) {
        zip_stat_index(zipU.get(), i, 0, &st);
        assert(st.index == i);
        PG_DisaFileInfo info = toFileInfo(st);
        if (info.getName().isEmpty() || info.getName().getPath().back() == '/') continue; //is folder

        auto findIt = std::ranges::find_if(*this, [&info](const PG_DisaFileEntry& e) { return e.getInfo().getName() == info; });
        if (findIt == end()) {
            PG_DisaFileEntry entry = PG_DisaFileEntry(this);
            entry.setInfo(std::move(info));
            entries.push_back(std::move(entry));
        }
        else {
            findIt->setInfo(std::move(info));
        }
    }

    { //to remove
        std::set<PG_ListModelLocation> locs;
        for (PG_DisaFileEntry& e : *this) {
           if (e.getInfo().getSize() == 0 && !e.isModified()) {
               locs.insert(getLocation(e.getLocation()));
           }
        }
        remove(locs);
    }

    append(std::make_move_iterator(entries.begin()), std::make_move_iterator(entries.end()));


    for (auto& item : *this) {
       if(auto c = item.getContainer()) c->refresh(item.createReaderOriginal());
    }
    m_originalZip = nullptr;
    m_originalReader.reset();
    m_filesRemoved = false;
    m_entryDataChanged = false;
    notifyIsModifiedChanged();

    return SUCCESS;
}

bool PG_ZIP::save(std::shared_ptr<PG::STREAM::Out> writer) const {
    if (!writer || empty()) return FAILURE;

    m_originalReader.reset();
    m_originalReader = createReaderOriginal();

    auto zipU = ZipWrapper::open(writer);
    if (!zipU) return FAILURE;
    auto zipOrg = ZipWrapper::open(m_originalReader);
    m_originalZip = zipOrg.get();

    zip_error_t error;

    for (const PG_DisaFileEntry& entry : *this) {
        auto reader = entry.createReader();
        if (!reader) continue;
        StreamDataMem* streamData = new StreamDataMem;
        streamData->stream = reader.get();
        streamData->size = reader->size();
        streamData->hold = std::move(reader);

        zip_source_t* src = zip_source_function_create(readCallback, streamData, &error);
        if (src == nullptr || zip_file_add(zipU.get(), entry.getInfo().getName().getPath().c_str(), src, ZIP_FL_OVERWRITE) < 0) {
            OUT_ERROR("Failed to add file to zip: " << zip_strerror(zipU.get()));
            if (src != nullptr) zip_source_free(src);
        }

    }

    m_originalZip = nullptr;
    zipU.reset();

    return SUCCESS;
}

static inline std::vector<unsigned char> readZipData(zip_file_t* zf) {
    std::vector<unsigned char> content;
    std::vector<unsigned char> buffer(1024);
    zip_int64_t bytesRead;
    constexpr std::size_t maxSize = (1024 * 1024 * 128);//max is 128 MB
    while ((bytesRead = zip_fread(zf, buffer.data(), buffer.size())) > 0) {
        content.insert(content.end(), buffer.begin(), std::next(buffer.begin(), bytesRead));
        if (content.size() > maxSize) {
            OUT_ERROR("Zip file is bigger the 128 MB!");
            return {};
        }
    }

    return content;
}

/*todo
* Could be done using 
* ZIP_EXTERN zip_int64_t zip_fread(zip_file_t *_Nonnull, void *_Nonnull, zip_uint64_t);
* ZIP_EXTERN zip_int8_t zip_fseek(zip_file_t *_Nonnull, zip_int64_t, int);
* ZIP_EXTERN zip_int64_t zip_ftell(zip_file_t *_Nonnull);
*/
class PG_StreamInByteData : public PG::STREAM::InByteArray {
public:
    PG_StreamInByteData(std::vector<unsigned char>&& data): 
        PG::STREAM::InByteArray(), m_data(std::move(data)) {
        m_bytes = (const char*)m_data.data();
        m_length = m_data.size();
    }

   std::vector<unsigned char> m_data;
};

std::shared_ptr<PG::STREAM::In> PG_ZIP::createEntryReaderOriginal(const PG_DisaFileEntry& entry) const {
    if (m_originalZip) {
        return createReaderFromZip(m_originalZip, entry);
    }
    auto containerReader = createReaderOriginal();
    containerReader->seek(0);
    auto zipU = ZipWrapper::open(containerReader);
    if (!zipU) return nullptr;
    return createReaderFromZip(zipU.get(), entry);
}

std::shared_ptr<PG::STREAM::In> PG_ZIP::createReaderFromZip(zip* zip, const PG_DisaFileEntry& entry) const {
    zip_file* zf = zip_fopen_index(zip, entry.getInfo().getOffset(), 0);
    if (!zf) {
        OUT_ERROR("Failed to find '" << entry.getName() << "' in zip!");
        return nullptr;
    }

    auto content = readZipData(zf);
    if (content.empty()) {
        OUT_ERROR("Failed to read '" << entry.getName() << "' in zip!");
        return nullptr;
    }
    return std::make_shared<PG_StreamInByteData>(std::move(content));
}


bool PG_ZIP::save() {
    if (getFile().isEmpty()) return FAILURE;

    if (m_filesRemoved || m_entryDataChanged) {
        return PG_DisaFileContainer::save();
    }

    //optimisation: if only one file changed then we check if its the same or smaller size, if yes then we just override it
    struct ChangedEntry {
        PG_DisaFileEntry* entry = nullptr;
        std::shared_ptr<PG::STREAM::In> reader;
        int index = -1;
    };

    std::vector<ChangedEntry> changes;

    for (PG_DisaFileEntry& entry : *this) {

        if (entry.getContainer() && entry.getContainer()->isModified()) {
            if (changes.size() > 4) {
                return PG_DisaFileContainer::save(); //more then one file changed
            }
            changes.emplace_back(&entry, entry.createReader());
        }

    }

    if (changes.empty()) return SUCCESS; //there is nothing to save
    if (!getFile().exists()) getFile().create();
    auto writer = std::make_shared<PG::STREAM::InOutByteFile>(getFile());
    if (!writer->isOpen()) return FAILURE;
    writer->seek(0);

    auto zipU = ZipWrapper::open(std::dynamic_pointer_cast<PG::STREAM::InOut>(writer));
    if (!zipU) 
        return FAILURE;

    {//check if file count is different
        zip_int64_t numEntries = zip_get_num_entries(zipU.get(), 0);
        zip_int64_t fileCount = 0;
        struct zip_stat st;
        for (zip_int64_t i = 0; i < numEntries; ++i) {
            zip_stat_index(zipU.get(), i, 0, &st);
            if (st.size == 0) continue; //is folder
            assert(st.index == i);
            PG_DisaFileInfo info = toFileInfo(st);
            OUT_INFO(i << ": " << info.getName());
            if (info.getName().isEmpty() || info.getName().getPath().back() == '/') continue; //is folder
            auto findIt = std::ranges::find_if(*this, [&info](const PG_DisaFileEntry& e) { return e.getInfo().getName() == info; });
            if (findIt == end()) { //file is not tracked
                zipU.reset();
                writer.reset();
                return PG_DisaFileContainer::save();
            }
            auto findChangeIt = std::ranges::find_if(changes, [&info](const PG_DisaFileEntry* e) { return e->getInfo().getName() == info; }, &ChangedEntry::entry);
            if (findChangeIt != changes.end()) {
                findChangeIt->index = i;
                /*
                //preform normal save if size differes
                if (st.size != findChangeIt->reader->size()) {
                    zip_discard(zipU);
                    writer.reset();
                    return PG_DisaFileContainer::save();
                }
                */
            }
            ++fileCount;
        }

        if (fileCount != size()) {
            zipU.reset();
            writer.reset();
            return PG_DisaFileContainer::save();
        }
    }

    if (!zipU) return FAILURE;
    OUT_INFO("Perform .zip diff save.");

    zip_error_t error;
    for (auto& change : changes) {
        if (!change.reader) continue;

        //holds the momery that shall be written
        StreamDataMem* streamData = new StreamDataMem;
        streamData->stream = change.reader.get();
        streamData->size = change.reader->size();
        if (streamData->size == 0) {
            OUT_ERROR("Invalid zero size!");
            delete streamData;
            zipU.discard();
            return FAILURE;
        }
        change.reader->seek(0);
        streamData->hold = std::move(change.reader); //take onwership of the shared ptr

        // create source_function, clean up will handle ziplib unless there is an error
        zip_source_t* src = zip_source_function_create(readCallback, streamData, &error);
        if (src == nullptr) {
            delete streamData;
            OUT_ERROR("Failed to create source_function: " << zip_strerror(zipU.get()));
            zipU.discard();
            return FAILURE;
        }

        //if index is smaller zero then its a new file
        if (change.index < 0) {
            OUT_INFO("Add file " << change.index);
            if (zip_file_add(zipU.get(), change.entry->getInfo().getName().getPath().c_str(), src, ZIP_FL_OVERWRITE) < 0) {
                OUT_ERROR("Failed to add file to zip: " << zip_strerror(zipU.get()));
                if (src != nullptr) zip_source_free(src);
            }
        }
        else {
            OUT_INFO("Replace file "<< change.index);
            if (zip_file_replace(zipU.get(), change.index, src, 0) < 0) {
                OUT_ERROR("Failed to replace file in zip: " << zip_strerror(zipU.get()));
                if (src != nullptr) zip_source_free(src);
            }
        }
    }

    zipU.reset();
    changes.clear();

    writer->rewind(); 
    refresh(writer);
    return SUCCESS;

}

// Extract a file from the ZIP archive and write it to the target folder
static inline bool extractFile(zip* archive, zip_file* zfile, const std::string& out_path, zip_stat_t& stat) {
    std::ofstream outputFile(out_path, std::ios::binary);
    if (!outputFile.is_open()) {
        std::cerr << "Failed to create file: " << out_path << std::endl;
        return false;
    }

    std::vector<char> buffer(1024 * 1024);  // 1 MB buffer
    zip_int64_t bytesRead = 0;
    zip_int64_t totalBytesRead = 0;

    while ((bytesRead = zip_fread(zfile, buffer.data(), buffer.size())) > 0) {
        outputFile.write(buffer.data(), bytesRead);
        totalBytesRead += bytesRead;
    }

    outputFile.close();
    return bytesRead >= 0;
}

bool PG_ZIP::unizipToFolder(const PG_File& zipFile, const PG_Dir& dir) {
    if (!dir.exists()) {
        OUT_ERROR("Directory \"" << dir<<"\" dosn't exist!");
        return FAILURE;
    }
    int err = 0;
    zip* archive = zip_open(zipFile.c_str(), 0, &err);
    if (!archive) {
        OUT_ERROR("Failed to open zip file: " << zipFile);
        return FAILURE;
    }

    zip_int64_t num_entries = zip_get_num_entries(archive, 0);
    for (zip_uint64_t i = 0; i < num_entries; ++i) {
        zip_stat_t stat;
        if (zip_stat_index(archive, i, 0, &stat) == 0) {
            std::string entryName = stat.name;
             
            std::string outPath = dir.getDir() + "/" + entryName;

            // If this is a directory, create it
            if (entryName.back() == '/') {
                if (!PG_Dir::createDir(outPath)) {
                    OUT_ERROR("Failed to create directory: " << outPath);
                    continue;
                }
         
            }
            else {
                // Ensure the directory for the file exists
               
                const std::string directory = PG_File(outPath).getDir();
                if (!PG_Dir::createDir(directory)) {
                    OUT_ERROR("Failed to create directory: " << directory);
                    continue;
                }

                // Open the file inside the ZIP archive
                zip_file* zfile = zip_fopen_index(archive, i, 0);
                if (!zfile) {
                    OUT_ERROR("Failed to open file in archive: " << entryName);
                    continue;
                }

                // Extract the file
                if (!extractFile(archive, zfile, outPath, stat)) {
                    OUT_ERROR("Failed to extract file: " << outPath);
                }

                // Close the zip file handle
                zip_fclose(zfile);
            }
        }
        else {
            OUT_ERROR("Failed to get file info for entry " << i );
        }
    }

    // Close the archive
    zip_close(archive);
    return SUCCESS;
}