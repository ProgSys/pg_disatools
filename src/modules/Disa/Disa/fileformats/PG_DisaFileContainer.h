/*
 * The MIT License (MIT)
 *
 *	Copyright (c) 2020 ProgSys
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */
#pragma once

#include <string>
#include <vector>
#include <variant>

#include <Util/Typedefs.h>
#include <Util/PG_File.h>
#include <Scene/Model/PG_TableModel.h>
#include <Disa/fileformats/PG_DisaFileInfo.h>
#include <Stream/PG_StreamIn.h>
#include <Stream/PG_StreamInOutByteArray.h>
#include <regex>

class PG_DisaFileContainer;
class PG_DisaContainerInterface;
class PG_DisaFileEntry;




class PG_DisaFileInterface {
public:
    friend class PG_DisaFileEntry;
    /**
    * @brief Create reader for current data
    * If current is modefied then it will store into buffer
    * Otherwise will create reader of original file
    */
    virtual std::shared_ptr<PG::STREAM::In> createReader() const = 0;
    /**
    * @brief Create reader for original data (not saved state)
    */
    virtual std::shared_ptr<PG::STREAM::In> createReaderOriginal() const = 0;
    /**
    * @brief Should return true when file was modefied
    */
    virtual bool isModified() const { return false; };

    virtual bool open(std::shared_ptr<PG::STREAM::In> reader) { return FAILURE; };
    virtual bool save(std::shared_ptr<PG::STREAM::Out> writer) const { return FAILURE; };
    /**
    * @brief Will read opened file to refresh all file entries
    * @returns true if refresh was successful
    */
    virtual bool refresh(std::shared_ptr<PG::STREAM::In> reader) { return open(reader); };
};


class PG_DisaFileEntry : public PG_DisaFileInterface {
public:
    enum EDisaFileEntryIdentity : Identity {
        Identity_Modified = EIdentity::Identity_Custom,
        Identity_Container,
    };

    friend PG_DisaFileContainer;

    PG_DisaFileEntry() {}
    PG_DisaFileEntry(PG_DisaContainerInterface* parent) : m_parent(parent){}
    PG_DisaFileEntry(const PG_DisaFileInfo& info, PG_DisaContainerInterface* parent) : m_info( info ), m_parent(parent){}
    PG_DisaFileEntry(PG_DisaFileInfo&& info, PG_DisaContainerInterface* parent) : m_info(std::move(info)) , m_parent(parent) {}

    //==== File info ====
    PG_DisaFileInfo& getInfo() { return m_info; }
    const PG_DisaFileInfo& getInfo() const { return m_info; }
    void setInfo(const PG_DisaFileInfo& info) { m_info = info; }
    void setInfo(PG_DisaFileInfo&& info) { m_info = std::move(info); }

    const PG_Variety& getExtraData() const { return m_extraData; }
    void setExtraData(const PG_Variety& data)  { m_extraData = data; }

    PG_Variety getPreviewImage() const;

    int getLocation() const { return m_index; }
    void setLocation(int index);
    bool isValid() const { return m_index >= 0 && m_parent; }

    inline std::string getName() const {
        return m_info.getName().getFile();
    }
    operator std::string() const {
        return getName();
    }

    //==== PG_DisaFileInterface ====
    std::shared_ptr<PG::STREAM::In> createReader() const override;
    std::shared_ptr<PG::STREAM::In> createReaderOriginal() const override;
    bool isModified() const override;
    bool open(std::shared_ptr<PG::STREAM::In> reader) override;
    bool save(std::shared_ptr<PG::STREAM::Out> writer) const override;

    //==== Operations ====
    operator bool() const { return static_cast<bool>(m_parent); }
    PG_DisaFileContainer* getParent() const;
    void setParent(PG_DisaContainerInterface* parent) { m_parent = parent; }


    template<typename T = PG_DisaContainerInterface>
    std::shared_ptr<T> getContainer() const { return std::dynamic_pointer_cast<T>(m_container.lock()); }
    void setContainer(const std::shared_ptr<PG_DisaContainerInterface>& container) {
        if (m_parent && getLocation() >= 0)
            setContainerInParent(container);
        else
            setContainerIntern(container);
    }
    void setContainerRestore(const std::shared_ptr<PG_DisaContainerInterface>& container) {
        m_container = container;
    }

    /**
    * @brief Will open file as given type
    * If file is open with same type then it will return existing
    * @returns Container. Is null if file allready open as differen type
    */
    template<typename T, typename... Args>
    std::shared_ptr<T> open(Args&&... args) {
        if (!m_parent) return nullptr;

        if (auto container = getContainer<T>()) return container;
        if (!m_container.expired()) return nullptr; //allready open as something else
        std::shared_ptr<T> container(PG_new<T>(std::forward<Args>(args)...));
        constexpr bool hasOpen = requires(T t, std::shared_ptr<PG::STREAM::In> r) {
            { t.open(r) } -> std::same_as<bool>;
        };
        setContainerInParent(container);
        if constexpr (hasOpen) {
            if (!container->open(createReader())) {
                return container;
            }
        }

        return container;
    }

    /**
    * @brief Will open file as given type
    * If file is open with same type it will be replaced by new one
    * @returns Container. Is null if there is no parent
    */
    template<typename T, typename... Args>
    std::shared_ptr<T> replace(Args&&... args) {
        if (!m_parent) return nullptr;

        std::shared_ptr<T> container = std::make_shared<T>(std::forward<Args>(args)...);
        constexpr bool hasOpen = requires(T t, std::shared_ptr<PG::STREAM::In> r) {
            { t.open(r) } -> std::same_as<bool>;
        };
        setContainerInParent(container);
        if constexpr (hasOpen) {
            if (!container->open(createReader())) {
                container.reset();
            }
        }

        return container;
    }

    /**
    * @brief Will force close opned file entry
    */
    bool close();

    bool refresh(std::shared_ptr<PG::STREAM::In> reader) override;

    //==== Misc ====
    bool saveTo(const PG_File& file) const;
    void triggerFileChanged();

    PG_Variety getData(std::size_t column, Identity identity) const;
    bool setData(const PG_Variety& data, Identity identity);
    static std::size_t dimension() { return 2; }
    static std::size_t getColumns() { return 4; }
private:
    void setContainerInParent(std::shared_ptr<PG_DisaContainerInterface> container);
    void setContainerIntern(std::shared_ptr<PG_DisaContainerInterface> container);

    PG_DisaFileInfo m_info; //original
    PG_DisaContainerInterface* m_parent = nullptr;
    int m_index = -1;
    std::weak_ptr<PG_DisaContainerInterface> m_container; //file is open to read/write
    PG_Variety m_extraData; // like chracters ID in SOLA
};


class PG_DisaContainerInterface : public PG_DisaFileInterface, public std::enable_shared_from_this<PG_DisaContainerInterface> {
public:
    friend class PG_DisaFileEntry;

    PG_DisaFileEntry getParentEntry() const;
    /**
    * @brief Create reader for original data to read the given entry (not saved state)
    */
    virtual std::shared_ptr<PG::STREAM::In> createEntryReaderOriginal(const PG_DisaFileEntry& entry) const;

    using PG_DisaFileInterface::save;
    /**
    * @brief Will save the container at original opened file, if possible
    * @returns true if save was successful
    */
    virtual bool save();
    using PG_DisaFileInterface::refresh;
    /**
    * @brief Will refresh data refrences used when file has changed
    * @returns Usualy called after save
    */
    virtual bool refresh() { return FAILURE; };
protected:
    virtual void fileEntryModefiedChanged(const PG_DisaFileEntry& entry) {/* does nothing by default*/ }

    int m_parentFileEntryLocation = -1;
    //std::shared_ptr<PG_DisaFileInterface> m_disaFileParent; //children keep parent alive, kinda like in real life
    std::weak_ptr<PG_DisaFileInterface> m_disaFileParent; //children keep parent alive, kinda like in real life
};

/**
* @brief Base contanier
*/
class PG_DisaFileContainer: public PG_DisaContainerInterface, public PG_TableModel<PG_DisaFileEntry> {
    PG_OBJECT(PG_DisaFileContainer);
    PG_PARENT(PG_TableModel<PG_DisaFileEntry>);
    PG_PROPERTY(bool isModified GET isModified CHANGED isModifiedChanged);
    friend PG_DisaFileEntry;
signals:
    void saveProgress(int current, int total);
    void isModifiedChanged();
public:

    using PG_TableModel<PG_DisaFileEntry>::PG_TableModel;

    void init();

    inline const PG_File& getFile() const { return m_file; }
    inline void setFile(const PG_File& file) { m_file = file; }

    PG_DisaFileEntry find(const PG_File& file, bool caseSensitive = true) const;
    std::vector<PG_DisaFileEntry> findAll(const std::regex& regex) const;
    PG_ListModelLocation findLocation(const PG_File& file, bool caseSensitive = true) const;

    inline bool open(const std::string& file) { return open(PG_File(file)); }
    virtual bool open(const PG_File& file);
    using PG_DisaFileInterface::open;
    using PG_DisaFileInterface::save;
    using PG_DisaFileInterface::refresh;

    /**
    * @brief Create reader for current
    * If current is modefied then it will store into buffer
    * Otherwise will create reader of original file
    */
    std::shared_ptr<PG::STREAM::In> createReader() const override;
    /**
    * @brief Create reader for original data (not saved state)
    */
    std::shared_ptr<PG::STREAM::In> createReaderOriginal() const override;
    /**
    * @brief Should return true when container was modified
    */
    bool isModified() const override;
    /**
    * @returns true if file format supports compression
    */
    virtual bool isCompressable() const { return false; };
    /**
    * @returns true if file is compressed
    */
    virtual bool isCompressed() const { return false; };
    /**
    * @brief Will uncompress the file, if it is uncompressed
    */
    virtual bool uncompress(PG::STREAM::Out* writer) { return SUCCESS; };

    void setReader(std::shared_ptr<PG::STREAM::In>&& reader) { m_originalReader = reader; }
receivers:
    bool save() override;
    bool refresh() override;
protected:
    void doneSaving() const { m_originalReader.reset(); }
    void fileEntryModefiedChanged(const PG_DisaFileEntry& entry) override;
    std::unordered_map<std::string, const PG_DisaFileEntry*> getEntiesWithContainer() const;

    PG_File m_file;
    bool m_filesRemoved = false;
    mutable std::shared_ptr<PG::STREAM::In> m_originalReader; //for optimasation so we don't create a new reader for each file none stop
};

using VisitPair = std::pair < std::string, std::function<std::shared_ptr<PG_DisaFileContainer> (PG_DisaFileEntry& entry)>>;
static inline std::shared_ptr<PG_DisaFileInterface> getReader(std::shared_ptr<PG_DisaFileContainer> container, const std::vector<VisitPair>& path) {
    for (auto& p : path) {
        PG_DisaFileEntry entry = container->find(PG_File(p.first), false);
        if (!entry) return nullptr;
        container = p.second(entry);
        if (!container) return nullptr;
    }
    return container;
}

/**
* @brief Conainer for modefierd data
*/
class PG_DisaChangedContainer : public PG_DisaContainerInterface, public PG_Object {
    PG_OBJECT(PG_DisaChangedContainer);
    PG_PROPERTY(bool isModified GET isModified CHANGED isModifiedChanged);
signals:
    void isModifiedChanged();
public:
    using GetDataVariant = std::variant<const std::vector<char>*, std::unique_ptr<std::vector<char>>>;

    using PG_Object::PG_Object;
    /**
    * @brief Create reader for original data (not saved state)
    */
    std::shared_ptr<PG::STREAM::In> createReaderOriginal() const override;
    /**
    * @brief Create reader for modefied data 
    */
    std::shared_ptr<PG::STREAM::In> createReader() const;

protected:
    void triggerFileChanged();
};

/**
* @brief Conainer for modefierd data
*/
class PG_DisaChangedDataContainer : public PG_DisaChangedContainer {
    PG_OBJECT(PG_DisaChangedDataContainer);
    PG_PARENT(PG_DisaChangedContainer);

public:
    using PG_DisaChangedContainer::PG_DisaChangedContainer;
    PG_DisaChangedDataContainer(std::vector<char>&& data) : m_data(std::move(data)) {}

    void setData(std::vector<char>&& data) { m_data = std::move(data); triggerFileChanged(); }
    void setData(const std::vector<char>& data) { m_data = data; triggerFileChanged();}

    bool open(std::shared_ptr<PG::STREAM::In> reader) override;

    bool save(std::shared_ptr<PG::STREAM::Out> writer) const override;
    /**
    * @brief Create reader for current
    * If current is modefied then it will store into buffer
    * Otherwise will create reader of original file
    */
    std::shared_ptr<PG::STREAM::In> createReader() const override;
    /**
    * @brief Is always true
    */
    bool isModified() const override { return true; }
private:
    std::vector<char> m_data;
};

/**
* @brief Conainer for modefierd data
*/
class PG_DisaChangedReaderContainer : public PG_DisaChangedContainer {
    PG_OBJECT(PG_DisaChangedReaderContainer);
    PG_PARENT(PG_DisaChangedContainer);

public:
    using PG_DisaChangedContainer::PG_DisaChangedContainer;
    PG_DisaChangedReaderContainer(std::shared_ptr<PG::STREAM::In> reader) : m_reader(reader) {}

    bool open(std::shared_ptr<PG::STREAM::In> reader) override { return true; }

    bool save(std::shared_ptr<PG::STREAM::Out> writer) const override;
    /**
    * @brief Create reader for current
    * If current is modefied then it will store into buffer
    * Otherwise will create reader of original file
    */
    std::shared_ptr<PG::STREAM::In> createReader() const override;
    /**
    * @brief Is always true
    */
    bool isModified() const override { return true; }
    /**
    * @brief Copy data from reader to writer
    * @param targetBuffer Optinal buffer to use. Otherwise default will be used.
    */
    static void copyData(PG::STREAM::In& reader, PG::STREAM::Out& writer);
    /**
    * @brief Copy data from reader to writer
    * @param targetBuffer Buffer to use to copy data.
    */
    static void copyData(PG::STREAM::In& reader, PG::STREAM::Out& writer, std::vector<char>& targetBuffer);
protected:
    std::shared_ptr<PG::STREAM::In> m_reader;
};

/**
* @brief Conainer for modefierd data
*/
class PG_DisaChangedFileContainer : public PG_DisaChangedContainer {
    PG_OBJECT(PG_DisaChangedFileContainer);
    PG_PARENT(PG_DisaChangedContainer);

public:
    using PG_DisaChangedContainer::PG_DisaChangedContainer;
    PG_DisaChangedFileContainer(const PG_File& file) : m_file(file) {}

    bool open(std::shared_ptr<PG::STREAM::In> reader) override { return true; }

    bool save(std::shared_ptr<PG::STREAM::Out> writer) const override;
    /**
    * @brief Create reader for current
    * If current is modefied then it will store into buffer
    * Otherwise will create reader of original file
    */
    std::shared_ptr<PG::STREAM::In> createReader() const override;
    /**
    * @brief Is always true
    */
    bool isModified() const override { return true; }
private:
    PG_File m_file;
};