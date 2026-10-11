/*
 * The MIT License (MIT)
 *
 *	Copyright (c) 2024 ProgSys
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
#include "PG_SH.h"
#include <algorithm>

#include <Base/PG_Typedefs.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamOutByteFile.h>
#include <Util/PG_Exception.h>
#include <Util/Debug/PG_DebugOut.h>
#include <math.h>

#define DUMP_ON_OPEN false


bool PG_SH::isOpen() const {
	return !m_addresses.empty();
}

bool PG_SH::open(const std::string& file) {
	if (file.empty()) {
		OUT_ERROR("Given filename is empty!");
		return FAILURE;
	}

	return open(PG_File(file));
}

bool PG_SH::open(const PG_File& file) {
	clear();
	if (!file.exists()) {
		OUT_ERROR("Target file doesn't exist!");
		return FAILURE;
	}


	PG::STREAM::InByteFile reader(file);
	if (!reader.isOpen()) {
		OUT_ERROR("Coudn't open file!");
		return FAILURE;
	}

	return open(&reader);
}

bool PG_SH::open(PG::STREAM::In* reader) {
	clear();
	if(!reader || reader->eof()) return FAILURE;

	try {
		const unsigned int file_size = reader->size();
		if (file_size < 32) {
			clear();
			OUT_ERROR("Given file is too small!");
			return FAILURE;
		}

		reader->read((char*)& m_header, sizeof(shfileHeader));

		m_addresses.resize(4);
		reader->read((char*)m_addresses.data(), m_addresses.size() * sizeof(int));

		if (file_size < m_addresses[3]) {
			OUT_ERROR("Given file is too small!");
			return FAILURE;
		}

		m_animations.resize(m_header.number_of_animations);
		m_layers.resize(m_header.number_of_bundels);
		m_numberOfColortables.resize(m_header.number_of_colortables);
		m_sheetsInfos.resize(m_header.number_of_sheetInfos);

		m_keyframesData.resize(m_header.number_of_keyframes);
		m_cutouts.resize(m_header.number_of_cutouts);

		reader->read((char*)m_animations.data(), m_animations.size() * sizeof(shfileAnimation));
		reader->read((char*)m_layers.data(), m_layers.size() * sizeof(shfileLayers));
		reader->read((char*)m_numberOfColortables.data(), m_numberOfColortables.size() * sizeof(unsigned int));
		reader->read((char*)m_sheetsInfos.data(), m_sheetsInfos.size() * sizeof(shfileSheetInfo));
		reader->seek(m_addresses[0]);
		reader->read((char*)m_keyframesData.data(), m_keyframesData.size() * sizeof(shfileKeyframe));
		reader->seek(m_addresses[1]);
		reader->read((char*)m_cutouts.data(), m_cutouts.size() * sizeof(shfileCutout));


		//read colortables
		reader->seek(m_addresses[2]);
		m_colortables.resize(m_header.number_of_colortables);

		for (ColorTable& colortable : m_colortables) {
			colortable.resize(16 * m_numberOfColortables[0]);
			reader->read((char*)colortable.data(), colortable.size() * sizeof(glm::rgba));
			//flip Red and Blue
			for (glm::rgba& color : colortable) {
				std::swap(color.r, color.b);
			}
			
		}


		//read sheet color IDs
		for (const shfileSheetInfo& sheet : m_sheetsInfos) {
			PG_UCharImage sheetIDs(sheet.width, sheet.height);
			reader->seek(sheet.offset);
			if (sheet.power_colortablesize == 4) { // 2^sheet.power_colortablesize
				//color table 16
				for (unsigned int i = 0; i < sheetIDs.size(); i += 2) {
					const char c = reader->readChar();
					sheetIDs[i] = c & 0x0F;
					sheetIDs[i + 1] = (c >> 4) & 0x0F;
				}
			}
			else {
				//color table 256
				reader->read((char*)sheetIDs.data(), sheetIDs.size());
			}

			m_spriteSheets.push_back(sheetIDs);
		}
		reader->close();

	}
	catch (PG_Exception& e) {
		OUT_ERROR("Failed to import SH! : " << e.what());
		clear();
		return FAILURE;
	}
	catch (...) {
		OUT_ERROR("Failed to import SH!");
		clear();
		return FAILURE;
	}
#if DUMP_ON_OPEN
	OUT_INFO("=== OPEN INFO ===");

	OUT_INFO("Header: ");
	OUT_INFO(" * number_of_animations: " << m_header.number_of_animations);
	OUT_INFO(" * number_of_bundels: " << m_header.number_of_bundels);
	OUT_INFO(" * number_of_colortables: " << m_header.number_of_colortables);
	OUT_INFO(" * number_of_sheetInfos: " << m_header.number_of_sheetInfos);
	OUT_INFO(" * number_of_cutouts: " << m_header.number_of_cutouts);
	OUT_INFO(" * number_of_keyframes: " << m_header.number_of_keyframes);
	OUT_INFO(" * number_of_colortable_data: " << m_header.number_of_colortable_data);
	OUT_INFO(" * number_of_sheet_data: " << m_header.number_of_sheet_data);
	OUT_INFO("m_animations: " << m_animations.size());
	OUT_INFO("m_layers: " << m_layers.size());
	if (!m_layers.empty()) {
		OUT_INFO(" * start_cutout: " << m_layers[0].start_cutout);
		OUT_INFO(" * number_of_cutouts: " << m_layers[0].number_of_cutouts);
	}
	OUT_INFO("m_numberOfColortables: " << m_numberOfColortables.size());
	OUT_INFO(" * num: " << m_numberOfColortables[0]);
	OUT_INFO("m_sheetsInfos: " << m_sheetsInfos.size());
	OUT_INFO(" * offset: " << m_sheetsInfos[0].offset);
	OUT_INFO(" * height: " << m_sheetsInfos[0].height);
	//OUT_INFO(" * width: " << m_sheektsInfos[0].width);
	OUT_INFO(" * power_colortablesize: " << m_sheetsInfos[0].power_colortablesize);
	OUT_INFO(" * power_width: " << int(m_sheetsInfos[0].power_width));
	OUT_INFO(" * power_height: " << int(m_sheetsInfos[0].power_height));
	OUT_INFO("m_keyframesData: " << m_keyframesData.size());
	OUT_INFO("m_cutouts: " << m_cutouts.size());
	OUT_INFO(" * sheet: " << (int)m_cutouts[0].sheet);
	OUT_INFO(" * external_sheet: " << (int)m_cutouts[0].external_sheet);
	OUT_INFO(" * colortable: " << (int)m_cutouts[0].colortable);
	OUT_INFO("m_colortables: " << m_colortables.size());
	OUT_INFO("m_spriteSheets: " << m_spriteSheets.size());

	OUT_INFO("=== OPEN INFO END ===");


	{
		auto tiani = m_animations.begin();
		ofstream keyframesfile;
		keyframesfile.open((file.getName() + "_Keyframes.txt").c_str());
		keyframesfile << "Keyframes:\n";
		for (unsigned int i = 0; i < m_keyframesData.size(); i++) {

			if (tiani != m_animations.end() && tiani->start_keyframe == i) {
				keyframesfile << " Keyframe Start: " << i << " ID: " << tiani->id << "\n";
				tiani++;
			}
			else
				keyframesfile << " Keyframe: " << i << "\n";

			//keyframesfile << " * bundel_index: "<<m_keyframesData[i].bundel_index <<"\n";
			keyframesfile << " * duration: " << (int)m_keyframesData[i].duration << "\n";
			keyframesfile << " * type: " << (int)m_keyframesData[i].type << "\n";

			keyframesfile << " * global_x: " << m_keyframesData[i].global_x << "\n";
			keyframesfile << " * global_y: " << m_keyframesData[i].global_y << "\n";
			keyframesfile << " ===============================\n";
		}

		keyframesfile.close();

	}

	{
		ofstream dump;
		dump.open((file.getName() + "_bundels.txt").c_str());
		dump << "Bundels/Layers:\n";
		unsigned int count = 0;
		for (const shfileLayers& lay : m_layers) {
			dump << " count: " << count << "\n";
			dump << " * start_cutout: " << lay.start_cutout << "\n";
			dump << " * number_of_cutouts: " << lay.number_of_cutouts << "\n";
			dump << " ===============================\n";
			count++;
		}

		dump.close();
	}

	{
		ofstream dump;
		dump.open((file.getName() + "_Cutouts.txt").c_str());
		unsigned int count = 0;
		dump << "Cutouts\n";
		for (const shfileCutout& cut : m_cutouts) {
			dump << " count: " << count << "\n";
			dump << " * sheet: " << (int)cut.sheet << "\n";
			dump << " * external_sheet: " << (int)cut.external_sheet << "\n";
			dump << " * colortable: " << (int)cut.colortable << "\n";
			dump << " * transparency: " << (int)cut.transparency << "\n";
			dump << " * mic: " << (int)cut.mic << "\n";
			dump << " * x: " << cut.x << "\n";
			dump << " * y: " << cut.y << "\n";
			dump << " * width: " << cut.width << "\n";
			dump << " * height: " << cut.height << "\n";
			dump << " * scalex: " << cut.scalex << "\n";
			dump << " * scaley: " << cut.scaley << "\n";
			dump << " ===============================\n";
			count++;
		}

		{
			ofstream dump;
			dump.open((file.getName() + "_sheetInfo.txt").c_str());
			dump << "SheetInfo\n";
			unsigned int count = 0;
			for (const shfileSheetInfo& info : m_sheetsInfos) {
				dump << " count: " << count << "\n";
				dump << " * offset: " << info.offset << "\n";
				dump << " * width: " << info.width << "\n";
				dump << " * height: " << info.height << "\n";
				dump << " * power_colortablesize: " << info.power_colortablesize << "\n";
				dump << " * power_width: " << (int)info.power_width << "\n";
				dump << " * power_height: " << (int)info.power_height << "\n";
				dump << " ===============================\n";
				count++;
			}
		}

		dump.close();
	}
#endif
	return SUCCESS;
}


bool PG_SH::save(const std::string& file) {
	if (file.empty()) {
		OUT_ERROR("Given filename is empty!");
		return FAILURE;
	}

	return save(PG_File(file));
}

bool PG_SH::save(const PG_File& file) {
	if (m_cutouts.empty() || m_colortables.empty() || m_spriteSheets.empty()) {
		OUT_ERROR("Not enough data for save operation!");
		return FAILURE;
	}

	for (ColorTable& colortable : m_colortables) {
		if (colortable.size() != m_colortables.front().size()) {
			OUT_ERROR("All color tables need to have the same size!");
			return FAILURE;
		}
	}
	bool result = SUCCESS;
	try {
		PG::STREAM::OutByteFile writer(file);

		if (!writer.isOpen()) {
			OUT_ERROR("Failed to write file: '" << file << "'!");
			writer.close();
			return FAILURE;
		}
		result = save(&writer);
	
		writer.close();


	}
	catch (PG_Exception& e) {
		OUT_ERROR("Failed to export SH! : " << e.what());
		clear();
		return FAILURE;
	}
	catch (...) {
		OUT_ERROR("Failed to export SH!");
		clear();
		return FAILURE;
	}

	return result;
}

bool PG_SH::save(PG::STREAM::Out* stream) {

	//write header
	m_header.number_of_animations = m_animations.size();
	m_header.number_of_bundels = m_layers.size();
	m_header.number_of_colortables = m_colortables.size();
	m_header.number_of_sheetInfos = m_spriteSheets.size();
	m_header.number_of_cutouts = m_cutouts.size();
	m_header.number_of_keyframes = m_keyframesData.size();

	m_header.number_of_colortable_data = m_colortables.size();
	m_header.number_of_sheet_data = m_spriteSheets.size();

	stream->write((char*)&m_header, sizeof(shfileHeader));

	//prepare data
	m_numberOfColortables.resize(m_header.number_of_colortables);
	for (unsigned int& i : m_numberOfColortables)
		i = m_colortables[i].size() / 16;


	m_sheetsInfos.resize(m_spriteSheets.size());

	//prepere colortable
	for (ColorTable& colortable : m_colortables) {
		//flip Red and Blue
		for (glm::rgba& color : colortable) {
			const char r = color.r;
			color.r = color.b;
			color.b = r;
		}
	}

	//prepere adresses
	m_addresses.resize(4);
	m_addresses[0] = sizeof(shfileHeader) + m_addresses.size() * sizeof(int) + m_animations.size() * sizeof(shfileAnimation)
		+ m_layers.size() * sizeof(shfileLayers) + m_numberOfColortables.size() * sizeof(unsigned int)
		+ m_sheetsInfos.size() * sizeof(shfileSheetInfo);
	m_addresses[1] = m_addresses[0] + m_keyframesData.size() * sizeof(shfileKeyframe);
	m_addresses[2] = m_addresses[1] + m_cutouts.size() * sizeof(shfileCutout);
	m_addresses[3] = m_addresses[2] + m_colortables.size() * m_colortables.front().size() * sizeof(glm::rgba);

	//prepere sheets infos
	unsigned int count = 0;
	unsigned int address_to_image = 0;
	for (shfileSheetInfo& info : m_sheetsInfos) {
		const PG_UCharImage& sheetIDs = m_spriteSheets[count];
		info.offset = m_addresses[3] + address_to_image;

		info.width = sheetIDs.getWidth();
		info.height = sheetIDs.getHeight();

		info.power_width = log10(sheetIDs.getWidth()) / 0.301029995f; //sqrt (sheetIDs.getWidth());
		info.power_height = log10(sheetIDs.getHeight()) / 0.301029995f;

		if (info.power_colortablesize <= 0) {
			int maxValue = 0;
			for (unsigned char c : sheetIDs)
				if (c > maxValue) maxValue = c;

			if (maxValue > 255)
				info.power_colortablesize = 8;
			else
				info.power_colortablesize = 4;

		}
		if (info.power_colortablesize > 4)
			address_to_image += sheetIDs.size();
		else
			address_to_image += sheetIDs.size() / 2;
		count++;
	}

	if (m_sheetsInfos.size() != m_spriteSheets.size()) {
		OUT_ERROR("The number of sheets infos is not the same as the number of sprite sheets!");
		return FAILURE;
	}

	stream->write((char*)m_addresses.data(), m_addresses.size() * sizeof(int));

	stream->write((char*)m_animations.data(), m_animations.size() * sizeof(shfileAnimation));
	stream->write((char*)m_layers.data(), m_layers.size() * sizeof(shfileLayers));
	stream->write((char*)m_numberOfColortables.data(), m_numberOfColortables.size() * sizeof(unsigned int));
	stream->write((char*)m_sheetsInfos.data(), m_sheetsInfos.size() * sizeof(shfileSheetInfo));
	stream->write((char*)m_keyframesData.data(), m_keyframesData.size() * sizeof(shfileKeyframe));
	stream->write((char*)m_cutouts.data(), m_cutouts.size() * sizeof(shfileCutout));


	//write colortables
	for (ColorTable& colortable : m_colortables)
		stream->write((char*)colortable.data(), colortable.size() * sizeof(glm::rgba));


	//write sheet color IDs
	count = 0;
	for (const PG_UCharImage& sheetIDs : m_spriteSheets) {
		if (m_sheetsInfos[count].power_colortablesize == 4) {
			//color table 16
			for (unsigned int i = 0; i < sheetIDs.size(); i += 2) {
				const char c = (sheetIDs[i] & 0x0F) | (sheetIDs[i + 1] & 0x0F) << 4;
				stream->writeChar(c);
			}
		}
		else {
			stream->write((char*)sheetIDs.data(), sheetIDs.size());
		}
		count++;
	}

	return SUCCESS;
}

void PG_SH::clear() {
	m_addresses.clear();
	m_animations.clear();
	m_layers.clear();
	m_numberOfColortables.clear();
	m_sheetsInfos.clear();

	m_keyframesData.clear();
	m_cutouts.clear();

	m_colortables.clear();
	m_spriteSheets.clear();
}