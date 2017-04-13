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
#include <pg/Stream/PG_StreamOutByteArray.h>
#include <cstring>
#include <string.h>

namespace PG {
namespace STREAM {

OutByteArray::OutByteArray(const char* bytes, unsigned int length):m_bytes(bytes),m_length(length){}


long OutByteArray::getPosition(){
	return m_pos;
}

void OutByteArray::setPosition(long position){
	m_pos = position;
}

void OutByteArray::goEnd(int i){
	m_pos = m_length-1;
}

void OutByteArray::writeChar(char c){
	write((char *)&c, sizeof(char));
}

void OutByteArray::writeInt(int i){
	write((char *)&i, sizeof(int));
}

void OutByteArray::writeLong(long l){
	write((char *)&l, sizeof(long));
}

void OutByteArray::writeLongLong(long long ll){
	write((char *)&ll, sizeof(long long));
}

void OutByteArray::writeShort(short s){
	write((char *)&s, sizeof(short));
}

void OutByteArray::writeFloat(float i){
	write((char *)&i, sizeof(float));
}

void OutByteArray::writeFloats(const std::vector<float>& floats){
	write((char *)&floats[0], sizeof(float)*floats.size());
}

void OutByteArray::writeDouble(double d){
	write((char *)&d, sizeof(double));
}

void OutByteArray::operator<<(const std::string& string){
	memcpy((void*)&m_bytes[m_pos], (void*) string[0] , string.size());
	m_pos += string.size();
}

void OutByteArray::writeString(const std::string& string){
	memcpy((void*)&m_bytes[m_pos], (void*) string[0] , string.size());
	m_pos += string.size();
}

void OutByteArray::write(char* data, unsigned int size){
	memcpy((void*)&m_bytes[m_pos], (void*) data , size);
	m_pos += size;
}


void OutByteArray::skip(unsigned int skip){
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
void OutByteArray::seek(unsigned int position){
	m_pos = position;
}
///Get the current position
unsigned int OutByteArray::pos(){
	return m_pos;
}
///Set the position to zero
void OutByteArray::rewind(){
	seek(0);
}

bool OutByteArray::isOpen() const{
	return true;
}

unsigned int OutByteArray::size(){
	return m_length;
}

void OutByteArray::close(){
	m_bytes = nullptr;
	m_length = 0;
	m_pos = 0;
}

OutByteArray::~OutByteArray() {
	close();
}

} /* namespace STREAM */
} /* namespace PG */
