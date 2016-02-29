/*
 * Wrapper class around std::ofstream.
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
