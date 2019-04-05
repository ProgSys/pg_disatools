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
#ifndef INCLUDE_PG_STREAM_PG_StreamIn_H_
#define INCLUDE_PG_STREAM_PG_StreamIn_H_

#include <string>
#include <vector>

namespace PG {
namespace STREAM {

class In {
public:
	In();
	///Read one byte, as a char (+BYTE).
	virtual char readChar() = 0;
	///Read one byte, as a unsigned char (+BYTE).
	virtual unsigned char readUnsignedChar()  = 0;

	///Read two bytes, as a short (+WORD).
	virtual short readShort()  = 0;
	///Read two bytes, as a unsigned short (+WORD).
	virtual unsigned short readUnsignedShort()  = 0;

	///Read four bytes, as a int (+Long WORD).
	virtual int readInt()  = 0;
	///Read four bytes, as a unsigned int (+Long WORD).
	virtual unsigned int readUnsignedInt()  = 0;

	///Read four bytes, as a long (+Long WORD 32bit).
	virtual long readLong()  = 0;
	///Read four bytes, as a unsigned long (+Long WORD 32bit).
	virtual unsigned long readUnsignedLong()  = 0;

	///Read eight bytes, as a long (+Quad WORD 64bit).
	virtual long long readLongLong()  = 0;
	///Read eight bytes, as a unsigned long (+Quad WORD 64bit).
	virtual unsigned long long readUnsignedLongLong()  = 0;


	/// Read four bytes, as a float (+/-Long WORD).
	virtual float readFloat() = 0;
	/// Read eight bytes, as a double (+/-IEEE).
	virtual double readDouble() = 0;

	///Read a string of given length.
	virtual std::string readString(unsigned int length)  = 0;
	///Read some bytes.
	virtual void read(char* data, unsigned int length)  = 0;
	inline void read(std::vector<char>& data, unsigned int length){data.resize(length);read(&data.front(),length);}
	inline void read(std::vector<unsigned char>& data, unsigned int length){data.resize(length);read((char*)&data.front(),length);}
	inline void read(std::vector<char>& data){read(&data.front(),data.size());}
	inline void read(std::vector<unsigned char>& data){read((char*)&data.front(),data.size());}

	///Skip the given number of bytes
	virtual void skip(unsigned int skip)  = 0;
	///Go to the given position
	virtual void seek(unsigned int position)  = 0;
	///Get the current position
	virtual unsigned int pos() = 0;
	///Set the position to zero
	virtual void rewind()  = 0;

	/*!
	 * @return true if position is at end.
	 */
	virtual bool eof() const = 0;
	virtual unsigned int size() = 0;

	///Closes the file stream, if it is a file stream else it does nothing.
	virtual void close() {};



	virtual ~In();
};

} /* namespace STREAM */
} /* namespace PG */

#endif /* INCLUDE_PG_STREAM_PG_StreamIn_H_ */
