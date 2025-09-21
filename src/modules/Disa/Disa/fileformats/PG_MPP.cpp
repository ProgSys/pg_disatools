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

#include "PG_MPP.h"
#include <algorithm>

#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamInByteFileWindow.h>
#include <Stream/PG_StreamOutByteFile.h>
#include <Stream/PG_StreamInOutByteVector.h>

#include <Util/PG_Image.h>
#include <Util/Debug/PG_DebugOut.h>
#include <Util/PG_Exception.h>


 // functions
bool PG_MPP::open(const std::string& file) {
	if (file.empty()) {
		OUT_ERROR("Given filename is empty!");
		return FAILURE;
	}

	return open(PG_File(file));
}

bool PG_MPP::open(const PG_File& file) {
	clear();
	if (!file.exists()) {
		OUT_ERROR("Target file \"" << file << "\" doesn't exist!");
		return FAILURE;
	}

	try {
		PG::STREAM::InByteFile reader(file);
		const bool result = open(&reader);
		reader.close();
		return result;

	}
	catch (PG_Exception& e) {
		OUT_ERROR("Failed to import MPP! : " << e.what());
		clear();
		return FAILURE;
	}
	catch (...) {
		OUT_ERROR("Failed to import MPP!");
		clear();
		return FAILURE;
	}

	return SUCCESS;
}

bool PG_MPP::open(PG::STREAM::In* reader) {
	clear();
	if (!reader) {
		return FAILURE;
	}
	if (!reader->isOpen()) {
		OUT_ERROR("Reader is not open!");
		return FAILURE;
	}

	const unsigned int file_size = reader->size();
	if (file_size < sizeof(mppfileHeader) + 32) {
		OUT_ERROR("Given file is too small!");
		return FAILURE;
	}

	reader->read((char*)&m_header, sizeof(mppfileHeader));
	if (m_header.number_of_textures > 25 || m_header.number_of_models > 25) {
		OUT_ERROR("Number of files is too big! (" << m_header.number_of_textures << ", " << m_header.has_normals << ", " << m_header.number_of_models << ")");
		return FAILURE;
	}

	if (m_header.file_size > file_size) {
		OUT_ERROR("Header references a bigger file size than actual file size! (" << m_header.file_size << " > " << file_size << ")");
		return FAILURE;
	}

	std::vector<unsigned int> file_offsets((m_header.has_normals) ? m_header.number_of_textures * 2 + m_header.number_of_models : m_header.number_of_textures + m_header.number_of_models);
	reader->read((char*)file_offsets.data(), file_offsets.size() * sizeof(unsigned int));

	auto seekOffsetStart = [reader, &file_offsets, &file_size](int index) {
		const unsigned file_start_offset = file_offsets[index];
		const unsigned file_end_offset = (index + 1 >= file_offsets.size()) ? file_size : file_offsets[index + 1];
		const int diff = file_end_offset - file_start_offset;
		if (diff <= 0) return FAILURE;

		reader->seek(file_start_offset);
		return SUCCESS;
	};

	auto readBytes = [reader, &file_offsets, &file_size](int index, Bytes& out) {
		const unsigned file_start_offset = file_offsets[index];
		const unsigned file_end_offset = (index + 1 >= file_offsets.size()) ? file_size : file_offsets[index + 1];
		const int diff = file_end_offset - file_start_offset;
		if (diff <= 0) return FAILURE;

		out.resize(diff);
		reader->seek(file_start_offset);
		reader->read((char*)out.data(), out.size() * sizeof(std::uint8_t));
		return SUCCESS;
	};

	for (int i = 0; i < m_header.number_of_textures; ++i) {
		PG_TX2 tex;
		if (!seekOffsetStart(i)) {
			OUT_ERROR("Failed to read bytes at: " << i);
			clear();
			return FAILURE;
		}
		if (!tex.open(reader)) {
			OUT_ERROR("Failed to read texture at: " << m_header.number_of_textures + i);
			clear();
			return FAILURE;
		}
		m_textures.push_back({ std::move(tex), {} });
	}

	std::uint32_t modelStart = m_header.number_of_textures;
	if (m_header.has_normals) {

		for (int i = 0; i < m_header.number_of_textures; ++i) {
			PG_TX2 tex;
			if (!seekOffsetStart(modelStart + i - 1)) {
				OUT_ERROR("Failed to read normal bytes at: " << m_header.number_of_textures + i);
				clear();
				return FAILURE;
			}
			tex.open(reader);
			if (!tex.open(reader)) {
				OUT_ERROR("Failed to read normal texture at: " << m_header.number_of_textures + i);
				clear();
				return FAILURE;
			}
			m_textures[i].second = std::move(tex);
		}
		modelStart += m_header.number_of_textures;
	}

	for (int i = 0; i < m_header.number_of_models; ++i) {
		PG_GEO geo;
		if (!seekOffsetStart(modelStart + i)) {
			OUT_ERROR("Failed to read bytes at: " << modelStart + i);
			clear();
			return FAILURE;
		}

		if (!geo.open(reader)) {
			OUT_ERROR("Failed to read model at: " << modelStart + i << " (" << i << ")");
			//clear();
			//return FAILURE;
		}
		else
			m_models.push_back(std::move(geo));
	}

	return SUCCESS;
}

void PG_MPP::clear() {
	m_header.file_size = 0;
	m_textures.clear();
	m_models.clear();
}

void PG_MPP::dump(const std::string& file) const {
	std::ofstream myfile;

	myfile.open(file.c_str());
	myfile << "Header:\n";
	myfile << "number_of_textures: " << m_header.number_of_textures;
	myfile << "has_normals: " << m_header.has_normals;
	myfile << "number_of_models: " << m_header.number_of_models;
	myfile << "unk1: " << m_header.unk1;
	myfile << "file_size: " << m_header.file_size;

	myfile.close();
}


bool PG_MPPContainer::open(std::shared_ptr<PG::STREAM::In> reader, const std::function<void(PG_DisaFileEntry&)>& visitor) {
	if (!reader) return FAILURE;

	const unsigned int file_size = reader->size();
	if (file_size < sizeof(mppfileHeader) + 32) {
		FILE_CONTAINER_ON_ERROR("Given file is too small!");
	}

	mppfileHeader header;
	reader->read((char*)&header, sizeof(mppfileHeader));

	if (header.number_of_textures > 25 || header.number_of_models > 25) {
		FILE_CONTAINER_ON_ERROR("Header declares too many items! (" << header.number_of_textures << ", " << header.has_normals << ", " << header.number_of_models << ")");
	}

	if (header.file_size > file_size) {
		FILE_CONTAINER_ON_ERROR("Header references a bigger file size than actual file size! (" << header.file_size << " > " << file_size << ")");
	}
	

	auto mpp_size = std::min(header.file_size, file_size);
	std::vector<unsigned int> file_offsets(header.number_of_textures * (header.has_normals ? 2 : 1) + header.number_of_models);
	reader->read((char*)file_offsets.data(), file_offsets.size() * sizeof(unsigned int));

	const auto getStartEnd = [&file_offsets, mpp_size](int index) ->std::pair<unsigned, unsigned> {
		unsigned file_start_offset = file_offsets[index];
		unsigned file_end_offset = (index + 1 >= file_offsets.size()) ? mpp_size : file_offsets[index + 1];
		return { file_start_offset, file_end_offset };
		};


	std::vector<PG_DisaFileEntry> contains;
	contains.reserve(file_offsets.size());
	PG_DisaFileEntry entry;
	PG_DisaFileInfo info;

	for (int i = 0; i < header.number_of_textures; ++i) {
		auto [start, end] = getStartEnd(i);
		entry = PG_DisaFileEntry(this);
		info.setName("Texture" + std::to_string(i));
		info.setType(PG_DisaFileInfo::TX2);
		info.setOffset(start);
		info.setSize(end - start);
		entry.setInfo(std::move(info));
		if (visitor) visitor(entry);
		contains.push_back(std::move(entry));
	}
	std::uint32_t indexOffset = header.number_of_textures;
	if (header.has_normals) {
		for (int i = 0; i < header.number_of_textures; ++i) {
			auto [start, end] = getStartEnd(indexOffset + i);
			entry = PG_DisaFileEntry(this);
			info.setName("Normal" + std::to_string(i));
			info.setType(PG_DisaFileInfo::TX2);
			info.setOffset(start);
			info.setSize(end - start);
			entry.setInfo(std::move(info));
			if (visitor) visitor(entry);
			contains.push_back(std::move(entry));
		}
		indexOffset += header.number_of_textures;
	}

	for (int i = 0; i < header.number_of_models; ++i) {
		auto [start, end] = getStartEnd(indexOffset + i);
		entry = PG_DisaFileEntry(this);
		info.setName("Geometry" + std::to_string(i));
		info.setType(PG_DisaFileInfo::GEO);
		info.setOffset(start);
		info.setSize(end - start);
		entry.setInfo(std::move(info));
		if (visitor) visitor(entry);
		contains.push_back(std::move(entry));
	}

	set(std::move(contains));

	return SUCCESS;
}

bool PG_MPPContainer::save(std::shared_ptr<PG::STREAM::Out> writer) const {

	//collect data
	mppfileHeader header;
	header.has_normals = 0;
	header.number_of_models = 0;
	header.number_of_textures = 0;
	header.file_size = 0;
	header.unk1 = 0;

	m_originalReader.reset();
	m_originalReader = createReaderOriginal();

	for (const auto& item : *this) {
		auto info = item.getInfo();
		switch (info.getType())
		{
		case PG_DisaFileInfo::GEO: ++header.number_of_models; break;
		case PG_DisaFileInfo::TX2: if (info.getName().getName()[0] == 'N') {
			header.has_normals = true;
		} else {
			++header.number_of_textures;
		}break;

		default:
			OUT_ERROR("Invalid file format!");
			doneSaving();
			return FAILURE;
			break;
		}
	}

	const unsigned int start = writer->pos();
	writer->writeData(header);
	std::vector<unsigned int> fileOffsets(size());
	writer->writeData(fileOffsets);
	const unsigned int fileStart = writer->pos() - start;
	auto it = fileOffsets.begin();
	for (const auto& item : *this) {
		*it = writer->pos() - start;
		++it;
		auto reader = item.createReader();
		if (!reader) {
			doneSaving();
			return FAILURE;
		}
		if (reader->size() == 0) continue;

		std::vector<unsigned char> data;
		reader->read(data, reader->size());
		if (data.size() > 999999999)
			return FAILURE;
		writer->writeData(data);
	}
	header.file_size = writer->pos() - start;
	writer->seek(start);
	writer->writeData(header);
	writer->writeData(fileOffsets);
	writer->seek(start + header.file_size);
	doneSaving();

	return SUCCESS;
}

PG_MPPContainer::Stats PG_MPPContainer::getStats() const {
	PG_MPPContainer::Stats stats;
	for (const auto& item : *this) {
		auto info = item.getInfo();

		if (info.getType() == PG_DisaFileInfo::GEO) {
			++stats.geoemtrySize;
		}
		else if (info.getType() == PG_DisaFileInfo::TX2) {
			if (info.getName().getPath()[0] == 'N')  ++stats.normalsSize;
			else ++stats.texturesSize;
		}
	}
	return stats;
}