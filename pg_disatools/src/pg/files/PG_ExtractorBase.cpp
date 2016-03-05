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

#include <pg/files/PG_ExtractorBase.h>
#include <pg/util/PG_Base.h>

#include <pg/util/PG_ByteInFileStream.h>
#include <pg/util/PG_BinaryFileWriter.h>
#include <pg/util/PG_Exception.h>


namespace PG {
namespace FILE {

fileInfo::fileInfo(){}
fileInfo::fileInfo(const std::string& _name,unsigned int _size,unsigned int _offset):
		name(_name),size(_size),offset(_offset){}
fileInfo::fileInfo(const PG::UTIL::File& _name,unsigned int _size,unsigned int _offset):
		name(_name),size(_size),offset(_offset){}


const PG::UTIL::File& fileInfo::getName() const{
	return name;
}
unsigned int fileInfo::getSize() const{
	return size;
}
unsigned int fileInfo::getOffset() const{
	return offset;
}
const PG::UTIL::File& fileInfo::getExternalName() const{
	return externalFile;
}

std::string fileInfo::getEileExtension() const{
	return name.getFileExtension();
}

void fileInfo::setName(const PG::UTIL::File& _name){
	name = _name;
}
void fileInfo::setSize(unsigned int _size){
	size = _size;
}
void fileInfo::setOffset(unsigned int _offset){
	offset = _offset;
}
void fileInfo::setExternalName(const PG::UTIL::File& _externalFile){
	externalFile = _externalFile;
}

bool fileInfo::isExternalFile() const{
	return !externalFile.isEmpty();
}
void fileInfo::clearExternalFile(){
	externalFile.clear();
}
void fileInfo::clear(){
	name.clear();
	size = 0;
	offset = 0;
	externalFile.clear();;
}


ExtractorBase::ExtractorBase() {
	// TODO Auto-generated constructor stub

}


bool ExtractorBase::exists(const PG::UTIL::File& file) const{
	if(isEmpty() || getOpendFile().isEmpty()){
		PG_ERROR_STREAM("No archive is opened.");
		return false;
	}

	fileInfo info;
	if(find(file, info)){
		return true;
	}

	return false;
}

bool ExtractorBase::extract(const PG::UTIL::File& file, const PG::UTIL::File& targetFile) const{
	if(isEmpty() || getOpendFile().isEmpty()){
		PG_ERROR_STREAM("No archive is opened.");
		return true;
	}

	if(getOpendFile().getPath() == targetFile.getPath()){
		PG_ERROR_STREAM("Target file can't the the archive itself!");
		return true;
	}

	fileInfo info;
	if(!find(file, info)){
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

			if(info.getSize() != file_size){
				PG_WARN_STREAM("File '"<<info.externalFile<<"' size is different than expected! ("<<info.getSize()<<" != "<<file_size<<")");
				info.setSize(file_size);
			}

			if(file_size == 0){
				PG_ERROR_STREAM("File '"<<info.externalFile<<"' has a size of zero!");
				return true;
			}

			c = new char[info.getSize()];
			reader.read(c, info.getSize());
			reader.close();

			PG::UTIL::BinaryFileWriter writer(targetFile);
			writer.write(c,info.getSize() );
			writer.close();

			delete[] c;
			c = nullptr;


		}else{
			PG::UTIL::ByteInFileStream reader(getOpendFile());

			c = new char[info.getSize()];
			reader.seek(info.getOffset());
			reader.read(c,info.getSize() );
			reader.close();

			PG::UTIL::BinaryFileWriter writer(targetFile);
			writer.write(c,info.getSize() );
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

unsigned int ExtractorBase::extract(const PG::UTIL::File& file, char* (&data) ) const{
	if(isEmpty() || getOpendFile().isEmpty()){
		PG_ERROR_STREAM("No PSPFS file opened.");
		return 0;
	}

	fileInfo info;
	if(!find(file, info)){
		PG_ERROR_STREAM("File '"<<file<<"' not found in archive!");
		return 0;
	}

	data = nullptr;
	try{
		if(info.isExternalFile()){
			PG::UTIL::ByteInFileStream reader(info.externalFile);
			const unsigned int filesize = reader.size();
			data = new char[filesize];
			reader.read(data,filesize);
			reader.close();
			return filesize;
		}else{
			PG::UTIL::ByteInFileStream reader(getOpendFile());

			data = new char[info.getSize()];
			reader.seek(info.getOffset());
			reader.read(data,info.getSize() );
			reader.close();
			return info.getSize();
		}
	}catch (PG::UTIL::Exception& e) {
		PG_ERROR_STREAM("Couldn't write file '"<<file<<"': "<<e.what());
		if(data) delete[] data;
		return 0;
	}catch (...) {
		PG_ERROR_STREAM("Couldn't write file '"<<file<<"'!");
		if(data) delete[] data;
		return 0;
	}
	return 0;
}

bool ExtractorBase::isChanged() const{
	return m_changed;
}

const fileInfo& ExtractorBase::operator[](unsigned int index) const{
	return get(index);
}
fileInfo* ExtractorBase::getDataPointer(unsigned int index) const{
	return const_cast<fileInfo*>(&get(index));
}

ExtractorBase::~ExtractorBase() {
}

} /* namespace FILE */
} /* namespace PG */
