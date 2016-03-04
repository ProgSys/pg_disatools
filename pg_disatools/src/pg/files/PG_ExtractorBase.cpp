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

#include <pg/files/PG_ExtractorBase.h>

namespace PG {
namespace FILE {

fileInfo::fileInfo(){}
fileInfo::fileInfo(const std::string& _name,unsigned int _size,unsigned int _offset):
		name(_name),size(_size),offset(_offset){}
fileInfo::fileInfo(const PG::UTIL::File& _name,unsigned int _size,unsigned int _offset):
		name(_name),size(_size),offset(_offset){}

ExtractorBase::ExtractorBase() {
	// TODO Auto-generated constructor stub

}

bool ExtractorBase::isChanged() const{
	return m_changed;
}

const fileInfo& ExtractorBase::operator[](unsigned int index) const{
	return get(index);
}
fileInfo* ExtractorBase::getDataPointer(unsigned int index) const{
	return const_cast<fileInfo*>(&get(index));
}

ExtractorBase::~ExtractorBase() {
}

} /* namespace FILE */
} /* namespace PG */
