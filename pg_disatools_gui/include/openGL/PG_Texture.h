/*
 * PG_Texture.h
 *
 *  Created on: 31.03.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_PG_OPENGL_PG_TEXTURE_H_
#define INCLUDE_PG_OPENGL_PG_TEXTURE_H_

#include <openGL/PG_GLItem.h>
#include <pg/util/PG_Image.h>

namespace PG {
namespace GL {

class Texture: public GLItem {
public:
	Texture();

	void bind(const PG::UTIL::IDImage& img);
	void bind(const PG::UTIL::RGBAImage& img);

	void apply() const;

	void release() const;

	virtual ~Texture();
private:
	void create();
};

} /* namespace GL */
} /* namespace PG */

#endif /* INCLUDE_PG_OPENGL_PG_TEXTURE_H_ */
