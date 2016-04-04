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

#ifndef INCLUDE_OPENGL_PG_PLANE_H_
#define INCLUDE_OPENGL_PG_PLANE_H_

#include <openGL/PG_GLItem.h>
#include <pg/util/PG_Vector.h>

namespace PG {
namespace GL {

class Plane: public GLItem {
public:
	Plane();

	///Parameterform
	bool bind(const UTIL::vec3& start, const UTIL::vec3& a, const UTIL::vec3& b);

	void apply() const final;
	void release() const final;

	virtual ~Plane();
private:
	unsigned int m_bufferIDs[3];
};

} /* namespace GL */
} /* namespace PG */

#endif /* INCLUDE_OPENGL_PG_PLANE_H_ */
