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
#include <spriteSheetEditor/GLSpriteWidget.h>
#include <QMessagebox>
#include <QImage>
#include <QFile>
#include <QTextStream>

#include <pg/files/PG_ImageFiles.h>
#include <pg/util/PG_Image.h>
#include <pg/util/PG_MatrixUtil.h>

#include <qdebug.h>
#include <QCoreApplication>


bool GLSpriteWidget::spriteShader::bind(){
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
    mirrorLoc = getUniformLocation("mirror");
    colorTableStartLoc = getUniformLocation("colortableStart");
    alphaMultLoc = getUniformLocation("alphaMult");

    PG::GL::Shader::release();
    return true;
}

void GLSpriteWidget::spriteShader::apply(const PG::UTIL::mat4& modelMatrix, const PG::UTIL::mat4& viewMatrix, const PG::UTIL::mat4& perspectiveMatrix) const{
	PG::GL::Shader::apply();

	PG::GL::Shader::setUniform(modelMatrixLoc, modelMatrix);
	PG::GL::Shader::setUniform(viewMatrixLoc, viewMatrix  );
	PG::GL::Shader::setUniform(projectionMatrixLoc, perspectiveMatrix);
    PG::GL::Shader::setUniform( idtextureLoc, 0);
    PG::GL::Shader::setUniform( colorTableLoc, 1);
    PG::GL::Shader::setUniform(alphaMultLoc, 1.0f);
}

bool GLSpriteWidget::objectShader::bind(){
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

void GLSpriteWidget::objectShader::apply(const PG::UTIL::mat4& modelMatrix, const PG::UTIL::mat4& viewMatrix, const PG::UTIL::mat4& perspectiveMatrix) const{
	PG::GL::Shader::apply();

	PG::GL::Shader::setUniform(modelMatrixLoc, modelMatrix);
	PG::GL::Shader::setUniform(viewMatrixLoc, viewMatrix  );
	PG::GL::Shader::setUniform(projectionMatrixLoc, perspectiveMatrix);
    PG::GL::Shader::setUniform( textureLoc, 0);
}

bool GLSpriteWidget::lineShader::bind(){
	const bool b = PG::GL::Shader::bind();
	if(!b) return false;
	PG::GL::Shader::apply();
    vertexLoc = getAttributeLocation("vertex");

    modelMatrixLoc = getUniformLocation("modelMatrix");
    projectionMatrixLoc = getUniformLocation("projectionMatrix");
    viewMatrixLoc = getUniformLocation("viewMatrix");
    PG::GL::Shader::release();

    return true;
}

void GLSpriteWidget::lineShader::apply(const PG::UTIL::mat4& modelMatrix, const PG::UTIL::mat4& viewMatrix, const PG::UTIL::mat4& perspectiveMatrix) const{
	PG::GL::Shader::apply();

	PG::GL::Shader::setUniform(modelMatrixLoc, modelMatrix);
	PG::GL::Shader::setUniform(viewMatrixLoc, viewMatrix  );
	PG::GL::Shader::setUniform(projectionMatrixLoc, perspectiveMatrix);
}

bool GLSpriteWidget::displayShader::bind(){
	const bool b = PG::GL::Shader::bind();
	if(!b) return false;
	PG::GL::Shader::apply();
    vertexLoc = getAttributeLocation("vertex");
    normalLoc = getAttributeLocation("normal");
    uvLoc = getAttributeLocation("uv");

    modelMatrixLoc = getUniformLocation("modelMatrix");
    projectionMatrixLoc = getUniformLocation("projectionMatrix");
    viewMatrixLoc = getUniformLocation("viewMatrix");
    textureLoc = getUniformLocation("imagetexture");
    PG::GL::Shader::release();

    return true;
}

void GLSpriteWidget::displayShader::apply(const PG::UTIL::mat4& modelMatrix, const PG::UTIL::mat4& viewMatrix, const PG::UTIL::mat4& perspectiveMatrix) const{
	PG::GL::Shader::apply();

	PG::GL::Shader::setUniform(modelMatrixLoc, modelMatrix);
	PG::GL::Shader::setUniform(viewMatrixLoc, viewMatrix  );
	PG::GL::Shader::setUniform(projectionMatrixLoc, perspectiveMatrix);
    PG::GL::Shader::setUniform( textureLoc, 0);
}



GLSpriteWidget::GLSpriteWidget(QWidget *parent): QOpenGLWidget(parent), m_clearcolor(5,79,121){
}

void GLSpriteWidget::setUpConnections(QWidget *parent){
	//connect(this, SIGNAL(animationAdded( const QString& )), parent, SLOT(addAnimation( const QString& )));

	//connect(this, SIGNAL(totalFrames( unsigned int )), parent, SLOT(setTotalFrames( unsigned int)));
	//connect(this, SIGNAL(currentFrame( unsigned int )), parent, SLOT(setCurrentFrame(  unsigned int )));
}

void GLSpriteWidget::setData(const SpriteData* spriteSheet){
	m_animationInfo.spriteData = spriteSheet;
}


void GLSpriteWidget::renderFrame(){
	update();
}

void GLSpriteWidget::renderFrame(int frame){
	if(m_currentFrame == frame) return;

	//is there a visual difference?
	for(const Layer* lay: m_animationInfo.getCurrentAnimation()->getLayers()){

		if(m_animationInfo.getCurrentKeyframe(lay,m_currentFrame) != m_animationInfo.getCurrentKeyframe(lay,frame)){
			m_currentFrame = frame;
			m_animationInfo.setFrame(frame);
			update();
			return;
		}
	}

	m_currentFrame = frame;
	m_animationInfo.setFrame(frame);
}

void GLSpriteWidget::displayExternalReferences(bool display){
	m_displayExternalReferences = display;
	update();
}

void GLSpriteWidget::displayGround(bool display){
	m_displayGround = display;
	update();
}

void GLSpriteWidget::displayShadow(bool display){
	m_displayShadow = display;
	update();
}

void GLSpriteWidget::setBackgroundColor(const QColor& color){
	m_clearcolor = color;
	update();
}

void GLSpriteWidget::updateAllColortables(){
	assert_Test("Sprite data is nullptr!", !m_animationInfo.spriteData);
	makeCurrent();

	if(m_animationInfo.colorTables.size() > m_animationInfo.spriteData->getNumberOfColortables()){
		for(unsigned int i = m_animationInfo.spriteData->getNumberOfColortables(); i < m_animationInfo.colorTables.size(); i++){
			delete m_animationInfo.colorTables[i];
		}
		m_animationInfo.colorTables.erase(m_animationInfo.colorTables.begin()+m_animationInfo.spriteData->getNumberOfColortables(),m_animationInfo.colorTables.end());
	}

	for(unsigned int i = 0; i < m_animationInfo.spriteData->getNumberOfColortables(); i++){
		if(i >= m_animationInfo.colorTables.size()){
			PG::GL::Texture* colorTable = new PG::GL::Texture();
			colorTable->bind(m_animationInfo.spriteData->getColortableGL(i), PG::GL::Texture::SPRITE);
			m_animationInfo.colorTables.push_back(colorTable);
		}else{
			 m_animationInfo.colorTables[i]->update(m_animationInfo.spriteData->getColortableGL(i));
		}
	}
	update();
}

void GLSpriteWidget::updateColortable(int index){
	if(!m_animationInfo.spriteData || index < 0 || index >= m_animationInfo.colorTables.size() || !m_animationInfo.colorTables[index]->isValid()) return;
	makeCurrent();
	m_animationInfo.colorTables[index]->update(m_animationInfo.spriteData->getColortableGL(index));
	update();
}

void GLSpriteWidget::addColortable(int index){
	if(!m_animationInfo.spriteData || index < 0) return;
	makeCurrent();
	PG::GL::Texture* colorTable = new PG::GL::Texture();
	colorTable->bind(m_animationInfo.spriteData->getColortableGL(index), PG::GL::Texture::SPRITE);
	m_animationInfo.colorTables.insert(m_animationInfo.colorTables.begin()+index, colorTable);
	update();
}

void GLSpriteWidget::removeColortable(int index){
	if(!m_animationInfo.spriteData || index < 0) return;
	makeCurrent();
	delete m_animationInfo.colorTables[index];
	m_animationInfo.colorTables.erase(m_animationInfo.colorTables.begin()+index);
	update();
}

void GLSpriteWidget::updateAllSpriteSheets(){
	assert_Test("Sprite data is nullptr!", !m_animationInfo.spriteData);
	makeCurrent();

	if(m_animationInfo.spriteSheetIDTextures.size() > m_animationInfo.spriteData->getNumberOfSpriteSheets()){
		for(unsigned int i = m_animationInfo.spriteData->getNumberOfSpriteSheets(); i < m_animationInfo.spriteSheetIDTextures.size(); i++){
			delete m_animationInfo.spriteSheetIDTextures[i];
		}
		m_animationInfo.spriteSheetIDTextures.erase(m_animationInfo.spriteSheetIDTextures.begin()+m_animationInfo.spriteData->getNumberOfSpriteSheets(),m_animationInfo.spriteSheetIDTextures.end());
	}

	for(unsigned int i = 0; i < m_animationInfo.spriteData->getNumberOfSpriteSheets(); i++){
		if(i >= m_animationInfo.spriteSheetIDTextures.size()){
			PG::GL::Texture* t = new PG::GL::Texture();
			t->bind(m_animationInfo.spriteData->getSpriteSheets()[i]->getSpriteSheet(), PG::GL::Texture::SPRITE);
			m_animationInfo.spriteSheetIDTextures.push_back(t);
		}else{
			m_animationInfo.spriteSheetIDTextures[i]->update(m_animationInfo.spriteData->getSpriteSheets()[i]->getSpriteSheet());
		}
	}


	update();
}

void GLSpriteWidget::updateSpriteSheet(int sheetID){
	if( !m_animationInfo.spriteData || sheetID < 0 || sheetID >= m_animationInfo.spriteSheetIDTextures.size() || !m_animationInfo.spriteSheetIDTextures[sheetID]->isValid()) return;
	makeCurrent();
	m_animationInfo.spriteSheetIDTextures[sheetID]->update(m_animationInfo.spriteData->getSpriteSheets()[sheetID]->getSpriteSheet());
	update();
}

void GLSpriteWidget::updateSpriteSheetAdded(){
	if(m_animationInfo.spriteSheetIDTextures.size() == m_animationInfo.spriteData->getSpriteSheets().size()) return;
	makeCurrent();
	PG::GL::Texture* t = new PG::GL::Texture();
	t->bind(m_animationInfo.spriteData->getSpriteSheets().last()->getSpriteSheet(), PG::GL::Texture::SPRITE);
	m_animationInfo.spriteSheetIDTextures.push_back(t);
}

void GLSpriteWidget::updateSpriteSheetRemove(int sheetID){
	if(sheetID >= m_animationInfo.spriteData->getSpriteSheets().size()) return;
	makeCurrent();
	PG::GL::Texture* t = m_animationInfo.spriteSheetIDTextures[sheetID];
	m_animationInfo.spriteSheetIDTextures.erase(m_animationInfo.spriteSheetIDTextures.begin()+sheetID);
	delete t;
}

void GLSpriteWidget::resetCamera(){
	viewMatrix = PG::UTIL::lookAt(PG::UTIL::vec3(1,1,1),PG::UTIL::vec3(0,0,0),PG::UTIL::vec3(0,1,0));
	m_scale = 400;
	const float wf = width()/m_scale;
	const float hf = height()/m_scale;
	perspectiveMatrix = PG::UTIL::orthogonal(-wf, wf, -hf+0.5f, hf+0.5f, -1.f, 12.0f);
	update();
}

void GLSpriteWidget::initializeGL(){
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

    //load shader
	m_spriteShader.addShaderFile(PG::GL::Shader::VERTEX, "./resources/shaders/sprite.vert");
	m_spriteShader.addShaderFile(PG::GL::Shader::FRAGMENT,"./resources/shaders/sprite.frag");

    if(!m_spriteShader.bind()){
		QMessageBox messageBox;
		messageBox.critical(0,"Error","Failed to init the 'sprite' shader!");
		exit (EXIT_FAILURE);
    }

    m_objectShader.addShaderFile(PG::GL::Shader::VERTEX, QCoreApplication::applicationDirPath()+"/resources/shaders/simple.vert");
    m_objectShader.addShaderFile(PG::GL::Shader::FRAGMENT, QCoreApplication::applicationDirPath()+"/resources/shaders/simple.frag");

    if(!m_objectShader.bind()){
		QMessageBox messageBox;
		messageBox.critical(0,"Error","Failed to init the 'simple' shader!");
		exit (EXIT_FAILURE);
    }

    m_lineShader.addShaderFile(PG::GL::Shader::VERTEX, QCoreApplication::applicationDirPath()+"/resources/shaders/line.vert");
    m_lineShader.addShaderFile(PG::GL::Shader::FRAGMENT, QCoreApplication::applicationDirPath()+"/resources/shaders/line.frag");

    if(!m_lineShader.bind()){
		QMessageBox messageBox;
		messageBox.critical(0,"Error","Failed to init the 'line' shader!");
		exit (EXIT_FAILURE);
    }

    m_displayShader.addShaderFile(PG::GL::Shader::VERTEX, QCoreApplication::applicationDirPath()+"/resources/shaders/display.vert");
    m_displayShader.addShaderFile(PG::GL::Shader::FRAGMENT, QCoreApplication::applicationDirPath()+"/resources/shaders/display.frag");

    if(!m_displayShader.bind()){
		QMessageBox messageBox;
		messageBox.critical(0,"Error","Failed to init the 'display' shader!");
		exit (EXIT_FAILURE);
    }

    //load texture
    {
		PG::UTIL::RGBAImage img;
		PG::FILE::loadTGA("./resources/materials/ground.tga", img);
		m_groundTexture.bind(img);
		PG::FILE::loadTGA("./resources/materials/shadow.tga", img);
		m_shadowTexture.bind(img);
		PG::FILE::loadTGA("./resources/materials/anchor_target.tga", img);
		m_anchorTexture.bind(img);
    }

    //load geometry
    m_spriteGeometry.bind(PG::UTIL::vec3(0,-1.f,0),PG::UTIL::vec3(1.f,0,0),PG::UTIL::vec3(0,1.f,0) );
    m_spriteOutline.bind( PG::UTIL::vec4(0,-1.f,0,1.f), PG::UTIL::vec4(1.f,-1.f,0,1.f), PG::UTIL::vec4(1.f,0,0,1.f ), PG::UTIL::vec4(0,0,0,1.f ),  PG::UTIL::vec4(0,-1.f,0,1.f));
    m_groundGeometry.bind(PG::UTIL::vec3(-5,0,-5),PG::UTIL::vec3(0,0,10),PG::UTIL::vec3(10,0,0), 10.0f );

    resetCamera();

    m_animationInfo.init();
}

void GLSpriteWidget::paintGL(){
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glClearColor(m_clearcolor.red()/255.0,m_clearcolor.green()/255.0,m_clearcolor.blue()/255.0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //ground pre
    if(m_displayGround){
    	glDepthMask(false);
		m_objectShader.apply(PG::UTIL::mat4(), viewMatrix, perspectiveMatrix);

		glActiveTexture(GL_TEXTURE0);
		m_groundTexture.apply();
		m_groundGeometry.apply();
		glDepthMask(true);
    }

    if(m_animationInfo && m_animationInfo.getCurrentAnimation()){
    	//PG_INFO_STREAM("RENDER!");
    	//sprite
    	modelMatrix = PG::UTIL::mat4();
    	glDepthFunc(GL_ALWAYS);
    	//glDepthMask(false);


    	for(const Layer* lay: m_animationInfo.getCurrentAnimation()->getLayers()){
    		if(lay->isHidden()) continue;
    		const Keyframe* keyframe = m_animationInfo.getCurrentKeyframe(lay);
    		if(keyframe){
        		if(!m_displayExternalReferences && m_animationInfo.spriteData->getCutouts()[keyframe->getCutoutID()]->isExternalSheet())
    					continue;

        		if(keyframe->isAdaptive()){
        			//http://www.andersriggelsen.dk/glblendfunc.php
        			//glEnable(GL_BLEND);
        			glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
        			glBlendEquation(GL_FUNC_ADD);
        		}

    			m_animationInfo.setCurrentModelMat(modelMatrix, keyframe);
    			m_spriteShader.apply(modelMatrix, viewMatrix, perspectiveMatrix);
        		m_animationInfo.setUniforms(m_spriteShader, keyframe);
        		m_animationInfo.apply(keyframe);
        		m_spriteGeometry.apply();

        		if(keyframe->isAdaptive())
        			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



        		if(keyframe->isSelected()){
        			//glDisable(GL_DEPTH_TEST);
        			m_lineShader.apply(modelMatrix, viewMatrix, perspectiveMatrix);
        	        m_spriteOutline.apply();

        	        PG::UTIL::mat4 anchorModelMatrix = PG::UTIL::translation(keyframe->getOffsetX()/50.0f, -keyframe->getOffsetY()/50.0f,0.0f)*PG::UTIL::scale(0.2f,0.2f,0.2f)*PG::UTIL::translation(-0.5f, 0.5f,0.0f);
        	        m_displayShader.apply(anchorModelMatrix, viewMatrix, perspectiveMatrix);
        	        glActiveTexture(GL_TEXTURE0);
        	        m_anchorTexture.apply();
        	        m_spriteGeometry.apply();
        	        //glEnable(GL_DEPTH_TEST);
        		}

    		}
    	}

    	glDepthFunc(GL_LEQUAL);
		//glDepthMask(true);
    }

    //ground
    if(m_displayGround){
		m_objectShader.apply(PG::UTIL::mat4(), viewMatrix, perspectiveMatrix);

		glActiveTexture(GL_TEXTURE0);
		m_groundTexture.apply();
		m_groundGeometry.apply();
    }

    if(m_animationInfo){
		//shadow
		if(m_displayShadow){
			modelMatrix = PG::UTIL::mat4();
			modelMatrix[0][0] = 0.5;
			modelMatrix[1][1] = 0.5;
			modelMatrix[2][2] = 0.5;

			modelMatrix[3][0] = -0.2;
			modelMatrix[3][1] = 0.02;
			modelMatrix[3][2] = -0.2;
			modelMatrix[3][3] = 1;

			PG::UTIL::mat4 rotMat(0);
			rotMat[0][0] = 1;
			rotMat[1][2] = -1;
			rotMat[2][1] = 1;
			rotMat[3][3] = 1;
			modelMatrix = modelMatrix*rotMat;

			m_objectShader.apply(modelMatrix, viewMatrix, perspectiveMatrix);
			glActiveTexture(GL_TEXTURE0);
			m_shadowTexture.apply();
			m_spriteGeometry.apply();
		}
    }

    //clean up
    m_spriteShader.release();
    m_spriteGeometry.release();

    glPopAttrib();
}

void GLSpriteWidget::resizeGL(int w, int h){
	//perspectiveMatrix = PG::UTIL::perspective(90.0f, w, h, 0.01f, 3.0f);
	const float wf = w/m_scale;
	const float hf = h/m_scale;
	perspectiveMatrix = PG::UTIL::orthogonal(-wf, wf, -hf+0.5f, hf+0.5f, -1.f, 10.0f);

}

void GLSpriteWidget::mousePressEvent(QMouseEvent * event){
	qDebug()<< "Mouse pressed: "<<event->x()<<", "<<event->y();
	if(event->buttons() & Qt::LeftButton){
		m_mouse.x = event->x();
		m_mouse.y = event->y();
	}
}

void GLSpriteWidget::mouseMoveEvent(QMouseEvent *event){
	if(event->buttons() & Qt::LeftButton){
		//qDebug()<< "Mouse move: "<<event->x()<<", "<<event->y()<<" but: "<<event->buttons();

		PG::UTIL::ivec2 mouseDelta = m_mouse - PG::UTIL::ivec2(event->x(),event->y());
		//viewMatrix = viewMatrix * PG::UTIL::translation(-mouseDiff.x/1000.0f, 0.0f, -mouseDiff.y/1000.0f);
		const PG::UTIL::vec3 vy = PG::UTIL::vec3(1,0,1)* (-(mouseDelta.y*2)/m_scale);
		const PG::UTIL::vec3 vx = PG::UTIL::vec3(1,0,-1)* (-(mouseDelta.x*2)/m_scale);
		viewMatrix = viewMatrix * PG::UTIL::translation(vy+vx);
		m_mouse.x = event->x();
		m_mouse.y = event->y();
		update();
	}
}

void GLSpriteWidget::wheelEvent ( QWheelEvent * event ){
	m_scale += event->delta();///2.0f;

	if(m_scale < 250.0f)
		m_scale = 250.f;

	if(m_scale > 900.0f)
		m_scale = 900.f;

	const float wf = width()/m_scale;
	const float hf = height()/m_scale;
	perspectiveMatrix = PG::UTIL::orthogonal(-wf, wf, -hf+0.5f, hf+0.5f, -2.f, 10.0f);
	update();
}

GLSpriteWidget::~GLSpriteWidget() {
	m_animationInfo.clearAll();
}

