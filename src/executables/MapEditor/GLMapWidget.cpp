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
#include <GLMapWidget.h>
#include <QMessageBox>

#include <Files/PG_MPD.h>
#include <Files/PG_MPP.h>
#include <Files/PG_ImageFiles.h>
#include <Util/PG_MatrixUtil.h>
#include <QDebug>
#include <QElapsedTimer>
#include <Util/Misc/ResourcePath.h>

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

	iChannel0Loc = getUniformLocation("iChannel0");
	iChannel1Loc = getUniformLocation("iChannel1");
	shadowmapLoc = getUniformLocation("shadowmap");
	iChannel1AlphaLoc = getUniformLocation("iChannel1Alpha");

    PG::GL::Shader::release();

    return true;
}

void GLMapWidget::objectShader::apply(const PG::UTIL::mat4& modelMatrix, const PG::UTIL::mat4& viewMatrix, const PG::UTIL::mat4& perspectiveMatrix, float channel1Alpha ) const{
	PG::GL::Shader::apply();

	PG::GL::Shader::setUniform(modelMatrixLoc, modelMatrix);
	PG::GL::Shader::setUniform(viewMatrixLoc, viewMatrix  );
	PG::GL::Shader::setUniform(projectionMatrixLoc, perspectiveMatrix);
    PG::GL::Shader::setUniform(iChannel0Loc, 0);
    PG::GL::Shader::setUniform(iChannel1Loc, 1);
    PG::GL::Shader::setUniform(shadowmapLoc, 2);
    PG::GL::Shader::setUniform(iChannel1AlphaLoc, channel1Alpha);
}


GLMapWidget::GLMapWidget(QWidget *parent ):QOpenGLWidget(parent) {
	// TODO Auto-generated constructor stub
	setFocusPolicy(Qt::StrongFocus);
}

GLMapWidget::~GLMapWidget() {
	for(const mapTile* tile: m_mapTiles)
		delete tile;
	for(PG::GL::Texture* t: m_textures)
		delete t;
	for(PG::GL::Texture* t: m_normalMaps)
		delete t;
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

    m_objectShader.addShaderFile(PG::GL::Shader::VERTEX, (getResourcePath()+"/shaders/tile.vert").toStdString());
    m_objectShader.addShaderFile(PG::GL::Shader::FRAGMENT, (getResourcePath() + "/shaders/tile.frag").toStdString());

    if(!m_objectShader.bind()){
		QMessageBox messageBox;
		messageBox.critical(0,"Error","Failed to init the 'tile' shader!");
		exit (EXIT_FAILURE);
    }

    //load texture
    {
		PG::UTIL::RGBAImage img;
		PG::FILE::loadTGA(getResourcePath().toStdString() + "/materials/ground.tga", img);
		m_groundTex.bind(img);
		PG::FILE::loadTGA(getResourcePath().toStdString() + "/materials/test_texture.tga", img);
		m_tileTex.bind(img);
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
	QElapsedTimer timer;
	timer.start();

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glClearColor(m_clearcolor.red()/255.0,m_clearcolor.green()/255.0,m_clearcolor.blue()/255.0,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_objectShader.apply(PG::UTIL::mat4(), camera.getVewMatrix(), perspectiveMatrix);
	glActiveTexture(GL_TEXTURE0);
	m_groundTex.apply();
	m_groundGeometry.apply();

	if(!m_mapTiles.empty()) {



		for(const mapTile* tile: m_mapTiles){
			float iChannel0transparency = 0;
			if(tile->iChannel0Index > m_textures.size()){
				glActiveTexture(GL_TEXTURE0);
				m_tileTex.apply();
			}else{
				glActiveTexture(GL_TEXTURE0);
				if(tile->iChannel0Index == 0)
					m_tileTex.apply();
				else
					m_textures[tile->iChannel0Index-1]->apply();

				if(tile->iChannel1Index > 0){
					glActiveTexture(GL_TEXTURE1);
					m_textures[tile->iChannel1Index-1]->apply();
					iChannel0transparency = tile->transparency;
				}
			}

			m_objectShader.apply(tile->modelMatrix, camera.getVewMatrix(), perspectiveMatrix, iChannel0transparency);
			tile->topPlane.apply();
		}
		m_tileTex.release();
	}

	glPopAttrib();

	m_deltaTime = timer.elapsed();
}

void GLMapWidget::resizeGL(int w, int h){
	perspectiveMatrix = PG::UTIL::perspective(-30.f, (float)w, (float)h, 1.f, 100.f);
}

void GLMapWidget::mousePressEvent(QMouseEvent * event){
	camera.mousePressEvent(event, m_deltaTime);
}

void GLMapWidget::mouseMoveEvent(QMouseEvent *event){
	camera.mouseMoveEvent(event, m_deltaTime);
	update();
}

void GLMapWidget::keyPressEvent(QKeyEvent * event){
	camera.keyPressEvent(event, m_deltaTime);
	update();

}

bool GLMapWidget::openMap( const QString& filepath ){
	if(filepath.isEmpty()) return false;
	if(!openMPD(filepath)) return false;
	QString mppPath = filepath;
	mppPath.replace(".MPD", ".MPP");
	if(!openMPP(mppPath)) {
		QMessageBox messageBox;
		messageBox.warning(0,"*.MPP not found","The *.MPP file for map not found! Make sure it is in the same folder and has the same name!");
		return false;
	}
	return true;
}

void getTexUV(const PG::FILE::mpdfileTexdata& texdata, PG::UTIL::vec2& outStart,  PG::UTIL::vec2& outEnd){
	outStart.x = texdata.u/255.0f;
	outStart.y = texdata.v/255.0f;

	outEnd.x = outStart.x+texdata.width/255.0f;
	outEnd.y = outStart.y+texdata.height/255.0f;


	if(texdata.mirror & 0x04){
	  float b = outStart.x;
	  outStart.x = outEnd.x;
	  outEnd.x = b;
	}
	if (texdata.mirror & 0x08) {
	  float b = outStart.y;
	  outStart.y = outEnd.y;
	  outEnd.y = b;
	}
}

bool GLMapWidget::openMPD( const QString& filepath ){
	if(filepath.isEmpty()) return false;
	PG::FILE::MPD mpd;
	if(mpd.open(filepath.toStdString())) return false;
	//glPushAttrib(GL_ALL_ATTRIB_BITS);
	makeCurrent();
	for(const mapTile* tile: m_mapTiles) delete tile;
	m_mapTiles.clear();
	for(const PG::FILE::mpdfileChunk& chunk: mpd.getChunks()){
		  for(const PG::FILE::mpdfileTile& tile: chunk.tiles){

			  mapTile* t = new mapTile;

			  // chunk.info.header.map_offset_x
			  t->modelMatrix = PG::UTIL::translation((float)tile.x, 0.f, (float)-tile.z);
			  t->iChannel0Index =  tile.textures[4].textureIndex;

			  PG::UTIL::vec2 uvStartLayer0(tile.textures[4].u/255.0f, tile.textures[4].v/255.0f);
			  PG::UTIL::vec2 uvEndLayer0(uvStartLayer0.x+tile.textures[4].width/255.0f, uvStartLayer0.y+tile.textures[4].height/255.0f);

			  if(tile.textures[4].mirror & 0x04){
				  float b = uvStartLayer0.x;
				  uvStartLayer0.x = uvEndLayer0.x;
				  uvEndLayer0.x = b;
			  }
			  if (tile.textures[4].mirror & 0x08) {
				  float b = uvStartLayer0.y;
				  uvStartLayer0.y = uvEndLayer0.y;
				  uvEndLayer0.y = b;
			  }



			  t->iChannel1Index =  tile.textures[9].textureIndex;

			  PG::UTIL::vec2 uvStartLayer1;
			  PG::UTIL::vec2 uvEndLayer1;

			  getTexUV(tile.textures[9], uvStartLayer1, uvEndLayer1);
			  t->transparency = 1.f - ((tile.textures[9].mirror >> 4) & 0x0F)/15.f;
			  qDebug() <<"t: "<<((tile.textures[9].mirror >> 4) & 0x0F)/15.f;

			  /*
			  t->topPlane.bind(PG::UTIL::vec3(0,-tile.corners[2]/10.f,0), PG::UTIL::vec3(0,-tile.corners[0]/10.f,1), PG::UTIL::vec3(1,-tile.corners[3]/10.f,0), PG::UTIL::vec3(1,-tile.corners[1]/10.f,1),
					  PG::UTIL::vec3(0,-tile.corners2[2]/10.f,0), PG::UTIL::vec3(0,-tile.corners2[0]/10.f,1), PG::UTIL::vec3(1,-tile.corners2[2]/10.f,0), PG::UTIL::vec3(1,-tile.corners2[3]/10.f,1),

					  uvStartLayer1,  PG::UTIL::vec2(uvStartLayer1.x,uvEndLayer1.y),  PG::UTIL::vec2(uvEndLayer1.x,uvStartLayer1.y),  uvEndLayer1,
					  //PG::UTIL::vec2(tile.textures[2].u/255.f,tile.textures[2].v/255.f),  PG::UTIL::vec2(tile.textures[0].u/255.f,tile.textures[0].v/255.f),  PG::UTIL::vec2(tile.textures[3].u/255.f,tile.textures[3].v/255.f),  PG::UTIL::vec2(tile.textures[1].u/255.f,tile.textures[1].v/255.f),
					  PG::UTIL::vec2(0,0),  PG::UTIL::vec2(0,1),  PG::UTIL::vec2(1,0),  PG::UTIL::vec2(1,1),
					  PG::UTIL::vec2(0,0),  PG::UTIL::vec2(0,1),  PG::UTIL::vec2(1,0),  PG::UTIL::vec2(1,1)

					  //
					  //PG::UTIL::vec2(tile.textures[4].u,tile.textures[4].v),  PG::UTIL::vec2(tile.textures[5].u,tile.textures[5].v),  PG::UTIL::vec2(tile.textures[6].u,tile.textures[6].v),  PG::UTIL::vec2(tile.textures[7].u,tile.textures[7].v),
					  //PG::UTIL::vec2(tile.textures[8].u,tile.textures[8].v),  PG::UTIL::vec2(tile.textures[9].u,tile.textures[9].v),  PG::UTIL::vec2(tile.textures[10].u,tile.textures[10].v),  PG::UTIL::vec2(tile.textures[11].u,tile.textures[11].v)
					  );
					  */
			  t->topPlane.bind(PG::UTIL::vec3(0,-tile.corners[2]/10.f,0), PG::UTIL::vec3(0,-tile.corners[0]/10.f,1), PG::UTIL::vec3(1,-tile.corners[3]/10.f,0), PG::UTIL::vec3(1,-tile.corners[1]/10.f,1),
					  uvStartLayer0,  PG::UTIL::vec2(uvStartLayer0.x,uvEndLayer0.y),  PG::UTIL::vec2(uvEndLayer0.x,uvStartLayer0.y),  uvEndLayer0, tile.textures[4].mirror & 0x03,
					  uvStartLayer1,  PG::UTIL::vec2(uvStartLayer1.x,uvEndLayer1.y),  PG::UTIL::vec2(uvEndLayer1.x,uvStartLayer1.y),  uvEndLayer1, tile.textures[9].mirror & 0x03
					  );

			  m_mapTiles.push_back(t);
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


bool GLMapWidget::openMPP( const QString& filepath ){
	if(filepath.isEmpty()) return false;
	PG::FILE::MPP mpp;
	if(mpp.open(filepath.toStdString())) return false;
	makeCurrent();
	for(PG::GL::Texture* t: m_textures)
		delete t;
	m_textures.clear();
	for(unsigned int i = 0; i < mpp.getNumberOfTextures(); i++){
		PG::GL::Texture* tex = new PG::GL::Texture();
		PG::FILE::tx2Image img;
		mpp.getTexture(i, img);
		tex->bind(img);
		m_textures.push_back(tex);
	}
	update();
	return true;
}
