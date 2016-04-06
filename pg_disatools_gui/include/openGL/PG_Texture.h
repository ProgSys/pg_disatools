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
#include <openGL/PG_GLItem.h>
#include <pg/util/PG_Image.h>

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
	void bind(const std::vector<PG::UTIL::rgba>& img, Texture::type texType = NORMAL);
	void bindTGA(const std::string& path, Texture::type texType = NORMAL);
	void bindPNM(const std::string& path, Texture::type texType = NORMAL);

	void clear();

	void apply() const;

	void release() const;

	virtual ~Texture();
private:
	bool create();
	void setTexParameter(Texture::type texType);
	bool setTexture(
			unsigned char* imagedata, int type,
			const unsigned int width, const unsigned int height,
			bool midmap = true, bool freedata = false, bool flipYAxis = false
			);
};

} /* namespace GL */
} /* namespace PG */

#endif /* INCLUDE_PG_OPENGL_PG_TEXTURE_H_ */
