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
#include <OpenGL/PG_Shader.h>

#include <algorithm>
#include <cstring>
#include <Util/PG_Base.h>
#include <OpenGL/PG_GLError.h>

#include <Util/PG_VectorUtil.h>
#include <Util/PG_MatrixUtil.h>
#include <fstream>

namespace PG {
namespace GL {

Shader::Shader() {
	// TODO Auto-generated constructor stub

}

void Shader::addShaderFile(Shader::type shaderType, const std::string& filepath){
	std::ifstream file;
	file.open(filepath, std::ofstream::in);
	std::string fileContent;
	if (file.is_open()) {
		//reseve space
		file.seekg(0, std::ios::end);
		fileContent.reserve(file.tellg());
		file.seekg(0, std::ios::beg);

		fileContent.assign((std::istreambuf_iterator<char>(file)),
				   std::istreambuf_iterator<char>());
		file.close();
	}
	if(!fileContent.empty())
		addShader(shaderType, fileContent);
}

void Shader::addShader(Shader::type shaderType, const std::string& shaderText){
	auto it = std::find_if(m_shaders.begin(), m_shaders.end(), [shaderType](const shaderInfo& info){
		return info.shaderType == shaderType;
	});
	if(it != m_shaders.end()){
		(*it).text = shaderText;
	}else
		m_shaders.push_back( {shaderType, shaderText} );
}

//checks a shader for compiler errors
inline bool checkShader(GLuint shaderID) {
    GLint status;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        GLint infoLogLength;
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar* infoLog = new GLchar[infoLogLength + 1];
        glGetShaderInfoLog(shaderID, infoLogLength, NULL, infoLog);

        PG_ERROR_STREAM("Unable to compile shader" << std::endl << infoLog );
        // Don't leak the shader.
        glDeleteShader(shaderID);
        delete[] infoLog;
        return true;
    } else {
        PG_SUCCESS_STREAM("Shader compiled ID: "<< shaderID);
        return false;
    }
}

//checks a program
inline bool checkProgram(GLuint programID) {
    GLint status;
    glGetProgramiv(programID, GL_LINK_STATUS, &status);

    if (status == GL_FALSE) {
        GLint infoLogLength;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar* infoLog = new GLchar[infoLogLength + 1];
        glGetShaderInfoLog(programID, infoLogLength, NULL, infoLog);

        PG_ERROR_STREAM("Unable to link ShaderSet: " << std::endl << infoLog );
        glDeleteProgram(programID);
        delete[] infoLog;
        return true;
    } else {
    	PG_SUCCESS_STREAM("ShaderSet linked ID: "<< programID);
    	return false;
    }
}


bool Shader::bind(){
	glUseProgram(0);
	if(getGLID() != INVALID_OGL_VALUE)
		glDeleteProgram(getGLID());
	m_GLID = 0;

	if(m_shaders.empty()) return false;

	std::vector<GLuint> ids;
	ids.reserve(m_shaders.size());

	for(const shaderInfo& info: m_shaders){
		GLuint id = glCreateShader(info.shaderType);
		ids.push_back(id);

		const char* source = info.text.c_str();
	    const GLint source_size = strlen(source); //info.text.size();//
		glShaderSource(id, 1, &source, &source_size);

		glCompileShader(id);
		if(checkShader(id)){
			for(GLuint id: ids)
				glDeleteShader(id);
			return false;
		}
	}


	//link shader programs
	m_GLID = glCreateProgram(); //ProgramID

	for(GLuint id: ids){
		glAttachShader(m_GLID, id);
	}

	glLinkProgram(getGLID());
	if(checkProgram(getGLID()))
		return false;

	for(GLuint id: ids){
		glDetachShader(m_GLID, id);
		glDeleteShader(id);
	}

	return !checkGLError();
}

void Shader::apply() const{
	glUseProgram(getGLID());
}
void Shader::release() const{
	glUseProgram(0);
}

int Shader::getAttributeLocation(const std::string& name) const{
	const int i = glGetAttribLocation( m_GLID, name.c_str());
	if(i == INVALID_OGL_VALUE)
		PG_WARN_STREAM("Attribute location with the name '" << name<<"', not found! Make sure it is active. (is used in shader)" );
	return i;
}

int Shader::getUniformLocation(const std::string& name) const{
	const int i = glGetUniformLocation( m_GLID, name.c_str());
	if(i == INVALID_OGL_VALUE)
		 PG_WARN_STREAM("Uniform location with the name '" << name<<"', not found! Make sure it is active. (is used in shader)" );
	return i;
}

void Shader::setUniform(int location, int value) const{
	glUniform1i(location, value);
}

void Shader::setUniform(int location, float value) const{
	glUniform1f(location, value);
}

void Shader::setUniform(int location, const PG::UTIL::vec2& value) const{
	glUniform2fv(location,1, PG::UTIL::value_ptr(value));
}

void Shader::setUniform(int location, const PG::UTIL::vec3& value) const{
	glUniform3fv(location,1, PG::UTIL::value_ptr(value));
}

void Shader::setUniform(int location, const PG::UTIL::vec4& value) const{
	glUniform4fv(location,1, PG::UTIL::value_ptr(value));
}

void Shader::setUniform(int location, const PG::UTIL::mat4& value) const{
	glUniformMatrix4fv(location, 1, GL_FALSE, PG::UTIL::value_ptr(value));
}

Shader::~Shader() {
	glUseProgram(0);
	if(getGLID() != INVALID_OGL_VALUE)
		glDeleteProgram(getGLID());

}

} /* namespace GL */
} /* namespace PG */
