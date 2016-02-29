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
#ifndef SRC_PG_ByteInFileStream_H_
#define SRC_PG_ByteInFileStream_H_

#include <iostream>
#include <fstream>
#include <cstring>
#include <cstddef>
#include <vector>

#include <pg/util/PG_File.h>
#include <pg/util/PG_InStream.h>

namespace PG {
namespace UTIL {
/*!
 * A wrapper around std::ifstream to read a file byte by byte.
 */
class ByteInFileStream: public InStream  {
public:
	ByteInFileStream(const std::string& path);
	ByteInFileStream(const File& file);
	/*!
	 * Read one byte, as a unsigned char (+BYTE).
	 */
	unsigned char readUnsignedChar() final;
	/*!
	 * Read one byte, as a char (+/-BYTE).
	 */
	char readChar() final;
	/*!
	 * Read two bytes, as a unsigned short (+WORD).
	 */
	unsigned short readUnsignedShort() final;
	/*!
	 * Read two bytes, as a short (+/-WORD).
	 */
	short readShort() final;
	/*!
	 * Read four bytes, as a unsigned int (+Long WORD).
	 */
	unsigned int readUnsignedInt() final;
	/*!
	 * Read four bytes, as a int (+/-Long WORD).
	 */
	int readInt() final;

	/*!
	 * Read four bytes, as a unsigned long (+Long WORD 32bit).
	 */
	unsigned long readUnsignedLong() final;
	/*!
	 * Read four bytes, as a long (+/-Long WORD 32bit).
	 */
	long readLong() final;

	/*!
	 * Read eight bytes, as a unsigned long (+Quad WORD 64bit).
	 */
	unsigned long long readUnsignedLongLong() final;
	/*!
	 * Read eight bytes, as a long (+/-Quad WORD 64bit).
	 */
	long long readLongLong() final;

	/*!
	 * Read four bytes, as a float (+/-Long WORD).
	 */
	 float readFloat() final;
	/*!
	 * Read eight bytes, as a double (+/-IEEE).
	 */
	 double readDouble() final;
	/*!
	 * Read a string with a defined byte length.
	 * \param bytes Length of the string to read in bytes.
	 */
	 std::string readString(unsigned int bytes) final;
	/*!
	 * Read a chars into a std::vector<char>
	 * \param inchars the vector in witch should be written
	 * \param count the number of bytes to read
	 */
	 void readChars(std::vector<char>& inchars, unsigned int count);
	/*!
	 * Read a chars into a std::vector<unsigned char>
	 * \param inchars the vector in witch should be written
	 * \param count the number of bytes to read
	 */
	 void readChars(std::vector<unsigned char>& inchars, unsigned int count);

	/*!
	 * Is the file open?
	 */
	 bool isopen();
	/*!
	 * Get the number of bytes.
	 */
	 unsigned int size() const final;

	 void read(char* data, unsigned int size) final;
	/*!
	 * Set the current position of the reader.
	 */
	 void seek(unsigned int newPos) final;
	/*!
	 * Get the current position of the reader.
	 */
	 unsigned int pos() final;
	/*!
	 * Skip a number of bytes.
	 */
	 void skip(unsigned int bytes) final;
	/*!
	 * Reset the reader to the start.
	 */
	 void rewind() final;
	/*!
	 * Go one byte back.
	 */
	 void goBack();
	/*!
	 * The next position is end of line.
	 */
	 bool isEmpty();
	/*!
	 * The end of line?
	 */
	 bool eof() const final;
	/*!
	 * Don't forget to close the reader when you done!
	 */
	 void close() final;



	 virtual ~ByteInFileStream();
private:
	std::ifstream m_inFile;
	unsigned int m_size;
};
} /* namespace UTIL */
} /* namespace PG */

#endif /* SRC_PG_ByteInFileStream_H_ */
