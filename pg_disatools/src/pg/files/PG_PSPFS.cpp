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
#include <pg/stream/PG_StreamInByteFile.h>
#include <pg/stream/PG_StreamOutByteFile.h>
#include <pg/util/PG_StringUtil.h>
#include <pg/util/PG_Exception.h>
#include <pg/files/PG_TX2.h>

#include <pg/files/PG_FileTests.h>

namespace PG {
namespace FILE {


PSPFS::PSPFS(){

}

PSPFS::PSPFS(const PG::UTIL::File& file){
	open(file);
}

PSPFS::PSPFS(const std::string& file):
		PSPFS(PG::UTIL::File(file)){}


bool PSPFS::open(const PG::UTIL::File& file, PercentIndicator* percent){
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

		 m_fileInfos.reserve(number_of_files);
		 //read the file infos
		 for(unsigned int i = 0; i < number_of_files; ++i){
			 if(percent) percent->percent = (i/float(number_of_files))*100.0;
			 fileInfo info;
			 std::string filename = reader.readString(40);
			 filename = filename.substr(0,filename.find_first_of(char(0x00)));
			 std::transform(filename.begin(), filename.end(), filename.begin(), ::toupper);
			 info.name = filename;
			 info.decompressedFileSize  = reader.readUnsignedInt();
			 info.setSize(reader.readUnsignedInt());
			 info.setOffset(reader.readUnsignedInt());

			 //tests
			 const unsigned int current_pos = reader.pos();
			 reader.seek(info.offset);

			 if(isIMYPackage(reader)){
				 info.fileType = fileInfo::COLA;
			 }else{
				 reader.seek(current_pos);
				 if(isIMY(reader)){
					 info.fileType = fileInfo::IMY;
				 }
			 }

			 reader.seek(current_pos);

			 m_fileInfos.push_back(info);
		 }

		 reader.close();
		 m_originalFileSize = m_fileInfos.size();

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


bool PSPFS::insert(const PG::UTIL::File& file){
	if(m_file.isEmpty()){
		PG_ERROR_STREAM("No PSPFS file opened.");
		return true;
	}

	if(!file.exists()){
		PG_ERROR_STREAM("File doesn't exist!");
		return true;
	}

	fileInfo addFile;
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


	addFile.externalFile = file;
	addFile.setSize(file.size());

	fileProperties prop(addFile);
	getFileProperties(prop);

	//file is already inside?
	auto it = std::find_if(m_fileInfos.begin(), m_fileInfos.end(), [addFile](const fileInfo& info){
		return info.getName() == addFile.getName();
	});

	if(it != m_fileInfos.end()){
		(*it) = addFile;
	}else{
		//replace a dummy file or add at end
		it = std::find_if(m_fileInfos.begin(), m_fileInfos.end(), [addFile](const fileInfo& info){
			return info.getName() == "DUMMY.DAT";
		});

		if(it != m_fileInfos.end()){
			(*it) = addFile;
			//PG_INFO_STREAM("Replaced dummy "<<addFile.name);
		}else{
			m_fileInfos.push_back(addFile);
			//PG_INFO_STREAM("Insert new "<<addFile.name);
		}
	}
	m_changed = true;

	return false;

}

bool PSPFS::remove(fileInfo& target){
	PG_INFO_STREAM(" target: "<<target.name);
	std::vector<fileInfo>::iterator  it(&target);
	const unsigned int index = std::distance(m_fileInfos.begin(), it);
	if(it < m_fileInfos.end()){
		if(index < m_originalFileSize){
			if(target.name == "DUMMY.DAT"){
				m_fileInfos.erase(it);
			}else{
				target.setAsDummy(target.offset);
			}
		}else{
			m_fileInfos.erase(it);
		}
		m_changed = true;
	}else{
		PG_ERROR_STREAM("File '"<<target.getName()<<" found!");
		return true;
	}
	return false;
}

//helper for some experiments, is not used
inline void swap(std::vector<fileInfo>& file_infos, unsigned int index1, unsigned int index2){
	fileInfo swap = file_infos[index1];
	file_infos[index1] = file_infos[index2];
	file_infos[index2] = swap;
}

inline void writeDUMMY(PG::STREAM::OutByteFile& writer,unsigned int& next_header_offset, unsigned int& next_file_offset, unsigned int& dummy_traget_offset){
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

bool PSPFS::save(const PG::UTIL::File& targetfile, PercentIndicator* percent){
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
	try{
		PG::STREAM::OutByteFile writer(target);
		if(!writer.isOpen()){
			PG_ERROR_STREAM("Failed to open/create '"<<target<<"'!");
			return FAILURE;
		}
		//setup header
		writer.writeString("PSPFS_V1"); //write magic number

		writer.writeInt(m_fileInfos.size()); //number of files
		writer.writeInt(0); // mystery int

		unsigned int next_header_offset = writer.getPosition(); //get the first header position

		c = new char[52](); // each header is exactly 52 byte long
		//allocate header space
		for(unsigned int i = 0; i < m_fileInfos.size()+10; ++i){ //also add a small empty buffer
			writer.write(c,52);
		}
		delete[] c;
		c = nullptr;

		unsigned int next_file_offset = writer.getPosition(); // the position where we start to write the data of the files

		unsigned int dummy_traget_offset = 0;

		PG::STREAM::InByteFile reader_dat(m_file);
		if(!reader_dat.isopen()) return true;

		unsigned int i = 0;
		for(const fileInfo& info: m_fileInfos){
			fileInfo current_info = info; //copy info
			if(percent){
				percent->percent = (i/float(m_fileInfos.size()))*100.0;
				i++;
			}

			if(current_info.isExternalFile()){
				if(current_info.getSize() == 0 || current_info.getName().isEmpty() ||!current_info.externalFile.exists()){
					PG_WARN_STREAM("External file '"<<current_info.externalFile<<"' doesn't exist! Writing empty dummy file instead!");
					current_info.setAsDummy(next_file_offset);
					writeDUMMY(writer,next_header_offset,next_file_offset, dummy_traget_offset );
				}else{
					PG_INFO_STREAM("Adding external file '"<<current_info.externalFile<<"'.");


					PG::STREAM::InByteFile reader_file(current_info.externalFile);
					current_info.decompressedFileSize = isIMYPackage(reader_file);
					reader_file.seek(0);
					const unsigned int file_size = reader_file.size();


					// check if valid
					if(file_size == 0)
						PG_WARN_STREAM("File '"<<current_info.externalFile<<"' has a size of zero!");

					if(info.getSize() != file_size){
						PG_WARN_STREAM("File '"<<current_info.externalFile<<"' size is different than expected! ("<<current_info.getSize()<<" != "<<file_size<<")");
						current_info.setSize(file_size);
					}

					//write header
					writer.setPosition(next_header_offset);
					writer.writeString(current_info.getName().getPath());
					for(unsigned short i = (unsigned short)current_info.getName().getPath().size(); i < 40; ++i) //fill rest with 0
						writer.writeChar(0);

					writer.writeInt(current_info.decompressedFileSize);
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

					writer.writeInt(current_info.decompressedFileSize);
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

void PSPFS::clear(){
	if(m_file_buffer.exists())
		m_file_buffer.remove();
	ExtractorBase::clear();
}

std::string PSPFS::getType() const{
	return "PSPFS_V1";
}


PSPFS::~PSPFS(){
	if(m_file_buffer.exists())
		m_file_buffer.remove();
}

} /* namespace FILE */
} /* namespace PG */
