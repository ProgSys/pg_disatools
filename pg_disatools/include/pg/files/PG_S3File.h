/*
 * Class for S3 compression files.
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

#ifndef INCLUDE_PG_FILES_PG_S3FILE_H_
#define INCLUDE_PG_FILES_PG_S3FILE_H_

#include <string>
#include <vector>
#include <pg/util/PG_Vector.h>

namespace PG {
namespace FILE {


class S3File {
public:
enum inFormat: unsigned int{
	INVALID = 0, VTF, TX2
};
enum outFormat: unsigned int{
	PGM, TGA
};

	__declspec(dllexport) S3File(const std::string& filepath);
	__declspec(dllexport) bool save(const std::string& outfilepath, outFormat out) const;

	__declspec(dllexport) unsigned int getWidth() const;
	__declspec(dllexport) unsigned int getHeight() const;

	__declspec(dllexport) virtual ~S3File();
private:
	bool readVTF(std::vector<PG::UTIL::rgba>& outRGBAData) const;
	bool readTX2(std::vector<PG::UTIL::rgba>& outRGBAData) const;
	//bool savePPM(const std::string& outfilepath) const;
	std::string m_filename;
	inFormat m_filetype;
	unsigned int m_width;
	unsigned int m_height;
};

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_S3FILE_H_ */
