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
#include <pg/Stream/PG_StreamInByteFile.h>
#include <string>

namespace PG {
namespace STREAM {
InByteFile::InByteFile(const std::string& path) {
	m_size = 0;

	if(path.empty()){
		PG_ERROR_STREAM("File path empty!");
		return;
	}

	//m_inFile.open( path.c_str(), std::ios::in | std::ios::app | std::ios::binary );
	m_inFile.open( path.c_str(), std::ios::in | std::ios::binary );

	if(!m_inFile.is_open()){
		PG_ERROR_STREAM("Can't open file: "<<path<<"!");
		return;
	}

	m_inFile.seekg(0, std::ios::end); // set the pointer to the end
	m_size = m_inFile.tellg() ; // get the length of the file
	//std::cout << "Size of file: " << m_size <<std::endl;
	m_inFile.seekg(0, std::ios::beg); // set the pointer to the beginning

}

InByteFile::InByteFile(const UTIL::File& file): InByteFile(file.getPath())
{}

unsigned char InByteFile::readUnsignedChar(){
	char c[1];
	m_inFile.read( c, 1 );
	return c[0];
	//return m_inFile.get();
}

char InByteFile::readChar(){
	char c[1];
	m_inFile.read( c, 1 );
	return c[0];
	//return m_inFile.get();
}

unsigned short InByteFile::readUnsignedShort(){
	unsigned short s;
	m_inFile.read( (char*)&s, sizeof(unsigned short)  );
	return s;
}

short InByteFile::readShort(){
	short us;
	m_inFile.read( (char*)&us, sizeof(short) );
	return us;
}

unsigned int InByteFile::readUnsignedInt(){
	unsigned int ui;
	m_inFile.read( (char*)&ui, sizeof(unsigned int));
	return ui;
}

int InByteFile::readInt(){
	int i;
	m_inFile.read( (char*)&i, sizeof(int) );
	return i;
}

unsigned long InByteFile::readUnsignedLong(){
	unsigned long ul;
	m_inFile.read( (char*)&ul, sizeof(unsigned long)  );
	return ul;
}

long InByteFile::readLong(){
	long l;
	m_inFile.read( (char*)&l, sizeof(long)  );
	return l;
}

unsigned long long InByteFile::readUnsignedLongLong(){
	unsigned long long ull;
	m_inFile.read( (char*)&ull, sizeof(unsigned long long)  );
	return  ull;
}

long long InByteFile::readLongLong(){
	long long ll;
	m_inFile.read( (char*)&ll, sizeof(long long)  );
	return ll;
}

float InByteFile::readFloat(){
	float f;
	m_inFile.read( (char*)&f, sizeof(float) );
	return f;
}

double InByteFile::readDouble(){
	double d;
	m_inFile.read( (char*)&d, sizeof(double) );
	return d;
}

std::string InByteFile::readString(unsigned int bytes){
	std::string str;
	str.resize(bytes);
	m_inFile.read( &str[0], bytes );
	return str;
}

void InByteFile::readChars(std::vector<char>& inchars, unsigned int count){
	if((pos() + count) >  m_size){
		count = m_size - pos();
	}
	inchars.resize(count);
	m_inFile.read( &inchars[0], count );
}

void InByteFile::readChars(std::vector<unsigned char>& inchars, unsigned int count){
	if((pos() + count) >  m_size){
		count = m_size - pos();
	}
	inchars.resize(count);
	//char* chars;
	m_inFile.read( (char*)&inchars[0], count );
	//delete[] chars;
	//free(chars);
}

void InByteFile::read(char* data, unsigned int size){
	m_inFile.read( data, size );
}

unsigned int InByteFile::size(){
	return m_size;
}

bool InByteFile::isopen(){
	return m_inFile.is_open();
}

void InByteFile::seek(unsigned int newPos){
	m_inFile.seekg(newPos, std::ios::beg);
}

unsigned int InByteFile::pos(){
	return m_inFile.tellg();
}

void InByteFile::skip(unsigned int bytes){
	m_inFile.ignore(bytes);
}

void InByteFile::rewind(){
	m_inFile.seekg(0, std::ios::beg);
}

void InByteFile::goBack(){
	m_inFile.unget();
}

bool InByteFile::isEmpty(){
	return m_inFile.peek() == std::ifstream::traits_type::eof();
}

bool InByteFile::eof() const{
	return m_inFile.eof();
}

void InByteFile::close(){
	m_inFile.close();
}

InByteFile::~InByteFile() {
	close();
}
} /* namespace STREAM */
} /* namespace PG */
