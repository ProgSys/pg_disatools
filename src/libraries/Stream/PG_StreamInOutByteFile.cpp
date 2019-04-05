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
#include <Stream/PG_StreamInOutByteFile.h>
#include <Util/PG_Base.h>
#include <Util/PG_Exception.h>

namespace PG {
namespace STREAM {



InOutByteFile::InOutByteFile(const std::string& path) {
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

InOutByteFile::InOutByteFile(const PG::UTIL::File& path): InOutByteFile(path.getPath()){

}

unsigned int InOutByteFile::pos(){
	return m_file.tellp();
}

void InOutByteFile::seek(unsigned int position){
	m_file.seekp(position,ios_base::beg);
}

void InOutByteFile::rewind(){
	m_file.seekp(0,ios_base::beg);
}

void InOutByteFile::goEnd(){
	m_file.seekp(0,ios_base::end);
}



char InOutByteFile::readChar(){
	char c[1];
	m_file.read( c, 1 );
	return c[0];
}
unsigned char InOutByteFile::readUnsignedChar(){
	char c[1];
	m_file.read( c, 1 );
	return c[0];
}
short InOutByteFile::readShort(){
	short s;
	m_file.read( (char*)&s, sizeof(short)  );
	return s;
}
unsigned short InOutByteFile::readUnsignedShort(){
	unsigned short s;
	m_file.read( (char*)&s, sizeof(short)  );
	return s;
}
int InOutByteFile::readInt(){
	int i;
	m_file.read( (char*)&i, sizeof(int)  );
	return i;
}
unsigned int InOutByteFile::readUnsignedInt(){
	unsigned int i;
	m_file.read( (char*)&i, sizeof(int)  );
	return i;
}

long InOutByteFile::readLong(){
	long l;
	m_file.read( (char*)&l, sizeof(long)  );
	return l;
}

unsigned long InOutByteFile::readUnsignedLong(){
	unsigned long l;
	m_file.read( (char*)&l, sizeof(unsigned long)  );
	return l;
}

long long InOutByteFile::readLongLong(){
	long long l;
	m_file.read( (char*)&l, sizeof(long long)  );
	return l;
}

unsigned long long InOutByteFile::readUnsignedLongLong(){
	unsigned long long l;
	m_file.read( (char*)&l, sizeof(unsigned long long)  );
	return l;
}

float InOutByteFile::readFloat(){
	float f;
	m_file.read( (char*)&f, sizeof(float)  );
	return f;
}
double InOutByteFile::readDouble(){
	double d;
	m_file.read( (char*)&d, sizeof(float)  );
	return d;
}

bool InOutByteFile::eof() const{
	return m_file.eof();
}

unsigned int InOutByteFile::size(){
	return m_size;
}



std::string InOutByteFile::readString(unsigned int size){
	std::string str;
	str.resize(size);
	m_file.read( &str[0], size );
	return str;
}

void InOutByteFile::skip(unsigned int bytes){
	m_file.ignore(bytes);
}

void InOutByteFile::read(char* data, unsigned int bytes){
	m_file.read( data, bytes);
}


void InOutByteFile::writeChar(char c){
	m_file.write((char *)&c, sizeof(char));
}

void InOutByteFile::writeInt(int i){
	m_file.write((char *)&i, sizeof(int));
}

void InOutByteFile::writeLong(long l){
	m_file.write((char *)&l, sizeof(long));
}

void InOutByteFile::writeLongLong(long long ll){
	m_file.write((char *)&ll, sizeof(long long));
}

void InOutByteFile::writeShort(short s){
	m_file.write((char *)&s, sizeof(short));
}

void InOutByteFile::writeFloat(float i){
	m_file.write((char *)&i, sizeof(float));
}

void InOutByteFile::writeFloats(const std::vector<float>& floats){
	m_file.write((char *)&floats[0], sizeof(float)*floats.size());
}

void InOutByteFile::writeDouble(double d){
	m_file.write((char *)&d, sizeof(double));
}


void InOutByteFile::operator<<(const std::string& string){
	m_file<<string;
}

void InOutByteFile::writeString(const std::string& string){
	m_file<<string;
}

void InOutByteFile::write(char* data, unsigned int size){
	m_file.write(data, size);
}

void InOutByteFile::close(){
	m_file.close();
}

InOutByteFile::~InOutByteFile() {
	close();
}

} /* namespace STREAM */
} /* namespace PG */
