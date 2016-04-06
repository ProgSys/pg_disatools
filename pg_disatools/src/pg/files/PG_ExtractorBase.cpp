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

#include <pg/stream/PG_StreamInByteFile.h>
#include <pg/stream/PG_StreamOutByteFile.h>
#include <pg/util/PG_Exception.h>
#include <pg/files/PG_FileTests.h>

#include <algorithm>

namespace PG {
namespace FILE {

fileInfo::fileInfo(){}
fileInfo::fileInfo(const std::string& _name,unsigned int _size,unsigned int _offset):
		name(_name),size(_size),offset(_offset){}
fileInfo::fileInfo(const PG::UTIL::File& _name,unsigned int _size,unsigned int _offset):
		name(_name),size(_size),offset(_offset){}
fileInfo::fileInfo(const fileInfo& info):
		name(info.name),size(info.size),offset(info.offset),decompressedFileSize(info.decompressedFileSize),externalFile(info.externalFile),
		fileType(info.fileType)
{}

void fileInfo::operator=(const fileInfo& info){
	name = info.name;
	size = info.size;
	decompressedFileSize = info.decompressedFileSize;
	offset = info.offset;
	externalFile = info.externalFile;

	fileType = info.fileType;
}

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

std::string fileInfo::getFileExtension() const{
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
void fileInfo::setAsDummy(unsigned int _offset){
	externalFile.clear();
	name = "DUMMY.DAT";
	size = 5;
	offset = _offset;
	decompressedFileSize = 0;
}


bool fileInfo::isExternalFile() const{
	return !externalFile.isEmpty();
}

bool fileInfo::isCompressed() const{
	return fileType == IMY || fileType == COLA;
}
bool fileInfo::isPackage() const{
	return fileType == OLA || fileType == PSPFS_V1 || fileType == PSPFS_V1 || fileType == COLA || fileType == SOLA;
}

bool fileInfo::isTexture() const{
	return fileType == TX2 ;
}

bool fileInfo::isValid() const{
	if(name.getPath().empty() || size == 0 || (offset < 16 && !isExternalFile()) )
		return false;
	return true;
}

void fileInfo::clearExternalFile(){
	externalFile.clear();
}
void fileInfo::clear(){
	name.clear();
	size = 0;
	offset = 0;
	externalFile.clear();
	decompressedFileSize = 0;
}


ExtractorBase::ExtractorBase() {
	// TODO Auto-generated constructor stub

}

bool ExtractorBase::save(PercentIndicator* percent){
	return save(getOpendFile(), percent);
}

bool ExtractorBase::insert(const PG::UTIL::File& file){
	if(m_file.isEmpty()){
		PG_ERROR_STREAM("No file opened.");
		return FAILURE;
	}

	if(!file.exists()){
		PG_ERROR_STREAM("File doesn't exist!");
		return FAILURE;
	}

	PG::UTIL::File fileName = PG::UTIL::File(file.getFile()).toUpper();
	//file is already inside?
	auto it = std::find_if(m_fileInfos.begin(), m_fileInfos.end(), [fileName](const fileInfo& info){
		return info.getName() == fileName;
	});

	PG_MARK;
	if(it != m_fileInfos.end()){
		(*it).externalFile = file;
		//tests
		fileProperties prop((*it));
		getFileProperties(prop);
		PG_INFO_STREAM(prop.info.isCompressed())
	}else{
		fileInfo info(fileName, file.size(), m_fileInfos.back().offset+m_fileInfos.back().size );
		info.externalFile = file;
		fileProperties prop(info);
		getFileProperties(prop);
		m_fileInfos.push_back(info);
	}

	m_changed = true;

	return SUCCESS;
}

bool ExtractorBase::remove(fileInfo& target){
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
	}else{
		PG_ERROR_STREAM("File '"<<target.getName()<<" found!");
		return FAILURE;
	}
	m_changed = true;
	return SUCCESS;
}

void ExtractorBase::clear(){
	m_file.clear();
	m_originalFileSize = 0;
	m_fileInfos.clear();
	m_changed = false;
}

bool ExtractorBase::find(const PG::UTIL::File& file, fileInfo& infoOut) const{
	std::string name = file.getFile();
	std::transform(name.begin(), name.end(), name.begin(), ::toupper);
	auto it = std::find_if(m_fileInfos.begin(), m_fileInfos.end(), [name](const fileInfo& info){
		return info.getName() == name;
	});

	if(it != m_fileInfos.end()){
		infoOut = (*it);
		return true;
	}

	return false;
}

bool ExtractorBase::isEmpty() const{
	return m_fileInfos.empty();
}

unsigned int ExtractorBase::size() const{
	return m_fileInfos.size();
}

const PG::UTIL::File& ExtractorBase::getOpendFile() const{
	return m_file;
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

unsigned int ExtractorBase::extract(const fileInfo& target, char* (&data) ) const{
	data = nullptr;
	try{
		if(target.isExternalFile()){
			PG::STREAM::InByteFile reader(target.externalFile);
			const unsigned int filesize = reader.size();
			data = new char[filesize];
			reader.read(data,filesize);
			reader.close();
			return filesize;
		}else{
			PG::STREAM::InByteFile reader(getOpendFile());

			data = new char[target.getSize()];
			reader.seek(target.getOffset());
			reader.read(data,target.getSize() );
			reader.close();
			return target.getSize();
		}
	}catch (PG::UTIL::Exception& e) {
		PG_ERROR_STREAM("Couldn't read file '"<<target.getName()<<"': "<<e.what());
		if(data) delete[] data;
		return 0;
	}catch (...) {
		PG_ERROR_STREAM("Couldn't write file '"<<target.getName()<<"'!");
		if(data) delete[] data;
		return 0;
	}
	return 0;
}

unsigned int ExtractorBase::extract(const PG::UTIL::File& file, char* (&data) ) const{
	fileInfo info;
	if(!find(file, info)){
		PG_ERROR_STREAM("File '"<<file<<"' not found in archive!");
		return 0;
	}

	return extract(info, data);
}

bool ExtractorBase::extract(const fileInfo& target, const PG::UTIL::File& targetFile ) const{
	if(isEmpty() || getOpendFile().isEmpty()){
		PG_ERROR_STREAM("No archive is opened.");
		return true;
	}

	if(getOpendFile().getPath() == targetFile.getPath()){
		PG_ERROR_STREAM("Target file can't the the archive itself!");
		return true;
	}

	char* c = nullptr;
	unsigned int size = extract(target, c);
	if(size == 0){
		if(c) delete[] c;
		PG_ERROR_STREAM("Couldn't read target extract file!");
		return FAILURE;
	}

	try{
		PG::STREAM::OutByteFile writer(targetFile);
		if(!writer.isOpen()){
			PG_ERROR_STREAM("Couldn't create file '"<<targetFile<<"'!");
			if(c) delete[] c;
			return true;
		}
		writer.write(c, size);

		delete[] c;
		c = nullptr;

		PG_INFO_STREAM("File extracted to '"<<targetFile<<"'.");
	}catch (PG::UTIL::Exception& e) {
		PG_ERROR_STREAM("Couldn't write file '"<<targetFile<<"': "<<e.what());
		if(c) delete[] c;
		return true;
	}catch (...) {
		PG_ERROR_STREAM("Couldn't write file '"<<targetFile<<"'!");
		if(c) delete[] c;
		return true;
	}

	return false;

}

bool ExtractorBase::extract(const PG::UTIL::File& file, const PG::UTIL::File& targetFile) const{
	fileInfo info;
	if(!find(file, info)){
		PG_ERROR_STREAM("File '"<<file.getPath()<<"' not found in archive!");
		return true;
	}

	if(!info.isValid()){
		PG_ERROR_STREAM("File info is wrong!");
		return true;
	}

	return extract(info,targetFile);
}


bool ExtractorBase::isChanged() const{
	return m_changed;
}

bool ExtractorBase::checkValid(std::string& errorMessageOut){
	if(!m_lastError.empty()){
		errorMessageOut = m_lastError;
		m_lastError.clear();
		return false;
	}

	return true;
}

fileInfo& ExtractorBase::get(unsigned int index){
	return m_fileInfos[index];
}
const fileInfo& ExtractorBase::get(unsigned int index) const{
	return m_fileInfos[index];
}

const fileInfo& ExtractorBase::operator[](unsigned int index) const{
	return m_fileInfos[index];
}
fileInfo* ExtractorBase::getDataPointer(unsigned int index) const{
	return const_cast<fileInfo*>(&m_fileInfos[index]);
}

void ExtractorBase::getFileProperties(fileProperties& target) const{
	if(isEmpty() || getOpendFile().isEmpty()){
		PG_ERROR_STREAM("No archive is opened.");
		return;
	}

	PG_INFO_STREAM("type: "<<target.info.fileType<<" == "<<fileInfo::UNKNOWN);

	PG::STREAM::InByteFile reader;
	if(target.info.isValid() && target.info.fileType == fileInfo::UNKNOWN){
		unsigned int resetPos = 0;
		if(target.info.isExternalFile()){
			reader.open(target.info.externalFile);
			if(!reader.isopen()){
				PG_ERROR_STREAM("Couldn't open external file '"<<target.info.externalFile<<"'!");
				return;
			}
		}else{
			reader.open(getOpendFile());
			resetPos = target.info.offset;
			if(!reader.isopen()){
				PG_ERROR_STREAM("Couldn't open '"<<target.info.externalFile<<"'!");
				return;
			}
			reader.seek(target.info.offset);
		}

		if( (target.info.decompressedFileSize = isIMYPackage(reader)) ){
			reader.close();
			target.info.fileType = fileInfo::COLA;
			return;
		}

		reader.seek(resetPos);
		if( isIMY(reader) ){
			reader.close();
			target.info.fileType = fileInfo::IMY;
			return;
		}

		reader.seek(resetPos);
		if( isTX2(reader)){

			target.info.fileType = fileInfo::TX2;

			reader.seek(target.info.offset);
			target.textureCompression = getTX2CompressionType(reader);
			reader.close();
			return;
		}

		reader.seek(resetPos);
		if( isPSPFS(reader)){
			reader.close();
			target.info.fileType = fileInfo::PSPFS_V1;
			return;
		}else{
			target.info.fileType = fileInfo::UNKNOWN;
		}
		reader.close();
	}else if(target.info.fileType == fileInfo::TX2){
		reader.seek(target.info.offset);
		target.textureCompression = getTX2CompressionType(reader);
	}


}

bool ExtractorBase::replace(fileInfo& target,const PG::UTIL::File& file, bool keepName){
	if(!file.exists()){
		PG_ERROR_STREAM("Replacement file not found!")
		return FAILURE;
	}

	target.externalFile = file;
	target.size = file.size();
	//tests
	fileProperties prop(target);
	getFileProperties(prop);

	if(!keepName){
		target.name = file.getFile();
		target.name = target.name.toUpper();
	}



	m_changed = true;
	return SUCCESS;
}

bool ExtractorBase::replace(const PG::UTIL::File& targetfile, const PG::UTIL::File& file, bool keepName){
	fileInfo info; //will this work?
	if(find(targetfile, info)){
		return replace(info, file, keepName);
	}
	return FAILURE;
}

bool ExtractorBase::remove(const PG::UTIL::File& file){
	fileInfo info;
	if(!find(file, info)){
		PG_ERROR_STREAM("File '"<<file.getPath()<<"' not found in archive!");
		return FAILURE;
	}

	return remove(info);
}

ExtractorBase::~ExtractorBase() {
}

} /* namespace FILE */
} /* namespace PG */
