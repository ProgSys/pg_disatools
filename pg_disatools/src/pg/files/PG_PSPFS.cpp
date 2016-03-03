/*
 * PG_PSPFS.cpp
 *
 *  Created on: 01.03.2016
 *      Author: ProgSys
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


bool filePSPFSInfo::isExternalFile() const{
	return !externalFile.isEmpty();
}



std::string filePSPFSInfo::getFileExtention() const{
	return PG::UTIL::File(name).getFileExtension();
}



PSPFS::PSPFS(){

}

PSPFS::PSPFS(const PG::UTIL::File& file){
	open(file);
}

PSPFS::PSPFS(const std::string& file):
		PSPFS(PG::UTIL::File(file)){}


bool PSPFS::open(const PG::UTIL::File& file){
	 if(m_file_buffer.exists())
		m_file_buffer.remove();

	 m_filePSPFSInfos.clear();
	 m_changed = false;
	 m_file = file;
	 m_file_buffer = PG::UTIL::File(file.getPath()+"_buffer");

	 if(!m_file.exists() ){
		 PG_ERROR_STREAM("Target file does not exist!");
		 m_file.clear();
		 return true;
	 }

	 try{
		 PG::UTIL::ByteInFileStream reader(m_file);

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
			 info.name = reader.readString(40);
			 info.name = info.name.substr(0,info.name.find_first_of(char(0x00)));
			 //PG::UTIL::trim(info.name);
			 std::transform(info.name.begin(), info.name.end(), info.name.begin(), ::toupper);

			 info.unknown  = reader.readUnsignedInt();
			 info.size  = reader.readUnsignedInt();
			 info.offset  = reader.readUnsignedInt();

			 if(info.name != "DUMMY.DAT")
				 m_filePSPFSInfos.push_back(info);
		 }

		 reader.close();

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

bool PSPFS::exists(const PG::UTIL::File& file) const{
	std::string name = file.getName();
	std::transform(name.begin(), name.end(), name.begin(), ::toupper);

	auto it = std::find_if(m_filePSPFSInfos.begin(), m_filePSPFSInfos.end(), [name](const filePSPFSInfo& info){
		return info.name == name;
	});

	if(it != m_filePSPFSInfos.end()){
		return true;
	}

	return false;
}

bool PSPFS::findfilePSPFSInfo(const PG::UTIL::File& file, filePSPFSInfo& infoOut) const{
	std::string name = file.getFile();
	std::transform(name.begin(), name.end(), name.begin(), ::toupper);
	//PG_INFO_STREAM("Searching for '"<<name<<"'")
	auto it = std::find_if(m_filePSPFSInfos.begin(), m_filePSPFSInfos.end(), [name](const filePSPFSInfo& info){
		return info.name == name;
	});

	if(it != m_filePSPFSInfos.end()){
		infoOut = (*it);
		return false;
	}

	return true;
}

bool PSPFS::extract(const PG::UTIL::File& file, const PG::UTIL::File& targetFile) const{
	if(m_file.isEmpty()){
		PG_ERROR_STREAM("No PSPFS file opened.");
		return true;
	}

	if(m_file.getPath() == targetFile.getPath()){
		PG_ERROR_STREAM("Target file can't the the archive itself!");
		return true;
	}

	filePSPFSInfo info;
	if(findfilePSPFSInfo(file, info)){
		PG_ERROR_STREAM("File '"<<file.getPath()<<"' not found in archive!");
		return true;
	}

	if(info.size == 0 || (info.offset < 16 && !info.isExternalFile())){
		PG_ERROR_STREAM("File info is wrong!");
		return true;
	}

	char* c = nullptr;
	try{

		if(info.isExternalFile()){
			if(!info.externalFile.exists()){
				PG_ERROR_STREAM("External file '"<<info.externalFile<<"' doesn't exist!");
				return true;
			}

			PG::UTIL::ByteInFileStream reader(info.externalFile);

			const unsigned file_size = reader.size();

			if(info.size != file_size){
				PG_WARN_STREAM("File '"<<info.externalFile<<"' size is different than expected! ("<<info.size<<" != "<<file_size<<")");
				info.size = file_size;
			}

			if(file_size == 0){
				PG_ERROR_STREAM("File '"<<info.externalFile<<"' has a size of zero!");
				return true;
			}

			c = new char[info.size];
			reader.read(c, info.size);
			reader.close();

			PG::UTIL::BinaryFileWriter writer(targetFile);
			writer.write(c,info.size );
			writer.close();

			delete[] c;
			c = nullptr;


		}else{
			PG::UTIL::ByteInFileStream reader(m_file);

			c = new char[info.size];
			reader.seek(info.offset);
			reader.read(c,info.size );
			reader.close();

			PG::UTIL::BinaryFileWriter writer(targetFile);
			writer.write(c,info.size );
			writer.close();

			delete[] c;
			c = nullptr;
		}
		PG_INFO_STREAM("File extracted to '"<<targetFile<<"'.");

	}catch (PG::UTIL::Exception& e) {
		PG_ERROR_STREAM("Couldn't write file '"<<file<<"': "<<e.what());
		if(c) delete[] c;
		return true;
	}catch (...) {
		PG_ERROR_STREAM("Couldn't write file '"<<file<<"'!");
		if(c) delete[] c;
		return true;
	}

	return false;

}


bool PSPFS::extractImage(const PG::UTIL::File& file, PG::UTIL::RGBAImage& imageOut, bool alpha) const{
	if(m_file.isEmpty()){
		PG_ERROR_STREAM("No PSPFS file opened.");
		return true;
	}


	if(file.getFileExtension() != "TX2"){
		PG_ERROR_STREAM("Target file '"<<file<<"' is not a image!");
		return true;
	}

	filePSPFSInfo info;
	if(findfilePSPFSInfo(file, info)){
		PG_ERROR_STREAM("File '"<<file<<"' not found in archive!");
		return true;
	}



	char* c = nullptr;
	try{

		PG::UTIL::ByteInFileStream reader(m_file);

		c = new char[info.size];
		reader.seek(info.offset);
		reader.read(c,info.size );
		reader.close();

		if(PG::FILE::decompressTX2(c,info.size ,imageOut)){
			PG_ERROR_STREAM("Coudn't load '"<<file<<"'!");
			delete[] c;
			c = nullptr;
			return true;
		}

		delete[] c;
		c = nullptr;

	}catch (PG::UTIL::Exception& e) {
		PG_ERROR_STREAM("Couldn't write file '"<<file<<"': "<<e.what());
		if(c) delete[] c;
		return true;
	}catch (...) {
		PG_ERROR_STREAM("Couldn't write file '"<<file<<"'!");
		if(c) delete[] c;
		return true;
	}

	if(!alpha){
		for(PG::UTIL::rgba& rgba: imageOut){
			rgba.a = 255;
		}
	}

	return false;

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
	addFile.name = file.getFile();
	if(addFile.name.size() > 40){
		PG_ERROR_STREAM("File name is too big, max 28 chars!");
		return true;
	}
	std::transform(addFile.name.begin(), addFile.name.end(), addFile.name.begin(), ::toupper);
	addFile.externalFile = file;
	addFile.size = file.size();

	//file is already inside?
	auto it = std::find_if(m_filePSPFSInfos.begin(), m_filePSPFSInfos.end(), [addFile](const filePSPFSInfo& info){
		return info.name == addFile.name;
	});


	if(it != m_filePSPFSInfos.end()){
		(*it) = addFile;
	}else{
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
		return info.name == name;
	});

	if(it != m_filePSPFSInfos.end()){
		m_filePSPFSInfos.erase(it);
		m_changed = true;
		return false;
	}

	PG_ERROR_STREAM("File '"<<file<<"' not found!");
	return true;
}

bool PSPFS::save(){
	return save(m_file);
}

bool PSPFS::save(const PG::UTIL::File& targetfile){
	// do we even have a DAT file open
	if(m_file.isEmpty()){
		PG_ERROR_STREAM("No PSPFS file opened.");
		return true;
	}

	PG::UTIL::File target = targetfile;

	//create a butter for file infos
	std::vector<filePSPFSInfo> file_infos;

	//test
	PG::UTIL::ByteInFileStream reader(m_file);
	reader.skip(8);
	const unsigned int number_of_files = reader.readUnsignedInt();
	PG_INFO_STREAM("number_of_files: " <<number_of_files);
	reader.skip(4);
	file_infos.reserve(number_of_files);
	for(unsigned int i = 0; i < number_of_files; ++i){
		filePSPFSInfo info;

		info.name = reader.readString(40);
		info.name = info.name.substr(0,info.name.find_first_of(char(0x00)));
		//PG::UTIL::trim(info.name);
		std::transform(info.name.begin(), info.name.end(), info.name.begin(), ::toupper);

		info.unknown  = reader.readUnsignedInt();
		info.size  = reader.readUnsignedInt();
		info.offset  = reader.readUnsignedInt();

		file_infos.push_back(info);
	}
	reader.close();

	char* c = nullptr;

	try{
		PG::UTIL::BinaryFileWriter writer(target);
		//setup header
		writer.writeString("PSPFS_V1"); //write magic number

		writer.writeInt(file_infos.size()); //number of files - this number gets letter rewritten my filePSPFSInfos.size()
		writer.writeInt(0); // mystery int

		unsigned int next_header_offset = writer.getPosition(); //get the first header position

		c = new char[52](); // each header is exactly 52 byte long
		//allocate header space
		for(unsigned int i = 0; i < file_infos.size(); ++i){
			writer.write(c,52);
		}
		delete[] c;
		c = nullptr;

		unsigned int next_file_offset = writer.getPosition(); // the position where we start to write the data of the files


		PG::UTIL::ByteInFileStream reader_dat(m_file);

		for(const filePSPFSInfo& info: file_infos){

			//write header
			writer.setPosition(next_header_offset);
			writer.writeString(info.name);
			for(unsigned short i = (unsigned short)info.name.size(); i < 40; ++i) //fill rest with 0
				writer.writeChar(0);
			writer.writeInt(info.unknown);
			writer.writeInt(info.size);
			if(info.name == "DUMMY.DAT")
				writer.writeInt(next_file_offset+876);
			else
				writer.writeInt(next_file_offset);
			next_header_offset += 52;


			if(info.name == "DUMMY.DAT"){
				PG_INFO_STREAM(info.name<<" O: "<<info.offset<<" size: "<<info.size);
				c = new char[1396]();
				c[876] = 'D';
				c[877] = 'U';
				c[878] = 'M';
				c[879] = 'M';
				c[880] = 'Y';
				writer.write(c,1396);
				delete[] c;
				c = nullptr;
				next_file_offset += 1396;
			}else{
				//write file
				writer.setPosition(next_file_offset);
				reader_dat.seek(info.offset);
				c = new char[info.size];
				reader_dat.read(c,info.size);
				writer.write(c, info.size);
				delete[] c;
				c = nullptr;
				next_file_offset += info.size;
			}
		}


	}catch (PG::UTIL::Exception& e) {
		 PG_ERROR_STREAM("Couldn't save PSPFS archive! : "<<e.what());
		 target.remove();
		 return true;
	}catch (...) {
		 PG_ERROR_STREAM("Couldn't save PSPFS archive!");
		 target.remove();
		 return true;
	}

	return false;

	/*
	if(m_file.isEmpty()){
		PG_ERROR_STREAM("No PSPFS file opened.");
		return true;
	}

	//do we overwrite the old file or do we create a new one
	PG::UTIL::File target;
	bool overwrite = false;
	if(targetfile == m_file){
		overwrite = true;
		target = m_file_buffer;
	}else{
		target = targetfile;
	}

	//create a butter for file infos
	std::vector<filePSPFSInfo> fileinfos;
	fileinfos.reserve((unsigned int)m_filePSPFSInfos.size());

	char* c = nullptr;

	try{
		PG::UTIL::BinaryFileWriter writer(target);
		writer.writeString("PSPFS_V1"); //write magic number

		writer.writeInt(m_filePSPFSInfos.size()); //number of files - this number gets letter rewritten my filePSPFSInfos.size()
		writer.writeInt(0); // mystery int

		unsigned int headerOffset = writer.getPosition(); //get the first header position

		c = new char[52](); // each header is exactly 52 byte long
		//allocate header space
		for(unsigned int i = 0; i < m_filePSPFSInfos.size()+40; ++i){
			writer.write(c,52);
		}
		delete[] c;
		c = nullptr;

		unsigned int fileOffset = writer.getPosition(); // the position where we start to write the data of the files

		//open the .DAT file
		PG::UTIL::ByteInFileStream reader(m_file);

		//DUMMY FILE Counter, we add a Empty DUMMY file every ~260 files, because the game needs them
		//for what ever reason
		int dummy_add_count = 0;
		unsigned int dummy_files_added = 0;

		//write header
		for(const filePSPFSInfo& fileinfo: m_filePSPFSInfos){

			dummy_add_count--;
			if(dummy_add_count <= 0){
				dummy_add_count = 275;
				//we are adding a dummy file!
				writer.setPosition(headerOffset);
				writer.writeString("DUMMY.DAT");
				for(unsigned short i = 9; i < 40; ++i) //fill rest with 0
					writer.writeChar(0);
				writer.writeInt(0);
				writer.writeInt(5); //we set the size to 5 even thogh the size itself is bigger
				writer.writeInt(fileOffset+876);
				headerOffset += 52;

				writer.setPosition(fileOffset);

				c = new char[1396]();
				c[876] = 'D';
				c[877] = 'U';
				c[878] = 'M';
				c[879] = 'M';
				c[880] = 'Y';
				writer.write(c,1396);
				fileOffset += 1396;

				delete[] c;
				c = nullptr;

				dummy_files_added++;
			}

			writer.setPosition(headerOffset);
			//copy the fileinfo
			filePSPFSInfo infoCopy = fileinfo;

			// There are two cases A the file comes from a external file witch was insert or B it comes from the original DATA.DAT file
			if(infoCopy.isExternalFile()){
				//copy from a file
				if(!infoCopy.externalFile.exists()){ //test if file exists else just skip it
						PG_WARN_STREAM("External file '"<<infoCopy.externalFile<<"' doesn't exist! Skipping it!");
						continue;
					}

				PG::UTIL::ByteInFileStream fileReader(infoCopy.externalFile);
				const unsigned file_size = fileReader.size();

				// check if valid
				if(file_size == 0){
					PG_WARN_STREAM("File '"<<infoCopy.externalFile<<"' has a size of zero! Skipping it!");
					continue;
				}

				if(infoCopy.size != file_size){
					PG_WARN_STREAM("File '"<<infoCopy.externalFile<<"' size is different than expected! ("<<infoCopy.size<<" != "<<file_size<<")");
					infoCopy.size = file_size;
				}


				//write header
				writer.writeString(infoCopy.name);
				for(unsigned short i = (unsigned short)infoCopy.name.size(); i < 40; ++i) //fill rest with 0
					writer.writeChar(0);
				writer.writeInt(infoCopy.unknown);
				writer.writeInt(infoCopy.size);
				writer.writeInt(fileOffset);
				infoCopy.offset = fileOffset;

				writer.setPosition(fileOffset);
				fileOffset += infoCopy.size;
				headerOffset += 52;

				c = new char[infoCopy.size];
				fileReader.read(c, infoCopy.size);
				fileReader.close();
				writer.write(c, infoCopy.size);

				delete[] c;
				c = nullptr;

				infoCopy.externalFile.clear();

			}else{
				//copy from original .DAT

				writer.writeString(infoCopy.name);
				for(unsigned short i = (unsigned short)infoCopy.name.size(); i < 40; ++i) //fill rest with 0
					writer.writeChar(0);

				writer.writeInt(infoCopy.unknown);
				writer.writeInt(infoCopy.size);
				writer.writeInt(fileOffset);

				reader.seek(infoCopy.offset);
				writer.setPosition(fileOffset);

				infoCopy.offset = fileOffset;

				fileOffset += infoCopy.size;
				headerOffset += 52;

				c = new char[infoCopy.size];
				reader.read(c, infoCopy.size);
				writer.write(c, infoCopy.size);

				delete[] c;
				c = nullptr;
			}

			fileinfos.push_back(infoCopy);
		}

		writer.setPosition(8);

		writer.writeInt(fileinfos.size()+dummy_files_added);
		PG_INFO_STREAM("dummy_files_added: "<<dummy_files_added);
		reader.close();
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
		 m_filePSPFSInfos = fileinfos;
	 }

	 m_changed = false;
	 return false;
	 */
}

void PSPFS::clear(){
	if(m_file_buffer.exists())
		m_file_buffer.remove();
	m_file_buffer.clear();
	m_file.clear();
	m_filePSPFSInfos.clear();
	m_changed = false;
}

bool PSPFS::isChanged() const{
	return m_changed;
}

unsigned int PSPFS::size() const{
	return m_filePSPFSInfos.size();
}

PSPFS::~PSPFS(){
	if(m_file_buffer.exists())
		m_file_buffer.remove();
}

} /* namespace FILE */
} /* namespace PG */
