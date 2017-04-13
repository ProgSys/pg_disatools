/*
 * PG_PS3FS.cpp
 *
 *  Created on: 13.01.2017
 *      Author: ProgSys
 */

#include <pg/files/PG_PS3FS.h>
#include <pg/util/PG_SizeDefinitions.h>
#include <pg/util/PG_Base.h>
#include <pg/stream/PG_StreamInByteFile.h>
#include <pg/stream/PG_StreamOutByteFile.h>
#include <pg/util/PG_StringUtil.h>
#include <pg/util/PG_Exception.h>
#include <pg/files/PG_TX2.h>

#include <pg/files/PG_FileTests.h>

namespace PG {
namespace FILE {


PS3FS::PS3FS() {
	// TODO Auto-generated constructor stub

}

PS3FS::PS3FS(const PG::UTIL::File& file){
	open(file);
}

PS3FS::PS3FS(const std::string& file):
		PS3FS(PG::UTIL::File(file)){}


//PS3FS_V1
typedef struct //total size 288
{
	char filename[48] = {0};
	ui64 size;
	ui64 offset;

	void set(const fileInfo& info){
		memcpy(&filename[0], &info.getName().getFile()[0], (info.getName().getFile().size() > 48)? 48: info.getName().getFile().size());
		size = info.size;
		offset = info.offset;
	}

	fileInfo get() const{
		return fileInfo(getName(),size,offset);
	}

	std::string getName() const{
		std::string str;
		for(char c: filename){
			if(c == '\0') break;
			str.push_back(c);
		}
		return str;
	}

} __attribute__((packed, aligned(1))) PS3FS_V1FileInfo;

bool PS3FS::open(const PG::UTIL::File& file, PercentIndicator* percent){
	 clear();

	 m_file = file;

	 if(!m_file.exists() ){
		 PG_ERROR_STREAM("Target file does not exist!");
		 m_file.clear();
		 return true;
	 }

	 try{
		 PG::STREAM::InByteFile reader(m_file);
		 if(!reader.isopen()) return true;

		 {
			 const std::string magic_number = reader.readString(8);
			 if( magic_number != "PS3FS_V1"){
				 PG_ERROR_STREAM("Wrong magic number, it should be 'PS3FS_V1' but it was '"<<magic_number<<"'!");
				 m_file.clear();
				 return FAILURE;
			 }
		 }

		 const unsigned long long number_of_files = reader.readUnsignedLongLong();
		 if(number_of_files > 900000){
			 PG_ERROR_STREAM("Number of files to big!");
			 m_file.clear();
			 return FAILURE;
		 }

		 m_fileInfos.reserve(number_of_files);

		 unsigned int count = 0;
		 for(unsigned long long i = 0; i < number_of_files; i++){
			if(percent){
				percent->percent = (count/float(number_of_files))*100.0;
				count++;
			}


			 PS3FS_V1FileInfo info;
			 reader.read((char*)&info, sizeof(PS3FS_V1FileInfo));
			 fileInfo fInfo(info.getName(),info.size,info.offset); //convert it into my standard format
			 PG_INFO_STREAM(fInfo.name);
			 /*
			 //tests
			 {
				 const unsigned int current_pos = reader.pos();
				 reader.seek(fInfo.offset);

				 if(isIMYPackage(reader)){
					 fInfo.fileType = fileInfo::COLA;
				 }else{
					 reader.seek(current_pos);
					 if(isIMY(reader)){
						 fInfo.fileType = fileInfo::IMY;
					 }
				 }

				 reader.seek(current_pos);
			 }
			*/

			 m_fileInfos.push_back(fInfo);
		 }

		 reader.close();
		 m_originalFileSize = m_fileInfos.size();

	 }catch (PG::UTIL::Exception& e) {
		 PG_ERROR_STREAM("Couldn't read given PSPFS file '"<<file<<"': "<<e.what());
		 m_file.clear();
		 return FAILURE;
	 }catch (...) {
		 PG_ERROR_STREAM("Couldn't read given PSPFS file '"<<file<<"'!");
		 m_file.clear();
		 return FAILURE;
	 }

	 return SUCCESS;

}



bool PS3FS::save(const PG::UTIL::File& targetfile, PercentIndicator* percent){
	// do we even have a DAT file open
	if(m_file.isEmpty()){
		PG_ERROR_STREAM("No PSPFS file opened.");
		return FAILURE;
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
	std::vector<fileInfo> file_infos;
	file_infos.reserve(m_fileInfos.size());
	char* c = nullptr;

	PG::STREAM::InByteFile reader(m_file);
	if(!reader.isopen()) return FAILURE;

	try{
		PG::STREAM::OutByteFile writer(target);
		if(!writer.isOpen()){
			PG_ERROR_STREAM("Failed to open/create '"<<target<<"'!");
			return FAILURE;
		}
		//setup header
		writer.writeString("PS3FS_V1"); //write magic number
		writer.writeLongLong(m_fileInfos.size());

		unsigned long long pos_header = writer.pos();
		c = new char[sizeof(PS3FS_V1FileInfo)]();
		for(unsigned int i = 0; i < (m_fileInfos.size()+5); ++i)
			writer.write(c, sizeof(PS3FS_V1FileInfo));
		delete[] c;
		unsigned long long pos_data = writer.pos();

		unsigned int count = 0;
		for(const fileInfo& finfo: m_fileInfos){
			if(percent){
				percent->percent = (count/float(m_fileInfos.size()))*100.0;
				count++;
			}

			fileInfo current_info = finfo; //copy info
			PS3FS_V1FileInfo infoDat;

			if(current_info.isExternalFile()){
				PG::STREAM::InByteFile reader_file(current_info.externalFile);
				reader_file.seek(0);
				const unsigned int file_size = reader_file.size();
				// check if valid
				if(file_size == 0)
					PG_WARN_STREAM("File '"<<current_info.externalFile<<"' has a size of zero!");

				if(current_info.getSize() != file_size){
					PG_WARN_STREAM("File '"<<current_info.externalFile<<"' size is different than expected! ("<<current_info.getSize()<<" != "<<file_size<<")");
					current_info.setSize(file_size);
				}

				c = new char[current_info.getSize()];
				reader_file.read(c,current_info.getSize());


			}else{
				reader.seek(finfo.offset);
				c = new char[finfo.size];
				reader.read(c,finfo.getSize());
			}

			//write header
			current_info.setOffset(pos_data);
			writer.seek(pos_header);
			infoDat.set(current_info);
			writer.write((char*)&infoDat, sizeof(PS3FS_V1FileInfo));
			pos_header = writer.pos();

			//write data
			writer.seek(pos_data);
			writer.write(c, current_info.getSize());
			delete[] c;
			c = nullptr;
			pos_data = writer.pos();

			file_infos.push_back(current_info);
		}


		//reader_dat.close();
		writer.close();
		reader.close();
	}catch (PG::UTIL::Exception& e) {
		 PG_ERROR_STREAM("Couldn't save PSPFS archive! : "<<e.what());
		 target.remove();
		 return FAILURE;
	}catch (...) {
		 PG_ERROR_STREAM("Couldn't save PSPFS archive!");
		 target.remove();
		 return FAILURE;
	}

	 if(overwrite){
		 m_file.remove();
		 target.rename(m_file.getPath());
		 m_fileInfos = file_infos;
		 m_changed = false;
	 }

	return SUCCESS;

}

char const* PS3FS::getType() const{
	return "PS3FS_V1";
}

PS3FS::~PS3FS() {
	// TODO Auto-generated constructor stub

}


} /* namespace FILE */
} /* namespace PG */
