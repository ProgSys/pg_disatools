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

#include <Util/Typedefs.h>
#include <Util/PG_File.h>

#define FILE_CONTAINER_ON_ERROR(message) \
	clear();\
	OUT_ERROR( reader->pos()<<": " << message);\
	return FAILURE;

#define FILE_CONTAINER_ERROR_TEST(x, message) \
if (x) {\
    FILE_CONTAINER_ON_ERROR("["<< #x <<"]: "<< message);\
}

class PG_DisaFileEntry;

namespace PG {
    namespace STREAM {
        class In;
    }
}

class PG_DisaFileInfo {
public:
    enum Type {
        PSPFS_V1,
        OLA,  /// Offset list archive
        COLA, /// Compressed offset list archive
        SOLA, /// Sprite sheets offset list archive (it also has a list of IDs)
        KM3,  /// Effect model archive, stores entry sizes instead of offsets (it also has a list of IDs)
        TX2,
        IMY,
        MPD,
        MPP,
        SH,   /// Sprite sheet
        SH2,   /// Sprite sheet Disgaea 2
        GEO,
        LZS, /// LZS compressed file
        PNG,
        /// Custom files
        JSON, 
        TALK_SCRIPT, 
        SCRIPT, 
        ZIP,
        OGG,
        UNKNOWN
    };

    PG_DisaFileInfo() {}
    PG_DisaFileInfo(const PG_File& externalFile);
    PG_DisaFileInfo(const PG_DisaFileInfo& other) = default;

    inline bool isValid() const { return m_size > 0 && !m_name.getPath().empty(); }
    inline operator bool() const { return isValid(); }

    inline const PG_File& getName() const {return m_name;}
    inline void setName(const PG_File& name)  {m_name = name;}
    inline void setName(PG_File&& name)  {m_name = std::move(name);}

    inline Type getType() const {return m_type;}
    inline void setType(Type type) {m_type = type;}
    void setType(const std::string& extention);
    inline void determinTypeFromFileExtention() { setType(m_name.getFileExtensionFormated()); }
    void determinTypeFromFileFormat(PG::STREAM::In* reader);
    static Type typeFromFileFormat(PG::STREAM::In* reader);
    static std::string_view toString(Type type);
    static std::string_view toFileExtension(Type type);


    inline unsigned int getSize() const {return m_size;}
    inline void setSize(unsigned int size)  { m_size = size;}

    inline unsigned int getDecompressedSize() const {return m_decompressedSize;}
    inline void setDecompressedSize(unsigned int size) { m_decompressedSize = size;}

    inline unsigned int getOffset() const {return m_offset;}
    inline void setOffset(unsigned int offset) { m_offset = offset;}


    inline void clear() {
        m_name.clear();
        m_size = 0; 
        m_offset = 0;
        m_decompressedSize = 0;
    }

    inline bool operator==(const PG_File& file) const { return file == m_name; }
    inline bool operator==(const PG_DisaFileInfo& info) const { return info.getSize() == m_size && info.getName() == m_name; }

    inline bool isCompressed() const { return isCompressedType() || (m_decompressedSize != 0 && m_decompressedSize != m_size); }
    inline bool isCompressedType() const { return m_type == IMY || m_type == COLA; }
    inline bool isPackageType() const { return m_type == OLA || m_type == PSPFS_V1 || m_type == COLA || m_type == SOLA || m_type == KM3; }
    inline bool isTextureType() const { return m_type == TX2; }
private:
    PG_File m_name;
    Type m_type = UNKNOWN;
    unsigned int m_size = 0; //size in byte
    unsigned int m_offset = 0; //offset from file beginning in byte
    unsigned int m_decompressedSize = 0;

};

