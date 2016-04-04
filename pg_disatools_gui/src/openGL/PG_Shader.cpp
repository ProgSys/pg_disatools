/*
 * PG_Shader.cpp
 *
 *  Created on: 04.04.2016
 *      Author: ProgSys
 */

#include <openGL/PG_Shader.h>

#include <algorithm>
#include <pg/util/PG_Base.h>

#include <openGL/PG_GLError.h>

namespace PG {
namespace GL {

Shader::Shader() {
	// TODO Auto-generated constructor stub

}

void Shader::addShaderFile(Shader::type shaderType, const std::string& filepath){
	std::string fileContent;
	std::string line;

	//open file and "parse" input
	std::ifstream file(filepath);
	if (file.is_open()) {
		while (!file.eof()){
			getline (file, line);
			fileContent += line + "\n";
		}
		file.close();
	}
	else{
		PG_ERROR_STREAM("Unable to open shader file '" << filepath<<"'." );
		return;
	}

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
inline void checkShader(GLuint shaderID) {
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
    } else {
        PG_SUCCESS_STREAM("Shader compiled ID: "<< shaderID);
    }
}

//checks a program
inline void checkProgram(GLuint programID) {
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
    } else {
    	PG_SUCCESS_STREAM("ShaderSet linked ID: "<< programID);
    }
}


bool Shader::bind(){
	glUseProgram(0);
	if(getGLID() != INVALID_OGL_VALUE)
		glDeleteProgram(getGLID());
	m_GLID = 0;

	if(m_shaders.empty()) return false;

	std::vector<GLuint> ids(m_shaders.size());

	for(const shaderInfo& info: m_shaders){
		GLuint id = glCreateShader(info.shaderType);
		ids.push_back(id);
	    const char* source = info.text.c_str();
	    const GLint source_size = info.text.size();//strlen(source);
		glShaderSource(id, 1, &source, &source_size);
		glCompileShader(id);
		checkShader(id);
	}

	//link shader programs
	m_GLID = glCreateProgram(); //ProgramID

	for(GLuint id: ids){
		glAttachShader(m_GLID, id);
		glDeleteShader(id);
	}

	glLinkProgram(getGLID());
	checkProgram(getGLID());
	return !checkGLError();
}

void Shader::apply() const{
	glUseProgram(getGLID());
}
void Shader::release() const{
	glUseProgram(0);
}

int Shader::getAttributeLocation(const std::string& name) const{
	return glGetAttribLocation( m_GLID, name.c_str());
}

int Shader::getUniformLocation(const std::string& name) const{
	return glGetUniformLocation( m_GLID, name.c_str());
}

void Shader::setUniform(int location, int value){
	glUniform1i(m_GLID, value);
}

void Shader::setUniform(int location, float value){
	glUniform1f(m_GLID, value);
}

void Shader::setUniform(int location, const PG::UTIL::vec2& value){
	glUniform2fv(m_GLID,1, PG::UTIL::value_ptr(value));
}

void Shader::setUniform(int location, const PG::UTIL::vec3& value){
	glUniform3fv(m_GLID,1, PG::UTIL::value_ptr(value));
}

void Shader::setUniform(int location, const PG::UTIL::vec4& value){
	glUniform4fv(m_GLID,1, PG::UTIL::value_ptr(value));
}

Shader::~Shader() {
	glUseProgram(0);
	if(getGLID() != INVALID_OGL_VALUE)
		glDeleteProgram(getGLID());

}

} /* namespace GL */
} /* namespace PG */
