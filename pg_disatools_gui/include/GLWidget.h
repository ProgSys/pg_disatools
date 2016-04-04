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
#ifndef INCLUDE_GLWIDGET_H_
#define INCLUDE_GLWIDGET_H_

// Open GL
#if __APPLE__
	#include <OpenGL/gl3.h>
#else
	#include <GL/glew.h>
#endif

#include <QOpenGLShader>
#include <QOpenGLShaderProgram>

#include <QOpenGLWidget>
#include <QMatrix>

#include <openGL/PG_Texture.h>
#include <openGL/PG_Plane.h>

class GLWidget : public QOpenGLWidget {
	Q_OBJECT
public:
	explicit GLWidget(QWidget *parent = 0);

    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;

    virtual ~GLWidget();
private:
    QOpenGLShaderProgram m_spriteShader;
    PG::GL::Texture m_spriteTexture;
    PG::GL::Plane m_spriteGeometry;

    struct spriteShader{
    	QMatrix4x4 modelMatrix;
    	QMatrix4x4 viewMatrix;
    	QMatrix4x4 perspectiveMatrix;

    	unsigned int vertexLoc;
    	unsigned int normalLoc;
    	unsigned int uvLoc;

    	unsigned int viewMatrixLoc;
    	unsigned int projectionMatrixLoc;
    	unsigned int modelMatrixLoc;

    	unsigned int idtextureLoc;
    } m_spriteShaderInfo;




};

#endif /* INCLUDE_GLWIDGET_H_ */
