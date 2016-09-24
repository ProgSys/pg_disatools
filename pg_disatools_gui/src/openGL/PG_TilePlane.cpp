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
#include <openGL/PG_TilePlane.h>
// Open GL
#if __APPLE__
	#include <OpenGL/gl3.h>
#else
	#include <GL/glew.h>
#endif
#include <pg/util/PG_VectorUtil.h>

namespace PG {
namespace GL {

TilePlane::TilePlane():GLItem() {
	// TODO Auto-generated constructor stub

}

TilePlane::~TilePlane() {
	glDeleteVertexArrays(1, &m_GLID);
	glDeleteBuffers(4, &m_bufferIDs[4]);
}


bool TilePlane::bind(const UTIL::vec4 (&vertices)[6], const UTIL::vec3 (&normals)[6], const UTIL::vec2 (&UVsChanel0)[6], const UTIL::vec2 (&UVsChanel1)[6]  ){
	glGenVertexArrays(1, &m_GLID);//vaoObjectHandle
	glBindVertexArray(getGLID());

	int ponterID = 0;
	glGenBuffers(4, &m_bufferIDs[0]);

	if(m_bufferIDs[0] <= 0) throw_Exception("Generated handle for vertices is zero!");
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferIDs[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)*4 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(ponterID);
	glVertexAttribPointer(ponterID, 4, GL_FLOAT, GL_FALSE, 0, 0);
	ponterID++;


	if(m_bufferIDs[1] <= 0) throw_Exception("Generated handle for normal is zero!");
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferIDs[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals)*3  * sizeof(GLfloat), normals, GL_STATIC_DRAW);
	glEnableVertexAttribArray(ponterID);
	glVertexAttribPointer(ponterID, 3, GL_FLOAT, GL_FALSE, 0, 0);
	ponterID++;

	if(m_bufferIDs[2] <= 0) throw_Exception("Generated handle for uv0 is zero!");
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(UVsChanel0)*2  * sizeof(GLfloat), UVsChanel0, GL_STATIC_DRAW);
	glEnableVertexAttribArray(ponterID);
	glVertexAttribPointer(ponterID, 2, GL_FLOAT, GL_FALSE, 0, 0);
	ponterID++;

	if(m_bufferIDs[3] <= 0) throw_Exception("Generated handle for uv1 is zero!");
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(UVsChanel1)*2  * sizeof(GLfloat), UVsChanel1, GL_STATIC_DRAW);
	glEnableVertexAttribArray(ponterID);
	glVertexAttribPointer(ponterID, 2, GL_FLOAT, GL_FALSE, 0, 0);
	ponterID++;

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

/*
 *   1----2
 *  /    /
 * 3----4
 *
 * t0: 2-1-3
 * t0: 2-3-4
 */
///Parameterform
bool TilePlane::bind(const UTIL::vec3& t1, const UTIL::vec3& t2, const UTIL::vec3& t3, const UTIL::vec3& t4,
		const UTIL::vec2& ca1,  const UTIL::vec2& ca2, const UTIL::vec2& ca3, const UTIL::vec2& ca4,
		const UTIL::vec2& cb1,  const UTIL::vec2& cb2, const UTIL::vec2& cb3, const UTIL::vec2& cb4
		){

	UTIL::vec4 const vertices[] = {
			//top y++
			UTIL::vec4(t2, 1.f), UTIL::vec4(t1, 1.f), UTIL::vec4(t3, 1.f),
			UTIL::vec4(t2, 1.f), UTIL::vec4(t3, 1.f), UTIL::vec4(t4, 1.f)
	};

	UTIL::vec3 normals[6];
	for(unsigned int i = 0 ; i < sizeof(normals); i+=3){
		const UTIL::vec3 n = PG::UTIL::cross(UTIL::vec3(vertices[i+1] - vertices[i]), UTIL::vec3(vertices[i+2] - vertices[i]));
		normals[i] = n;
		normals[i+1] = n;
		normals[i+2] = n;
	}

	UTIL::vec2 const UVsChanel0[] = {
			ca2, ca1, ca3,
			ca2, ca3, ca4
	};

	UTIL::vec2 const UVsChanel1[] = {
			cb2, cb1, cb3,
			cb2, cb3, cb4
	};

	return bind(vertices,normals, UVsChanel0, UVsChanel1);

}


inline void setUV( UTIL::vec2 (&UVsChanel)[6], const UTIL::vec2& ca1,  const UTIL::vec2& ca2, const UTIL::vec2& ca3, const UTIL::vec2& ca4, unsigned char spinA){
	switch (spinA) {
		case 3:
		{UVsChanel[0] = ca4; UVsChanel[1] = ca2;UVsChanel[2] = ca1;
		UVsChanel[3] = ca4;UVsChanel[4] = ca1;UVsChanel[5] = ca3;}
			break;
		case 2:
		{UVsChanel[0] = ca3; UVsChanel[1] = ca4;UVsChanel[2] = ca2;
		UVsChanel[3] = ca3;UVsChanel[4] = ca2;UVsChanel[5] = ca1;}
			break;
		case 1:
		{UVsChanel[0] = ca1; UVsChanel[1] = ca3;UVsChanel[2] = ca4;
		UVsChanel[3] = ca1;UVsChanel[4] = ca4;UVsChanel[5] = ca2;}
			break;
		case 0:
		default:
		{UVsChanel[0] = ca2; UVsChanel[1] = ca1;UVsChanel[2] = ca3;
		UVsChanel[3] = ca2;UVsChanel[4] = ca3;UVsChanel[5] = ca4;}
			break;
	}

}

bool TilePlane::bind(const UTIL::vec3& t1, const UTIL::vec3& t2, const UTIL::vec3& t3, const UTIL::vec3& t4,
		const UTIL::vec2& ca1,  const UTIL::vec2& ca2, const UTIL::vec2& ca3, const UTIL::vec2& ca4, unsigned char spinA,
		const UTIL::vec2& cb1,  const UTIL::vec2& cb2, const UTIL::vec2& cb3, const UTIL::vec2& cb4, unsigned char spinB
		){

	UTIL::vec4 const vertices[] = {
			//top y++
			UTIL::vec4(t2, 1.f), UTIL::vec4(t1, 1.f), UTIL::vec4(t3, 1.f),
			UTIL::vec4(t2, 1.f), UTIL::vec4(t3, 1.f), UTIL::vec4(t4, 1.f)
	};

	UTIL::vec3 normals[6];
	for(unsigned int i = 0 ; i < 6; i+=3){
		const UTIL::vec3 n = PG::UTIL::cross(UTIL::vec3(vertices[i+1] - vertices[i]), UTIL::vec3(vertices[i+2] - vertices[i]));
		normals[i] = n;
		normals[i+1] = n;
		normals[i+2] = n;
	}

	UTIL::vec2 UVsChanel0[6];
	setUV(UVsChanel0, ca1, ca2, ca3, ca4,spinA);


	UTIL::vec2 UVsChanel1[6];
	setUV(UVsChanel1, cb1, cb2, cb3, cb4,spinB);

	return bind(vertices,normals, UVsChanel0, UVsChanel1);
}

void TilePlane::apply() const{
	if(m_GLID){
		//PG_INFO_STREAM("Render plane: "<<m_GLID);
		glBindVertexArray(m_GLID);
		glDrawArrays(GL_TRIANGLES, 0,6);
	}else
		PG_INFO_STREAM("Render GL ID is invalid!");
}
void TilePlane::release() const{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

} /* namespace GL */
} /* namespace PG */
