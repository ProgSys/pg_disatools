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

namespace PG {
namespace FILE {

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
			 info.name = reader.readString(28);
			 PG::UTIL::trim(info.name);
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
	std::string name = file.getName();
	std::transform(name.begin(), name.end(), name.begin(), ::toupper);

	auto it = std::find_if(m_filePSPFSInfos.begin(), m_filePSPFSInfos.end(), [name](const filePSPFSInfo& info){
		return info.name == name;
	});

	if(it != m_filePSPFSInfos.end()){
		infoOut = (*it);
		return false;
	}

	return true;
}

bool PSPFS::extract(const PG::UTIL::File& file, const PG::UTIL::File& targetFile){
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
	if(addFile.name.size() > 28){
		PG_ERROR_STREAM("File name is too big, max 28 chars!");
		return true;
	}
	std::transform(addFile.name.begin(), addFile.name.end(), addFile.name.begin(), ::toupper);
	addFile.externalFile = file;
	addFile.size = file.size();

	//file is already inside?
	auto it = std::find_if(m_filePSPFSInfos.begin(), m_filePSPFSInfos.end(), [addFile](const filePSPFSInfo& info){
		return info.name == addFile.name;
	}
	);


	if(it != m_filePSPFSInfos.end()){
		(*it) = addFile;
	}else{
		m_filePSPFSInfos.push_back(addFile);
	}

	return false;


	/*
	fileInsertInfo addFile;
	addFile.name = file.getFile();
	if(addFile.name.size() > 28){
		PG_ERROR_STREAM("File name is too big, max 28 chars!");
		return true;
	}
	std::transform(addFile.name.begin(), addFile.name.end(), addFile.name.begin(), ::toupper);

	char* c = nullptr;
	try{
		PG::UTIL::ByteInFileStream reader(file);
		addFile.size = reader.size();
		c = new char[addFile.size];
		reader.read(c, addFile.size);
		reader.close();


		if(!m_file_buffer.exists()){
			assert_Test("m_insertFiles size should be zero!", m_insertFiles.size != 0);
			PG::UTIL::BinaryFileWriter writer(m_file_buffer);
			writer.writeString("PSPFS_V1PGBUFFER"); // 16 byte
			writer.writeInt(0); // number of files inside
			writer.writeInt(16+4+4); // end
			writer.close();
		}else{
			//file is already inside?
			auto it = std::find(m_insertFiles.begin(), m_insertFiles.end(), [addFile](const fileInsertInfo& info){
				return info.insert && (info.name == addFile.name);
			});

			if(it != m_insertFiles.end()){
				//insert at end
				fileInsertInfo& sameFileInside = (*it);

				//can be replaced?
				if(addFile.size <= sameFileInside.size ){
					//the file can be replaced
					PG::UTIL::ByteInOutFileStream stream(m_file_buffer);
					stream.seek(sameFileInside.offset);
					stream.writeChar(0);
					stream.writeString(addFile.name);
					//fill rest with 0
					for(short i = addFile.name.size(); i < 28; ++i)
						stream.writeChar(0);
					stream.skip(4);
					stream.writeInt(addFile.size);
					stream.write(c, addFile.size);
					delete[] c;
					c = nullptr;
					stream.close();
					sameFileInside.size = addFile.size;
					return false;

				}else{
					//mark as invalid and then add to end
					PG::UTIL::ByteInOutFileStream stream(m_file_buffer);
					stream.seek(sameFileInside.offset);
					stream.writeChar(1);
					stream.close();
					sameFileInside.insert = false;
				}

			}
		}



		//add file to the end
		PG::UTIL::ByteInOutFileStream stream(m_file_buffer);
		stream.skip(16);
		const unsigned int file_number =  stream.readUnsignedInt();
		const unsigned int beginning_pos = stream.readUnsignedInt();
		addFile.offset = beginning_pos;
		stream.seek(beginning_pos);
		stream.writeChar(0); // save? 0 = valid; 1 = not valid
		stream.writeString(addFile.name);
		//fill rest with 0
		for(short i = addFile.name.size(); i < 28; ++i)
			stream.writeChar(0);
		stream.writeInt(addFile.size+4); // next
		stream.writeInt(addFile.size); // filesize
		stream.write(c, addFile.size);
		delete[] c;
		c = nullptr;
		const unsigned int end_pos = stream.pos();
		stream.seek(16);
		//update number of files end offset
		stream.writeInt(file_number+1);
		stream.writeInt(end_pos);
		m_insertFiles.push_back(addFile);

	 }catch (PG::UTIL::Exception& e) {
		 PG_ERROR_STREAM("Couldn't buffer file '"<<file<<"': "<<e.what());
		 if(c) delete[] c;
		 return true;
	 }catch (...) {
		 PG_ERROR_STREAM("Couldn't buffer file '"<<file<<"'!");
		 if(c) delete[] c;
		 return true;
	 }

	//remove from m_filePSPFSInfos it it is inside
	auto it = std::find(m_filePSPFSInfos.begin(), m_filePSPFSInfos.end(), [addFile](const filePSPFSInfo& info){
		return info.name == addFile.name;
	});

	if(it != m_filePSPFSInfos.end())
		m_filePSPFSInfos.erase(it);

	 return false;
	 */

}

bool PSPFS::remove(const PG::UTIL::File& file){
	if(m_file.isEmpty()){
		PG_ERROR_STREAM("No PSPFS file opened.");
		return true;
	}

	std::string name = file.getFile();
	if(name.size() > 28){
		PG_ERROR_STREAM("File name is too big, max 28 chars!");
		return true;
	}

	auto it = std::find_if(m_filePSPFSInfos.begin(), m_filePSPFSInfos.end(), [name](const filePSPFSInfo& info){
		return info.name == name;
	});

	if(it != m_filePSPFSInfos.end()){
		m_filePSPFSInfos.erase(it);
		return false;
	}

	PG_ERROR_STREAM("File '"<<file<<"' not found!");
	return true;

	/*
	std::string name = file.getFile();
	if(name.size() > 28){
		PG_ERROR_STREAM("File name is too big, max 28 chars!");
		return true;
	}
	std::transform(name.begin(), name.end(), name.begin(), ::toupper);

	//remove from m_filePSPFSInfos it it is inside
	auto it = std::find(m_filePSPFSInfos.begin(), m_filePSPFSInfos.end(), [name](const filePSPFSInfo& info){
		return info.name == name;
	});

	if(it != m_filePSPFSInfos.end()){
		m_filePSPFSInfos.erase(it);
		return false;
	}

	// set flag to not insert in insertFiles
	auto it2 = std::find(m_insertFiles.begin(), m_insertFiles.end(), [name](const fileInsertInfo& info){
			return info.insert && (info.name == name);
	});

	if(it2 != m_insertFiles.end()){
		assert_Test("Buffer file is missing!", !m_file_buffer.exists());
		PG::UTIL::ByteInOutFileStream stream(m_file_buffer);
		stream.seek((*it2).offset);
		stream.writeChar(1);
		stream.close();
		(*it2).insert = 0;
		return false;
	}

	PG_ERROR_STREAM("File '"<<file<<"' not found!");
	return true;
	*/
}

bool PSPFS::save(){
	if(m_file.isEmpty()){
		PG_ERROR_STREAM("No PSPFS file opened.");
		return true;
	}

	char* c = nullptr;
	try{
		PG::UTIL::BinaryFileWriter writer(m_file_buffer);
		writer.writeString("PSPFS_V1");

		unsigned int true_count = 0;
		writer.writeInt(m_filePSPFSInfos.size());
		writer.writeInt(0); // mystery int

		PG::UTIL::ByteInFileStream reader(m_file);

		unsigned int offset = 16+52*m_filePSPFSInfos.size();

		for(filePSPFSInfo& info: m_filePSPFSInfos){
			if(info.size == 0){
				PG_WARN_STREAM("File '"<<info.name<<"' has a size of zero! Skipping it!");
				continue;
			}

			if(info.isExternalFile()){
				if(!info.externalFile.exists()){
					PG_WARN_STREAM("External file '"<<info.externalFile<<"' doesn't exist! Skipping it!");
					continue;
				}

				PG::UTIL::ByteInFileStream fileReader(info.externalFile);

				const unsigned file_size = fileReader.size();

				if(info.size != file_size){
					PG_WARN_STREAM("File '"<<info.externalFile<<"' size is different than expected! ("<<info.size<<" != "<<file_size<<")");
					info.size = file_size;
				}

				if(file_size == 0){
					PG_WARN_STREAM("File '"<<info.externalFile<<"' has a size of zero! Skipping it!");
					continue;
				}

				writer.writeString(info.name);
				for(unsigned short i = (unsigned short)info.name.size(); i < 28; ++i) //fill rest with 0
					writer.writeChar(0);

				writer.writeInt(info.unknown);
				writer.writeInt(file_size);
				writer.writeInt(offset);

				offset += file_size;
				info.size = file_size;
				info.offset = offset;

				c = new char[info.size];
				fileReader.read(c, info.size);
				fileReader.close();

				writer.write(c, info.size);
				delete[] c;
				c = nullptr;

				info.externalFile.clear();

			}else{
				writer.writeString(info.name);
				for(unsigned short i = (unsigned short)info.name.size(); i < 28; ++i) //fill rest with 0
					writer.writeChar(0);

				writer.writeInt(info.unknown);
				writer.writeInt(info.size);
				writer.writeInt(offset);

				offset += info.size;
				reader.seek(info.offset);
				info.offset = offset;

				c = new char[info.size];
				reader.read(c, info.size);
				writer.write(c, info.size);
				delete[] c;
				c = nullptr;

			}


		}

	 }catch (PG::UTIL::Exception& e) {
		 PG_ERROR_STREAM("Couldn't save PSPFS archive! : "<<e.what());
		 return true;
	 }catch (...) {
		 PG_ERROR_STREAM("Couldn't save PSPFS archive!");
		 return true;
	}

	 m_file.remove();
	 m_file_buffer.rename(m_file.getPath());

	 return false;
}

PSPFS::~PSPFS(){
	if(m_file_buffer.exists())
		m_file_buffer.remove();
}

} /* namespace FILE */
} /* namespace PG */
