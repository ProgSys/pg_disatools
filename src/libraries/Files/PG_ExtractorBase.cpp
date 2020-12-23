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

#include <Files/PG_ExtractorBase.h>
#include <Util/PG_Base.h>

#include <Stream/PG_StreamInByteFile.h>
#include <Stream/PG_StreamOutByteFile.h>
#include <Util/PG_Exception.h>
#include <Files/PG_FileTests.h>

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
char const* fileInfo::getNameConst() const{
	return name.getPath().c_str();
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
	return fileType == OLA || fileType == PSPFS_V1 || fileType == PS_FS_V1 || fileType == COLA || fileType == SOLA;
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

	if(it != m_fileInfos.end()){
		(*it).externalFile = file;
		getFileProperties(*it);
	}else{
		fileInfo info(fileName, file.size(), m_fileInfos.back().offset+m_fileInfos.back().size );
		info.externalFile = file;
		getFileProperties(info);
		m_fileInfos.push_back(info);
	}

	m_changed = true;

	return SUCCESS;
}

bool ExtractorBase::remove(std::vector<fileInfo>::iterator it){
	if (it == m_fileInfos.end()) return false;
	const unsigned int index = std::distance(m_fileInfos.begin(), it);
	if(it < m_fileInfos.end()){
		if(index < m_originalFileSize){
			if(it->name == "DUMMY.DAT"){
				m_fileInfos.erase(it);
			}else{
				it->setAsDummy(it->offset);
			}
		}else{
			m_fileInfos.erase(it);
		}
	}else{
		PG_ERROR_STREAM("File '"<<it->getName()<<"not found!");
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

std::vector<fileInfo>::const_iterator ExtractorBase::find(const PG::UTIL::File& file, fileInfo& infoOut) const{
	std::string name = file.getFile();
	std::transform(name.begin(), name.end(), name.begin(), ::toupper);
	auto it = std::find_if(m_fileInfos.cbegin(), m_fileInfos.cend(), [&name](const fileInfo& info){
		return info.getName() == name;
	});

	if(it != m_fileInfos.end()){
		infoOut = (*it);
	}

	return it;
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

ExtractorBase::fileInfo_iterator ExtractorBase::toIterator(fileInfo* ptr) {
	#ifdef _MSC_VER
	return m_fileInfos.begin() + ((ptr - &m_fileInfos.front())/sizeof(fileInfo));
	#else
	return ExtractorBase::fileInfo_iterator(ptr);
	#endif
}

ExtractorBase::fileInfo_const_iterator ExtractorBase::toIterator(fileInfo* ptr) const {
	#ifdef _MSC_VER
	return m_fileInfos.cbegin() + ((ptr - &m_fileInfos.front()) / sizeof(fileInfo));
	#else
	return ExtractorBase::fileInfo_iterator(ptr);
	#endif
}

bool ExtractorBase::exists(const PG::UTIL::File& file) const{
	if(isEmpty() || getOpendFile().isEmpty()){
		PG_ERROR_STREAM("No archive is opened.");
		return false;
	}

	fileInfo info;
	return find(file, info) != m_fileInfos.end();
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
	if(find(file, info) == m_fileInfos.end()){
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
	if(find(file, info) == m_fileInfos.end()){
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

char const* ExtractorBase::getError(){

	if(!m_errors.empty()){
		const std::string errorMessageOut = m_errors.top();
		m_errors.pop();
		return errorMessageOut.c_str();
	}

	return "";
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

fileProperties ExtractorBase::getFileProperties(fileInfo& info) const{
	if(isEmpty() || getOpendFile().isEmpty()){
		PG_ERROR_STREAM("No archive is opened.");
		return fileProperties("");
	}

	PG::STREAM::InByteFile reader;
	unsigned int resetPos = 0;
	if(info.isValid()){
		if(info.isExternalFile()){
			reader.open(info.externalFile);
			if(!reader.isopen()){
				PG_ERROR_STREAM("Couldn't open external file '"<<info.externalFile<<"'!");
				return fileProperties("");
			}
		}else{
			reader.open(getOpendFile());
			resetPos = info.offset;
			if(!reader.isopen()){
				PG_ERROR_STREAM("Couldn't open '"<<info.externalFile<<"'!");
				return fileProperties("");
			}
			reader.seek(info.offset);
		}
	}else return fileProperties("");

	fileProperties properties(info.name.getFile().c_str());
	properties.size = info.size;
	properties.offset = info.offset;
	properties.type = info.fileType;
	properties.isExternal = info.isExternalFile();

	if(properties.type == fileInfo::UNKNOWN){
		if( (info.decompressedFileSize = isIMYPackage(reader)) ){
			reader.close();
			properties.type = fileInfo::COLA;
			info.fileType = fileInfo::COLA;
			return properties;
		}

		reader.seek(resetPos);
		if( isIMY(reader) ){
			reader.close();
			properties.type = fileInfo::IMY;
			info.fileType = fileInfo::IMY;
			return properties;
		}

		reader.seek(resetPos);
		if( isTX2(reader)){

			properties.type = fileInfo::TX2;
			info.fileType = fileInfo::TX2;

			reader.seek(info.offset);
			properties.textureCompression = getTX2CompressionType(reader);
			reader.close();
			return properties;
		}

		reader.seek(resetPos);
		if(isPSPFS(reader)){
			reader.close();
			properties.type = fileInfo::PSPFS_V1;
			info.fileType = fileInfo::PSPFS_V1;
			return properties;
		}
			

		reader.seek(resetPos);
		if (isPSFS(reader)) {
			reader.close();
			properties.type = fileInfo::PS_FS_V1;
			info.fileType = fileInfo::PS_FS_V1;
			return properties;
		}

		properties.type =  fileInfo::UNKNOWN;
		reader.close();



	}else if(properties.type == fileInfo::TX2){
		reader.seek(info.offset);
		properties.textureCompression = getTX2CompressionType(reader);
	}

	return properties;
}

bool ExtractorBase::replace(fileInfo& target,const PG::UTIL::File& file, bool keepName){
	if(!file.exists()){
		PG_ERROR_STREAM("Replacement file not found!")
		return FAILURE;
	}

	target.externalFile = file;
	target.size = file.size();

	if(!keepName){
		target.name = file.getFile();
		target.name = target.name.toUpper();

	}

	getFileProperties(target);

	m_changed = true;
	return SUCCESS;
}

bool ExtractorBase::replace(const PG::UTIL::File& targetfile, const PG::UTIL::File& file, bool keepName){
	fileInfo info; //will this work?
	if(find(targetfile, info) != m_fileInfos.end()){
		return replace(info, file, keepName);
	}
	return FAILURE;
}

bool ExtractorBase::remove(const PG::UTIL::File& file){
	fileInfo info;
	auto it = find(file, info);
	if(it == m_fileInfos.end()){
		PG_ERROR_STREAM("File '"<<file.getPath()<<"' not found in archive!");
		return FAILURE;
	}
	const int  i = std::distance(m_fileInfos.cbegin(), it);
	return remove(m_fileInfos.begin() + i);
}

void ExtractorBase::pushError(const std::string& error){
	m_errors.push(error);
}

ExtractorBase::~ExtractorBase() {
}

} /* namespace FILE */
} /* namespace PG */
