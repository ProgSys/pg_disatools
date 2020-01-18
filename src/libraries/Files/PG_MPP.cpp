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

#include <Files/PG_MPP.h>

#include <vector>
#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamOutByteFile.h>
#include <Util/PG_StringUtil.h>
#include <Util/PG_Exception.h>
#include <Stream/PG_StreamInByteArray.h>
#include <cassert>

namespace PG {
namespace FILE {

MPP::MPP() {
	// TODO Auto-generated constructor stub

}

MPP::MPP(const PG::UTIL::File& file){
	open(file);
}

bool MPP::open(const PG::UTIL::File& file, PercentIndicator* percent){
	clear();
	m_file = file;

	 if(!m_file.exists() ){
		 PG_ERROR_STREAM("Target file '"<<file<<"' does not exist!");
		 m_file.clear();
		 return true;
	 }

	 try{
		 PG::STREAM::InByteFile reader(m_file);
		 if(!reader.isopen()) return true;

		 const unsigned short number_of_offsets_set1 = reader.readUnsignedShort();
		 m_hasNormals = reader.readShort();
		 const unsigned short number_of_offsets_set2 = reader.readUnsignedShort();
		 reader.skip(2);


		 if(number_of_offsets_set1 > 25 || number_of_offsets_set2 > 25) {
			 PG_ERROR_STREAM("Number of files is too big '"<<file<<"'! ("<<number_of_offsets_set1<<", "<<m_hasNormals<<", "<<number_of_offsets_set2<<")");
			 m_file.clear();
			 return true;
		 }

		 const unsigned int file_size = reader.readUnsignedInt();

		 if(file_size < m_file.size()){
			 PG_ERROR_STREAM("File too small '"<<file<<"'!");
			 m_file.clear();
			 return true;
		 }

		 std::vector<unsigned int> file_offsets((m_hasNormals)? number_of_offsets_set1*2+number_of_offsets_set2: number_of_offsets_set1+number_of_offsets_set2);
		 reader.read((char*) &file_offsets[0],file_offsets.size()*sizeof(unsigned int) );

		 unsigned int total_file_number;
		 if(m_hasNormals)
			 total_file_number = number_of_offsets_set1*2+number_of_offsets_set2;
		 else
			 total_file_number = number_of_offsets_set1+number_of_offsets_set2;
		 m_fileInfos.reserve(total_file_number);

		 std::size_t texturesEnd, normalsEnd;

		 for(unsigned int i = 0; i < number_of_offsets_set1; i++){
			 const unsigned file_start_offset = file_offsets[i];
			 const unsigned file_end_offset = (i+1 >= file_offsets.size())? file_size : file_offsets[i+1];


			 if(file_start_offset == 0 || file_end_offset == 0) break;

			 if( file_start_offset >= file_end_offset)
				 throw_Exception("File order is wrong!");

			 fileInfo info;
			 info.setOffset(file_start_offset); //start offset
			 info.setSize(file_end_offset-info.getOffset()); //end offset

			 std::stringstream o;
				 o << "TEXTURE"<<i<<".TX2";
			 info.setName(o.str());
			 info.fileType = fileInfo::TX2;
			 m_fileInfos.push_back(info);
			 if(percent) percent->percent = m_fileInfos.size()/float(total_file_number)*100;
		 }
		 texturesEnd = m_fileInfos.size();

		 if(m_hasNormals){
			 for(unsigned int i = 0; i < number_of_offsets_set1; i++){
				 const unsigned int index = i + number_of_offsets_set1;
				 const unsigned file_start_offset = file_offsets[index];
				 const unsigned file_end_offset = (index+1 >= file_offsets.size())? file_size : file_offsets[index+1];

				 if(file_start_offset == 0 || file_end_offset == 0) break;

				 if( file_start_offset >= file_end_offset)
					 throw_Exception("File order is wrong!");

				 fileInfo info;
				 info.setOffset(file_start_offset); //start offset
				 info.setSize(file_end_offset-info.getOffset()); //end offset

				 std::stringstream o;
					 o << "NORMAL"<<i<<".TX2";
				 info.setName(o.str());
				 info.fileType = fileInfo::TX2;
				 m_fileInfos.push_back(info);
				 if(percent) percent->percent = m_fileInfos.size()/float(total_file_number)*100;
			 }

		 }
		 normalsEnd = m_fileInfos.size();
		 
		 const unsigned int texturesSize = m_fileInfos.size();
		 for(unsigned int i = 0; i < number_of_offsets_set2; i++){

			 const unsigned int index = i + texturesSize;

			 const unsigned file_start_offset = file_offsets[index];
			 const unsigned file_end_offset = (index+1 >= file_offsets.size())? file_size : file_offsets[index+1];

			 if(file_start_offset == 0 || file_end_offset == 0) break;

			 if( file_start_offset >= file_end_offset)
				 throw_Exception("File order is wrong!");

			 fileInfo info;
			 info.setOffset(file_start_offset); //start offset
			 info.setSize(file_end_offset-info.getOffset()); //end offset

			 std::stringstream o;
				 o << "GEOMETRY"<<i<<".GEO";
			 info.setName(o.str());
			 info.fileType = fileInfo::GEO;
			 m_fileInfos.push_back(info);
			 if(percent) percent->percent = m_fileInfos.size()/float(total_file_number)*100;
		 }

		 m_normalsEnd = std::next(m_fileInfos.begin(), normalsEnd);
		 m_texturesEnd = std::next(m_fileInfos.begin(), texturesEnd);

		 assert(m_fileInfos.size() == total_file_number);
		 assert(std::distance(m_fileInfos.begin(), m_normalsEnd) > 0);
		 assert(std::distance(m_fileInfos.begin(), m_texturesEnd) > 0);

		 if(m_fileInfos.empty()){
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

bool MPP::replace(fileInfo& target,const PG::UTIL::File& file, bool keepName){
	if(!file.exists()){
		pushError("Replacement file not found!");
		PG_ERROR_STREAM(m_errors.top())
		return FAILURE;
	}

	PG::UTIL::File fileName = PG::UTIL::File(file.getFile()).toUpper();
	const std::string fileExt = fileName.getFileExtension();

	if(fileExt != "TX2" && fileExt != "GEO"){
		pushError("Replacement file not found!");
		PG_ERROR_STREAM(m_errors.top())
		return FAILURE;
	}
	if(fileExt == "TX2")
		target.fileType = fileInfo::TX2;
	else
		target.fileType = fileInfo::GEO;

	target.size = file.size();
	target.externalFile = file;
	m_changed = true;
	return SUCCESS;
}

bool MPP::insert(const PG::UTIL::File& file){
	if(m_file.isEmpty()){
		PG_ERROR_STREAM("No file opened.");
		return FAILURE;
	}

	if(!file.exists()){
		PG_ERROR_STREAM("File doesn't exist!");
		return FAILURE;
	}

	PG::UTIL::File fileName = PG::UTIL::File(file.getFile()).toUpper();
	const std::string fileExt = fileName.getFileExtension();

	if(fileExt != "TX2" && fileExt != "GEO"){
		pushError("You can only insert TX2 or GEO files into a MPP!");
		PG_ERROR_STREAM(m_errors.top())
		return FAILURE;
	}

	//file is already inside?
	auto it = std::find_if(m_fileInfos.begin(), m_fileInfos.end(), [fileName](const fileInfo& info){
		return info.getName() == fileName;
	});

	if(it != m_fileInfos.end()){
		(*it).externalFile = file;
	}else{
		std::string name = fileName.getName();
		if(fileExt == "TX2"){
			fileInfo info(fileName, file.size(), m_fileInfos.back().offset+m_fileInfos.back().size );
			info.fileType = fileInfo::TX2;
			if(name.size() > 5 && name.substr(0,6) == "NORMAL"){
				info.externalFile = file;
				m_hasNormals = true;
				m_fileInfos.insert(m_normalsEnd,info);
				m_normalsEnd++;
			}else if(name.size() > 6 && name.substr(0,7) == "TEXTURE"){
				info.externalFile = file;
				m_fileInfos.insert(m_texturesEnd,info);
				m_texturesEnd++;
				m_normalsEnd++;
			}else{
				pushError("A TX2 file should start with 'NORMAL' or 'TEXTURE'!");
				PG_ERROR_STREAM(m_errors.top())
				return FAILURE;
			}
		}else if(fileExt == "GEO"){
			std::stringstream o;
				 o << "GEOMETRY"<<std::distance(m_normalsEnd, m_fileInfos.end())<<".GEO";
			fileInfo info(o.str(), file.size(), m_fileInfos.back().offset+m_fileInfos.back().size );
			info.fileType = fileInfo::GEO;
			info.externalFile = file;
			m_fileInfos.push_back(info);
		}else
			return FAILURE;

	}

	return SUCCESS;

}

bool MPP::remove(std::vector<fileInfo>::iterator it){
	const unsigned int index = std::distance(m_fileInfos.begin(), it);
	if(it < m_fileInfos.end()){
		if(m_fileInfos.size() == 1){
			m_fileInfos.clear();
			m_hasNormals = false;
			m_texturesEnd = m_fileInfos.end();
			m_normalsEnd = m_fileInfos.end();
			return SUCCESS;
		}
		if(it < m_texturesEnd){
			m_texturesEnd--;
			m_normalsEnd--;
		}else if(it < m_normalsEnd)
			m_normalsEnd--;

		m_fileInfos.erase(it);

		if(m_normalsEnd == m_texturesEnd)
			m_hasNormals = false;

		m_changed = true;
	}else{
		PG_ERROR_STREAM("File '"<< it->getName()<<" found!");
		return FAILURE;
	}
	return SUCCESS;
}

bool MPP::save(const PG::UTIL::File& targetfile, PercentIndicator* percent){
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

	const unsigned short number_of_textures = std::distance(m_fileInfos.begin(),m_texturesEnd);
	const unsigned short number_of_normals = std::distance(m_texturesEnd, m_normalsEnd);
	const unsigned short number_of_geometry = std::distance(m_normalsEnd, m_fileInfos.end());

	if(m_hasNormals && number_of_textures != number_of_normals){
		pushError("The number of textures and normals needs to be the same!");
		PG_ERROR_STREAM(m_errors.top())
		return FAILURE;
	}

	std::vector<fileInfo> fileInfos;
	fileInfos.reserve(m_fileInfos.size());

	char* c = nullptr;
	try{
		PG::STREAM::OutByteFile writer(target);
		if(size() == 0) return false;
		//setup header
		writer.writeShort(number_of_textures); // number of textures
		writer.writeShort(m_hasNormals);
		writer.writeShort(number_of_geometry); // number of geometry files
		writer.writeShort(0);

		writer.writeInt(0); //totalfilesize

		unsigned int header_start_offset = writer.getPosition();

		//allocate space for the header
		for(unsigned int i = 0; i < m_fileInfos.size(); ++i)
			writer.writeInt(0);

		unsigned int fileEnd = writer.getPosition();

		PG::STREAM::InByteFile reader_mpp(m_file);
		if(!reader_mpp.isopen()) return true;

		for(const fileInfo& info: m_fileInfos){
			fileInfo new_info = info;

			if(new_info.isExternalFile()){
				PG::STREAM::InByteFile reader_file(new_info.externalFile);
				const unsigned int file_size = reader_file.size();
				if(new_info.size != file_size){
					PG_WARN_STREAM("File '"<<new_info.externalFile<<"' size is different than expected! ("<<new_info.getSize()<<" != "<<file_size<<")");
					new_info.setSize(file_size);
				}
				new_info.offset = writer.getPosition();
				c = new char[info.size];
				reader_file.read(c, info.size);
				reader_file.close();
				writer.write(c, info.size);
				delete c;
				c = nullptr;
				new_info.clearExternalFile();
			}else{

				new_info.offset = writer.getPosition();
				reader_mpp.seek(info.offset);
				c = new char[info.size];
				reader_mpp.read(c, info.size);
				writer.write(c, info.size);
				delete c;
				c = nullptr;
			}
			fileEnd = writer.getPosition();

			writer.seek(header_start_offset);
			writer.writeInt(new_info.offset);
			header_start_offset += sizeof(int);
			writer.seek(fileEnd);

			fileInfos.push_back(new_info);
			if(percent) percent->percent = (fileInfos.size()/float(m_fileInfos.size()))*100;
		}

		reader_mpp.close();

		writer.setPosition(8);
		writer.writeInt(fileEnd);

		writer.close();

	}catch (PG::UTIL::Exception& e) {
		 PG_ERROR_STREAM("Couldn't save PSPFS archive! : "<<e.what());
		 target.remove();
		 if(c) delete c;
		 return true;
	}catch (...) {
		 PG_ERROR_STREAM("Couldn't save PSPFS archive!");
		 target.remove();
		 if(c) delete c;
		 return true;
	}

	 if(overwrite){
		 m_file.remove();
		 target.rename(m_file.getPath());
		 m_fileInfos = fileInfos;
		 m_changed = false;
	 }

	return false;

}

void MPP::clear(){
	ExtractorBase::clear();
	m_hasNormals = false;
	m_texturesEnd = m_fileInfos.end();
	m_normalsEnd = m_fileInfos.end();
	//m_lastError.clear();
}


bool MPP::hasModels() const{
	return getNumberOfModels();
}

bool MPP::hasTextures() const{
	return getNumberOfTextures();
}

bool MPP::hasNormalMaps() const{
	return getNumberOfNormalMaps();
}

int MPP::getNumberOfModels() const{
	return std::distance<std::vector<fileInfo>::const_iterator>(m_normalsEnd, m_fileInfos.end());
}

int MPP::getNumberOfTextures() const{
	return std::distance<std::vector<fileInfo>::const_iterator>(m_fileInfos.begin(),m_texturesEnd);
}
int MPP::getNumberOfNormalMaps() const{
	return std::distance(m_texturesEnd, m_normalsEnd);
}

bool MPP::getTexture(int index, tx2Image& imgOut) const{
	if(index >= getNumberOfTextures()) return false;
	char* data = nullptr;

	unsigned int size = ExtractorBase::extract( m_fileInfos[index],data);
	if(size < 17) {
		delete data;
		return false;
	}

	PG::STREAM::InByteArray reader(data,size);
	PG::FILE::readTX2Header(&reader, imgOut.header);
	reader.read(imgOut.data, size-16);
	reader.close();
	delete data;
	return true;
}

char const* MPP::getError(){
	std::string errorMsg = ExtractorBase::getError();
	if(!errorMsg.empty()) return errorMsg.c_str();

	if(!m_fileInfos.empty() && ( std::distance(m_fileInfos.begin(), m_texturesEnd) != std::distance(m_texturesEnd, m_normalsEnd) && m_hasNormals )){
		errorMsg = "The number of normal textures should be the same as the number of textures!";
		PG_WARN_STREAM(errorMsg);
		return errorMsg.c_str();
	}
	return "";
}

char const* MPP::getType() const{
	return "MPP";
}


MPP::~MPP() {
	// TODO Auto-generated destructor stub
}

} /* namespace FILE */
} /* namespace PG */
