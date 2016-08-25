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
#include <openGL/PG_LinePath.h>

// Open GL
#if __APPLE__
	#include <OpenGL/gl3.h>
#else
	#include <GL/glew.h>
#endif

#include <pg/util/PG_Exception.h>
#include <pg/util/PG_VectorUtil.h>

namespace PG {
namespace GL {

LinePath::LinePath() {
	m_bufferID = 0;

}

bool LinePath::bind(const UTIL::vec4& a, const UTIL::vec4& b){
	std::vector<UTIL::vec4> points = {a,b};
	return bind(points);
}
bool LinePath::bind(const UTIL::vec4& a, const UTIL::vec4& b, const UTIL::vec4& c){
	std::vector<UTIL::vec4> points = {a,b,c};
	return bind(points);
}
bool LinePath::bind(const UTIL::vec4& a, const UTIL::vec4& b, const UTIL::vec4& c, const UTIL::vec4& d){
	std::vector<UTIL::vec4> points = {a,b,c,d};
	return bind(points);
}
bool LinePath::bind(const UTIL::vec4& a, const UTIL::vec4& b, const UTIL::vec4& c, const UTIL::vec4& d, const UTIL::vec4& e){
	std::vector<UTIL::vec4> points = {a,b,c,d,e};
	return bind(points);
}
bool LinePath::bind(const std::vector<UTIL::vec4>& points){
	if(points.empty()) return false;

	glGenVertexArrays(1, &m_GLID);//vaoObjectHandle
	glBindVertexArray(getGLID());

	glGenBuffers(1, &m_bufferID);

	if(m_bufferID <= 0) throw_Exception("Generated handle for vertices is zero!");
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferID);
	glBufferData(GL_ARRAY_BUFFER, 4*points.size() * sizeof(GLfloat), &(points[0].x), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	m_size = points.size();
	return true;
}

void LinePath::apply() const{
	if(m_GLID){
		//PG_INFO_STREAM("Render plane: "<<m_GLID);
		glBindVertexArray(m_GLID);
		glDrawArrays(GL_LINE_STRIP, 0,m_size);
	}
}

void LinePath::release() const{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

LinePath::~LinePath() {
	glDeleteVertexArrays(1, &m_GLID);
	glDeleteBuffers(1, &m_bufferID);
}

} /* namespace GL */
} /* namespace PG */
