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
#include <openGL/PG_Plane.h>

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



Plane::Plane(){

}

bool Plane::bind(const UTIL::vec3& start, const UTIL::vec3& a, const UTIL::vec3& b, float textureScale){
	GLfloat const planeVertices[] = {
		start.x,start.y,start.z, 1.0f,
		start.x+a.x,start.y+a.y,start.z+a.z, 1.0f,
		start.x+a.x+b.x,start.y+a.y+b.y,start.z+a.z+b.z, 1.0f,

		start.x,start.y,start.z, 1.0f,
		start.x+a.x+b.x,start.y+a.y+b.y,start.z+a.z+b.z, 1.0f,
		start.x+b.x,start.y+b.y,start.z+b.z, 1.0f,
	};

	PG::UTIL::vec3 normal = PG::UTIL::cross(a,b);
	GLfloat const planeNoramls[] = {
		normal.x, normal.y, normal.z,
		normal.x, normal.y, normal.z,
		normal.x, normal.y, normal.z,

		normal.x, normal.y, normal.z,
		normal.x, normal.y, normal.z,
		normal.x, normal.y, normal.z,
	};

	GLfloat const planeUVs[] = {
		0.0f, textureScale,
		textureScale, textureScale,
		textureScale, 0.0f,

		0.0f, textureScale,
		textureScale, 0.0f,
	    0.0f, 0.0f
	};

	glGenVertexArrays(1, &m_GLID);//vaoObjectHandle
	glBindVertexArray(getGLID());

	int ponterID = 0;

	glGenBuffers(3, &m_bufferIDs[0]);

	if(m_bufferIDs[0] <= 0) throw_Exception("Generated handle for vertices is zero!");
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferIDs[0]);
	glBufferData(GL_ARRAY_BUFFER, 4*6 * sizeof(GLfloat), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(ponterID);
	glVertexAttribPointer(ponterID, 4, GL_FLOAT, GL_FALSE, 0, 0);
	ponterID++;


	if(m_bufferIDs[1] <= 0) throw_Exception("Generated handle for normal is zero!");
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferIDs[1]);
	glBufferData(GL_ARRAY_BUFFER, 3*6 * sizeof(GLfloat), planeNoramls, GL_STATIC_DRAW);
	glEnableVertexAttribArray(ponterID);
	glVertexAttribPointer(ponterID, 3, GL_FLOAT, GL_FALSE, 0, 0);
	ponterID++;

	if(m_bufferIDs[2] <= 0) throw_Exception("Generated handle for uv is zero!");
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, 2*6 * sizeof(GLfloat), planeUVs, GL_STATIC_DRAW);
	glEnableVertexAttribArray(ponterID);
	glVertexAttribPointer(ponterID, 2, GL_FLOAT, GL_FALSE, 0, 0);
	ponterID++;

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

void Plane::apply() const{
	if(m_GLID){
		//PG_INFO_STREAM("Render plane: "<<m_GLID);
		glBindVertexArray(m_GLID);
		glDrawArrays(GL_TRIANGLES, 0,6);
	}
}
void Plane::release() const{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Plane::~Plane() {
	glDeleteVertexArrays(1, &m_GLID);
	glDeleteBuffers(3, &m_bufferIDs[0]);
}

} /* namespace GL */
} /* namespace PG */
