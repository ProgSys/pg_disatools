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

		 const unsigned int number_of_tx2_files = reader.readUnsignedInt();

		 if(number_of_tx2_files > 1000){
			 PG_ERROR_STREAM("Number of tx2 files is to big '"<<file<<"'!");
			 m_file.clear();
			 return true;
		 }

		 const unsigned int number_of_something_files = reader.readUnsignedInt();

		 const unsigned int file_size = reader.readUnsignedInt();

		 if(file_size < m_file.size()){
			 PG_ERROR_STREAM("File too small '"<<file<<"'!");
			 m_file.clear();
			 return true;
		 }

		 for(unsigned int i = 0; i < number_of_something_files; ++i){

			 fileMPPinfo info;
			 info.offset = reader.readUnsignedInt();
			 info.size = reader.readUnsignedInt();

			 if(info.offset == 0 || info.size == 0){
				 break;
			 }

			 if(info.offset < m_fileMPPinfos.back().offset){
				 throw_Exception("File order is wrong!");
			 }

			 std::stringstream o;
			 o << "MPPFILE"<<i<<".TX2";
			 info.file = o.str();
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


	return false;
}

MPP::~MPP() {
	// TODO Auto-generated destructor stub
}

} /* namespace FILE */
} /* namespace PG */
