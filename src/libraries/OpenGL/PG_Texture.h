/*
 *  GNU Lesser General Public License (LGPL):
 *
 *	Copyright (C) 2016  ProgSys
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation, version 3 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	You should have received a copy of the GNU Lesser General Public License
 *	along with this program.  If not, see http://doc.qt.io/qt-5/lgpl.html
 *	or http://www.gnu.org/licenses/
 */
#ifndef INCLUDE_PG_OPENGL_PG_TEXTURE_H_
#define INCLUDE_PG_OPENGL_PG_TEXTURE_H_

#include <vector>
#include <OpenGL/PG_GLItem.h>
#include <Util/PG_Image.h>
#include <Files/PG_TX2.h>

#define SUPPORT_QT false
#if SUPPORT_QT
#include <QImage>
#include <QList>
#include <QColor>
#endif

namespace PG {
namespace GL {

class Texture: public GLItem {
public:
	enum type{
		NORMAL,
		MEDIUM,
		LOW,
		SPRITE
	};

	Texture();

	void bind(const PG::UTIL::IDImage& img, Texture::type texType = NORMAL);
	void bind(const PG::UTIL::RGBImage& img, Texture::type texType = NORMAL);
	void bind(const PG::UTIL::RGBAImage& img, Texture::type texType = NORMAL);
	void bind(const PG::FILE::tx2Image& img, Texture::type texType = NORMAL);
#if SUPPORT_QT
	void bind(const QList<PG::UTIL::rgba>& colors, Texture::type texType = NORMAL);
	void bind(const QList<QColor>& colors, Texture::type texType = NORMAL);
#endif
	void bind(const std::vector<PG::UTIL::rgba>& img, Texture::type texType = NORMAL);
	void bindTGA(const std::string& path, Texture::type texType = NORMAL);
	void bindPNM(const std::string& path, Texture::type texType = NORMAL);
	void bindPNG(const std::string& path, Texture::type texType = NORMAL);

	void update(const PG::UTIL::IDImage& img);
	void update(const std::vector<PG::UTIL::rgba>& img);

	void changeType(Texture::type texType = NORMAL);

	void clear();

	void apply() const;

	void release() const;

	unsigned int getWidth() const;
	unsigned int getHeight() const;

	virtual ~Texture();
private:
	bool create();
	void setTexParameter(Texture::type texType);
	bool setTexture(
			unsigned char* imagedata, int type,
			const unsigned int width, const unsigned int height,
			bool midmap = true, bool freedata = false, bool flipYAxis = false
			);
	unsigned int m_width = 0;
	unsigned int m_height = 0;
};

} /* namespace GL */
} /* namespace PG */

#endif /* INCLUDE_PG_OPENGL_PG_TEXTURE_H_ */
