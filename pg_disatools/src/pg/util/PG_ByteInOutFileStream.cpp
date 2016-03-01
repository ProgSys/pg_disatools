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
#include <pg/util/PG_ByteInOutFileStream.h>
#include <pg/util/PG_Base.h>
#include <pg/util/PG_Exception.h>

namespace PG {
namespace UTIL {



ByteInOutFileStream::ByteInOutFileStream(const std::string& path) {
	if(path.empty()){
		PG_ERROR_STREAM("File path empty!");
		throw_Exception("Could not create file! File path empty!");
		return;
	}

	m_file.open(path.c_str(),std::ios::in | std::ios::out | std::ios::binary);

	if (!m_file.is_open()){
		PG_ERROR_STREAM("Could not create file: "<<path);
		throw_Exception("Could not create file.");
		return;
	}

	goEnd();
	m_size = pos();
	rewind();
}

ByteInOutFileStream::ByteInOutFileStream(const PG::UTIL::File& path): ByteInOutFileStream(path.getPath()){

}

unsigned int ByteInOutFileStream::pos(){
	return m_file.tellp();
}

void ByteInOutFileStream::seek(unsigned int position){
	m_file.seekp(position,ios_base::beg);
}

void ByteInOutFileStream::rewind(){
	m_file.seekp(0,ios_base::beg);
}

void ByteInOutFileStream::goEnd(){
	m_file.seekp(0,ios_base::end);
}



char ByteInOutFileStream::readChar(){
	char c[1];
	m_file.read( c, 1 );
	return c[0];
}
unsigned char ByteInOutFileStream::readUnsignedChar(){
	char c[1];
	m_file.read( c, 1 );
	return c[0];
}
short ByteInOutFileStream::readShort(){
	short s;
	m_file.read( (char*)&s, sizeof(short)  );
	return s;
}
unsigned short ByteInOutFileStream::readUnsignedShort(){
	unsigned short s;
	m_file.read( (char*)&s, sizeof(short)  );
	return s;
}
int ByteInOutFileStream::readInt(){
	int i;
	m_file.read( (char*)&i, sizeof(int)  );
	return i;
}
unsigned int ByteInOutFileStream::readUnsignedInt(){
	unsigned int i;
	m_file.read( (char*)&i, sizeof(int)  );
	return i;
}

long ByteInOutFileStream::readLong(){
	long l;
	m_file.read( (char*)&l, sizeof(long)  );
	return l;
}

unsigned long ByteInOutFileStream::readUnsignedLong(){
	unsigned long l;
	m_file.read( (char*)&l, sizeof(unsigned long)  );
	return l;
}

long long ByteInOutFileStream::readLongLong(){
	long long l;
	m_file.read( (char*)&l, sizeof(long long)  );
	return l;
}

unsigned long long ByteInOutFileStream::readUnsignedLongLong(){
	unsigned long long l;
	m_file.read( (char*)&l, sizeof(unsigned long long)  );
	return l;
}

float ByteInOutFileStream::readFloat(){
	float f;
	m_file.read( (char*)&f, sizeof(float)  );
	return f;
}
double ByteInOutFileStream::readDouble(){
	double d;
	m_file.read( (char*)&d, sizeof(float)  );
	return d;
}

bool ByteInOutFileStream::eof() const{
	return m_file.eof();
}

unsigned int ByteInOutFileStream::size() const{
	return m_size;
}



std::string ByteInOutFileStream::readString(unsigned int size){
	std::string str;
	str.resize(size);
	m_file.read( &str[0], size );
	return str;
}

void ByteInOutFileStream::skip(unsigned int bytes){
	m_file.ignore(bytes);
}

void ByteInOutFileStream::read(char* data, unsigned int bytes){
	m_file.read( data, bytes);
}


void ByteInOutFileStream::writeChar(char c){
	m_file.write((char *)&c, sizeof(char));
}

void ByteInOutFileStream::writeInt(int i){
	m_file.write((char *)&i, sizeof(int));
}

void ByteInOutFileStream::writeLong(long l){
	m_file.write((char *)&l, sizeof(long));
}

void ByteInOutFileStream::writeLongLong(long long ll){
	m_file.write((char *)&ll, sizeof(long long));
}

void ByteInOutFileStream::writeShort(short s){
	m_file.write((char *)&s, sizeof(short));
}

void ByteInOutFileStream::writeFloat(float i){
	m_file.write((char *)&i, sizeof(float));
}

void ByteInOutFileStream::writeFloats(const std::vector<float>& floats){
	m_file.write((char *)&floats[0], sizeof(float)*floats.size());
}

void ByteInOutFileStream::operator<<(const std::string& string){
	m_file<<string;
}

void ByteInOutFileStream::writeString(const std::string& string){
	m_file<<string;
}

void ByteInOutFileStream::write(char* data, unsigned int size){
	m_file.write(data, size);
}

void ByteInOutFileStream::close(){
	m_file.close();
}

ByteInOutFileStream::~ByteInOutFileStream() {
	close();
}

} /* namespace UTIL */
} /* namespace PG */
