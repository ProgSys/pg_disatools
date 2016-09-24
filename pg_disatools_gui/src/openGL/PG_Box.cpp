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
#include <openGL/PG_Box.h>
// Open GL
#if __APPLE__
	#include <OpenGL/gl3.h>
#else
	#include <GL/glew.h>
#endif
#include <pg/util/PG_VectorUtil.h>
namespace PG {
namespace GL {
Box::Box():GLItem() {
	// TODO Auto-generated constructor stub

}
/*
 *   1----2
 *  /|   /|
 * 3----4 |
 * | 5--|-6
 * |/   |/
 * 7----8
 */
bool Box::bind(const UTIL::vec3& t1, const UTIL::vec3& t2, const UTIL::vec3& t3, const UTIL::vec3& t4,
		const UTIL::vec3& b5, const UTIL::vec3& b6, const UTIL::vec3& b7, const UTIL::vec3& b8,

		const UTIL::vec2& uvTop1   , const UTIL::vec2& uvTop2   , const UTIL::vec2& uvTop3   , const UTIL::vec2& uvTop4 ,
		const UTIL::vec2& uvLeft4  , const UTIL::vec2& uvLeft2  , const UTIL::vec2& uvLeft8  , const UTIL::vec2& uvLeft6 ,
		const UTIL::vec2& uvFront3 , const UTIL::vec2& uvFront4 , const UTIL::vec2& uvFront7 , const UTIL::vec2& uvFront8
){

	UTIL::vec4 const boxVertices[] = {
			//top y++
			UTIL::vec4(t1, 1.f), UTIL::vec4(t3, 1.f), UTIL::vec4(t2, 1.f),
			UTIL::vec4(t2, 1.f), UTIL::vec4(t3, 1.f), UTIL::vec4(t4, 1.f),

			//bottom y--
			UTIL::vec4(b5, 1.f), UTIL::vec4(b8, 1.f), UTIL::vec4(b7, 1.f),
			UTIL::vec4(b5, 1.f), UTIL::vec4(b6, 1.f), UTIL::vec4(b8, 1.f),

			//x++
			UTIL::vec4(t3, 1.f), UTIL::vec4(b7, 1.f), UTIL::vec4(t4, 1.f),
			UTIL::vec4(t4, 1.f), UTIL::vec4(b7, 1.f), UTIL::vec4(b8, 1.f),

			//x--
			UTIL::vec4(t1, 1.f), UTIL::vec4(t2, 1.f), UTIL::vec4(b5, 1.f),
			UTIL::vec4(t2, 1.f), UTIL::vec4(b6, 1.f), UTIL::vec4(b5, 1.f),

			//z++
			UTIL::vec4(t4, 1.f), UTIL::vec4(b8, 1.f), UTIL::vec4(t2, 1.f),
			UTIL::vec4(t2, 1.f), UTIL::vec4(b8, 1.f), UTIL::vec4(b6, 1.f),

			//z--
			UTIL::vec4(t3, 1.f), UTIL::vec4(t1, 1.f), UTIL::vec4(b5, 1.f),
			UTIL::vec4(t3, 1.f), UTIL::vec4(b5, 1.f), UTIL::vec4(b7, 1.f),

	};

	UTIL::vec3 boxNormals[12*3];
	for(unsigned int i = 0 ; i < 12*3; i+=3){
		const UTIL::vec3 n = PG::UTIL::cross(UTIL::vec3(boxVertices[i+1] - boxVertices[i]), UTIL::vec3(boxVertices[i+2] - boxVertices[i]));
		boxNormals[i] = n;
		boxNormals[i+1] = n;
		boxNormals[i+2] = n;
	}

	UTIL::vec2 const boxUVs[12*3] = {
			uvTop1, uvTop3, uvTop2,
			uvTop2, uvTop3, uvTop4,

			uvTop1, uvTop4, uvTop3,
			uvTop1, uvTop2, uvTop4,

			uvFront3, uvFront7, uvFront4,
			uvFront4, uvFront7, uvFront8,

			uvFront3, uvFront4, uvFront7,
			uvFront4, uvFront8, uvFront7,

			uvLeft4, uvLeft8, uvLeft2,
			uvLeft2, uvLeft8, uvLeft6,

			uvLeft4, uvLeft2, uvLeft6,
			uvLeft4, uvLeft6, uvLeft8
	};


	glGenVertexArrays(1, &m_GLID);//vaoObjectHandle
	glBindVertexArray(getGLID());

	int ponterID = 0;

	glGenBuffers(3, &m_bufferIDs[0]);

	if(m_bufferIDs[0] <= 0) throw_Exception("Generated handle for vertices is zero!");
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferIDs[0]);
	glBufferData(GL_ARRAY_BUFFER, 4*3*12 * sizeof(GLfloat), boxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(ponterID);
	glVertexAttribPointer(ponterID, 4, GL_FLOAT, GL_FALSE, 0, 0);
	ponterID++;


	if(m_bufferIDs[1] <= 0) throw_Exception("Generated handle for normal is zero!");
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferIDs[1]);
	glBufferData(GL_ARRAY_BUFFER, 3*3*12  * sizeof(GLfloat), boxNormals, GL_STATIC_DRAW);
	glEnableVertexAttribArray(ponterID);
	glVertexAttribPointer(ponterID, 3, GL_FLOAT, GL_FALSE, 0, 0);
	ponterID++;

	if(m_bufferIDs[2] <= 0) throw_Exception("Generated handle for uv is zero!");
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, 2*3*12  * sizeof(GLfloat), boxUVs, GL_STATIC_DRAW);
	glEnableVertexAttribArray(ponterID);
	glVertexAttribPointer(ponterID, 2, GL_FLOAT, GL_FALSE, 0, 0);
	ponterID++;

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

void Box::apply() const{
	if(m_GLID){
		//PG_INFO_STREAM("Render plane: "<<m_GLID);
		glBindVertexArray(m_GLID);
		glDrawArrays(GL_TRIANGLES, 0,2*3*12);
	}else
		PG_INFO_STREAM("Render GL ID is invalid!");
}

void Box::release() const{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Box::~Box() {
	glDeleteVertexArrays(1, &m_GLID);
	glDeleteBuffers(3, &m_bufferIDs[0]);
}

}
}
