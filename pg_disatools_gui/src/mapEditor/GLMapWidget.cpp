/*
 * GLMapWidget.cpp
 *
 *  Created on: 10.09.2016
 *      Author: ProgSys
 */

#include <mapEditor/GLMapWidget.h>
#include <QMessageBox>

#include <pg/files/PG_MPD.h>
#include <pg/files/PG_ImageFiles.h>
#include <pg/util/PG_MatrixUtil.h>
#include <QDebug>

#define MOVEMENT_SPEED 0.3f

bool GLMapWidget::objectShader::bind(){
	const bool b = PG::GL::Shader::bind();
	if(!b) return false;
	PG::GL::Shader::apply();
    vertexLoc = getAttributeLocation("vertex");
    normalLoc = getAttributeLocation("normal");
    uvLoc = getAttributeLocation("uv");

    modelMatrixLoc = getUniformLocation("modelMatrix");
    projectionMatrixLoc = getUniformLocation("projectionMatrix");
    viewMatrixLoc = getUniformLocation("viewMatrix");
    textureLoc = getUniformLocation("texture01");
    PG::GL::Shader::release();

    return true;
}

void GLMapWidget::objectShader::apply(const PG::UTIL::mat4& modelMatrix, const PG::UTIL::mat4& viewMatrix, const PG::UTIL::mat4& perspectiveMatrix) const{
	PG::GL::Shader::apply();

	PG::GL::Shader::setUniform(modelMatrixLoc, modelMatrix);
	PG::GL::Shader::setUniform(viewMatrixLoc, viewMatrix  );
	PG::GL::Shader::setUniform(projectionMatrixLoc, perspectiveMatrix);
    PG::GL::Shader::setUniform( textureLoc, 0);
}


GLMapWidget::GLMapWidget(QWidget *parent ):QOpenGLWidget(parent) {
	// TODO Auto-generated constructor stub
	setFocusPolicy(Qt::StrongFocus);
}

GLMapWidget::~GLMapWidget() {
	for(const mapTile* tile: m_mapTiles)
		delete tile;
}

void GLMapWidget::initializeGL(){
	GLenum err = glewInit();
	if(err != GLEW_OK){
		QMessageBox messageBox;
		messageBox.critical(0,"Error","Couldn't init OpenGL 3.0!");
		exit (EXIT_FAILURE);
	}

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.196,0.38,0.6588,1);

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
		PG::FILE::loadTGA("resources/materials/ground_noalpha.tga", img);
		m_tileTex.bind(img);
		PG::FILE::loadTGA("resources/materials/grass_test.tga", img);
		m_grassTestTex.bind(img);
    }
    //load geometry
    m_groundGeometry.bind(PG::UTIL::vec3(0,0,0),PG::UTIL::vec3(0,0,100),PG::UTIL::vec3(100,0,0), 100.0f );

    perspectiveMatrix = PG::UTIL::perspective(-30.f, (float)width(), (float)height(), 1.f, 100.f);

    /*
	  m_mapTiles.push_back(mapTile());
	  mapTile& t = m_mapTiles.last();

	  t.box.bind(PG::UTIL::vec3(0,1,0), PG::UTIL::vec3(0,1.2,1), PG::UTIL::vec3(1,1,0), PG::UTIL::vec3(1,1,1),
			  PG::UTIL::vec3(0,0,0), PG::UTIL::vec3(0,0,1), PG::UTIL::vec3(1,0,0), PG::UTIL::vec3(1,0,1),
			  PG::UTIL::vec2(0,0),  PG::UTIL::vec2(0,1),  PG::UTIL::vec2(1,0),  PG::UTIL::vec2(1,1),
			  PG::UTIL::vec2(0,0),  PG::UTIL::vec2(0,1),  PG::UTIL::vec2(1,0),  PG::UTIL::vec2(1,1),
			  PG::UTIL::vec2(0,0),  PG::UTIL::vec2(0,1),  PG::UTIL::vec2(1,0),  PG::UTIL::vec2(1,1)
			  );

	  t.planeTest.bind(PG::UTIL::vec3(0,1,0), PG::UTIL::vec3(0,0,1), PG::UTIL::vec3(1,0,0), 1.f);
	  */
}

void GLMapWidget::paintGL(){
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glClearColor(m_clearcolor.red()/255.0,m_clearcolor.green()/255.0,m_clearcolor.blue()/255.0,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_objectShader.apply(PG::UTIL::mat4(), camera.getVewMatrix(), perspectiveMatrix);
	glActiveTexture(GL_TEXTURE0);
	m_tileTex.apply();
	m_groundGeometry.apply();

	if(!m_mapTiles.empty()) {

		glActiveTexture(GL_TEXTURE0);
		m_grassTestTex.apply();

		for(const mapTile* tile: m_mapTiles){
			m_objectShader.apply(tile->modelMatrix, camera.getVewMatrix(), perspectiveMatrix);
			tile->box.apply();
			//tile->planeTest.apply();
		}
		m_tileTex.release();
	}

	glPopAttrib();
}

void GLMapWidget::resizeGL(int w, int h){
	perspectiveMatrix = PG::UTIL::perspective(-30.f, (float)w, (float)h, 1.f, 100.f);
}

void GLMapWidget::mousePressEvent(QMouseEvent * event){
	camera.mousePressEvent(event);
}

void GLMapWidget::mouseMoveEvent(QMouseEvent *event){
	camera.mouseMoveEvent(event);
	update();
}

void GLMapWidget::keyPressEvent(QKeyEvent * event){
	camera.keyPressEvent(event);
	update();

}

bool GLMapWidget::openMPD( const QString& filepath ){
	if(filepath.isEmpty()) return false;
	PG::FILE::MPD mpd;
	if(mpd.open(filepath.toStdString())) return false;
	//glPushAttrib(GL_ALL_ATTRIB_BITS);
	makeCurrent();
	for(const mapTile* tile: m_mapTiles)
		delete tile;
	m_mapTiles.clear();
	for(const PG::FILE::mpdfileChunk& chunk: mpd.getChunks()){
		  for(const PG::FILE::mpdfileTile& tile: chunk.tiles){

			  mapTile* t = new mapTile;

			  // chunk.info.header.map_offset_x
			  t->modelMatrix = PG::UTIL::translation((float)tile.x, 0.f, (float)-tile.z);
			  const PG::UTIL::vec2 uvStart((chunk.info.header.map_offset_x+tile.x)/10.f,(chunk.info.header.map_offset_z-tile.z)/10.f);
			  t->box.bind(PG::UTIL::vec3(0,-tile.corners[2]/10.f,0), PG::UTIL::vec3(0,-tile.corners[0]/10.f,1), PG::UTIL::vec3(1,-tile.corners[3]/10.f,0), PG::UTIL::vec3(1,-tile.corners[1]/10.f,1),
					  PG::UTIL::vec3(0,-tile.corners2[2]/10.f,0), PG::UTIL::vec3(0,-tile.corners2[0]/10.f,1), PG::UTIL::vec3(1,-tile.corners2[2]/10.f,0), PG::UTIL::vec3(1,-tile.corners2[3]/10.f,1),

					  uvStart,  uvStart+PG::UTIL::vec2(0,0.1),  uvStart+PG::UTIL::vec2(0.1,0),  uvStart+PG::UTIL::vec2(0.1,0.1),
					  //PG::UTIL::vec2(tile.textures[2].u/255.f,tile.textures[2].v/255.f),  PG::UTIL::vec2(tile.textures[0].u/255.f,tile.textures[0].v/255.f),  PG::UTIL::vec2(tile.textures[3].u/255.f,tile.textures[3].v/255.f),  PG::UTIL::vec2(tile.textures[1].u/255.f,tile.textures[1].v/255.f),
					  PG::UTIL::vec2(0,0),  PG::UTIL::vec2(0,1),  PG::UTIL::vec2(1,0),  PG::UTIL::vec2(1,1),
					  PG::UTIL::vec2(0,0),  PG::UTIL::vec2(0,1),  PG::UTIL::vec2(1,0),  PG::UTIL::vec2(1,1)

					  //
					  //PG::UTIL::vec2(tile.textures[4].u,tile.textures[4].v),  PG::UTIL::vec2(tile.textures[5].u,tile.textures[5].v),  PG::UTIL::vec2(tile.textures[6].u,tile.textures[6].v),  PG::UTIL::vec2(tile.textures[7].u,tile.textures[7].v),
					  //PG::UTIL::vec2(tile.textures[8].u,tile.textures[8].v),  PG::UTIL::vec2(tile.textures[9].u,tile.textures[9].v),  PG::UTIL::vec2(tile.textures[10].u,tile.textures[10].v),  PG::UTIL::vec2(tile.textures[11].u,tile.textures[11].v)
					  );
			  m_mapTiles.push_back(t);
			  PG_INFO_STREAM("c: "<<(int)tile.textures[1].u);
		  }
	}
	//glPopAttrib();
	qDebug()<<"Found "<<m_mapTiles.size()<<" tiles!";
	if(!m_mapTiles.empty()){
		qDebug()<<"First tile at ("<<m_mapTiles.last()->modelMatrix[3].x<<" "<<m_mapTiles.last()->modelMatrix[3].y<<" "<<m_mapTiles.last()->modelMatrix[3].z<<")";

		//viewMatrix = PG::UTIL::lookAt(PG::UTIL::vec3(m_mapTiles.last()->modelMatrix[3])+PG::UTIL::vec3(14,14,14), PG::UTIL::vec3(m_mapTiles.last()->modelMatrix[3]), PG::UTIL::vec3(0,1.f,0));
	}
	mpd.dump("mapLoadDump.txt");
	 update();
	return true;
}
