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

#include <pg/files/PG_PSPFS.h>
#include <pg/util/PG_Base.h>
#include <pg/util/PG_ByteInFileStream.h>
#include <pg/util/PG_BinaryFileWriter.h>
#include <pg/util/PG_StringUtil.h>
#include <pg/util/PG_Exception.h>
#include <pg/files/PG_TX2.h>

namespace PG {
namespace FILE {


filePSPFSInfo::filePSPFSInfo(){}

filePSPFSInfo::filePSPFSInfo(const filePSPFSInfo& info):
		baseInfo(info.baseInfo), unknown(info.unknown)
{}

bool filePSPFSInfo::isExternalFile() const{
	return baseInfo.isExternalFile();
}

void filePSPFSInfo::setNameUnclean(std::string file){
	file = file.substr(0,file.find_first_of(char(0x00)));
	std::transform(file.begin(), file.end(), file.begin(), ::toupper);
	baseInfo.name = file;
}

std::string filePSPFSInfo::getFileExtention() const{
	return baseInfo.name.getFileExtension();
}

void filePSPFSInfo::setAsDummy(unsigned int offset){
	baseInfo.externalFile.clear();
	unknown = 0;
	baseInfo.name = "DUMMY.DAT";
	baseInfo.size = 5;
	baseInfo.offset = offset;

}

void filePSPFSInfo::clearExternalFile(){
	baseInfo.clearExternalFile();
}

bool filePSPFSInfo::operator< (const filePSPFSInfo& str) const{
        return (baseInfo.size < str.baseInfo.size);
}


bool isPSPFS(const std::string& filepath){
	PG::UTIL::ByteInFileStream reader(filepath);
	if(!reader.isopen()) return false;

	if(reader.readString(8) == "PSPFS_V1"){
		return true;
	}
	return false;
}

PSPFS::PSPFS(){

}

PSPFS::PSPFS(const PG::UTIL::File& file){
	open(file);
}

PSPFS::PSPFS(const std::string& file):
		PSPFS(PG::UTIL::File(file)){}


bool PSPFS::open(const PG::UTIL::File& file){
	 clear();

	 m_file = file;
	 m_file_buffer = PG::UTIL::File(file.getPath()+"_buffer");

	 if(!m_file.exists() ){
		 PG_ERROR_STREAM("Target file does not exist!");
		 m_file.clear();
		 return true;
	 }

	 try{
		 PG::UTIL::ByteInFileStream reader(m_file);
		 if(!reader.isopen()) return true;

		 //Check magic number
		 {
			 const std::string magic_number = reader.readString(8);
			 if(magic_number != "PSPFS_V1"){
				 PG_ERROR_STREAM("Wrong magic number, it should be 'PSPFS_V1' but it was '"<<magic_number<<"'!");
				 m_file.clear();
				 return true;
			 }
		 }

		 //Get the number of files
		 const unsigned int number_of_files = reader.readUnsignedInt();
		 if(number_of_files > 50000){
			 PG_ERROR_STREAM("Number_of_files ("<<number_of_files<<") is too big! ");
			 return true;
		 }

		 reader.readUnsignedInt(); //Unknown

		 m_filePSPFSInfos.reserve(number_of_files);
		 //read the file infos
		 for(unsigned int i = 0; i < number_of_files; ++i){
			 filePSPFSInfo info;
			 info.setNameUnclean(reader.readString(40));
			 info.unknown  = reader.readUnsignedInt();
			 info.setSize(reader.readUnsignedInt());
			 info.setOffset(reader.readUnsignedInt());

			 //if(info.name != "DUMMY.DAT")
			 m_filePSPFSInfos.push_back(info);
		 }

		 reader.close();
		 m_originalFileSize = m_filePSPFSInfos.size();

	 }catch (PG::UTIL::Exception& e) {
		 PG_ERROR_STREAM("Couldn't read given PSPFS file '"<<file<<"': "<<e.what());
		 m_file.clear();
		 return true;
	 }catch (...) {
		 PG_ERROR_STREAM("Couldn't read given PSPFS file '"<<file<<"'!");
		 m_file.clear();
		 return true;
	 }

	 return false;

}


bool PSPFS::findfilePSPFSInfo(const PG::UTIL::File& file, filePSPFSInfo& infoOut) const{
	std::string name = file.getFile();
	std::transform(name.begin(), name.end(), name.begin(), ::toupper);
	//PG_INFO_STREAM("Searching for '"<<name<<"'")
	auto it = std::find_if(m_filePSPFSInfos.begin(), m_filePSPFSInfos.end(), [name](const filePSPFSInfo& info){
		return info.getName() == name;
	});

	if(it != m_filePSPFSInfos.end()){
		infoOut = (*it);
		return false;
	}

	return true;
}

bool PSPFS::insert(const PG::UTIL::File& file){
	if(m_file.isEmpty()){
		PG_ERROR_STREAM("No PSPFS file opened.");
		return true;
	}

	if(!file.exists()){
		PG_ERROR_STREAM("File doesn't exist!");
		return true;
	}

	filePSPFSInfo addFile;
	std::string name = file.getFile();
	if(name.size() > 40){
		PG_ERROR_STREAM("File name is too big, max 28 chars!");
		return true;
	}
	std::transform(name.begin(), name.end(), name.begin(), ::toupper);
	addFile.setName(name);

	if(name == "DUMMY.DAT"){
		PG_ERROR_STREAM("You can't add a file with the name DUMMY.DAT.");
		return true;
	}


	addFile.setExternalFile(file);
	addFile.setSize(file.size());

	//file is already inside?
	auto it = std::find_if(m_filePSPFSInfos.begin(), m_filePSPFSInfos.end(), [addFile](const filePSPFSInfo& info){
		return info.getName() == addFile.getName();
	});


	if(it != m_filePSPFSInfos.end()){
		addFile.unknown = (*it).unknown;
		(*it) = addFile;
	}else{
		//replace a dummy file or add at end
		it = std::find_if(m_filePSPFSInfos.begin(), m_filePSPFSInfos.end(), [addFile](const filePSPFSInfo& info){
			return info.getName() == "DUMMY.DAT";
		});

		if(it != m_filePSPFSInfos.end()){
			(*it) = addFile;
		}else
			m_filePSPFSInfos.push_back(addFile);
	}
	m_changed = true;

	return false;

}

bool PSPFS::remove(const PG::UTIL::File& file){
	if(m_file.isEmpty()){
		PG_ERROR_STREAM("No PSPFS file opened.");
		return true;
	}

	std::string name = file.getFile();
	if(name.size() > 40){
		PG_ERROR_STREAM("File name is too big, max 28 chars!");
		return true;
	}

	auto it = std::find_if(m_filePSPFSInfos.begin(), m_filePSPFSInfos.end(), [name](const filePSPFSInfo& info){
		return info.getName() == name;
	});

	if(it != m_filePSPFSInfos.end()){
		//remove the file or make it a dummy file to keep the file order intact
		if(std::distance(m_filePSPFSInfos.begin(), it) > m_originalFileSize){
			m_filePSPFSInfos.erase(it);
		}else{
			(*it).setAsDummy();
		}

		m_changed = true;
		return false;

	}

	PG_ERROR_STREAM("File '"<<file<<"' not found!");
	return true;
}

bool PSPFS::save(){
	return save(m_file);
}

//helper for some experiments, is not used
inline void swap(std::vector<filePSPFSInfo>& file_infos, unsigned int index1, unsigned int index2){
	filePSPFSInfo swap = file_infos[index1];
	file_infos[index1] = file_infos[index2];
	file_infos[index2] = swap;
}

inline void writeDUMMY(PG::UTIL::BinaryFileWriter& writer,unsigned int& next_header_offset, unsigned int& next_file_offset, unsigned int& dummy_traget_offset){
	if(dummy_traget_offset == 0){
		dummy_traget_offset = next_file_offset;

		writer.setPosition(dummy_traget_offset);
		char* d = new char[512]();
		d[0] = 'D';
		d[1] = 'U';
		d[2] = 'M';
		d[3] = 'M';
		d[4] = 'Y';
		writer.write(d,512);
		delete[] d;
		next_file_offset += 512;
	}

	writer.setPosition(next_header_offset);
	writer.writeString("DUMMY.DAT");
	for(unsigned short i = 9; i < 40; ++i) //fill rest with 0
		writer.writeChar(0);

	writer.writeInt(0);
	writer.writeInt(5);
	writer.writeInt(dummy_traget_offset);
	next_header_offset += 52;

}

bool PSPFS::save(const PG::UTIL::File& targetfile){
	// do we even have a DAT file open
	if(m_file.isEmpty()){
		PG_ERROR_STREAM("No PSPFS file opened.");
		return true;
	}

	//do we overwrtie the original or do we save as?
	bool overwrite = false;
	PG::UTIL::File target;
	if(targetfile == m_file){
		target = PG::UTIL::File(m_file.getPath()+"_buffer");
		overwrite = true;
	}else{
		target = targetfile;
	}

	//create a butter for file infos
	std::vector<filePSPFSInfo> file_infos;
	file_infos.reserve(m_filePSPFSInfos.size());
	char* c = nullptr;
	try{
		PG::UTIL::BinaryFileWriter writer(target);
		//setup header
		writer.writeString("PSPFS_V1"); //write magic number

		writer.writeInt(m_filePSPFSInfos.size()); //number of files
		writer.writeInt(0); // mystery int

		unsigned int next_header_offset = writer.getPosition(); //get the first header position

		c = new char[52](); // each header is exactly 52 byte long
		//allocate header space
		for(unsigned int i = 0; i < m_filePSPFSInfos.size()+10; ++i){ //also add a small empty buffer
			writer.write(c,52);
		}
		delete[] c;
		c = nullptr;

		unsigned int next_file_offset = writer.getPosition(); // the position where we start to write the data of the files

		unsigned int dummy_traget_offset = 0;

		PG::UTIL::ByteInFileStream reader_dat(m_file);
		if(!reader_dat.isopen()) return true;

		for(const filePSPFSInfo& info: m_filePSPFSInfos){
			filePSPFSInfo current_info = info; //copy info

			if(current_info.isExternalFile()){
				if(current_info.getSize() == 0 || current_info.getName().isEmpty() ||!current_info.getExternalFile().exists()){
					PG_WARN_STREAM("External file '"<<current_info.getExternalFile()<<"' doesn't exist! Writing empty dummy file instead!");
					current_info.setAsDummy(next_file_offset);
					writeDUMMY(writer,next_header_offset,next_file_offset, dummy_traget_offset );
				}else{
					PG_INFO_STREAM("Adding external file '"<<current_info.getExternalFile()<<"'.");
					PG::UTIL::ByteInFileStream reader_file(current_info.getExternalFile());
					const unsigned int file_size = reader_file.size();


					// check if valid
					if(file_size == 0)
						PG_WARN_STREAM("File '"<<current_info.getExternalFile()<<"' has a size of zero!");

					if(info.getSize() != file_size){
						PG_WARN_STREAM("File '"<<current_info.getExternalFile()<<"' size is different than expected! ("<<current_info.getSize()<<" != "<<file_size<<")");
						current_info.setSize(file_size);
					}

					//write header
					writer.setPosition(next_header_offset);
					writer.writeString(current_info.getName().getPath());
					for(unsigned short i = (unsigned short)current_info.getName().getPath().size(); i < 40; ++i) //fill rest with 0
						writer.writeChar(0);

					writer.writeInt(current_info.unknown);
					writer.writeInt(current_info.getSize());
					writer.writeInt(next_file_offset);
					current_info.setOffset(next_file_offset);
					next_header_offset += 52;

					//write file
					writer.setPosition(next_file_offset);
					c = new char[current_info.getSize()];
					reader_file.read(c,current_info.getSize());
					reader_file.close();
					writer.write(c, current_info.getSize());
					delete[] c;
					c = nullptr;
					next_file_offset += current_info.getSize();
					current_info.clearExternalFile();
				}

			}else{
				//PG_INFO_STREAM(current_info.getName()<<" o: "<<current_info.getOffset()<<" s: "<<current_info.getSize());
				if(current_info.getSize() == 0 || current_info.getName().isEmpty() || current_info.getName() == "DUMMY.DAT"){
					current_info.setAsDummy(next_file_offset);
					writeDUMMY(writer,next_header_offset,next_file_offset,dummy_traget_offset );
				}else{

					//write header
					writer.setPosition(next_header_offset);
					writer.writeString(current_info.getName().getFile());
					for(unsigned short i = (unsigned short)current_info.getName().getPath().size(); i < 40; ++i) //fill rest with 0
						writer.writeChar(0);

					writer.writeInt(current_info.unknown);
					writer.writeInt(current_info.getSize());
					writer.writeInt(next_file_offset);
					next_header_offset += 52;

					//write file
					writer.setPosition(next_file_offset);
					reader_dat.seek(current_info.getOffset());
					current_info.setOffset(next_file_offset);
					c = new char[current_info.getSize()];
					reader_dat.read(c,current_info.getSize());
					writer.write(c, current_info.getSize());
					delete[] c;
					c = nullptr;
					next_file_offset += current_info.getSize();
				}

				file_infos.push_back(current_info);
			}



		}// for loop end

		reader_dat.close();
		writer.close();

	}catch (PG::UTIL::Exception& e) {
		 PG_ERROR_STREAM("Couldn't save PSPFS archive! : "<<e.what());
		 target.remove();
		 return true;
	}catch (...) {
		 PG_ERROR_STREAM("Couldn't save PSPFS archive!");
		 target.remove();
		 return true;
	}

	 if(overwrite){
		 m_file.remove();
		 target.rename(m_file.getPath());
		 m_filePSPFSInfos = file_infos;
		 m_changed = false;
	 }

	return false;

}

void PSPFS::clear(){
	if(m_file_buffer.exists())
		m_file_buffer.remove();
	m_file_buffer.clear();
	m_file.clear();
	m_filePSPFSInfos.clear();
	m_changed = false;
	m_originalFileSize = 0;
}

unsigned int PSPFS::size() const{
	return m_filePSPFSInfos.size();
}

const PG::UTIL::File& PSPFS::getOpendFile() const{
	return m_file;
}

bool PSPFS::isEmpty() const{
	return m_filePSPFSInfos.empty();
}

bool PSPFS::find(const PG::UTIL::File& file, fileInfo& infoOut) const{
	std::string name = file.getFile();
	std::transform(name.begin(), name.end(), name.begin(), ::toupper);
	auto it = std::find_if(m_filePSPFSInfos.begin(), m_filePSPFSInfos.end(), [name](const filePSPFSInfo& info){
		return info.getName() == name;
	});

	if(it != m_filePSPFSInfos.end()){
		infoOut = (*it).baseInfo;
		return true;
	}

	return false;
}

const fileInfo& PSPFS::get(unsigned int index) const{
	return m_filePSPFSInfos[index].baseInfo;
}

PSPFS::~PSPFS(){
	if(m_file_buffer.exists())
		m_file_buffer.remove();
}

} /* namespace FILE */
} /* namespace PG */
