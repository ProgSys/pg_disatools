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

#include <Util/PG_File.h>

#include <sys/stat.h>
//#include <unistd.h>
#include <io.h>
#include <cstdio>
#include <algorithm>
#include <fstream>
#include <Util/PG_Exception.h>

namespace PG {
namespace UTIL {


File::File(const std::string& path): m_path(path){

}


std::string const& File::getPath() const{
	return m_path;
}

std::string File::getFileExtension() const{
	return m_path.substr(m_path.find_last_of(".")+1);
}

std::string  File::getFile() const{
	return m_path.substr(m_path.find_last_of("\\/")+1);
}


std::string File::getName() const{
	const unsigned int start = m_path.find_last_of("\\/")+1;
	const unsigned int end = m_path.find_last_of(".");
	if(end < start){
		return "";
	}else
		return m_path.substr(start, end);
}

File File::toUpper() const{
	std::string path = m_path;
	std::transform(path.begin(), path.end(), path.begin(), ::toupper);
	return File(path);
}

File File::toLower() const{
	std::string path = m_path;
	std::transform(path.begin(), path.end(), path.begin(),  ::tolower);
	return File(path);
}

void File::operator=(const std::string& path){
	m_path = path;
}
void File::operator=(const File& file){
	m_path = file.getPath();
}

bool File::operator==(const std::string& path) const{
	return m_path == path;
}
bool File::operator==(const File& file) const{
	return m_path == file.getPath();
}

bool File::operator<(const File& file) const{
	return m_path < file.getPath();
}

File& File::operator+=(const File& file) {
	m_path += file.getPath();
	return *this;
}

File& File::operator+=(const std::string& str) {
	m_path += str;
	return *this;
}

void File::clear(){
	m_path = "";
}

bool File::isEmpty() const{
	return m_path.empty();
}

void File::set(const std::string& path){
	m_path = path;
}

unsigned int File::size() const{
	if(isEmpty()) return 0;
    struct stat stat_buf;
    int rc = stat(m_path.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

bool File::exists() const{
	  if(isEmpty()) return false;
	  struct stat buffer;
	  return (stat (m_path.c_str(), &buffer) == 0);
}

bool File::create(const std::string& text) const{
	if(isEmpty()) return false;
	std::ofstream outfile (m_path);
	if(!outfile.is_open()){
		outfile.close();
		return false;
	}
	outfile << text;
	outfile.close();
	return true;
}

bool File::rename(const std::string& name) const{
	if(isEmpty() || name.empty()) return false;
	return std::rename( m_path.c_str(), name.c_str() );
}
bool File::remove() const{
	if(isEmpty()) return false;
	return std::remove( m_path.c_str() );
}

File::~File() {
	// TODO Auto-generated destructor stub
}

} /* namespace UTIL */
} /* namespace PG */
