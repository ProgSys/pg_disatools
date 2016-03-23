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
#ifndef INCLUDE_PG_UTIL_PG_StreamInOutByteFile_H_
#define INCLUDE_PG_UTIL_PG_StreamInOutByteFile_H_

#include <fstream>
#include <string>
#include <vector>
#include <pg/util/PG_File.h>
#include <pg/Stream/PG_StreamInOut.h>

namespace PG {
namespace STREAM {

/*!
 * \brief Allows you to read and write a binary file.
 */
class InOutByteFile: public InOut {
public:
	InOutByteFile(const PG::UTIL::File& path);
	InOutByteFile(const std::string& path);

	char readChar() final;
	unsigned char readUnsignedChar() final;

	short readShort() final;
	unsigned short readUnsignedShort() final;

	int readInt() final;
	unsigned int readUnsignedInt() final;

	long readLong() final;
	unsigned long readUnsignedLong() final;

	long long readLongLong() final;
	unsigned long long readUnsignedLongLong() final;

	float readFloat() final;
	double readDouble() final;

	std::string readString(unsigned int size) final;

	void read(char* data, unsigned int bytes);

	unsigned int pos() final;
	void skip(unsigned int skip) final;
	void seek(unsigned int position) final;
	void rewind() final;
	void goEnd();

	bool eof() const final;
	unsigned int size() final;


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

	void writeDouble(double d);

	void operator<<(const std::string& string);
	void writeString(const std::string& string);

	void write(char* data, unsigned int bytes);


	/*!
	 * \brief Do not forget to close it when you done!
	 */
	void close() final;
	virtual ~InOutByteFile();
private:
	std::fstream m_file;
	unsigned int m_size;
};

} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_UTIL_PG_StreamInOutByteFile_H_ */
