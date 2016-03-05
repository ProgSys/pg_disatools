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
	clear();
	m_file = file;

	 if(!m_file.exists() ){
		 PG_ERROR_STREAM("Target file '"<<file<<"' does not exist!");
		 m_file.clear();
		 return true;
	 }

	 try{
		 PG::UTIL::ByteInFileStream reader(m_file);
		 if(!reader.isopen()) return true;

		 const unsigned short number_of_offsets_set1 = reader.readUnsignedShort();
		 m_isNew = reader.readShort();
		 const unsigned short number_of_offsets_set2 = reader.readUnsignedShort();
		 reader.skip(2);


		 if(number_of_offsets_set1 > 25 || number_of_offsets_set2 > 25) {
			 PG_ERROR_STREAM("Number of files is too big '"<<file<<"'! ("<<number_of_offsets_set1<<", "<<m_isNew<<", "<<number_of_offsets_set2<<")");
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

		 m_fileTextureInfos.reserve(number_of_offsets_set1);
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
				 o << "TEXTURE"<<i<<".TX2";
			 info.setName(o.str());
			 m_fileTextureInfos.push_back(info);
		 }

		 m_fileGeometryInfos.reserve(number_of_offsets_set2);
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
				 o << "GEOMETRY"<<i<<".GEO";
			 info.setName(o.str());
			 m_fileGeometryInfos.push_back(info);
		 }

		 if(m_fileTextureInfos.empty() && m_fileGeometryInfos.empty()){
			 PG_ERROR_STREAM("No files found inside!");
			 clear();
			 return true;
		 }

	 }catch (PG::UTIL::Exception& e) {
		 PG_ERROR_STREAM("Couldn't read given PSPFS file '"<<file<<"': "<<e.what());
		 clear();
		 return true;
	 }catch (...) {
		 PG_ERROR_STREAM("Couldn't read given PSPFS file '"<<file<<"'!");
		 clear();;
		 return true;
	 }
	 PG_INFO_STREAM("Opening done");

	return false;
}

bool MPP::insert(const PG::UTIL::File& file){
	if(!file.exists()){
		PG_INFO_STREAM("File doesn't exist. '"<<file<<"'!");
		return true;
	}

	PG::UTIL::File ufile = file.toUpper();
	const std::string fileExt = ufile.getFileExtension();
	const std::string fileName = ufile.getFile();
	if(fileExt == "TX2"){
		auto it = std::find_if(m_fileTextureInfos.begin(), m_fileTextureInfos.end(), [fileName](const fileInfo& info){
			return info.getName() == fileName;
		});

		if(it != m_fileTextureInfos.end()){
			(*it).externalFile = file;
			(*it).size = file.size();
			return false;
		}else{
			fileInfo info(fileName, file.size(), 0);
			info.externalFile = file;
			m_fileTextureInfos.push_back(info);
			return false;
		}
	}else if(fileExt == "GEO"){
		auto it = std::find_if(m_fileGeometryInfos.begin(), m_fileGeometryInfos.end(), [fileName](const fileInfo& info){
			return info.getName() == fileName;
		});

		if(it != m_fileGeometryInfos.end()){
			(*it).externalFile = file;
			(*it).size = file.size();
			return false;
		}else{
			fileInfo info(fileName, file.size(), 0);
			info.externalFile = file;
			m_fileTextureInfos.push_back(info);
			return false;
		}
	}else{
		PG_INFO_STREAM("Only texture (*.TX2) or geometry (*.GEO) files can be added! '"<<file<<"'!");
		return true;

	}

}

bool MPP::remove(const PG::UTIL::File& file){
	PG::UTIL::File ufile = file.toUpper();
	const std::string fileExt = ufile.getFileExtension();
	const std::string fileName = ufile.getFile();
	if(fileExt == "TX2"){
		auto it = std::find_if(m_fileTextureInfos.begin(), m_fileTextureInfos.end(), [fileName](const fileInfo& info){
			return info.getName() == fileName;
		});

		if(it != m_fileTextureInfos.end()){
			m_fileTextureInfos.erase(it);
			return false;
		}
	}else if(fileExt == "GEO"){
		auto it = std::find_if(m_fileGeometryInfos.begin(), m_fileGeometryInfos.end(), [fileName](const fileInfo& info){
			return info.getName() == fileName;
		});

		if(it != m_fileGeometryInfos.end()){
			m_fileGeometryInfos.erase(it);
			return false;
		}
	}

	return true;
}
bool MPP::save(){
	return save(m_file);
}

inline void saveLoop(const std::vector<fileInfo>& original, std::vector<fileInfo> target,
		PG::UTIL::ByteInFileStream& reader_mpp, PG::UTIL::BinaryFileWriter& writer,
		char* (&c), unsigned int& header_start_offset, unsigned int& start_offset){

	for(const fileInfo& info: original){
		PG_INFO_STREAM(" header_start_offset: "<<header_start_offset<<" start_offset: "<<start_offset<<" w: "<<writer.getPosition());
		fileInfo current_info = info; //copy info
		writer.setPosition(header_start_offset);
		writer.writeInt(start_offset);
		header_start_offset += 4;

		writer.setPosition(start_offset);

		if(current_info.isExternalFile()){
			PG::UTIL::ByteInFileStream reader_file(current_info.externalFile);

			const unsigned int file_size = reader_file.size();

			// check if valid
			if(file_size == 0)
				PG_WARN_STREAM("File '"<<current_info.externalFile<<"' has a size of zero!");

			if(info.getSize() != file_size){
				PG_WARN_STREAM("File '"<<current_info.externalFile<<"' size is different than expected! ("<<current_info.getSize()<<" != "<<file_size<<")");
				current_info.setSize(file_size);
			}

			c = new char[current_info.getSize()];
			reader_mpp.read(c,current_info.getSize());
			writer.write(c, current_info.getSize());
			delete[] c;
			c = nullptr;

			start_offset = writer.getPosition();

			current_info.clearExternalFile();
		}else{
			PG_INFO_STREAM(current_info.getOffset() <<" size: "<<current_info.getSize()<<" name: "<<current_info.name<<" p: "<<writer.getPosition());
			reader_mpp.seek(current_info.getOffset());
			c = new char[current_info.getSize()];
			reader_mpp.read(c,current_info.getSize());
			writer.write(c, current_info.getSize());

			delete[] c;
			c = nullptr;

			start_offset += current_info.getSize();
		}
		target.push_back(current_info);
	} //for loop end
}

bool MPP::save(const PG::UTIL::File& targetfile){
	if(m_file.isEmpty()){
		PG_ERROR_STREAM("No MPP file opened.");
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

	std::vector<fileInfo> fileTextureInfos;
	fileTextureInfos.reserve(m_fileTextureInfos.size());
	std::vector<fileInfo> fileGeometryInfos;
	fileGeometryInfos.reserve(m_fileGeometryInfos.size());

	char* c = nullptr;
	try{
		PG::UTIL::BinaryFileWriter writer(target);
		if(size() == 0) return false;
		//setup header
		writer.writeShort(m_fileTextureInfos.size()); // number of textures
		writer.writeShort(m_isNew);
		writer.writeShort(m_fileGeometryInfos.size()); // number of geometry files
		writer.writeShort(0);

		writer.writeInt(0); //totalfilesize

		unsigned int header_start_offset = writer.getPosition();
		//allocate space for the header

		const unsigned int header_count = (size() == 1)? 1 : 5 + ((size()-1)/5) * 4;
		for(unsigned int i = 0; i < header_count; ++i)
			writer.writeInt(0);

		unsigned int start_offset = writer.getPosition();
		PG_INFO_STREAM(" START: header_count: "<<header_count <<" start_offset: "<<start_offset);
		//unsigned int end_offset = 0;


		PG::UTIL::ByteInFileStream reader_mpp(m_file);
		if(!reader_mpp.isopen()) return true;


		saveLoop(m_fileTextureInfos, fileTextureInfos, reader_mpp, writer, c, header_start_offset, start_offset);
		saveLoop(m_fileGeometryInfos, fileGeometryInfos, reader_mpp, writer, c, header_start_offset, start_offset);
		reader_mpp.close();

		writer.setPosition(8);
		writer.writeInt(start_offset);

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
		 m_fileTextureInfos = fileTextureInfos;
		 m_fileGeometryInfos = fileGeometryInfos;
		 m_changed = false;
	 }

	return false;

}

void MPP::clear(){
	m_fileTextureInfos.clear();
	m_fileGeometryInfos.clear();
	m_file.clear();
	m_changed = false;
	m_isNew = false;
}

bool MPP::isEmpty() const{
	return m_fileTextureInfos.empty() && m_fileGeometryInfos.empty();
}

const PG::UTIL::File& MPP::getOpendFile() const{
	return m_file;
}

unsigned int MPP::size() const{
	return m_fileTextureInfos.size() + m_fileGeometryInfos.size();
}

bool MPP::find(const PG::UTIL::File& file, fileInfo& infoOut) const{
	PG::UTIL::File ufile = file.toUpper();
	const std::string fileExt = ufile.getFileExtension();
	if(fileExt == "TX2"){
		auto it = std::find_if(m_fileTextureInfos.begin(), m_fileTextureInfos.end(), [ufile](const fileInfo& info){
			return info.getName() == ufile.getPath();
		});

		if(it != m_fileTextureInfos.end()){
			infoOut = (*it);
			return true;
		}
	}else if(fileExt == "GEO"){
		auto it = std::find_if(m_fileGeometryInfos.begin(), m_fileGeometryInfos.end(), [ufile](const fileInfo& info){
			return info.getName() == ufile.getPath();
		});

		if(it != m_fileGeometryInfos.end()){
			infoOut = (*it);
			return true;
		}
	}

	return false;
}

const fileInfo& MPP::get(unsigned int index) const{
	if(index >= m_fileTextureInfos.size()){
		return m_fileGeometryInfos[index-m_fileTextureInfos.size()];
	}else{
		return m_fileTextureInfos[index];
	}
}



MPP::~MPP() {
	// TODO Auto-generated destructor stub
}

} /* namespace FILE */
} /* namespace PG */
