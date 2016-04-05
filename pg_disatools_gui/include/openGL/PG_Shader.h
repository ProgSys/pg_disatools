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
#ifndef INCLUDE_OPENGL_PG_SHADER_H_
#define INCLUDE_OPENGL_PG_SHADER_H_

#include <openGL/PG_GLItem.h>

// Open GL
#if __APPLE__
	#include <OpenGL/gl3.h>
#else
	#include <GL/glew.h>
#endif

#include <string>
#include <vector>

#include <pg/util/PG_Vector.h>
#include <pg/util/PG_Matrix.h>

namespace PG {
namespace GL {

class Shader: public GLItem {
public:
	enum type{
		VERTEX = GL_VERTEX_SHADER,
		TESSELLATION_CONTROL = GL_TESS_CONTROL_SHADER,
		TESSELLATION_EVALUATION = GL_TESS_EVALUATION_SHADER,
		GEOMETRY = GL_GEOMETRY_SHADER,
		FRAGMENT = GL_FRAGMENT_SHADER,
		COMPUTE = GL_COMPUTE_SHADER
	};

	Shader();

	void addShaderFile(Shader::type shaderType, const std::string& filepath);
	void addShader(Shader::type shaderType, const std::string& shaderText);

	///Some call it link
	virtual bool bind();

	void apply() const;
	void release() const;


	int getAttributeLocation(const std::string& name) const;
	int getUniformLocation(const std::string& name) const;

	void setUniform(int location, int value) const;
	void setUniform(int location, float value) const;

	void setUniform(int location, const PG::UTIL::vec2& value) const;
	void setUniform(int location, const PG::UTIL::vec3& value) const;
	void setUniform(int location, const PG::UTIL::vec4& value) const;

	void setUniform(int location, const PG::UTIL::mat4& value) const;
	//void setUniform(int location, const PG::UTIL::vec4& value);
	//void setUniform(int location, const PG::UTIL::vec4& value);

	virtual ~Shader();
protected:
	struct shaderInfo{
		type shaderType;
		std::string text;
	};

	std::vector<shaderInfo> m_shaders;
};

} /* namespace GL */
} /* namespace PG */

#endif /* INCLUDE_OPENGL_PG_SHADER_H_ */
