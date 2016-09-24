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

#ifndef INCLUDE_MAPEDITOR_GLMAPWIDGET_H_
#define INCLUDE_MAPEDITOR_GLMAPWIDGET_H_

// Open GL
#if __APPLE__
	#include <OpenGL/gl3.h>
#else
	#include <GL/glew.h>
#endif
#include <QMouseEvent>

#include <QOpenGLWidget>
#include <QMatrix>
#include <QString>
#include <QTimer>
#include <QColor>
#include <QList>

#include <pg/util/PG_MatrixUtil.h>
#include <openGL/PG_TilePlane.h>
#include <openGL/PG_Texture.h>
#include <openGL/PG_Shader.h>
#include <openGL/PG_Plane.h>
#include <openGL/PG_CameraFreeFly.h>


struct mapTile{
	PG::GL::TilePlane topPlane;
	PG::UTIL::mat4 modelMatrix;

	int iChannel0Index = -1;
	int iChannel1Index = -1;
	float transparency = 1.0f;
};

class GLMapWidget  : public QOpenGLWidget{
	Q_OBJECT
public:
	explicit GLMapWidget(QWidget *parent = 0);
	virtual ~GLMapWidget();

    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;

    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent *event);

    void keyPressEvent(QKeyEvent * event);

public slots:
	bool openMap( const QString& filepath );
	bool openMPD( const QString& filepath );
	bool openMPP( const QString& filepath );
private:
    QColor m_clearcolor = QColor(5,79,121);

    PG::GL::Texture m_groundTex;
    PG::GL::Texture m_tileTex;
    QList<PG::GL::Texture*> m_textures;
    QList<PG::GL::Texture*> m_normalMaps;

    PG::GL::Plane m_groundGeometry;

    PG::GL::CAM::CameraFreeFly camera;
	PG::UTIL::mat4 perspectiveMatrix;

	QList<mapTile*> m_mapTiles;
	float m_deltaTime = 0;


    struct objectShader: public PG::GL::Shader{

    	int vertexLoc = -1;
    	int normalLoc = -1;
    	int uvLoc = -1;

    	int viewMatrixLoc = -1;
    	int projectionMatrixLoc = -1;
    	int modelMatrixLoc = -1;

    	int iChannel0Loc = -1;
    	int iChannel1Loc = -1;
    	int shadowmapLoc = -1;
    	int iChannel1AlphaLoc = -1;

    	objectShader(){}

    	objectShader(const std::string& vert, const std::string& frag){
    		addShaderFileStd(PG::GL::Shader::VERTEX, vert);
    		addShaderFileStd(PG::GL::Shader::FRAGMENT, frag);
    	}

    	bool bind();

    	void apply(const PG::UTIL::mat4& modelMatrix, const PG::UTIL::mat4& viewMatrix, const PG::UTIL::mat4& perspectiveMatrix, float channel1Alpha = 0.0) const;


    } m_objectShader;
};

#endif /* INCLUDE_MAPEDITOR_GLMAPWIDGET_H_ */
