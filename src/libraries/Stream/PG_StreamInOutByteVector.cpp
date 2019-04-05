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
#include <Stream/PG_StreamInOutByteVector.h>
#include <Util/PG_Base.h>
#include <Util/PG_Exception.h>
#include <cstring>
#include <string.h>

namespace PG {
namespace STREAM {



InOutByteVector::InOutByteVector(unsigned int buffer): m_data(buffer){}


unsigned int InOutByteVector::pos(){
	return m_pos;
}

void InOutByteVector::seek(unsigned int position){
	m_pos = position;
	if(m_pos >= m_data.size() )
		m_data.resize(m_pos);
}

void InOutByteVector::rewind(){
	m_pos = 0;
}

void InOutByteVector::goEnd(){
	m_pos = m_data.size()-1;
}



char InOutByteVector::readChar(){
	char c[1];
	read( c, 1 );
	return c[0];
}
unsigned char InOutByteVector::readUnsignedChar(){
	char c[1];
	read( c, 1 );
	return c[0];
}
short InOutByteVector::readShort(){
	short s;
	read( (char*)&s, sizeof(short)  );
	return s;
}
unsigned short InOutByteVector::readUnsignedShort(){
	unsigned short s;
	read( (char*)&s, sizeof(short)  );
	return s;
}
int InOutByteVector::readInt(){
	int i;
	read( (char*)&i, sizeof(int)  );
	return i;
}
unsigned int InOutByteVector::readUnsignedInt(){
	unsigned int i;
	read( (char*)&i, sizeof(int)  );
	return i;
}

long InOutByteVector::readLong(){
	long l;
	read( (char*)&l, sizeof(long)  );
	return l;
}

unsigned long InOutByteVector::readUnsignedLong(){
	unsigned long l;
	read( (char*)&l, sizeof(unsigned long)  );
	return l;
}

long long InOutByteVector::readLongLong(){
	long long l;
	read( (char*)&l, sizeof(long long)  );
	return l;
}

unsigned long long InOutByteVector::readUnsignedLongLong(){
	unsigned long long l;
	read( (char*)&l, sizeof(unsigned long long)  );
	return l;
}

float InOutByteVector::readFloat(){
	float f;
	read( (char*)&f, sizeof(float)  );
	return f;
}
double InOutByteVector::readDouble(){
	double d;
	read( (char*)&d, sizeof(float)  );
	return d;
}

bool InOutByteVector::eof() const{
	return m_pos >= m_data.size()-1;
}

unsigned int InOutByteVector::size(){
	return m_data.size();
}



std::string InOutByteVector::readString(unsigned int size){
	std::string str;
	str.resize(size);
	read( &str[0], size );
	return str;
}

void InOutByteVector::skip(unsigned int bytes){
	seek(m_pos+bytes);
}

void InOutByteVector::read(char* data, unsigned int bytes){
	unsigned int targetReadSize = bytes;
	if(m_pos+targetReadSize >= m_data.size())
		targetReadSize = m_data.size()-m_pos;
	memcpy((void*) data, (void*) &m_data[m_pos], bytes);
	m_pos += bytes;
}


void InOutByteVector::writeChar(char c){
	write((char *)&c, sizeof(char));
}

void InOutByteVector::writeInt(int i){
	write((char *)&i, sizeof(int));
}

void InOutByteVector::writeLong(long l){
	write((char *)&l, sizeof(long));
}

void InOutByteVector::writeLongLong(long long ll){
	write((char *)&ll, sizeof(long long));
}

void InOutByteVector::writeShort(short s){
	write((char *)&s, sizeof(short));
}

void InOutByteVector::writeFloat(float i){
	write((char *)&i, sizeof(float));
}

void InOutByteVector::writeFloats(const std::vector<float>& floats){
	write((char *)&floats[0], sizeof(float)*floats.size());
}

void InOutByteVector::writeDouble(double d){
	write((char *)&d, sizeof(double));
}


void InOutByteVector::operator<<(const std::string& string){
	writeString(string);
}

void InOutByteVector::writeString(const std::string& string){
	write((char*)&string[0],string.size());
}

void InOutByteVector::write(char* data, unsigned int size){
	if(m_pos+size >= m_data.size())
		m_data.reserve(m_pos+size );
	memcpy((void*)&m_data[m_pos], (void*) data , size);
	m_pos += size;
}

void InOutByteVector::close(){
	m_data.clear();
	m_pos = 0;
}

InOutByteVector::~InOutByteVector() {
	close();
}

} /* namespace STREAM */
} /* namespace PG */
