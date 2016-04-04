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
#include <QOpenGLTexture>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>


GLWidget::GLWidget(QWidget *parent): QOpenGLWidget(parent){

}

void GLWidget::initializeGL(){

    glClearColor(0.196,0.38,0.6588,1);

    /*

    glBindTexture(GL_TEXTURE_2D,0);
    unsigned int GLID;
    glGenTextures( 1, &GLID);
    //if(glGetError()){
       // return;
  //  }

    glBindTexture( GL_TEXTURE_2D, GLID);

    QOpenGLShader shader(QOpenGLShader::Vertex);
    shader.compileSourceCode(code);

    QOpenGLShaderProgram program(this);
    program.addShader(&shader);
    program.link();

    program.bind();
    */

}

void GLWidget::paintGL(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLWidget::resizeGL(int w, int h){

}

GLWidget::~GLWidget() {
	// TODO Auto-generated destructor stub
}

