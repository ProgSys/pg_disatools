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

#ifndef INCLUDE_PG_STREAM_PG_STREAMOUTBYTEARRAY_H_
#define INCLUDE_PG_STREAM_PG_STREAMOUTBYTEARRAY_H_

#include <fstream>
#include <string>
#include <vector>
#include <pg/Stream/PG_StreamOut.h>
#include <pg/util/PG_File.h>

namespace PG {
namespace STREAM {

/*!
 * \brief Allows you to write a binary array.
 */
class OutByteArray: public Out {
public:
	OutByteArray(const char* bytes, unsigned int length);

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
	void writeDouble(double d);

	void operator<<(const std::string& string);
	void writeString(const std::string& string);

	void write(char* data, unsigned int size);


	///Skip the given number of bytes, if at end write zero
	void skip(unsigned int skip);
	///Go to the given position
	void seek(unsigned int position);
	///Get the current position
	unsigned int pos();
	///Set the position to zero
	void rewind();


	bool isOpen() const;
	unsigned int size();

	/*!
	* \brief Do not forget to close it when you done!
	*/
	void close();
	virtual ~OutByteArray();
private:
	const char* m_bytes;
	unsigned int m_length;
	unsigned int m_pos = 0;

};

} /* namespace STREAM */
} /* namespace PG */

#endif /* INCLUDE_PG_STREAM_PG_STREAMOUTBYTEARRAY_H_ */
