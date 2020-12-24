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

		inline void writeDUMMYPSPFS(PG::STREAM::OutByteFile& writer, PSPFS::FileTableEntry& entry, unsigned int& next_file_offset, unsigned int& start_dummy_offset) {

			//set header data
			memcpy(entry.name, "DUMMY.DAT", 9);
			entry.decompressedFileSize = 0;
			entry.fileSize = 5; //the file is small, the buffer is big

			//write dummy data
			if (start_dummy_offset == 0) {
				writer.setPosition(next_file_offset);
				start_dummy_offset = next_file_offset;
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

			entry.offset = start_dummy_offset;
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

				const unsigned int start_header_offset = writer.getPosition(); //get the first header position

				std::vector<FileTableEntry> fileTable(file_infos.size());
				//fill empty header (will be overwritten later)
				writer.write((char*)fileTable.data(), fileTable.size() * sizeof(FileTableEntry));

				const unsigned int start_file_offset = writer.getPosition(); // the position where we start to write the data of the files
				unsigned int start_dummy_offset = 0; // dummy file is only stored once

				PG::STREAM::InByteFile reader_dat(m_file);
				if (!reader_dat.isopen()) return FAILURE;

				//we keep the memory order (indeces must stay the same)
				struct OffsetOrderInfo {
					std::vector<FileTableEntry>::iterator itTable;
					std::vector<fileInfo>::iterator itInfo;
					fileInfo oldInfo;
				};
				std::vector<OffsetOrderInfo> sortedInfos;
				{
					sortedInfos.reserve(m_fileInfos.size());
					auto itTable = fileTable.begin();
					auto itInfo = file_infos.begin();
					for (const fileInfo& oldInfo : m_fileInfos) {
						*itInfo = oldInfo; //copy
						sortedInfos.emplace_back(OffsetOrderInfo{ itTable, itInfo, oldInfo });
						++itTable;
						++itInfo;
					}
					std::sort(sortedInfos.begin(), sortedInfos.end(), [](const OffsetOrderInfo& a, const OffsetOrderInfo& b) {
						return a.oldInfo.offset < b.oldInfo.offset;
						});
				}

				auto setHeader = [](std::vector<FileTableEntry>::iterator itTable, fileInfo& info, int file_offset) {
					const std::string fileName = info.getName().getFile();
					memcpy(itTable->name, fileName.c_str(), std::min(fileName.size(), (std::size_t)40));
					itTable->fileSize = info.getSize();
					itTable->decompressedFileSize = info.decompressedFileSize;
					itTable->offset = file_offset;
					info.setOffset(file_offset);
				};

				//write to file
				unsigned int i = 0;
				unsigned int current_file_offset = start_file_offset;
				for (auto& [itTable, itInfo, oldInfo] : sortedInfos) {
					if (percent) {
						percent->percent = (i / float(sortedInfos.size())) * 100.0;
						i++;
					}

					if (itInfo->isExternalFile()) {
						if (itInfo->getSize() == 0 || itInfo->getName().isEmpty() || !itInfo->externalFile.exists()) {
							PG_WARN_STREAM("External file '" << itInfo->externalFile << "' doesn't exist! Writing empty dummy file instead!");
							writeDUMMYPSPFS(writer, *itTable, current_file_offset, start_dummy_offset);
							itInfo->setAsDummy(start_dummy_offset);
						}
						else {
							PG_INFO_STREAM("Adding external file '" << itInfo->externalFile << "'.");


							PG::STREAM::InByteFile reader_file(itInfo->externalFile);
							itInfo->decompressedFileSize = isIMYPackage(reader_file);
							reader_file.seek(0);
							const unsigned int file_size = reader_file.size();


							// check if valid
							if (file_size == 0)
								PG_WARN_STREAM("File '" << itInfo->externalFile << "' has a size of zero!");

							if (itInfo->getSize() != file_size) {
								PG_WARN_STREAM("File '" << itInfo->externalFile << "' size is different than expected! (" << itInfo->getSize() << " != " << file_size << ")");
								itInfo->setSize(file_size);
							}

							//set header
							setHeader(itTable, *itInfo, current_file_offset);

							//write file
							writer.setPosition(current_file_offset);
							c = new char[itInfo->getSize()];
							reader_file.read(c, itInfo->getSize());
							reader_file.close();
							writer.write(c, itInfo->getSize());
							delete[] c;
							c = nullptr;
							current_file_offset += itInfo->getSize();
							itInfo->clearExternalFile();
						}

					}
					else {
						//PG_INFO_STREAM(current_info.getName()<<" o: "<<current_info.getOffset()<<" s: "<<current_info.getSize());
						if (itInfo->getSize() == 0 || itInfo->getName().isEmpty() || itInfo->getName() == "DUMMY.DAT") {
							writeDUMMYPSPFS(writer, *itTable, current_file_offset, start_dummy_offset);
							itInfo->setAsDummy(start_dummy_offset);
						}
						else {

							//set header
							const int oldOffset = itInfo->getOffset();
							setHeader(itTable, *itInfo, current_file_offset);

							//write file
							writer.setPosition(current_file_offset);
							reader_dat.seek(oldOffset);
							c = new char[itInfo->getSize()];
							reader_dat.read(c, itInfo->getSize());
							writer.write(c, itInfo->getSize());
							delete[] c;
							c = nullptr;
							current_file_offset += itInfo->getSize();
						}

					}
				
				}// for loop end

				file_infos.erase(
					std::remove_if(file_infos.begin(), file_infos.end(), [](const fileInfo& info) { return info.size == 0; }),
					file_infos.end()
				);

				//write header
				writer.seek(start_header_offset);
				writer.write((char*)fileTable.data(), fileTable.size() * sizeof(FileTableEntry));

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
