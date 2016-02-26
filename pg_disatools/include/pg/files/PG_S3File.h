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

#ifndef INCLUDE_PG_FILES_PG_S3FILE_H_
#define INCLUDE_PG_FILES_PG_S3FILE_H_

#include <string>
#include <vector>
#include <pg/util/PG_Colors.h>

namespace PG {
namespace FILE {

struct S3Base{
	/*!
	 * @brief Returns a block of 4x4 RGBA pixels.
	 */
	//virtual void getColors(std::vector<PG::UTIL::RGBA>& outRGBAData) const = 0;
	//virtual ~S3Base() = default;
};

struct DXT1block: public S3Base{
	unsigned char color[8];
	/*!
	 * @brief Returns a block of 4x4 RGBA pixels.
	 */
	void getColors(std::vector<PG::UTIL::RGBA>& outRGBAData) const;
};

struct DXT5block: public S3Base{
	unsigned char alpha[8];
	unsigned char color[8];
	/*!
	 * @brief Returns a block of 4x4 RGBA pixels.
	 */
	void getColors(std::vector<PG::UTIL::RGBA>& outRGBAData) const;
};

template<class blockType>
static bool uncompressS3(unsigned int width, unsigned int height, const std::vector<blockType>& inData, std::vector<PG::UTIL::RGBA>& outRGBAData);

#define uncompressDXT1(args...)  PG::FILE::uncompressS3<PG::FILE::DXT1block>(args)
#define uncompressDXT5(args...)  PG::FILE::uncompressS3<PG::FILE::DXT5block>(args)

class S3File {
public:
enum inFormat: unsigned int{
	INVALID = 0, VTF, TX2
};
enum outFormat: unsigned int{
	PGM, TGA
};

	S3File(const std::string& filepath);
	bool save(const std::string& outfilepath, outFormat out) const;

	unsigned int getWidth() const;
	unsigned int getHeight() const;

	virtual ~S3File();
private:
	bool readVTF(std::vector<PG::UTIL::RGBA>& outRGBAData) const;
	bool readTX2(std::vector<PG::UTIL::RGBA>& outRGBAData) const;
	//bool savePPM(const std::string& outfilepath) const;
	std::string m_filename;
	inFormat m_filetype;
	unsigned int m_width;
	unsigned int m_height;
};

} /* namespace FILE */
} /* namespace PG */

#endif /* INCLUDE_PG_FILES_PG_S3FILE_H_ */
