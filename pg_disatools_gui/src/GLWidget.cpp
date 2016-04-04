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





GLWidget::GLWidget(QWidget *parent): QOpenGLWidget(parent){

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

    glClearColor(0.196,0.38,0.6588,1);

    m_spriteShaderInfo.modelMatrix.ortho(rect());
	m_spriteShaderInfo.viewMatrix.ortho(rect());
	m_spriteShaderInfo.perspectiveMatrix.ortho(rect());

    //load shader
    m_spriteShader.addShaderFromSourceFile(QOpenGLShader::Vertex, "resources/shaders/sprite.vert");
    m_spriteShader.addShaderFromSourceFile(QOpenGLShader::Fragment, "resources/shaders/sprite.frag");
    if(!m_spriteShader.link()){
		QMessageBox messageBox;
		messageBox.critical(0,"Error","Couldn't init shaders!");
		exit (EXIT_FAILURE);
    }
    m_spriteShader.bind();
    m_spriteShaderInfo.vertexLoc = 0;//m_spriteShader.attributeLocation("vertex");
    m_spriteShaderInfo.normalLoc = 1;//m_spriteShader.attributeLocation("normal");
    m_spriteShaderInfo.uvLoc = 2;//m_spriteShader.attributeLocation("uv");

    qDebug()<< "vertex loc: "<<QString::number(m_spriteShaderInfo.vertexLoc);
    qDebug()<< "normal loc: "<<QString::number( m_spriteShaderInfo.normalLoc);
    qDebug()<< "UV loc: "<<QString::number(m_spriteShaderInfo.uvLoc);

    m_spriteShaderInfo.modelMatrixLoc = m_spriteShader.uniformLocation("modelMatrix");
    m_spriteShaderInfo.projectionMatrixLoc = m_spriteShader.uniformLocation("projectionMatrix");
    m_spriteShaderInfo.viewMatrixLoc = m_spriteShader.uniformLocation("viewMatrix");
    m_spriteShaderInfo.idtextureLoc = m_spriteShader.uniformLocation("idtexture");

    m_spriteShader.release();

    //load texture
    PG::UTIL::RGBAImage img;
    PG::FILE::loadTGA("resources/test.tga", img);
    m_spriteTexture.bind(img);

    //load geometry
    m_spriteGeometry.bind(PG::UTIL::vec3(0,0,0),PG::UTIL::vec3(0,0.4,0),PG::UTIL::vec3(0.4,0,0) );

}

void GLWidget::paintGL(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_spriteShader.bind();
    m_spriteShader.setUniformValue(m_spriteShaderInfo.modelMatrixLoc, m_spriteShaderInfo.modelMatrix);
    m_spriteShader.setUniformValue(m_spriteShaderInfo.viewMatrixLoc, m_spriteShaderInfo.viewMatrix);
    m_spriteShader.setUniformValue(m_spriteShaderInfo.projectionMatrixLoc, m_spriteShaderInfo.perspectiveMatrix);

    m_spriteShader.enableAttributeArray(m_spriteShaderInfo.vertexLoc);
    m_spriteShader.enableAttributeArray(m_spriteShaderInfo.normalLoc);
    m_spriteShader.enableAttributeArray(m_spriteShaderInfo.uvLoc);

    glActiveTexture(GL_TEXTURE0);
    m_spriteShader.setUniformValue( m_spriteShaderInfo.idtextureLoc, 0);
    m_spriteTexture.apply();

    m_spriteGeometry.apply();

    m_spriteTexture.release();
    m_spriteShader.release();
}

void GLWidget::resizeGL(int w, int h){

}

GLWidget::~GLWidget() {
	//if(m_spriteTexture) delete m_spriteTexture;
}

