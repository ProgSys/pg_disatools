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
#include <Files/PG_NISPACK.h>

#include <Util/PG_Base.h>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamOutByteFile.h>
#include <Util/PG_StringUtil.h>
#include <Util/PG_Exception.h>
#include <Files/PG_TX2.h>

#include <Files/PG_FileTests.h>

namespace PG {
namespace FILE {

NISPACK::NISPACK() {
	// TODO Auto-generated constructor stub

}

NISPACK::NISPACK(const PG::UTIL::File& file){
	open(file);
}

NISPACK::NISPACK(const std::string& file):
		NISPACK(PG::UTIL::File(file)){}

NISPACK::~NISPACK() {
	// TODO Auto-generated destructor stub
}

bool NISPACK::open(const PG::UTIL::File& file, PercentIndicator* percent){
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
			 const std::string magic_number = reader.readString(7);
			 if(magic_number != "NISPACK"){
				 PG_ERROR_STREAM("Wrong magic number, it should be 'NISPACK' but it was '"<<magic_number<<"'!");
				 m_file.clear();
				 return true;
			 }
		 }
		 reader.skip(1);
		 //check version
		 {
			 const unsigned int version = reader.readUnsignedInt();
			 if(version != 0){
				 PG_ERROR_STREAM("Only version 0 is supported of 'NISPACK'!");
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

		 m_fileInfos.reserve(number_of_files);
		 //read the file infos
		 for(unsigned int i = 0; i < number_of_files; ++i){
			 if(percent) percent->percent = (i/float(number_of_files))*100.0;
			 fileInfo info;
			 std::string filename = reader.readString(32);
			 filename = filename.substr(0,filename.find_first_of(char(0x00)));
			 //std::transform(filename.begin(), filename.end(), filename.begin(), ::toupper);
			 info.name = filename;
			 info.setOffset(reader.readUnsignedInt());
			 info.setSize(reader.readUnsignedInt());
			 const unsigned int unknown = reader.readUnsignedInt();
			 if(1223127090 != unknown) PG_INFO_STREAM(filename<<": '"<<unknown<<"'");
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

bool NISPACK::save(const PG::UTIL::File& targetfile, PercentIndicator* percent){
	return true;
}

char const* NISPACK::getType() const{
	return "NISPACK";
}

} /* namespace FILE */
} /* namespace PG */
