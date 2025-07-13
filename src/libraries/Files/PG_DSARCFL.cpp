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
#include "Files/PG_DSARCFL.h"

#include <Util/PG_Base.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamOutByteFile.h>
#include <Util/PG_StringUtil.h>
#include <Util/PG_Exception.h>
#include <Files/PG_TX2.h>

#include <Files/PG_FileTests.h>

namespace PG {
namespace FILE {

DSARC_FL::DSARC_FL() {
	// TODO Auto-generated constructor stub

}

DSARC_FL::DSARC_FL(const PG::UTIL::File& file){
	open(file);
}

DSARC_FL::DSARC_FL(const std::string& file):
		DSARC_FL(PG::UTIL::File(file)){}


DSARC_FL::~DSARC_FL() {
	// TODO Auto-generated destructor stub
}

bool DSARC_FL::open(const PG::UTIL::File& file, PercentIndicator* percent){
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
			 if(magic_number != "DSARC FL"){
				 PG_ERROR_STREAM("Wrong magic number, it should be 'DSARC FL' but it was '"<<magic_number<<"'!");
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

		 reader.skip(4); //zero

		 m_fileInfos.reserve(number_of_files);
		 //read the file infos
		 for(unsigned int i = 0; i < number_of_files; ++i){
			 if(percent) percent->percent = (i/float(number_of_files))*100.0;
			 fileInfo info;
			 std::string filename = reader.readString(40);
			 filename = filename.substr(0,filename.find_first_of(char(0x00)));
			 //std::transform(filename.begin(), filename.end(), filename.begin(), ::toupper);
			 info.name = filename;
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

			 m_fileInfos.push_back(std::move(info));
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

bool DSARC_FL::save(const PG::UTIL::File& targetfile, PercentIndicator* percent){
	// do we even have a DAT file open
	if(m_file.isEmpty()){
		PG_ERROR_STREAM("No DSARC FL file opened.");
		return FAILURE;
	}

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

	char* c = nullptr;
	try{
		PG::STREAM::OutByteFile writer(target);
		if(!writer.isOpen()){
			PG_ERROR_STREAM("Failed to open/create '"<<target<<"'!");
			return FAILURE;
		}
		//setup header
		writer.writeString("DSARC FL"); //write magic number

		writer.writeInt(m_fileInfos.size()); //number of files
		writer.writeInt(0); // mystery int

		unsigned int posAddresses = writer.pos();
		c = new char[48](); // each header is exactly 52 byte long
		//allocate header space
		for(unsigned int i = 0; i < m_fileInfos.size()+10; ++i){ //also add a small empty buffer
			writer.write(c,48);
		}
		delete[] c;

		PG::STREAM::InByteFile reader_dat(m_file);
		if(!reader_dat.isopen()) return FAILURE;

		unsigned int i = 0;
		for(const fileInfo& info: m_fileInfos){
			fileInfo current_info = info; //copy info
			if(percent){
				percent->percent = (i/float(m_fileInfos.size()))*100.0;
				i++;
			}

			if(current_info.isExternalFile()){
				PG::STREAM::InByteFile reader_file(current_info.externalFile);
				reader_file.seek(0);
				const unsigned int file_size = reader_file.size();
				// check if valid
				if(file_size == 0)
					PG_WARN_STREAM("File '"<<current_info.externalFile<<"' has a size of zero!");

				if(info.getSize() != file_size){
					PG_WARN_STREAM("File '"<<current_info.externalFile<<"' size is different than expected! ("<<current_info.getSize()<<" != "<<file_size<<")");
					current_info.setSize(file_size);
				}

				const unsigned int dataStart = writer.pos();
				//write header
				writer.seek(posAddresses);
				writer.writeString(info.getName().getFile());
				for(unsigned short i = (unsigned short)current_info.getName().getPath().size(); i < 40; ++i) //fill rest with 0
					writer.writeChar(0);
				writer.writeInt(file_size);
				writer.writeInt(dataStart);
				posAddresses+= 48;
				writer.seek(dataStart);

				//copy data
				current_info.setOffset(dataStart);
				c = new char[current_info.getSize()];
				reader_file.read(c,current_info.getSize());
				writer.write(c, current_info.getSize());
				delete[] c;
				c = nullptr;
				current_info.clearExternalFile();
			}else{
				const unsigned int dataStart = writer.pos();
				//write header
				writer.seek(posAddresses);
				writer.writeString(info.getName().getFile());
				for(unsigned short i = (unsigned short)current_info.getName().getPath().size(); i < 40; ++i) //fill rest with 0
					writer.writeChar(0);
				writer.writeInt(info.getSize());
				writer.writeInt(dataStart);
				posAddresses+= 48;
				writer.seek(dataStart);

				//copy data
				reader_dat.seek(current_info.getOffset());
				current_info.setOffset(dataStart);
				c = new char[current_info.getSize()];
				reader_dat.read(c,current_info.getSize());
				writer.write(c, current_info.getSize());
				delete[] c;
				c = nullptr;

			}
			file_infos.push_back(current_info);


			//buffer
			unsigned int bufferSize = (~(writer.pos())) & 0xFF;
			if(bufferSize != 255){
				bufferSize+=1;
				c = new char[bufferSize]();
				writer.write(c, bufferSize);
				delete[] c;
				c = nullptr;
			}
		}

	}catch (PG::UTIL::Exception& e) {
		 PG_ERROR_STREAM("Couldn't save DSARC FL archive! : "<<e.what());
		 target.remove();
		 return FAILURE;
	}catch (...) {
		 PG_ERROR_STREAM("Couldn't save DSARC FL archive!");
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

char const* DSARC_FL::getType() const{
	return "DSARC_FL";
}

} /* namespace FILE */
} /* namespace PG */
