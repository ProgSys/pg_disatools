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
#ifndef INCLUDE_OPENGL_PG_TILEPLANE_H_
#define INCLUDE_OPENGL_PG_TILEPLANE_H_

#include <OpenGL/PG_GLItem.h>
#include <Util/PG_Vector.h>

namespace PG {
namespace GL {

class TilePlane: public GLItem {
public:
	TilePlane();

	/*
	 *   1----2
	 *  /    /
	 * 3----4
	 */
	///Parameterform
	bool bind(const UTIL::vec4 (&vertices)[6], const UTIL::vec3 (&normals)[6], const UTIL::vec2 (&UVsChanel0)[6], const UTIL::vec2 (&UVsChanel1)[6] );
	bool bind(const UTIL::vec3& t1, const UTIL::vec3& t2, const UTIL::vec3& t3, const UTIL::vec3& t4,
			const UTIL::vec2& ca1,  const UTIL::vec2& ca2, const UTIL::vec2& ca3, const UTIL::vec2& ca4,
			const UTIL::vec2& cb1,  const UTIL::vec2& cb2, const UTIL::vec2& cb3, const UTIL::vec2& cb4
			);

	bool bind(const UTIL::vec3& t1, const UTIL::vec3& t2, const UTIL::vec3& t3, const UTIL::vec3& t4,
			const UTIL::vec2& ca1,  const UTIL::vec2& ca2, const UTIL::vec2& ca3, const UTIL::vec2& ca4, unsigned char spinA,
			const UTIL::vec2& cb1,  const UTIL::vec2& cb2, const UTIL::vec2& cb3, const UTIL::vec2& cb4, unsigned char spinB
			);

	void apply() const final;
	void release() const final;

	virtual ~TilePlane();
private:
	unsigned int m_bufferIDs[4];
};

} /* namespace GL */
} /* namespace PG */

#endif /* INCLUDE_OPENGL_PG_TILEPLANE_H_ */
