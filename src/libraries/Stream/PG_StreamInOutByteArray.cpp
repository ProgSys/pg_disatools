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
#include <Stream/PG_StreamInOutByteArray.h>
#include <Util/PG_Base.h>
#include <Util/PG_Exception.h>
#include <cstring>
#include <string.h>

namespace PG {
namespace STREAM {



InOutByteArray::InOutByteArray(char* bytes, unsigned int length): m_bytes(bytes), m_length(length) {}


unsigned int InOutByteArray::pos(){
	return m_pos;
}

void InOutByteArray::seek(unsigned int position){
	m_pos = position;
}

void InOutByteArray::rewind(){
	m_pos = 0;
}

void InOutByteArray::goEnd(){
	m_pos = m_length-1;
}



char InOutByteArray::readChar(){
	char c[1];
	read( c, 1 );
	return c[0];
}
unsigned char InOutByteArray::readUnsignedChar(){
	char c[1];
	read( c, 1 );
	return c[0];
}
short InOutByteArray::readShort(){
	short s;
	read( (char*)&s, sizeof(short)  );
	return s;
}
unsigned short InOutByteArray::readUnsignedShort(){
	unsigned short s;
	read( (char*)&s, sizeof(short)  );
	return s;
}
int InOutByteArray::readInt(){
	int i;
	read( (char*)&i, sizeof(int)  );
	return i;
}
unsigned int InOutByteArray::readUnsignedInt(){
	unsigned int i;
	read( (char*)&i, sizeof(int)  );
	return i;
}

long InOutByteArray::readLong(){
	long l;
	read( (char*)&l, sizeof(long)  );
	return l;
}

unsigned long InOutByteArray::readUnsignedLong(){
	unsigned long l;
	read( (char*)&l, sizeof(unsigned long)  );
	return l;
}

long long InOutByteArray::readLongLong(){
	long long l;
	read( (char*)&l, sizeof(long long)  );
	return l;
}

unsigned long long InOutByteArray::readUnsignedLongLong(){
	unsigned long long l;
	read( (char*)&l, sizeof(unsigned long long)  );
	return l;
}

float InOutByteArray::readFloat(){
	float f;
	read( (char*)&f, sizeof(float)  );
	return f;
}
double InOutByteArray::readDouble(){
	double d;
	read( (char*)&d, sizeof(float)  );
	return d;
}

bool InOutByteArray::eof() const{
	return m_pos >= m_length;
}

unsigned int InOutByteArray::size(){
	return m_length;
}



std::string InOutByteArray::readString(unsigned int size){
	std::string str;
	str.resize(size);
	read( &str[0], size );
	return str;
}

void InOutByteArray::skip(unsigned int bytes){
	m_pos+=bytes;
}

void InOutByteArray::read(char* data, unsigned int bytes){
	memcpy((void*) data, (void*) &m_bytes[m_pos], bytes);
	m_pos += bytes;
}


void InOutByteArray::writeChar(char c){
	write((char *)&c, sizeof(char));
}

void InOutByteArray::writeInt(int i){
	write((char *)&i, sizeof(int));
}

void InOutByteArray::writeLong(long l){
	write((char *)&l, sizeof(long));
}

void InOutByteArray::writeLongLong(long long ll){
	write((char *)&ll, sizeof(long long));
}

void InOutByteArray::writeShort(short s){
	write((char *)&s, sizeof(short));
}

void InOutByteArray::writeFloat(float i){
	write((char *)&i, sizeof(float));
}

void InOutByteArray::writeFloats(const std::vector<float>& floats){
	write((char *)&floats[0], sizeof(float)*floats.size());
}

void InOutByteArray::writeDouble(double d){
	write((char *)&d, sizeof(double));
}


void InOutByteArray::operator<<(const std::string& string){
	memcpy((void*)&m_bytes[m_pos], (void*) &string[0] , string.size());
	m_pos += string.size();
}

void InOutByteArray::writeString(const std::string& string){
	memcpy((void*)&m_bytes[m_pos], (void*) &string[0] , string.size());
	m_pos += string.size();
}

void InOutByteArray::write(char* data, unsigned int size){
	memcpy((void*)&m_bytes[m_pos], (void*) data , size);
	m_pos += size;
}

void InOutByteArray::close(){
	m_bytes = nullptr;
	m_length = 0;
	m_pos = 0;
}

InOutByteArray::~InOutByteArray() {
	close();
}

} /* namespace STREAM */
} /* namespace PG */
