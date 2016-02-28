/*
 * PG_ByteStream.cpp
 *
 *  Created on: 28.02.2016
 *      Author: ProgSys
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

void ByteStream::read(char* data, unsigned int length){
	memcpy((void*) data, (void*) &m_bytes[m_pos], length);
	m_pos += length;
}

void ByteStream::seek(unsigned int position){
	m_pos = position;
}

ByteStream::~ByteStream() {
	// TODO Auto-generated destructor stub
}

} /* namespace UTIL */
} /* namespace PG */
