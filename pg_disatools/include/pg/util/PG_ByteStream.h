/*
 * PG_ByteStream.h
 *
 *  Created on: 28.02.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_PG_UTIL_PG_BYTESTREAM_H_
#define INCLUDE_PG_UTIL_PG_BYTESTREAM_H_

#include <pg/util/PG_ApiUtil.h>
namespace PG {
namespace UTIL {

class ByteStream {
public:
	PG_UTIL_API ByteStream(const char* bytes, unsigned int length);

	char PG_UTIL_API readChar();
	char PG_UTIL_API readUnsignedChar();

	short PG_UTIL_API readShort();
	unsigned short PG_UTIL_API readUnsignedShort();

	int PG_UTIL_API readInt();
	unsigned int PG_UTIL_API readUnsignedInt();

	void PG_UTIL_API read(char* data, unsigned int length);

	void PG_UTIL_API skip(unsigned int skip);
	void PG_UTIL_API seek(unsigned int position);

	virtual PG_UTIL_API ~ByteStream();
private:
	const char* m_bytes;
	unsigned int m_length;
	unsigned int m_pos;
};

} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_UTIL_PG_BYTESTREAM_H_ */
