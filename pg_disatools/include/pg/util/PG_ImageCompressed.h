/*
 * PG_ImageCompressed.h
 *
 *  Created on: 14.01.2017
 *      Author: ProgSys
 */

#ifndef INCLUDE_PG_UTIL_PG_IMAGECOMPRESSED_H_
#define INCLUDE_PG_UTIL_PG_IMAGECOMPRESSED_H_

#include <vector>
#include <pg/util/PG_Base.h>
#include <pg/files/PG_TX2.h>
#include <pg/files/PG_IMY.h>

namespace PG {
namespace UTIL {


class ImageCompressed {
public:
	enum fileType: unsigned int{
		PG, TX2, IMY
	};

	enum dataType: unsigned int {
		RGBA, BGRA, DXT1, DXT5,
		COLORTABLE_BGRA16, COLORTABLE_RGBA16, COLORTABLE_BGRA256, COLORTABLE_RGBA256
	};

	ImageCompressed();

	bool save(const std::string& path);
	virtual bool save(PG::STREAM::Out* out) = 0;

	bool open(const std::string& path);
	virtual bool open(PG::STREAM::In* in)  = 0;
	bool open(const PG::UTIL::RGBAImage& image_in, ImageCompressed::dataType type);
	virtual bool open(const char* image_in, unsigned int width, unsigned int height, ImageCompressed::dataType type)  = 0;

	virtual bool getRGBA(char* image_out, unsigned int colorTableID) const = 0;
	virtual bool getRGBA(PG::UTIL::RGBAImage& image_out, unsigned int colorTableID) const = 0;

	virtual fileType getFileType() const = 0;
	virtual dataType getDataType() const = 0;
	virtual void setDataType(dataType type) = 0;

	virtual unsigned int getWidth() const = 0;
	virtual unsigned int getHeight() const = 0;

	inline unsigned int getColorTableSize() const{
		if(m_colortables.empty())
			return 0;
		else
			return m_colortables[0].size();
	}

	inline unsigned int getNumberOfColorTables() const{
		return m_colortables.size();
	}

	inline std::vector<PG::FILE::ColorTable>& getColorTables(){
		return m_colortables;
	}

	inline const std::vector<PG::FILE::ColorTable>& getColorTables() const{
		return m_colortables;
	}


	virtual ~ImageCompressed();

protected:
	std::vector<char> m_data;
	std::vector<PG::FILE::ColorTable> m_colortables;
};

class TX2ImageB: public ImageCompressed {
public:
	TX2ImageB();
	TX2ImageB(const std::string& path);
	TX2ImageB(PG::STREAM::In* in);
	TX2ImageB(PG::UTIL::RGBAImage& image_in, ImageCompressed::dataType type);
	TX2ImageB(const char* image_in, unsigned int width, unsigned int height, ImageCompressed::dataType type);

	virtual bool save(PG::STREAM::Out* out);

	virtual bool open(PG::STREAM::In* in);
	virtual bool open(const char* image_in, unsigned int width, unsigned int height, ImageCompressed::dataType type);

	virtual bool getRGBA(char* image_out, unsigned int colorTableID) const;
	virtual bool getRGBA(PG::UTIL::RGBAImage& image_out, unsigned int colorTableID) const;

	virtual ImageCompressed::fileType getFileType() const;
	virtual ImageCompressed::dataType getDataType() const;
	virtual void setDataType(dataType type);

	virtual unsigned int getWidth() const;
	virtual unsigned int getHeight() const;

	virtual ~TX2ImageB();
private:
	PG::FILE::tx2HeaderOrg m_header;
};

class IMYImage: public ImageCompressed {
public:
	IMYImage();
	IMYImage(const std::string& path);
	IMYImage(PG::STREAM::In* in);
	IMYImage(const PG::UTIL::RGBAImage& image_in, ImageCompressed::dataType type);
	IMYImage(const char* image_in, unsigned int width, unsigned int height, ImageCompressed::dataType type);

	virtual bool save(PG::STREAM::Out* out);

	virtual bool open(PG::STREAM::In* in);
	virtual bool open(const char* image_in, unsigned int width, unsigned int height, ImageCompressed::dataType type);

	virtual bool getRGBA(char* image_out, unsigned int colorTableID) const;
	virtual bool getRGBA(PG::UTIL::RGBAImage& image_out, unsigned int colorTableID) const;

	virtual ImageCompressed::fileType getFileType() const;
	virtual ImageCompressed::dataType getDataType() const;
	virtual void setDataType(dataType type);

	virtual unsigned int getWidth() const;
	virtual unsigned int getHeight() const;

	virtual ~IMYImage();
private:
	PG::FILE::imyHeader m_header;
};

ImageCompressed* convertTo(ImageCompressed* img, ImageCompressed::fileType type, ImageCompressed::dataType data);

} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_UTIL_PG_IMAGECOMPRESSED_H_ */
