/*
 * PG_Script.cpp
 *
 *  Created on: 01.06.2025
 *      Author: ProgSys
 */

#include <Files/PG_Script.h>
#include <Util/PG_Base.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamOutByteFile.h>
#include <Util/PG_StringUtil.h>
#include <Util/PG_Exception.h>
#include <Files/PG_FileTests.h>
#include <iomanip>

namespace PG {
namespace FILE {


Script::Script() {
	// TODO Auto-generated constructor stub

}

Script::Script(const PG::UTIL::File& file){
	open(file);
}
Script::Script(const std::string& file): Script(PG::UTIL::File(file)){}


bool Script::open(const PG::UTIL::File& file, PercentIndicator* percent){
	clear();

	 m_file = file;
	 m_file_buffer = PG::UTIL::File(file.getPath()+"_buffer");

	 if(!m_file.exists() ){
		 PG_ERROR_STREAM("Target file does not exist!");
		 m_file.clear();
		 return true;
	 }

	 try{
			 PG::STREAM::InByteFile reader(m_file);
			 if(!reader.isopen()) return true;

			 const unsigned int number_of_scripts = reader.readUnsignedInt();
			 const unsigned int zero = reader.readUnsignedInt();

			 if(zero != 0 ) return true;

			 auto fileSize = reader.size();
			 if(fileSize < number_of_scripts * sizeof(int) * 2) return true;


			 std::vector<unsigned int> addresses(number_of_scripts);
			 reader.read((char*)addresses.data(), number_of_scripts * sizeof(int));
			 std::vector<unsigned int> ids(number_of_scripts);
			 reader.read((char*)ids.data(), number_of_scripts * sizeof(int));

			 auto addressIt = addresses.begin();
			 auto idIt = ids.begin();
			 size_t i = 0;
			 m_fileInfos.reserve(number_of_scripts);
			 for (; addressIt != addresses.end(); ++addressIt, ++idIt, ++i) {
				 if (*addressIt >= fileSize) {
					 clear();
					 return true;
				 }

				 fileInfo info;
				 std::stringstream ss;
				 ss << "SCRIPT_" << std::setfill('0') << std::setw(3) << *idIt << ".bin";
				 info.name = ss.str();
				 info.size = 0;
				 info.offset = *addressIt;
				 info.fileType = fileInfo::Script;
				 m_fileInfos.push_back(std::move(info));
			}

			 reader.close();
			 m_originalFileSize = m_fileInfos.size();

	 }catch (PG::UTIL::Exception& e) {
		 PG_ERROR_STREAM("Couldn't read given Script Disgaea PC 2 file '"<<file<<"': "<<e.what());
		 m_file.clear();
		 return true;
	 }catch (...) {
		 PG_ERROR_STREAM("Couldn't read given Script Disgaea PC 2 file '"<<file<<"'!");
		 m_file.clear();
		 return true;
	 }

	 return false;
}

bool Script::save(const PG::UTIL::File& targetfile, PercentIndicator* percent){
	// do we even have a DAT file open
	if(m_file.isEmpty()){
		PG_ERROR_STREAM("No ANM Disgaea PC 2 file opened.");
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

	try{
		PG::STREAM::OutByteFile writer(target);
		if(!writer.isOpen()){
			PG_ERROR_STREAM("Failed to open/create '"<<target<<"'!");
			return FAILURE;
		}

		writer.writeInt(m_fileInfos.size());
		writer.writeInt(0);
		std::vector<unsigned int> addresses(m_fileInfos.size());
		std::vector<unsigned int> ids(m_fileInfos.size());

		writer.write((char*)addresses.data(), addresses.size() * sizeof(int));
		writer.write((char*)ids.data(), ids.size() * sizeof(int));

		PG::STREAM::InByteFile reader_dat(m_file);
		if(!reader_dat.isopen()) return true;

		unsigned int i = 0;
		for(const fileInfo& info: m_fileInfos){
			fileInfo current_info = info; //copy info
			if(percent){
				percent->percent = (i/float(m_fileInfos.size()))*100.0;
				i++;
			}
			//TODO
			if(current_info.isExternalFile()){
				PG::STREAM::InByteFile reader_file(current_info.externalFile);
				const unsigned int file_size = reader_file.size();

				if(info.getSize() != file_size){
					PG_WARN_STREAM("File '"<<current_info.externalFile<<"' size is different than expected! ("<<current_info.getSize()<<" != "<<file_size<<")");
					current_info.setSize(file_size);
				}

				writer.writeInt(current_info.size);
				current_info.offset = writer.pos();
				c = new char[current_info.getSize()];
				reader_file.read(c,current_info.getSize());
				reader_file.close();
				writer.write(c, current_info.getSize());
				delete[] c;
				c = nullptr;
			}else{
				reader_dat.seek(current_info.offset);
				writer.writeInt(current_info.size+4);
				c = new char[current_info.getSize()];
				reader_dat.read(c,current_info.getSize());
				writer.write(c, current_info.getSize());
				delete[] c;
				c = nullptr;
			}
			file_infos.push_back(current_info);


		}// for loop end

		reader_dat.close();
		writer.close();

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

char const* Script::getType() const{
	return "Script";
}

Script::~Script() {
	// TODO Auto-generated destructor stub
}

} /* namespace FILE */
} /* namespace PG */
