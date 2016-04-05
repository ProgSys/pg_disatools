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
#include <GLWidget.h>

#include <QMessagebox>
#include <pg/files/PG_ImageFiles.h>
#include <pg/util/PG_Image.h>
#include <pg/util/PG_MatrixUtil.h>

bool GLWidget::spriteShader::bind(){
	const bool b = PG::GL::Shader::bind();
	if(!b) return false;
	PG::GL::Shader::apply();
    vertexLoc = getAttributeLocation("vertex");
    normalLoc = getAttributeLocation("normal");
    uvLoc = getAttributeLocation("uv");

    modelMatrixLoc = getUniformLocation("modelMatrix");
    projectionMatrixLoc = getUniformLocation("projectionMatrix");
    viewMatrixLoc = getUniformLocation("viewMatrix");
    idtextureLoc = getUniformLocation("idtexture");
    colorTableLoc = getUniformLocation("colorTable");

    spriteSizeLoc = getUniformLocation("spriteSize");
    startLoc = getUniformLocation("start");
    sizeLoc = getUniformLocation("size");

    PG::GL::Shader::release();
    return true;
}

void GLWidget::spriteShader::apply(const PG::UTIL::mat4& modelMatrix, const PG::UTIL::mat4& viewMatrix, const PG::UTIL::mat4& perspectiveMatrix) const{
	PG::GL::Shader::apply();

	PG::GL::Shader::setUniform(modelMatrixLoc, modelMatrix);
	PG::GL::Shader::setUniform(viewMatrixLoc, viewMatrix  );
	PG::GL::Shader::setUniform(projectionMatrixLoc, perspectiveMatrix);
    PG::GL::Shader::setUniform( idtextureLoc, 0);
    PG::GL::Shader::setUniform( colorTableLoc, 1);
}

bool GLWidget::objectShader::bind(){
	const bool b = PG::GL::Shader::bind();
	if(!b) return false;
	PG::GL::Shader::apply();
    vertexLoc = getAttributeLocation("vertex");
    normalLoc = getAttributeLocation("normal");
    uvLoc = getAttributeLocation("uv");

    modelMatrixLoc = getUniformLocation("modelMatrix");
    projectionMatrixLoc = getUniformLocation("projectionMatrix");
    viewMatrixLoc = getUniformLocation("viewMatrix");
    textureLoc = getUniformLocation("texture");
    PG::GL::Shader::release();

    return true;
}

void GLWidget::objectShader::apply(const PG::UTIL::mat4& modelMatrix, const PG::UTIL::mat4& viewMatrix, const PG::UTIL::mat4& perspectiveMatrix) const{
	PG::GL::Shader::apply();

	PG::GL::Shader::setUniform(modelMatrixLoc, modelMatrix);
	PG::GL::Shader::setUniform(viewMatrixLoc, viewMatrix  );
	PG::GL::Shader::setUniform(projectionMatrixLoc, perspectiveMatrix);
    PG::GL::Shader::setUniform( textureLoc, 0);
}


GLWidget::GLWidget(QWidget *parent): QOpenGLWidget(parent){

}

bool GLWidget::openSprite(const QString& spriteFile){
	if(spriteFile.isEmpty()) return false;

	if(m_SpriteSheet.open(spriteFile.toStdString())){
		qDebug()<<"Coudn't open sprite sheet!";
		return false;
	}

	m_currentAnimation.setTextures(m_SpriteSheet);
	setAnimation(20);

	return true;
}

void GLWidget::setAnimation(unsigned int index){
	if(m_SpriteSheet.isOpen() && index < m_SpriteSheet.getNumberOfAnimations()){
		m_currentAnimation.index = index;
		m_SpriteSheet.getKeyframes(index, m_currentAnimation.keyframes);
		m_currentAnimation.keyframe = 0;
	}
}

void GLWidget::initializeGL(){


	GLenum err = glewInit();
	if(err != GLEW_OK){
		QMessageBox messageBox;
		messageBox.critical(0,"Error","Couldn't init OpenGL 3.0!");
		exit (EXIT_FAILURE);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.196,0.38,0.6588,1);

    //load shader
	m_spriteShader.addShaderFile(PG::GL::Shader::VERTEX, "resources/shaders/sprite.vert");
	m_spriteShader.addShaderFile(PG::GL::Shader::FRAGMENT, "resources/shaders/sprite.frag");

    if(!m_spriteShader.bind()){
		QMessageBox messageBox;
		messageBox.critical(0,"Error","Failed to init the 'sprite' shader!");
		exit (EXIT_FAILURE);
    }

    m_objectShader.addShaderFile(PG::GL::Shader::VERTEX, "resources/shaders/simple.vert");
    m_objectShader.addShaderFile(PG::GL::Shader::FRAGMENT, "resources/shaders/simple.frag");

    if(!m_objectShader.bind()){
		QMessageBox messageBox;
		messageBox.critical(0,"Error","Failed to init the 'simple' shader!");
		exit (EXIT_FAILURE);
    }

    //load texture
    {
		PG::UTIL::RGBAImage img;
		//PG::FILE::loadTGA("resources/materials/test.tga", img);
		//m_spriteIDTexture.bind(img);
		//m_spriteColortable.bind(img);

		PG::FILE::loadTGA("resources/materials/ground.tga", img);
		m_groundTexture.bind(img);
    }

    //load geometry
    m_spriteGeometry.bind(PG::UTIL::vec3(-0.5f,-0.5f,0),PG::UTIL::vec3(1.f,0,0),PG::UTIL::vec3(0,1.f,0) );
    m_groundGeometry.bind(PG::UTIL::vec3(-5,0,-5),PG::UTIL::vec3(0,0,10),PG::UTIL::vec3(10,0,0), 10.0f );

    //set mat
    modelMatrix[3][0] = 0.0f;
    modelMatrix[3][1] = 0.5f;
    modelMatrix[3][2] = 0.0f;

    viewMatrix = PG::UTIL::lookAt(PG::UTIL::vec3(1,1,1),PG::UTIL::vec3(0,0,0),PG::UTIL::vec3(0,1,0));

    openSprite("C:/Users/ProgSys/Desktop/Disgaea/PC/IMY/LAHARL.SH");
}

void GLWidget::paintGL(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //ground
    m_objectShader.apply(PG::UTIL::mat4(), viewMatrix, perspectiveMatrix);

    glActiveTexture(GL_TEXTURE0);
    m_groundTexture.apply();
    m_groundGeometry.apply();

    //sprite
    if(m_currentAnimation){
		m_spriteShader.apply(modelMatrix, viewMatrix, perspectiveMatrix);
		m_currentAnimation.setUniforms(m_spriteShader, m_SpriteSheet);
		m_currentAnimation.apply();
		m_spriteGeometry.apply();
    }

    //clean up
    m_spriteShader.release();
    m_spriteGeometry.release();

    m_currentAnimation.next();
}

void GLWidget::resizeGL(int w, int h){
	//perspectiveMatrix = PG::UTIL::perspective(90.0f, w, h, 0.01f, 3.0f);
	const float wf = w/400.f;
	const float hf = h/400.f;
	perspectiveMatrix = PG::UTIL::orthogonal(-wf, wf, -hf, hf, -1.f, 10.0f);

}

GLWidget::~GLWidget() {
	m_currentAnimation.clear();
}

