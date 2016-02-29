/*
 * Wrapper class around const char* (bytes).
 *
 *  Copyright (C) 2016  ProgSys
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <pg/util/PG_ByteStream.h>
#include  <cstring>

namespace PG {
namespace UTIL {

ByteStream::ByteStream(const char* bytes, unsigned int length): m_bytes(bytes),m_length(length), m_pos(0)  {

}

char ByteStream::readChar(){
	char c;
	read( &c, sizeof(char));
	return c;
}

char ByteStream::readUnsignedChar(){
	unsigned char c;
	read( (char*)&c, sizeof(unsigned char));
	return c;
}

short ByteStream::readShort(){
	short s;
	read( (char*)&s, sizeof(short));
	return s;
}

unsigned short ByteStream::readUnsignedShort(){
	unsigned short s;
	read( (char*)&s, sizeof(unsigned short));
	return s;
}

int ByteStream::readInt(){
	int i;
	read( (char*)&i, sizeof(int));
	return i;
}

unsigned int ByteStream::readUnsignedInt(){
	unsigned int i;
	read( (char*)&i, sizeof(unsigned int));
	return i;
}

long ByteStream::readLong(){
	long l;
	read( (char*)&l, sizeof(long));
	return l;
}
unsigned long ByteStream::readUnsignedLong(){
	unsigned long l;
	read( (char*)&l, sizeof(unsigned long));
	return l;
}

long long ByteStream::readLongLong(){
	long long ll;
	read( (char*)&ll, sizeof(long long));
	return ll;
}
unsigned long long ByteStream::readUnsignedLongLong(){
	unsigned long long ll;
	read( (char*)&ll, sizeof(unsigned long long));
	return ll;
}

std::string ByteStream::readString(unsigned int length){
	std::string str;
	str.resize(length);
	read( &str[0], length );
	return str;
}

void ByteStream::read(char* data, unsigned int length){
	memcpy((void*) data, (void*) &m_bytes[m_pos], length);
	m_pos += length;
}

unsigned int ByteStream::getSize() const{
	return m_length;
}

void ByteStream::skip(unsigned int skip){
	m_pos+=skip;
}

void ByteStream::seek(unsigned int position){
	m_pos = position;
}

void ByteStream::reset(){
	m_pos = 0;
}

bool ByteStream::eof() const{
	return  m_pos >= m_length;
}

ByteStream::~ByteStream() {
	// TODO Auto-generated destructor stub
}

} /* namespace UTIL */
} /* namespace PG */
