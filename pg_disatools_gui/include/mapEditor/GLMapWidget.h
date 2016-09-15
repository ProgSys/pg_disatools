/*
 * GLMapWidget.h
 *
 *  Created on: 10.09.2016
 *      Author: ProgSys
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
#include <openGL/PG_Box.h>
#include <openGL/PG_Texture.h>
#include <openGL/PG_Shader.h>
#include <openGL/PG_Plane.h>
#include <openGL/PG_CameraFreeFly.h>


struct mapTile{
	PG::GL::Box box;
	PG::UTIL::mat4 modelMatrix;
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
	bool openMPD( const QString& filepath );
private:
    QColor m_clearcolor = QColor(5,79,121);
    PG::GL::Texture m_tileTex;
    PG::GL::Texture m_grassTestTex;
    PG::GL::Plane m_groundGeometry;

    PG::GL::CAM::CameraFreeFly camera;
	PG::UTIL::mat4 perspectiveMatrix;

	QList<mapTile*> m_mapTiles;


    struct objectShader: public PG::GL::Shader{

    	int vertexLoc = -1;
    	int normalLoc = -1;
    	int uvLoc = -1;

    	int viewMatrixLoc = -1;
    	int projectionMatrixLoc = -1;
    	int modelMatrixLoc = -1;

    	int textureLoc = -1;

    	objectShader(){}

    	objectShader(const std::string& vert, const std::string& frag){
    		addShaderFile(PG::GL::Shader::VERTEX, vert);
    		addShaderFile(PG::GL::Shader::FRAGMENT, frag);
    	}

    	bool bind();

    	void apply(const PG::UTIL::mat4& modelMatrix, const PG::UTIL::mat4& viewMatrix, const PG::UTIL::mat4& perspectiveMatrix) const;


    } m_objectShader;
};

#endif /* INCLUDE_MAPEDITOR_GLMAPWIDGET_H_ */
