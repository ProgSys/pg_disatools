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

    //load shader
	m_spriteShader.addShaderFile(PG::GL::Shader::VERTEX, "resources/shaders/sprite.vert");
	m_spriteShader.addShaderFile(PG::GL::Shader::FRAGMENT, "resources/shaders/sprite.frag");

    if(!m_spriteShader.bind()){
		QMessageBox messageBox;
		messageBox.critical(0,"Error","Couldn't init shaders!");
		exit (EXIT_FAILURE);
    }

    m_spriteShader.apply();

    m_spriteShaderInfo.vertexLoc = m_spriteShader.getAttributeLocation("vertex");
    m_spriteShaderInfo.normalLoc = m_spriteShader.getAttributeLocation("normal");
    m_spriteShaderInfo.uvLoc = m_spriteShader.getAttributeLocation("uv");

    qDebug()<< "vertex loc: "<<QString::number(m_spriteShaderInfo.vertexLoc);
    qDebug()<< "normal loc: "<<QString::number( m_spriteShaderInfo.normalLoc);
    qDebug()<< "UV loc: "<<QString::number(m_spriteShaderInfo.uvLoc);

    m_spriteShaderInfo.modelMatrixLoc = m_spriteShader.getUniformLocation("modelMatrix");
    m_spriteShaderInfo.projectionMatrixLoc = m_spriteShader.getUniformLocation("projectionMatrix");
    m_spriteShaderInfo.viewMatrixLoc = m_spriteShader.getUniformLocation("viewMatrix");
    m_spriteShaderInfo.idtextureLoc = m_spriteShader.getUniformLocation("idtexture");
    qDebug()<< QString::number( m_spriteShaderInfo.modelMatrixLoc)<<", "<< QString::number( m_spriteShaderInfo.projectionMatrixLoc)<<", "
    		<< QString::number( m_spriteShaderInfo.viewMatrixLoc)<<", "<< QString::number( m_spriteShaderInfo.idtextureLoc);

    m_spriteShaderInfo.modelMatrix[3][0] = -0.1f;
    m_spriteShaderInfo.modelMatrix[3][1] = -0.1f;
   // m_spriteShader.setUniform(m_spriteShaderInfo.viewMatrixLoc, m_spriteShaderInfo.viewMatrix);

    m_spriteShader.release();

    //load texture
    {
		PG::UTIL::RGBAImage img;
		PG::FILE::loadTGA("resources/materials/test.tga", img);
		m_spriteTexture.bind(img);

		PG::FILE::loadTGA("resources/materials/ground.tga", img);
		m_groundTexture.bind(img);
    }

    //load geometry
    m_spriteGeometry.bind(PG::UTIL::vec3(0,0,0),PG::UTIL::vec3(1,0,0),PG::UTIL::vec3(0,1,0) );
    m_groundGeometry.bind(PG::UTIL::vec3(-5,0,-5),PG::UTIL::vec3(0,0,10),PG::UTIL::vec3(10,0,0), 10.0f );

    //set mat
    m_spriteShaderInfo.viewMatrix = PG::UTIL::lookAt(PG::UTIL::vec3(1,1,1),PG::UTIL::vec3(0,0,0),PG::UTIL::vec3(0,1,0));
}

void GLWidget::paintGL(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //shader
    m_spriteShader.apply();
    m_spriteShader.setUniform(m_spriteShaderInfo.modelMatrixLoc, m_spriteShaderInfo.modelMatrix);
    m_spriteShader.setUniform(m_spriteShaderInfo.viewMatrixLoc, m_spriteShaderInfo.viewMatrix  );
    m_spriteShader.setUniform(m_spriteShaderInfo.projectionMatrixLoc, m_spriteShaderInfo.perspectiveMatrix);
    glActiveTexture(GL_TEXTURE0);


    //sprite
    m_spriteShader.setUniform( m_spriteShaderInfo.idtextureLoc, 0);
    m_spriteTexture.apply();
    m_spriteGeometry.apply();

    //ground
    m_groundTexture.apply();
    m_spriteShader.setUniform(m_spriteShaderInfo.modelMatrixLoc, PG::UTIL::mat4());
    m_groundGeometry.apply();

    //clean up
    m_groundTexture.release();
    m_spriteShader.release();
}

void GLWidget::resizeGL(int w, int h){
	m_spriteShaderInfo.perspectiveMatrix = PG::UTIL::perspective(90.0f, w, h, 0.01f, 3.0f);
}

GLWidget::~GLWidget() {
	//if(m_spriteTexture) delete m_spriteTexture;
}

