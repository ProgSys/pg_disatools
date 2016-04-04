/*
 * GLWidget.cpp
 *
 *  Created on: 04.04.2016
 *      Author: ProgSys
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

