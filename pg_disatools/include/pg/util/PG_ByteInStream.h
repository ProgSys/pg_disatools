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
#ifndef INCLUDE_PG_UTIL_PG_BYTEINSTREAM_H_
#define INCLUDE_PG_UTIL_PG_BYTEINSTREAM_H_

#include <pg/util/PG_ApiUtil.h>
#include <pg/util/PG_InStream.h>
#include <string>
namespace PG {
namespace UTIL {

/*!
 * @brief Allows you to read different values form a byte array.
 * @see OutByteInStream
 */
class ByteInStream: public InStream {
public:
	ByteInStream(const char* bytes, unsigned int length);

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

	std::string readString(unsigned int length) final;

	void read(char* data, unsigned int length) final;

	void skip(unsigned int skip) final;


	void seek(unsigned int position) final;
	unsigned int pos() final;
	void rewind() final;

	bool eof() const final;

	unsigned int size() const final;

	virtual ~ByteInStream();
private:
	const char* m_bytes;
	unsigned int m_length;
	unsigned int m_pos;
};

} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_UTIL_PG_BYTEINSTREAM_H_ */
