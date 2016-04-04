/*
 * PG_Shader.h
 *
 *  Created on: 04.04.2016
 *      Author: ProgSys
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
	bool bind();

	void apply() const final;
	void release() const final;


	int getAttributeLocation(const std::string& name) const;
	int getUniformLocation(const std::string& name) const;

	void setUniform(int location, int value);
	void setUniform(int location, float value);

	void setUniform(int location, const PG::UTIL::vec2& value);
	void setUniform(int location, const PG::UTIL::vec3& value);
	void setUniform(int location, const PG::UTIL::vec4& value);

	//void setUniform(int location, const PG::UTIL::vec4& value);
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
