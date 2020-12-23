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

#include <Files/PG_PSPFS.h>
#include <Util/PG_Base.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamOutByteFile.h>
#include <Util/PG_StringUtil.h>
#include <Util/PG_Exception.h>
#include <Files/PG_TX2.h>

#include <Files/PG_FileTests.h>

#include <algorithm>

namespace PG {
	namespace FILE {


		PSPFS::PSPFS() {

		}

		PSPFS::PSPFS(const PG::UTIL::File& file) {
			open(file);
		}

		PSPFS::PSPFS(const std::string& file) :
			PSPFS(PG::UTIL::File(file)) {}


		bool PSPFS::open(const PG::UTIL::File& file, PercentIndicator* percent) {
			clear();

			m_file = file;
			m_file_buffer = PG::UTIL::File(file.getPath() + "_buffer");

			if (!m_file.exists()) {
				PG_ERROR_STREAM("Target file does not exist!");
				m_file.clear();
				return true;
			}

			try {
				PG::STREAM::InByteFile reader(m_file);
				if (!reader.isopen()) return true;

				//Check magic number
				{
					const std::string magic_number = reader.readString(8);
					if (magic_number != "PSPFS_V1") {
						PG_ERROR_STREAM("Wrong magic number, it should be 'PSPFS_V1' but it was '" << magic_number << "'!");
						m_file.clear();
						return true;
					}
				}

				//Get the number of files
				const unsigned int number_of_files = reader.readUnsignedInt();
				if (number_of_files > 50000) {
					PG_ERROR_STREAM("Number_of_files (" << number_of_files << ") is too big! ");
					return true;
				}

				reader.readUnsignedInt(); //Unknown

				m_fileInfos.reserve(number_of_files);
				//read the file infos
				for (unsigned int i = 0; i < number_of_files; ++i) {
					if (percent) percent->percent = (i / float(number_of_files)) * 100.0;
					fileInfo info;
					std::string filename = reader.readString(40);
					filename = filename.substr(0, filename.find_first_of(char(0x00)));
					std::transform(filename.begin(), filename.end(), filename.begin(), ::toupper);
					info.name = filename;
					info.decompressedFileSize = reader.readUnsignedInt();
					info.setSize(reader.readUnsignedInt());
					info.setOffset(reader.readUnsignedInt());

					//tests
					const unsigned int current_pos = reader.pos();
					reader.seek(info.offset);

					if (isIMYPackage(reader)) {
						info.fileType = fileInfo::COLA;
					}
					else {
						reader.seek(current_pos);
						if (isIMY(reader)) {
							info.fileType = fileInfo::IMY;
						}
					}

					reader.seek(current_pos);

					m_fileInfos.push_back(info);
				}

				reader.close();
				m_originalFileSize = m_fileInfos.size();

			}
			catch (PG::UTIL::Exception& e) {
				PG_ERROR_STREAM("Couldn't read given PSPFS_V1 file '" << file << "': " << e.what());
				m_file.clear();
				return true;
			}
			catch (...) {
				PG_ERROR_STREAM("Couldn't read given PSPFS_V1 file '" << file << "'!");
				m_file.clear();
				return true;
			}

			return false;

		}


		bool PSPFS::insert(const PG::UTIL::File& file) {
			if (m_file.isEmpty()) {
				PG_ERROR_STREAM("No PSPFS_V1 file opened.");
				return true;
			}

			if (!file.exists()) {
				PG_ERROR_STREAM("File doesn't exist!");
				return true;
			}

			fileInfo addFile;
			std::string name = file.getFile();
			if (name.size() > 40) {
				PG_ERROR_STREAM("File name is too big, max 28 chars!");
				return true;
			}
			std::transform(name.begin(), name.end(), name.begin(), ::toupper);
			addFile.setName(name);

			if (name == "DUMMY.DAT") {
				PG_ERROR_STREAM("You can't add a file with the name DUMMY.DAT.");
				return true;
			}


			addFile.externalFile = file;
			addFile.setSize(file.size());

			getFileProperties(addFile);

			//file is already inside?
			auto it = std::find_if(m_fileInfos.begin(), m_fileInfos.end(), [addFile](const fileInfo& info) {
				return info.getName() == addFile.getName();
				});

			if (it != m_fileInfos.end()) {
				(*it) = addFile;
			}
			else {
				//replace a dummy file or add at end
				it = std::find_if(m_fileInfos.begin(), m_fileInfos.end(), [addFile](const fileInfo& info) {
					return info.getName() == "DUMMY.DAT";
					});

				if (it != m_fileInfos.end()) {
					(*it) = addFile;
					//PG_INFO_STREAM("Replaced dummy "<<addFile.name);
				}
				else {
					m_fileInfos.push_back(addFile);
					//PG_INFO_STREAM("Insert new "<<addFile.name);
				}
			}
			m_changed = true;

			return false;

		}

		bool PSPFS::remove(fileInfo_iterator it) {
			if (it == m_fileInfos.end()) return false;
			const unsigned int index = std::distance(m_fileInfos.begin(), it);
			if (it < m_fileInfos.end()) {
				if (index < m_originalFileSize) {
					if (it->name == "DUMMY.DAT") {
						m_fileInfos.erase(it);
					}
					else {
						it->setAsDummy(it->offset);
					}
				}
				else {
					m_fileInfos.erase(it);
				}
				m_changed = true;
			}
			else {
				PG_ERROR_STREAM("File '" << it->getName() << " found!");
				return true;
			}
			return false;
		}

		//helper for some experiments, is not used
		inline void swap(std::vector<fileInfo>& file_infos, unsigned int index1, unsigned int index2) {
			fileInfo swap = file_infos[index1];
			file_infos[index1] = file_infos[index2];
			file_infos[index2] = swap;
		}

		inline void writeDUMMY(PG::STREAM::OutByteFile& writer, unsigned int& next_header_offset, unsigned int& next_file_offset, unsigned int& dummy_traget_offset) {
			if (dummy_traget_offset == 0) {
				dummy_traget_offset = next_file_offset;

				writer.setPosition(dummy_traget_offset);
				char* d = new char[512]();
				d[0] = 'D';
				d[1] = 'U';
				d[2] = 'M';
				d[3] = 'M';
				d[4] = 'Y';
				writer.write(d, 512);
				delete[] d;
				next_file_offset += 512;
			}

			writer.setPosition(next_header_offset);
			writer.writeString("DUMMY.DAT");
			for (unsigned short i = 9; i < 40; ++i) //fill rest with 0
				writer.writeChar(0);

			writer.writeInt(0);
			writer.writeInt(5);
			writer.writeInt(dummy_traget_offset);
			next_header_offset += 52;

		}

		bool PSPFS::save(const PG::UTIL::File& targetfile, PercentIndicator* percent) {
			// do we even have a DAT file open
			if (m_file.isEmpty()) {
				PG_ERROR_STREAM("No PSPFS file opened.");
				return FAILURE;
			}

			//do we overwrtie the original or do we save as?
			bool overwrite = false;
			PG::UTIL::File target;
			if (targetfile == m_file) {
				target = PG::UTIL::File(m_file.getPath() + "_buffer");
				overwrite = true;
			}
			else {
				target = targetfile;
			}

			//create a butter for file infos
			std::vector<fileInfo> file_infos;
			file_infos.resize(m_fileInfos.size());
			char* c = nullptr;
			try {
				PG::STREAM::OutByteFile writer(target);
				if (!writer.isOpen()) {
					PG_ERROR_STREAM("Failed to open/create '" << target << "'!");
					return FAILURE;
				}
				//setup header
				writer.writeString("PSPFS_V1"); //write magic number

				writer.writeInt(m_fileInfos.size()); //number of files
				writer.writeInt(0); // mystery int

				unsigned int next_header_offset = writer.getPosition(); //get the first header position

				c = new char[52](); // each header is exactly 52 byte long
				//allocate header space
				for (unsigned int i = 0; i < m_fileInfos.size() + 10; ++i) { //also add a small empty buffer
					writer.write(c, 52);
				}
				delete[] c;
				c = nullptr;

				unsigned int next_file_offset = writer.getPosition(); // the position where we start to write the data of the files
				unsigned int dummy_traget_offset = 0;
				PG::STREAM::InByteFile reader_dat(m_file);
				if (!reader_dat.isopen()) return FAILURE;

				struct fileInfoHeader
				{
					int index;
					unsigned int headerPos;
					fileInfo info;
				};
				std::vector<fileInfoHeader> sortedInfos;
				sortedInfos.reserve(m_fileInfos.size());
				int index = 0;
				std::transform(m_fileInfos.begin(), m_fileInfos.end(), std::back_inserter(sortedInfos), [&index, &next_header_offset](const fileInfo& info) {
					fileInfoHeader item = { index, next_header_offset , info };
					next_header_offset += 52;
					index++;
					return item;
					});
				std::sort(sortedInfos.begin(), sortedInfos.end(), [](const fileInfoHeader& a, const fileInfoHeader& b) {
					return a.info.offset < b.info.offset;
					});

				unsigned int i = 0;
				for (fileInfoHeader& it : sortedInfos) {
					if (percent) {
						percent->percent = (i / float(sortedInfos.size())) * 100.0;
						i++;
					}

					if (it.info.isExternalFile()) {
						if (it.info.getSize() == 0 || it.info.getName().isEmpty() || !it.info.externalFile.exists()) {
							PG_WARN_STREAM("External file '" << it.info.externalFile << "' doesn't exist! Writing empty dummy file instead!");
							it.info.setAsDummy(next_file_offset);
							writeDUMMY(writer, it.headerPos, next_file_offset, dummy_traget_offset);
						}
						else {
							PG_INFO_STREAM("Adding external file '" << it.info.externalFile << "'.");


							PG::STREAM::InByteFile reader_file(it.info.externalFile);
							it.info.decompressedFileSize = isIMYPackage(reader_file);
							reader_file.seek(0);
							const unsigned int file_size = reader_file.size();


							// check if valid
							if (file_size == 0)
								PG_WARN_STREAM("File '" << it.info.externalFile << "' has a size of zero!");

							if (it.info.getSize() != file_size) {
								PG_WARN_STREAM("File '" << it.info.externalFile << "' size is different than expected! (" << it.info.getSize() << " != " << file_size << ")");
								it.info.setSize(file_size);
							}

							//write header
							writer.setPosition(it.headerPos);
							writer.writeString(it.info.getName().getPath());
							for (unsigned short i = (unsigned short)it.info.getName().getPath().size(); i < 40; ++i) //fill rest with 0
								writer.writeChar(0);

							writer.writeInt(it.info.decompressedFileSize);
							writer.writeInt(it.info.getSize());
							writer.writeInt(next_file_offset);
							it.info.setOffset(next_file_offset);

							//write file
							writer.setPosition(next_file_offset);
							c = new char[it.info.getSize()];
							reader_file.read(c, it.info.getSize());
							reader_file.close();
							writer.write(c, it.info.getSize());
							delete[] c;
							c = nullptr;
							next_file_offset += it.info.getSize();
							it.info.clearExternalFile();
						}

					}
					else {
						//PG_INFO_STREAM(current_info.getName()<<" o: "<<current_info.getOffset()<<" s: "<<current_info.getSize());
						if (it.info.getSize() == 0 || it.info.getName().isEmpty() || it.info.getName() == "DUMMY.DAT") {
							it.info.setAsDummy(next_file_offset);
							writeDUMMY(writer, it.headerPos, next_file_offset, dummy_traget_offset);
						}
						else {

							//write header
							writer.setPosition(it.headerPos);
							writer.writeString(it.info.getName().getFile());
							for (unsigned short i = (unsigned short)it.info.getName().getPath().size(); i < 40; ++i) //fill rest with 0
								writer.writeChar(0);

							writer.writeInt(it.info.decompressedFileSize);
							writer.writeInt(it.info.getSize());
							writer.writeInt(next_file_offset);

							//write file
							writer.setPosition(next_file_offset);
							reader_dat.seek(it.info.getOffset());
							it.info.setOffset(next_file_offset);
							c = new char[it.info.getSize()];
							reader_dat.read(c, it.info.getSize());
							writer.write(c, it.info.getSize());
							delete[] c;
							c = nullptr;
							next_file_offset += it.info.getSize();
						}

					}
					file_infos[it.index] = it.info;
				}// for loop end

				file_infos.erase(
					std::remove_if(file_infos.begin(), file_infos.end(), [](const fileInfo& info) { return info.size == 0; }),
					file_infos.end()
				);

				reader_dat.close();
				writer.close();

			}
			catch (PG::UTIL::Exception& e) {
				PG_ERROR_STREAM("Couldn't save PSPFS archive! : " << e.what());
				target.remove();
				return FAILURE;
			}
			catch (...) {
				PG_ERROR_STREAM("Couldn't save PSPFS archive!");
				target.remove();
				return FAILURE;
			}

			if (overwrite) {
				m_file.remove();
				target.rename(m_file.getPath());
				m_fileInfos = std::move(file_infos);
				m_changed = false;
			}

			return SUCCESS;

		}

		void PSPFS::clear() {
			if (m_file_buffer.exists())
				m_file_buffer.remove();
			ExtractorBase::clear();
		}

		char const* PSPFS::getType() const {
			return "PSPFS_V1";
		}


		PSPFS::~PSPFS() {
			if (m_file_buffer.exists())
				m_file_buffer.remove();
		}

	} /* namespace FILE */
} /* namespace PG */
