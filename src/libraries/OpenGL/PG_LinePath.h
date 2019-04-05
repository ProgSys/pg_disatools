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
#ifndef INCLUDE_OPENGL_PG_LINEPATH_H_
#define INCLUDE_OPENGL_PG_LINEPATH_H_

#include <OpenGL/PG_GLItem.h>
#include <Util/PG_Vector.h>
#include <vector>

namespace PG {
namespace GL {

class LinePath : public GLItem  {
public:
	LinePath();

	///Parameterform
	bool bind(const UTIL::vec4& a, const UTIL::vec4& b);
	bool bind(const UTIL::vec4& a, const UTIL::vec4& b, const UTIL::vec4& c);
	bool bind(const UTIL::vec4& a, const UTIL::vec4& b, const UTIL::vec4& c, const UTIL::vec4& d);
	bool bind(const UTIL::vec4& a, const UTIL::vec4& b, const UTIL::vec4& c, const UTIL::vec4& d, const UTIL::vec4& e);
	bool bind(const std::vector<UTIL::vec4>& points);

	void apply() const final;
	void release() const final;

	virtual ~LinePath();
private:
	unsigned int m_bufferID;
	unsigned int m_size = 0;
};

} /* namespace GL */
} /* namespace PG */

#endif /* INCLUDE_OPENGL_PG_LINEPATH_H_ */
