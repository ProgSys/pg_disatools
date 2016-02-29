/*
 * Wrapper class around std::ofstream.
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
