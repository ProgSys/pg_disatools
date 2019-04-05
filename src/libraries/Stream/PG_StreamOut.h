/*
 * PG_StreamOut.h
 *
 *  Created on: 08.03.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_PG_STREAM_PG_STREAMOUT_H_
#define INCLUDE_PG_STREAM_PG_STREAMOUT_H_

#include <string>

namespace PG {
namespace STREAM {

class Out {
public:
	Out();

	virtual void writeChar(char c) = 0;
	virtual void writeShort(short s) = 0;
	virtual void writeInt(int i) = 0;
	virtual void writeLong(long l) = 0;
	virtual void writeLongLong(long long ll)  = 0;

	virtual void writeFloat(float i) = 0;
	virtual void writeDouble(double d) = 0;

	virtual void operator<<(const std::string& string) = 0;
	virtual void writeString(const std::string& string) = 0;

	virtual void write(char* data, unsigned int bytes) = 0;

	///Skip the given number of bytes, if at end write zero
	virtual void skip(unsigned int skip)  = 0;
	///Go to the given position
	virtual void seek(unsigned int position)  = 0;
	///Get the current position
	virtual unsigned int pos() = 0;
	///Set the position to zero
	virtual void rewind()  = 0;

	virtual unsigned int size() = 0;

	///Closes the file stream, if it is a file stream else it does nothing.
	virtual void close() {};

	virtual ~Out();
};

} /* namespace STREAM */
} /* namespace PG */

#endif /* INCLUDE_PG_STREAM_PG_STREAMOUT_H_ */
