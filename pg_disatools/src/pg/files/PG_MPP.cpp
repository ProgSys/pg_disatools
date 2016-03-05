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

#include <pg/files/PG_MPP.h>

#include <vector>
#include <pg/util/PG_ByteInFileStream.h>
#include <pg/util/PG_BinaryFileWriter.h>
#include <pg/util/PG_StringUtil.h>
#include <pg/util/PG_Exception.h>

namespace PG {
namespace FILE {

MPP::MPP() {
	// TODO Auto-generated constructor stub

}

MPP::MPP(const PG::UTIL::File& file){
	open(file);
}

bool MPP::open(const PG::UTIL::File& file){
		m_file = file;
		m_fileMPPinfos.clear();
		m_changed = false;

	 try{
		 PG::UTIL::ByteInFileStream reader(m_file);

		 const unsigned int number_of_offsets_set1 = reader.readUnsignedInt();
		 const unsigned int number_of_offsets_set2 = reader.readUnsignedInt();

		 if(number_of_offsets_set1 > 25 || number_of_offsets_set2 > 25) {
			 PG_ERROR_STREAM("Number of files is too big '"<<file<<"'!");
			 m_file.clear();
			 return true;
		 }

		 const unsigned int file_size = reader.readUnsignedInt();

		 if(file_size < m_file.size()){
			 PG_ERROR_STREAM("File too small '"<<file<<"'!");
			 m_file.clear();
			 return true;
		 }


		 std::vector<unsigned int> file_offsets_set1(number_of_offsets_set1);
		 std::vector<unsigned int> file_offsets_set2(number_of_offsets_set2);

		 reader.read((char*) &file_offsets_set1[0],number_of_offsets_set1*sizeof(unsigned int) );
		 reader.read((char*) &file_offsets_set2[0],number_of_offsets_set2*sizeof(unsigned int) );

		 for(unsigned int i = 0; i < number_of_offsets_set1; i++){
			 const unsigned file_start_offset = file_offsets_set1[i];
			 const unsigned file_end_offset =
					 ((i+1) >= number_of_offsets_set1)?
					 ((file_offsets_set2.empty())? file_size : file_offsets_set2.front())
					: file_offsets_set1[i+1];


			 if(file_start_offset == 0 || file_end_offset == 0) break;

			 if( file_start_offset >= file_end_offset)
				 throw_Exception("File order is wrong!");

			 fileInfo info;
			 info.setOffset(file_start_offset); //start offset
			 info.setSize(file_end_offset-info.getOffset()); //end offset

			 std::stringstream o;
				 o << "MPPSET1FILE"<<i/2<<".TX2";
			 info.setName(o.str());
			 m_fileMPPinfos.push_back(info);
		 }

		 for(unsigned int i = 0; i < number_of_offsets_set2; i++){
			 const unsigned file_start_offset = file_offsets_set2[i];
			 const unsigned file_end_offset = ((i+1) >= number_of_offsets_set2)? file_size : file_offsets_set2[i+1];

			 if(file_start_offset == 0 || file_end_offset == 0) break;

			 if( file_start_offset >= file_end_offset)
				 throw_Exception("File order is wrong!");

			 fileInfo info;
			 info.setOffset(file_start_offset); //start offset
			 info.setSize(file_end_offset-info.getOffset()); //end offset

			 std::stringstream o;
				 o << "MPPSET2FILE"<<i/2<<".UNKNOWN";
			 info.setName(o.str());
			 m_fileMPPinfos.push_back(info);
		 }

		 if(m_fileMPPinfos.empty()){
			 PG_ERROR_STREAM("No files found inside!");
			 m_file.clear();
			 m_fileMPPinfos.clear();
			 return true;
		 }

	 }catch (PG::UTIL::Exception& e) {
		 PG_ERROR_STREAM("Couldn't read given PSPFS file '"<<file<<"': "<<e.what());
		 m_file.clear();
		 m_fileMPPinfos.clear();
		 return true;
	 }catch (...) {
		 PG_ERROR_STREAM("Couldn't read given PSPFS file '"<<file<<"'!");
		 m_file.clear();
		 m_fileMPPinfos.clear();
		 return true;
	 }
	 PG_INFO_STREAM("Opening done");

	return false;
}


bool MPP::insert(const PG::UTIL::File& file){
	return true;
}
bool MPP::remove(const PG::UTIL::File& file){
	return true;
}
bool MPP::save(){
	return save(m_file);
}
bool MPP::save(const PG::UTIL::File& targetfile){
	return true;
}
void MPP::clear(){
	m_fileMPPinfos.clear();
	m_file.clear();
	m_changed = false;
}

bool MPP::isEmpty() const{
	return m_fileMPPinfos.empty();
}

const PG::UTIL::File& MPP::getOpendFile() const{
	return m_file;
}

unsigned int MPP::size() const{
	return m_fileMPPinfos.size();
}

bool MPP::find(const PG::UTIL::File& file, fileInfo& infoOut) const{
	std::string name = file.getFile();
	std::transform(name.begin(), name.end(), name.begin(), ::toupper);
	//PG_INFO_STREAM("Searching for '"<<name<<"'")
	auto it = std::find_if(m_fileMPPinfos.begin(), m_fileMPPinfos.end(), [name](const fileInfo& info){
		return info.getName() == name;
	});

	if(it != m_fileMPPinfos.end()){
		infoOut = (*it);
		return true;
	}

	return false;
}

const fileInfo& MPP::get(unsigned int index) const{
	return m_fileMPPinfos[index];
}



MPP::~MPP() {
	// TODO Auto-generated destructor stub
}

} /* namespace FILE */
} /* namespace PG */
