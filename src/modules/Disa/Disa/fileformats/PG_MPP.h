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
#include <array>
#include <ostream>
#include <optional>
#include <cstdint>

#include <Util/Typedefs.h>
#include <Util/PG_File.h>
#include <Util/PG_File.h>
#include <Util/PG_Image.h>
#include <Disa/fileformats/PG_TX2.h>
#include <Disa/fileformats/PG_GEO.h>
#include <Stream/PG_StreamIn.h>

PACK(struct mppfileHeader// 16 bytes
{
	std::uint16_t number_of_textures;
	std::uint16_t has_normals; // if true then number of normals is same as number of textures
	std::uint16_t number_of_models;
	std::uint16_t unk1;
	std::uint32_t file_size;
});

class PG_MPP {
public:
	typedef std::vector<std::uint8_t> Bytes;
	typedef std::vector<std::pair<PG_TX2, std::optional<PG_TX2>>> TexturePair;

	bool open(const std::string& file);
	bool open(const PG_File& file);
    bool open(PG::STREAM::In* reader);

	bool isEmpty() const { return m_header.file_size <= 0 && m_textures.empty() && m_models.empty(); }
	void clear();

	void dump(const std::string& file) const;

	inline const mppfileHeader& getHeader() const { return m_header; }
	inline const TexturePair& getTextures() const { return m_textures; }
	inline const std::vector<PG_GEO>& getModels() const { return m_models; }

private:
	mppfileHeader m_header;
	TexturePair m_textures; //Texture and optinal normal map
    std::vector<PG_GEO> m_models; //model data
};

#include <Disa/fileformats/PG_DisaFileContainer.h>

class PG_MPPContainer : public PG_DisaFileContainer {
public:
	using PG_DisaFileContainer::PG_DisaFileContainer;
	typedef std::vector<std::uint8_t> Bytes;

	using PG_DisaFileContainer::open;
	bool open(std::shared_ptr<PG::STREAM::In> reader, const std::function<void(PG_DisaFileEntry&)>& visitor) final;
	bool save(std::shared_ptr<PG::STREAM::Out> writer) const final;

	struct Stats {
		unsigned texturesSize = 0;
		unsigned normalsSize = 0;
		unsigned geometrySize = 0;
		unsigned totalSize() const { return texturesSize + normalsSize + geometrySize; }
	};

	Stats getStats() const;
};