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
#include <pg/util/PG_BinaryFileTokenizer.h>
#include <pg/util/PG_Base.h>
#include <string>

namespace PG {
namespace UTIL {
BinaryFileTokenizer::BinaryFileTokenizer(std::string path) {
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

unsigned char BinaryFileTokenizer::getUnsignedChar(){
	char c[1];
	m_inFile.read( c, 1 );
	return c[0];
	//return m_inFile.get();
}

char BinaryFileTokenizer::getChar(){
	char c[1];
	m_inFile.read( c, 1 );
	return c[0];
	//return m_inFile.get();
}

unsigned short BinaryFileTokenizer::getUnsignedShort(){
	short s;
	m_inFile.read( (char*)&s, sizeof(short)  );
	return s;
}

short BinaryFileTokenizer::getShort(){
	unsigned short us;
	m_inFile.read( (char*)&us, sizeof(unsigned short) );
	return us;
}

unsigned int BinaryFileTokenizer::getUnsignedInt(){
	unsigned int ui;
	m_inFile.read( (char*)&ui, sizeof(unsigned int));
	return ui;
}

int BinaryFileTokenizer::getInt(){
	int i;
	m_inFile.read( (char*)&i, sizeof(int) );
	return i;
}

unsigned long BinaryFileTokenizer::getUnsignedLong(){
	unsigned long ul;
	m_inFile.read( (char*)&ul, sizeof(unsigned long)  );
	return ul;
}

long BinaryFileTokenizer::getLong(){
	long l;
	m_inFile.read( (char*)&l, sizeof(long)  );
	return l;
}

unsigned long long BinaryFileTokenizer::getUnsignedLongLong(){
	unsigned long long ull;
	m_inFile.read( (char*)&ull, sizeof(unsigned long long)  );
	return  ull;
}

long long BinaryFileTokenizer::getLongLong(){
	long long ll;
	m_inFile.read( (char*)&ll, sizeof(long long)  );
	return ll;
}

float BinaryFileTokenizer::getFloat(){
	float f;
	m_inFile.read( (char*)&f, sizeof(float) );
	return f;
}

double BinaryFileTokenizer::getDouble(){
	double d;
	m_inFile.read( (char*)&d, sizeof(double) );
	return d;
}

std::string BinaryFileTokenizer::getString(unsigned int bytes){
	std::string str;
	str.resize(bytes);
	m_inFile.read( &str[0], bytes );
	return str;
}

void BinaryFileTokenizer::getChars(std::vector<char>& inchars, unsigned int count){
	if((getPos() + count) >  m_size){
		count = m_size - getPos();
	}
	inchars.resize(count);
	m_inFile.read( &inchars[0], count );
}

void BinaryFileTokenizer::getChars(std::vector<unsigned char>& inchars, unsigned int count){
	if((getPos() + count) >  m_size){
		count = m_size - getPos();
	}
	inchars.resize(count);
	//char* chars;
	m_inFile.read( (char*)&inchars[0], count );
	//delete[] chars;
	//free(chars);
}

void BinaryFileTokenizer::get(char* data, unsigned int size){
	if((getPos() + size) >  m_size){
		size = m_size - getPos();
	}
	m_inFile.read( data, size );
}

void BinaryFileTokenizer::read(char* __s, int __n){
	m_inFile.read(  __s, __n );
}

unsigned int BinaryFileTokenizer::getSize() const{
	return m_size;
}

bool BinaryFileTokenizer::isopen(){
	return m_inFile.is_open();
}

void BinaryFileTokenizer::setPos(int newPos){
	m_inFile.seekg(newPos, std::ios::beg);
}

unsigned int BinaryFileTokenizer::getPos(){
	return m_inFile.tellg();
}

void BinaryFileTokenizer::skip(unsigned int bytes){
	m_inFile.ignore(bytes);
}

void BinaryFileTokenizer::rewind(){
	m_inFile.seekg(0, std::ios::beg);
}

void BinaryFileTokenizer::goBack(){
	m_inFile.unget();
}

bool BinaryFileTokenizer::isEmpty(){
	return m_inFile.peek() == std::ifstream::traits_type::eof();
}

bool BinaryFileTokenizer::eof(){
	return m_inFile.eof();
}

void BinaryFileTokenizer::close(){
	m_inFile.close();
}

BinaryFileTokenizer::~BinaryFileTokenizer() {
	close();
}
} /* namespace UTIL */
} /* namespace PG */
