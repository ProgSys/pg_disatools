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
#include <pg/Util/PG_Base.h>
#include <pg/Util/PG_Exception.h>
#include <pg/Stream/PG_StreamOutByteFile.h>

namespace PG {
namespace STREAM {

OutByteFile::OutByteFile(const PG::UTIL::File& path, bool append):
		OutByteFile(path.getPath(), append){}

OutByteFile::OutByteFile(const std::string& path, bool append) {
	if(path.empty()){
		PG_ERROR_STREAM("File path empty!");
		throw_Exception("Could not create file! File path empty!");
		return;
	}

	if(append)
		m_outFile.open(path.c_str(),std::ios::binary | ios::app);
	else
		m_outFile.open(path.c_str(),std::ios::binary);

	if (!m_outFile.is_open()){
		PG_ERROR_STREAM("Could not create file: "<<path);
		throw_Exception("Could not create file.");
		return;
	}

}

long OutByteFile::getPosition(){
	return m_outFile.tellp();
}

void OutByteFile::setPosition(long position){
	m_outFile.seekp(position,ios_base::beg);
}

void OutByteFile::goEnd(int i){
	m_outFile.seekp(i,ios_base::end);
}

void OutByteFile::writeChar(char c){
	m_outFile.write((char *)&c, sizeof(char));
}

void OutByteFile::writeInt(int i){
	m_outFile.write((char *)&i, sizeof(int));
}

void OutByteFile::writeLong(long l){
	m_outFile.write((char *)&l, sizeof(long));
}

void OutByteFile::writeLongLong(long long ll){
	m_outFile.write((char *)&ll, sizeof(long long));
}

void OutByteFile::writeShort(short s){
	m_outFile.write((char *)&s, sizeof(short));
}

void OutByteFile::writeFloat(float i){
	m_outFile.write((char *)&i, sizeof(float));
}

void OutByteFile::writeFloats(const std::vector<float>& floats){
	m_outFile.write((char *)&floats[0], sizeof(float)*floats.size());
}

void OutByteFile::writeDouble(double d){
	m_outFile.write((char *)&d, sizeof(double));
}

void OutByteFile::operator<<(const std::string& string){
	m_outFile<<string;
}

void OutByteFile::writeString(const std::string& string){
	m_outFile<<string;
}

void OutByteFile::write(char* data, unsigned int size){
	m_outFile.write(data, size);
}


void OutByteFile::skip(unsigned int skip){
	const unsigned int currentPos = pos();
	const unsigned int filesize = size();

	if(currentPos+skip >= filesize){
		goEnd();
		for(unsigned int i = filesize; i < currentPos+skip; i++ )
			writeChar(0);
	}else{
		seek(currentPos+skip);
	}
}
///Go to the given position
void OutByteFile::seek(unsigned int position){
	m_outFile.seekp(position,ios_base::beg);
}
///Get the current position
unsigned int OutByteFile::pos(){
	return m_outFile.tellp();
}
///Set the position to zero
void OutByteFile::rewind(){
	seek(0);
}

bool OutByteFile::isOpen() const{
	return m_outFile.is_open();
}

unsigned int OutByteFile::size(){
	const unsigned int currPos = pos();
	goEnd();
	const unsigned int size = pos();
	seek(currPos);
	return size;
}

void OutByteFile::close(){
	m_outFile.close();
}

OutByteFile::~OutByteFile() {
	close();
}

} /* namespace STREAM */
} /* namespace PG */
