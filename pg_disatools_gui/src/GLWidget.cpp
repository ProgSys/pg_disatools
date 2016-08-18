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
#include <QImage>
#include <QFile>
#include <QTextStream>

#include <pg/files/PG_ImageFiles.h>
#include <pg/util/PG_Image.h>
#include <pg/util/PG_MatrixUtil.h>

#include <qdebug.h>


bool GLWidget::spriteShader::bind(){
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

    PG::GL::Shader::release();
    return true;
}

void GLWidget::spriteShader::apply(const PG::UTIL::mat4& modelMatrix, const PG::UTIL::mat4& viewMatrix, const PG::UTIL::mat4& perspectiveMatrix) const{
	PG::GL::Shader::apply();

	PG::GL::Shader::setUniform(modelMatrixLoc, modelMatrix);
	PG::GL::Shader::setUniform(viewMatrixLoc, viewMatrix  );
	PG::GL::Shader::setUniform(projectionMatrixLoc, perspectiveMatrix);
    PG::GL::Shader::setUniform( idtextureLoc, 0);
    PG::GL::Shader::setUniform( colorTableLoc, 1);
}

bool GLWidget::objectShader::bind(){
	const bool b = PG::GL::Shader::bind();
	if(!b) return false;
	PG::GL::Shader::apply();
    vertexLoc = getAttributeLocation("vertex");
    normalLoc = getAttributeLocation("normal");
    uvLoc = getAttributeLocation("uv");

    modelMatrixLoc = getUniformLocation("modelMatrix");
    projectionMatrixLoc = getUniformLocation("projectionMatrix");
    viewMatrixLoc = getUniformLocation("viewMatrix");
    textureLoc = getUniformLocation("texture");
    PG::GL::Shader::release();

    return true;
}

void GLWidget::objectShader::apply(const PG::UTIL::mat4& modelMatrix, const PG::UTIL::mat4& viewMatrix, const PG::UTIL::mat4& perspectiveMatrix) const{
	PG::GL::Shader::apply();

	PG::GL::Shader::setUniform(modelMatrixLoc, modelMatrix);
	PG::GL::Shader::setUniform(viewMatrixLoc, viewMatrix  );
	PG::GL::Shader::setUniform(projectionMatrixLoc, perspectiveMatrix);
    PG::GL::Shader::setUniform( textureLoc, 0);
}


GLWidget::GLWidget(QWidget *parent): QOpenGLWidget(parent), m_clearcolor(5,79,121),m_spriteSheet(nullptr){
}

void GLWidget::setUpConnections(QWidget *parent){
	//connect(this, SIGNAL(animationAdded( const QString& )), parent, SLOT(addAnimation( const QString& )));

	//connect(this, SIGNAL(totalFrames( unsigned int )), parent, SLOT(setTotalFrames( unsigned int)));
	//connect(this, SIGNAL(currentFrame( unsigned int )), parent, SLOT(setCurrentFrame(  unsigned int )));
}

bool GLWidget::open(const SpriteData* spriteSheet){

	m_spriteSheet = spriteSheet;
	if(!m_spriteSheet)
		return false;

	if(!m_animationInfo.open(spriteSheet)){
		qDebug()<<"Coudn't load sprites for open GL!";
		return false;
	}

	return true;
}

bool GLWidget::dump(const QString& filepath){
	if(!m_spriteSheet || filepath.isEmpty() || !m_animationInfo) return false;

	QFile file(filepath);
	if(file.open(QIODevice::WriteOnly)){
		QTextStream out(&file);
		std::stringstream o;
		//TODO
		o << "TODO";
		out << QString::fromStdString(o.str());
		file.close();
	}else{
		file.close();
		return false;
	}
	return true;
}

int GLWidget::exportSprites(const QString& folder, const QString& type ){
	if(!m_spriteSheet) return 0;
	//TODO
	/*
	bool png = false;
	if(type == "PNG"){
		png = true;
	}else if(type != "TGA")
		return 0;
	if(folder.isEmpty() || !m_animationInfo) return 0;

	//convertToRGBA
	std::vector< PG::UTIL::RGBAImage > images;
	for(const PG::UTIL::IDImage& sheetIDs: m_spriteSheet->getSheets()){
		images.push_back(PG::UTIL::RGBAImage(sheetIDs.getWidth(), sheetIDs.getHeight()));
		//just in case there are some undefined values
		for(unsigned int i = 0; i < sheetIDs.size(); ++i)
			images.back()[i] = m_spriteSheet->getColortable()[sheetIDs[i]];
	}

	for(const PG::FILE::animation& ani: m_spriteSheet->getAnimations()){
		for(const PG::FILE::keyframe& key: ani.keyframes){
			for(const PG::FILE::layer& lay: key.layers){
				const PG::FILE::cutout& cut = m_spriteSheet->getCutout(lay.cutoutID);
				if(cut.isExternalSheet || cut.sheetID >= m_spriteSheet->getNumberOfSheets()) continue;

				const PG::UTIL::uvec2 dim(cut.width,cut.height);
				const PG::UTIL::uvec2 start(cut.x,cut.y);
				const PG::UTIL::IDImage& sheetIDs = m_spriteSheet->getSheet(cut.sheetID);

				const int width = (start.x+dim.x > sheetIDs.getWidth())? sheetIDs.getWidth()-start.x : dim.x;
				const int height = (start.y+dim.y > sheetIDs.getHeight())? sheetIDs.getHeight()-start.y : dim.y;

				if(width <= 0 || height <= 0)
					continue;

				const PG::FILE::ColorTable& colortabel =  m_spriteSheet->getColortable();
				PG::UTIL::RGBAImage& imageOut = images[cut.sheetID];

				PG::UTIL::IDImage sheetIDsWindow;
				sheetIDs.getWindow(start, PG::UTIL::uvec2(width, height), sheetIDsWindow);

				PG::UTIL::RGBAImage rgbaWindow(sheetIDsWindow.getWidth(), sheetIDsWindow.getHeight());
				for(unsigned int i = 0; i < sheetIDsWindow.size(); ++i){
					rgbaWindow[i] = colortabel[lay.colortableID*16 + sheetIDsWindow[i]];
				}

				imageOut.setWindow(start, rgbaWindow);
			}
		}
	}

	int imgCount = 0;
	if(png){
		for(const PG::UTIL::RGBAImage& image: images){
			QImage qimg( &(image[0].r), image.getWidth() , image.getHeight(), QImage::Format_RGBA8888 );
			qimg.save(folder+"/"+QString::fromStdString( m_spriteSheet->getOpenedFile().getName()) + "_"+QString::number(imgCount)+".png", 0, 100);
			imgCount++;
		}

	}else{
		for(const PG::UTIL::RGBAImage& image: images){
			std::stringstream o;
			o<< folder.toStdString()<<"/"<< m_spriteSheet->getOpenedFile().getName()<<"_"<<imgCount<<".tga";
			PG::FILE::saveTGA(o.str(),image);
			imgCount++;
		}
	}

	return imgCount;
	*/
	return false;
}


void GLWidget::renderFrame(){
	update();
}

void GLWidget::renderFrame(int frame){
	if(m_currentFrame != frame){
		m_currentFrame = frame;
		m_animationInfo.setFrame(frame);
		update();
	}
}

void GLWidget::displayExternalReferences(bool display){
	m_displayExternalReferences = display;
	update();
}

void GLWidget::displayGround(bool display){
	m_displayGround = display;
	update();
}

void GLWidget::displayShadow(bool display){
	m_displayShadow = display;
	update();
}

void GLWidget::setBackgroundColor(const QColor& color){
	m_clearcolor = color;
	update();
}

void GLWidget::initializeGL(){
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
	m_spriteShader.addShaderFile(PG::GL::Shader::VERTEX, "resources/shaders/sprite.vert");
	m_spriteShader.addShaderFile(PG::GL::Shader::FRAGMENT, "resources/shaders/sprite.frag");

    if(!m_spriteShader.bind()){
		QMessageBox messageBox;
		messageBox.critical(0,"Error","Failed to init the 'sprite' shader!");
		exit (EXIT_FAILURE);
    }

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
		PG::FILE::loadTGA("resources/materials/ground.tga", img);
		m_groundTexture.bind(img);
		PG::FILE::loadTGA("resources/materials/shadow.tga", img);
		m_shadowTexture.bind(img);
    }

    //load geometry
    m_spriteGeometry.bind(PG::UTIL::vec3(0,-1.f,0),PG::UTIL::vec3(1.f,0,0),PG::UTIL::vec3(0,1.f,0) );
    m_groundGeometry.bind(PG::UTIL::vec3(-5,0,-5),PG::UTIL::vec3(0,0,10),PG::UTIL::vec3(10,0,0), 10.0f );

    viewMatrix = PG::UTIL::lookAt(PG::UTIL::vec3(1,1,1),PG::UTIL::vec3(0,0,0),PG::UTIL::vec3(0,1,0));

    m_animationInfo.init();
}

void GLWidget::paintGL(){
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

    if(m_animationInfo){
    	//sprite
    	modelMatrix = PG::UTIL::mat4();
    	glDepthFunc(GL_ALWAYS);
    	//glDepthMask(false);

    	for(const Layer* lay: m_animationInfo.getCurrentAnimation()->getLayers()){
    		//PG_INFO_STREAM("Render Layer '"<<lay->getName().toStdString()<<"'!");
    		const Keyframe* keyframe = m_animationInfo.getCurrentKeyframe(lay);
    		if(keyframe){
        		if(!m_displayExternalReferences && m_spriteSheet->getCutouts()[keyframe->getCutoutID()]->isExternalSheet())
    					continue;
        		//PG_INFO_STREAM("Render Keyframe!");
    			m_animationInfo.setCurrentModelMat(modelMatrix, keyframe);
    			m_spriteShader.apply(modelMatrix, viewMatrix, perspectiveMatrix);
        		m_animationInfo.setUniforms(m_spriteShader, keyframe);
        		m_animationInfo.apply(keyframe);
        		m_spriteGeometry.apply();
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

void GLWidget::resizeGL(int w, int h){
	//perspectiveMatrix = PG::UTIL::perspective(90.0f, w, h, 0.01f, 3.0f);
	const float wf = w/400.f;
	const float hf = h/400.f;
	perspectiveMatrix = PG::UTIL::orthogonal(-wf, wf, -hf+0.5f, hf+0.5f, -1.f, 10.0f);

}


GLWidget::~GLWidget() {
	m_animationInfo.clearAll();
}

