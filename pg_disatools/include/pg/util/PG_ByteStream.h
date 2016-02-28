/*
 * PG_ByteStream.h
 *
 *  Created on: 28.02.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_PG_UTIL_PG_BYTESTREAM_H_
#define INCLUDE_PG_UTIL_PG_BYTESTREAM_H_

namespace PG {
namespace UTIL {

class ByteStream {
public:
	ByteStream(const char* bytes, unsigned int length);

	char readChar();
	char readUnsignedChar();

	int readInt();
	unsigned int readUnsignedInt();

	void read(char* data, unsigned int length);

	void seek(unsigned int position);

	virtual ~ByteStream();
private:
	const char* m_bytes;
	unsigned int m_length;
	unsigned int m_pos;
};

} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_UTIL_PG_BYTESTREAM_H_ */
