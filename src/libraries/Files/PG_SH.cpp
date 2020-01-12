/*
 * The MIT License (MIT)
 *
 *	Copyright (c) 2016 ProgSys
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
#include <Files/PG_SH.h>
#include <algorithm>
#include <Util/PG_Base.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamOutByteFile.h>
#include <Util/PG_Exception.h>
#include <math.h>

#define DUMP_ON_OPEN false

using namespace PG::FILE;

bool SH::isOpen() const {
	return !m_openedFile.isEmpty();
}

bool SH::open(const std::string& file) {
	if (file.empty()) {
		PG_ERROR_STREAM("Given filename is empty!");
		return FAILURE;
	}

	return open(PG::UTIL::File(file));
}

bool SH::open(const PG::UTIL::File& file) {
	clear();
	if (!file.exists()) {
		PG_ERROR_STREAM("Target file doesn't exist!");
		return FAILURE;
	}

	try {
		PG::STREAM::InByteFile reader(file);
		if (!reader.isopen()) {
			PG_ERROR_STREAM("Coudn't open file!");
			return FAILURE;
		}

		const unsigned int file_size = reader.size();
		if (file_size < 32) {
			clear();
			PG_ERROR_STREAM("Given file is too small!");
			return FAILURE;
		}

		reader.read((char*)& m_header, sizeof(shfileHeader));

		m_addresses.resize(4);
		reader.read((char*)m_addresses.data(), m_addresses.size() * sizeof(int));

		if (file_size < m_addresses[3]) {
			PG_ERROR_STREAM("Given file is too small!");
			return FAILURE;
		}

		m_openedFile = file;

		m_animations.resize(m_header.number_of_animations);
		m_layers.resize(m_header.number_of_bundels);
		m_numberOfColortables.resize(m_header.number_of_colortables);
		m_sheetsInfos.resize(m_header.number_of_sheetInfos);

		m_keyframesData.resize(m_header.number_of_keyframes);
		m_cutouts.resize(m_header.number_of_cutouts);

		reader.read((char*)m_animations.data(), m_animations.size() * sizeof(shfileAnimation));
		reader.read((char*)m_layers.data(), m_layers.size() * sizeof(shfileLayers));
		reader.read((char*)m_numberOfColortables.data(), m_numberOfColortables.size() * sizeof(unsigned int));
		reader.read((char*)m_sheetsInfos.data(), m_sheetsInfos.size() * sizeof(shfileSheetInfo));
		reader.seek(m_addresses[0]);
		reader.read((char*)m_keyframesData.data(), m_keyframesData.size() * sizeof(shfileKeyframe));
		reader.seek(m_addresses[1]);
		reader.read((char*)m_cutouts.data(), m_cutouts.size() * sizeof(shfileCutout));


		//read colortables
		reader.seek(m_addresses[2]);
		m_colortables.resize(m_header.number_of_colortables);

		for (ColorTable& colortable : m_colortables) {
			colortable.resize(16 * m_numberOfColortables[0]);
			reader.read((char*)colortable.data(), colortable.size() * sizeof(PG::UTIL::rgba));
			//flip Red and Blue
			for (PG::UTIL::rgba& color : colortable) {
				const char r = color.r;
				color.r = color.b;
				color.b = r;
			}
		}


		//read sheet color IDs
		for (const shfileSheetInfo& sheet : m_sheetsInfos) {
			PG::UTIL::IDImage sheetIDs(sheet.width, sheet.height);
			reader.seek(sheet.offset);
			if (sheet.power_colortablesize == 4) { // 2^sheet.power_colortablesize
				//color table 16
				for (unsigned int i = 0; i < sheetIDs.size(); i += 2) {
					const char c = reader.readChar();
					sheetIDs[i] = c & 0x0F;
					sheetIDs[i + 1] = (c >> 4) & 0x0F;
				}
			}
			else {
				//color table 256
				reader.read((char*)sheetIDs.data(), sheetIDs.size());
			}

			m_spriteSheets.push_back(sheetIDs);
		}
		reader.close();

	}
	catch (PG::UTIL::Exception& e) {
		PG_ERROR_STREAM("Failed to import SH! : " << e.what());
		clear();
		return FAILURE;
	}
	catch (...) {
		PG_ERROR_STREAM("Failed to import SH!");
		clear();
		return FAILURE;
	}
#ifdef _DEBUG
	PG_INFO_STREAM("=== OPEN INFO ===")

		PG_INFO_STREAM("Header: ")
		PG_INFO_STREAM(" * number_of_animations: " << m_header.number_of_animations)
		PG_INFO_STREAM(" * number_of_bundels: " << m_header.number_of_bundels)
		PG_INFO_STREAM(" * number_of_colortables: " << m_header.number_of_colortables)
		PG_INFO_STREAM(" * number_of_sheetInfos: " << m_header.number_of_sheetInfos)
		PG_INFO_STREAM(" * number_of_cutouts: " << m_header.number_of_cutouts)
		PG_INFO_STREAM(" * number_of_keyframes: " << m_header.number_of_keyframes)
		PG_INFO_STREAM(" * number_of_colortable_data: " << m_header.number_of_colortable_data)
		PG_INFO_STREAM(" * number_of_sheet_data: " << m_header.number_of_sheet_data)
		PG_INFO_STREAM("m_animations: " << m_animations.size());
	PG_INFO_STREAM("m_layers: " << m_layers.size());
	if (!m_layers.empty()) {
		PG_INFO_STREAM(" * start_cutout: " << m_layers[0].start_cutout)
			PG_INFO_STREAM(" * number_of_cutouts: " << m_layers[0].number_of_cutouts)
	}
		PG_INFO_STREAM("m_numberOfColortables: " << m_numberOfColortables.size());
	PG_INFO_STREAM(" * num: " << m_numberOfColortables[0]);
		PG_INFO_STREAM("m_sheetsInfos: " << m_sheetsInfos.size());
	PG_INFO_STREAM(" * offset: " << m_sheetsInfos[0].offset);
		PG_INFO_STREAM(" * height: " << m_sheetsInfos[0].height);
		//PG_INFO_STREAM(" * width: " << m_sheektsInfos[0].width);
		PG_INFO_STREAM(" * power_colortablesize: " << m_sheetsInfos[0].power_colortablesize);
		PG_INFO_STREAM(" * power_width: " << int(m_sheetsInfos[0].power_width));
		PG_INFO_STREAM(" * power_height: " << int(m_sheetsInfos[0].power_height));
		PG_INFO_STREAM("m_keyframesData: " << m_keyframesData.size());
	PG_INFO_STREAM("m_cutouts: " << m_cutouts.size());
	PG_INFO_STREAM(" * sheet: " << (int)m_cutouts[0].sheet);
		PG_INFO_STREAM(" * external_sheet: " << (int)m_cutouts[0].external_sheet);
		PG_INFO_STREAM(" * colortable: " << (int)m_cutouts[0].colortable);
		PG_INFO_STREAM("m_colortables: " << m_colortables.size());
	PG_INFO_STREAM("m_spriteSheets: " << m_spriteSheets.size());

	PG_INFO_STREAM("=== OPEN INFO END ===")

#if true
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
#endif
	return SUCCESS;
}

bool SH::save(const std::string& file) {
	if (file.empty()) {
		PG_ERROR_STREAM("Given filename is empty!");
		return FAILURE;
	}

	return save(PG::UTIL::File(file));
}

bool SH::save(const PG::UTIL::File& file) {
	if (m_cutouts.empty() || m_colortables.empty() || m_spriteSheets.empty()) {
		PG_ERROR_STREAM("Not enough data for save operation!");
		return FAILURE;
	}

	for (ColorTable& colortable : m_colortables) {
		if (colortable.size() != m_colortables.front().size()) {
			PG_ERROR_STREAM("All color tables need to have the same size!");
			return FAILURE;
		}
	}

	try {
		PG::STREAM::OutByteFile writer(file);

		if (!writer.isOpen()) {
			PG_ERROR_STREAM("Failed to write file: '" << file << "'!");
			writer.close();
			return FAILURE;
		}

		//write header
		m_header.number_of_animations = m_animations.size();
		m_header.number_of_bundels = m_layers.size();
		m_header.number_of_colortables = m_colortables.size();
		m_header.number_of_sheetInfos = m_spriteSheets.size();
		m_header.number_of_cutouts = m_cutouts.size();
		m_header.number_of_keyframes = m_keyframesData.size();

		m_header.number_of_colortable_data = m_colortables.size();
		m_header.number_of_sheet_data = m_spriteSheets.size();

		writer.write((char*)& m_header, sizeof(shfileHeader));

		//prepare data
		m_numberOfColortables.resize(m_header.number_of_colortables);
		for (unsigned int& i : m_numberOfColortables)
			i = m_colortables[i].size() / 16;


		m_sheetsInfos.resize(m_spriteSheets.size());

		//prepere colortable
		for (ColorTable& colortable : m_colortables) {
			//flip Red and Blue
			for (PG::UTIL::rgba& color : colortable) {
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
		m_addresses[3] = m_addresses[2] + m_colortables.size() * m_colortables.front().size() * sizeof(PG::UTIL::rgba);

		//prepere sheets infos
		unsigned int count = 0;
		unsigned int address_to_image = 0;
		for (shfileSheetInfo& info : m_sheetsInfos) {
			const PG::UTIL::IDImage& sheetIDs = m_spriteSheets[count];
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

		assert_Test("There are no sheets infos!", m_sheetsInfos.empty());
		assert_Test("There are no sprite sheets!", m_spriteSheets.empty());
		assert_Test("The number of sheets infos is not the same as the number of sprite sheets!", m_sheetsInfos.size() != m_spriteSheets.size());

		writer.write((char*)m_addresses.data(), m_addresses.size() * sizeof(int));

		writer.write((char*)m_animations.data(), m_animations.size() * sizeof(shfileAnimation));
		writer.write((char*)m_layers.data(), m_layers.size() * sizeof(shfileLayers));
		writer.write((char*)m_numberOfColortables.data(), m_numberOfColortables.size() * sizeof(unsigned int));
		writer.write((char*)m_sheetsInfos.data(), m_sheetsInfos.size() * sizeof(shfileSheetInfo));
		//reader.seek(m_addresses[0]);
		assert_Test("Keyframes start address is wrong", m_addresses[0] != writer.pos());
		writer.write((char*)m_keyframesData.data(), m_keyframesData.size() * sizeof(shfileKeyframe));
		assert_Test("Cutout start address is wrong", m_addresses[1] != writer.pos());
		//m_addresses[1] = writer.pos();
		//reader.seek(m_addresses[1]);
		writer.write((char*)m_cutouts.data(), m_cutouts.size() * sizeof(shfileCutout));


		//write colortables
		assert_Test("Color table start address is wrong", m_addresses[2] != writer.pos());
		//m_addresses[2] = writer.pos();

		for (ColorTable& colortable : m_colortables)
			writer.write((char*)colortable.data(), colortable.size() * sizeof(PG::UTIL::rgba));


		//write sheet color IDs
		count = 0;
		for (const PG::UTIL::IDImage& sheetIDs : m_spriteSheets) {
			if (m_sheetsInfos[count].power_colortablesize == 4) {
				//color table 16
				for (unsigned int i = 0; i < sheetIDs.size(); i += 2) {
					const char c = (sheetIDs[i] & 0x0F) | (sheetIDs[i + 1] & 0x0F) << 4;
					writer.writeChar(c);
				}
			}
			else {
				writer.write((char*)sheetIDs.data(), sheetIDs.size());
			}
			count++;
		}

		writer.close();


	}
	catch (PG::UTIL::Exception& e) {
		PG_ERROR_STREAM("Failed to export SH! : " << e.what());
		clear();
		return FAILURE;
	}
	catch (...) {
		PG_ERROR_STREAM("Failed to export SH!");
		clear();
		return FAILURE;
	}

	return SUCCESS;
}

void SH::clear() {
	m_openedFile.clear();
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

SH::~SH() {
	// TODO Auto-generated destructor stub
}

