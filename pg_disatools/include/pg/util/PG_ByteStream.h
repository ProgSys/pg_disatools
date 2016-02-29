/*
 * Wrapper class around const char* (bytes).
 *
 *  Copyright (C) 2016  ProgSys
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INCLUDE_PG_UTIL_PG_BYTESTREAM_H_
#define INCLUDE_PG_UTIL_PG_BYTESTREAM_H_

#include <pg/util/PG_ApiUtil.h>
#include <string>
namespace PG {
namespace UTIL {

class ByteStream {
public:
	ByteStream(const char* bytes, unsigned int length);

	char readChar();
	char readUnsignedChar();

	short readShort();
	unsigned short readUnsignedShort();

	int readInt();
	unsigned int readUnsignedInt();

	long readLong();
	unsigned long readUnsignedLong();

	long long readLongLong();
	unsigned long long readUnsignedLongLong();

	std::string readString(unsigned int length);

	void read(char* data, unsigned int length);

	/*!
	 * Get the number of bytes.
	 */
	unsigned int getSize() const;

	void skip(unsigned int skip);
	void seek(unsigned int position);
	void reset();

	/*!
	 * @return true if position is at end.
	 */
	bool eof() const;

	virtual ~ByteStream();
private:
	const char* m_bytes;
	unsigned int m_length;
	unsigned int m_pos;
};

} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_UTIL_PG_BYTESTREAM_H_ */
