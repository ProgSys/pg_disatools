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
#include <pg/util/PG_BinaryFileWriter.h>
#include <pg/util/PG_Base.h>
#include <pg/util/PG_Exception.h>

namespace PG {
namespace UTIL {

BinaryFileWriter::BinaryFileWriter(std::string path, bool append) {
	if(path.empty()){
		PG_ERROR_STREAM("File path empty!");
		throw_Exception("Could not create file! File path empty!");
		return;
	}

	if(append)
		m_outFile.open(path.c_str(),std::ios::binary | ios::app);
	else
		m_outFile.open(path.c_str(),std::ios::binary);

	if (!m_outFile.is_open()){
		PG_ERROR_STREAM("Could not create file: "<<path);
		throw_Exception("Could not create file.");
		return;
	}

}

long BinaryFileWriter::getPosition(){
	return m_outFile.tellp();
}

void BinaryFileWriter::setPosition(long position){
	m_outFile.seekp(position,ios_base::beg);
}

void BinaryFileWriter::goEnd(int i){
	m_outFile.seekp(i,ios_base::end);
}

void BinaryFileWriter::writeChar(char c){
	m_outFile.write((char *)&c, sizeof(char));
}

void BinaryFileWriter::writeInt(int i){
	m_outFile.write((char *)&i, sizeof(int));
}

void BinaryFileWriter::writeLong(long l){
	m_outFile.write((char *)&l, sizeof(long));
}

void BinaryFileWriter::writeLongLong(long long ll){
	m_outFile.write((char *)&ll, sizeof(long long));
}

void BinaryFileWriter::writeShort(short s){
	m_outFile.write((char *)&s, sizeof(short));
}

void BinaryFileWriter::writeFloat(float i){
	m_outFile.write((char *)&i, sizeof(float));
}

void BinaryFileWriter::writeFloats(const std::vector<float>& floats){
	m_outFile.write((char *)&floats[0], sizeof(float)*floats.size());
}

void BinaryFileWriter::operator<<(const std::string& string){
	m_outFile<<string;
}

void BinaryFileWriter::writeString(const std::string& string){
	m_outFile<<string;
}

void BinaryFileWriter::write(char* data, unsigned int size){
	m_outFile.write(data, size);
}

void BinaryFileWriter::close(){
	m_outFile.close();
}

BinaryFileWriter::~BinaryFileWriter() {
	close();
}

} /* namespace UTIL */
} /* namespace PG */
