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

#ifndef INCLUDE_PG_UTIL_PG_BINARYFILEWRITER_H_
#define INCLUDE_PG_UTIL_PG_BINARYFILEWRITER_H_

#include <fstream>
#include <string>
#include <vector>

namespace PG {
namespace UTIL {

/*!
 * \brief Allows you to write a binary file.
 */
class BinaryFileWriter {
public:
	 BinaryFileWriter(std::string path, bool append = false);

	 long getPosition();
	 void setPosition(long  position);
	 void goEnd(int i = 0);
	/*!
	 * \brief Write a char into the file.
	 */
	 void writeChar(char c);
	 void writeShort(short s);
	 void writeInt(int i);
	 void writeLong(long l);
	 void writeLongLong(long long ll);

	 void writeFloat(float i);
	 void writeFloats(const std::vector<float>& floats);
	 void operator<<(const std::string& string);
	 void writeString(const std::string& string);

	 void write(char* data, unsigned int size);

	/*!
	 * \brief Do not forget to close it when you done!
	 */
	 void close();
	 virtual ~BinaryFileWriter();
private:
	std::ofstream m_outFile;
};

} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_UTIL_PG_BINARYFILEWRITER_H_ */
