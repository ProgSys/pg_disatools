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


#include <pg/STREAM/PG_StreamInByteArray.h>
#include  <cstring>

namespace PG {
namespace STREAM {

InByteArray::InByteArray(const char* bytes, unsigned int length): m_bytes(bytes),m_length(length), m_pos(0)  {

}

char InByteArray::readChar(){
	char c;
	read( &c, sizeof(char));
	return c;
}

unsigned char InByteArray::readUnsignedChar(){
	unsigned char c;
	read( (char*)&c, sizeof(unsigned char));
	return c;
}

short InByteArray::readShort(){
	short s;
	read( (char*)&s, sizeof(short));
	return s;
}

unsigned short InByteArray::readUnsignedShort(){
	unsigned short s;
	read( (char*)&s, sizeof(unsigned short));
	return s;
}

int InByteArray::readInt(){
	int i;
	read( (char*)&i, sizeof(int));
	return i;
}

unsigned int InByteArray::readUnsignedInt(){
	unsigned int i;
	read( (char*)&i, sizeof(unsigned int));
	return i;
}

long InByteArray::readLong(){
	long l;
	read( (char*)&l, sizeof(long));
	return l;
}
unsigned long InByteArray::readUnsignedLong(){
	unsigned long l;
	read( (char*)&l, sizeof(unsigned long));
	return l;
}

long long InByteArray::readLongLong(){
	long long ll;
	read( (char*)&ll, sizeof(long long));
	return ll;
}
unsigned long long InByteArray::readUnsignedLongLong(){
	unsigned long long ll;
	read( (char*)&ll, sizeof(unsigned long long));
	return ll;
}

float InByteArray::readFloat(){
	float f;
	read( (char*)&f, sizeof(float));
	return f;
}

double InByteArray::readDouble(){
	double d;
	read( (char*)&d, sizeof(double));
	return d;
}

std::string InByteArray::readString(unsigned int length){
	std::string str;
	str.resize(length);
	read( &str[0], length );
	return str;
}

void InByteArray::read(char* data, unsigned int length){
	memcpy((void*) data, (void*) &m_bytes[m_pos], length);
	m_pos += length;
}

void InByteArray::skip(unsigned int skip){
	m_pos+=skip;
}

void InByteArray::seek(unsigned int position){
	m_pos = position;
}

unsigned int InByteArray::pos(){
	return m_pos;
}

void InByteArray::rewind(){
	m_pos = 0;
}

bool InByteArray::eof() const{
	return  m_pos >= m_length;
}

unsigned int InByteArray::size(){
	return m_length;
}

InByteArray::~InByteArray() {
	// TODO Auto-generated destructor stub
}

} /* namespace STREAM */
} /* namespace PG */
