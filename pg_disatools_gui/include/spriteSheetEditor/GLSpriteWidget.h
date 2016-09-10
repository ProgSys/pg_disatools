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
#ifndef INCLUDE_GLSPRITEWIDGET_H_
#define INCLUDE_GLSPRITEWIDGET_H_

// Open GL
#if __APPLE__
	#include <OpenGL/gl3.h>
#else
	#include <GL/glew.h>
#endif
#include <QMouseEvent>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>

#include <QOpenGLWidget>
#include <QMatrix>
#include <QString>
#include <QTimer>
#include <QColor>

#include <openGL/PG_Shader.h>
#include <openGL/PG_Texture.h>
#include <openGL/PG_Plane.h>
#include <openGL/PG_LinePath.h>

#include <spriteSheetEditor/SpriteData.h>

#include <pg/util/PG_MatrixUtil.h>
#include <cmath>
#include <bitset>


#define PI 3.14159265
#define toRad(x) x * PI / 180.0
#define ANIMATION_SPEED 15

inline PG::UTIL::mat4 scaleMat(const Cutout* cut, const Keyframe* key){
	PG::UTIL::mat4 mat;

	mat[0][0] = (cut->getWidth()/50.0) * (key->getScaleX()/100.0);
	mat[1][1] = (cut->getHeight()/50.0) * (key->getScaleY()/100.0);
	return mat;
}

inline PG::UTIL::mat4 anchorOffsetMat(const Cutout* cut,const Keyframe* key){
	PG::UTIL::mat4 mat;

	mat[3][0] = (key->getAnchorX()/(float)cut->getWidth());
	mat[3][1] = (-key->getAnchorY()/(float)cut->getHeight());
	return mat;
}

inline PG::UTIL::mat4 positionOffsetMat(const Keyframe* key){
	PG::UTIL::mat4 mat;
	mat[3][0] = (key->getOffsetX()/50.0);
	mat[3][1] = (-key->getOffsetY()/50.0);
	return mat;
}

class GLSpriteWidget : public QOpenGLWidget {
	Q_OBJECT
public:
	explicit GLSpriteWidget(QWidget *parent = 0);
	void setUpConnections(QWidget *parent);

    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;

    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent ( QWheelEvent * event );

    virtual ~GLSpriteWidget();
public slots:
	///returns true on success
	bool open(const SpriteData* spriteSheet);

	void renderFrame(int frame);
	void renderFrame();

	void displayExternalReferences(bool display);
	void displayGround(bool display);
	void displayShadow(bool display);

	void setBackgroundColor(const QColor& color);
	void updateColortable();

	void updateSpriteSheet(int sheetID);
	void updateSpriteSheetAdded();
	void updateSpriteSheetRemove(int sheetID);

	void resetCamera();

private:
	//data
	const SpriteData* m_spriteSheet;

    //render

    PG::UTIL::mat4 modelMatrix;
	PG::UTIL::mat4 viewMatrix;
	PG::UTIL::mat4 perspectiveMatrix;

	PG::UTIL::ivec2 m_mouse;
	float m_scale = 400;

    PG::GL::Plane m_spriteGeometry;
    PG::GL::LinePath m_spriteOutline;
    PG::GL::Texture m_anchorTexture;

    PG::GL::Texture m_groundTexture;
    PG::GL::Plane m_groundGeometry;

    PG::GL::Texture m_shadowTexture;

    QColor m_clearcolor;

    //settings
    bool m_displayExternalReferences = true;
    bool m_displayGround = true;
    bool m_displayShadow = true;

    //play feedback
    unsigned int m_currentFrame = 0;

    struct spriteShader: public PG::GL::Shader{

    	int vertexLoc = -1;
    	int normalLoc = -1;
    	int uvLoc = -1;

    	int viewMatrixLoc = -1;
    	int projectionMatrixLoc = -1;
    	int modelMatrixLoc = -1;

    	int spriteSizeLoc = -1;
    	int startLoc = -1;
    	int sizeLoc = -1;
    	int mirrorLoc = -1;

    	int colorTableStartLoc = -1;
    	int alphaMultLoc = -1;

    	int idtextureLoc = -1;
    	int colorTableLoc = -1;


    	spriteShader(){}

    	spriteShader(const std::string& vert, const std::string& frag){
    		addShaderFile(PG::GL::Shader::VERTEX, vert);
    		addShaderFile(PG::GL::Shader::FRAGMENT, frag);
    	}

    	bool bind();

    	void apply(const PG::UTIL::mat4& modelMatrix, const PG::UTIL::mat4& viewMatrix, const PG::UTIL::mat4& perspectiveMatrix) const;


    } m_spriteShader;

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

    struct lineShader: public PG::GL::Shader{

    	int vertexLoc = -1;

    	int viewMatrixLoc = -1;
    	int projectionMatrixLoc = -1;
    	int modelMatrixLoc = -1;

    	lineShader(){}

    	lineShader(const std::string& vert, const std::string& frag){
    		addShaderFile(PG::GL::Shader::VERTEX, vert);
    		addShaderFile(PG::GL::Shader::FRAGMENT, frag);
    	}

    	bool bind();

    	void apply(const PG::UTIL::mat4& modelMatrix, const PG::UTIL::mat4& viewMatrix, const PG::UTIL::mat4& perspectiveMatrix) const;


    } m_lineShader;

    struct displayShader: public PG::GL::Shader{

    	int vertexLoc = -1;
    	int normalLoc = -1;
    	int uvLoc = -1;

    	int viewMatrixLoc = -1;
    	int projectionMatrixLoc = -1;
    	int modelMatrixLoc = -1;

    	int textureLoc = -1;


    	displayShader(){}

    	displayShader(const std::string& vert, const std::string& frag){
    		addShaderFile(PG::GL::Shader::VERTEX, vert);
    		addShaderFile(PG::GL::Shader::FRAGMENT, frag);
    	}

    	bool bind();

    	void apply(const PG::UTIL::mat4& modelMatrix, const PG::UTIL::mat4& viewMatrix, const PG::UTIL::mat4& perspectiveMatrix) const;


    } m_displayShader;


	struct animationInfo{
        //data
    	const SpriteData* spriteData = nullptr;

    	unsigned int frame = 0;

		std::vector<PG::GL::Texture* > spriteSheetIDTextures;
		PG::GL::Texture* colorTable = nullptr;
		PG::GL::Texture* externalSheet = nullptr;



		void init(){
			if(!externalSheet){
				externalSheet = new PG::GL::Texture();
				externalSheet->bindTGA("resources/materials/external_sheet.tga", PG::GL::Texture::SPRITE);
			}
		}

		bool open(const SpriteData* spriteSheetIn){
			spriteData = nullptr;
			clear();
			if(!spriteSheetIn)
				return false;
			spriteData = spriteSheetIn;

			setTextures();

			return true;
		}

		void setTextures(){
			spriteSheetIDTextures.reserve(spriteData->getNumberOfSpriteSheets());
			PG_INFO_STREAM("Number of cutouts: "<<spriteData->getNumberOfCutouts());
			PG_INFO_STREAM("Number of sprite sheets: "<<spriteData->getNumberOfSpriteSheets());

			for(const SpriteSheet* spriteSheet: spriteData->getSpriteSheets()){
				assert_Test("Given sprite sheet has zero size!", spriteSheet->getWidth() <= 0 || spriteSheet->getHeight() <= 0);
				PG::GL::Texture* t = new PG::GL::Texture();
				t->bind(spriteSheet->getSpriteSheet(), PG::GL::Texture::SPRITE);
				spriteSheetIDTextures.push_back(t);
			}


			//spriteSheetIDTextures[0]->changeType();

			colorTable = new PG::GL::Texture();
			colorTable->bind(spriteData->getColortableGL(), PG::GL::Texture::SPRITE);
		}

		const SpriteAnimation* getCurrentAnimation() const{
			return spriteData->getCurrentAnimation();
		}


		unsigned int getCurrentFrame() const{
			return frame;
		}

		unsigned int getNumberOfLayers() const{
			return getCurrentAnimation()->getNumberOfLayers();
		}

		const Keyframe* getCurrentKeyframe(const Layer* lay, int testframe) const{
			if(!lay) return nullptr;
			for(const Keyframe* key: lay->getKeyframes()){
				if(testframe >= key->getStart() && testframe < key->getEnd() )
					return key;
			}
			if(!lay->getKeyframes().empty() && testframe == lay->getKeyframes().last()->getEnd())
				return lay->getKeyframes().last();
			return nullptr;
		}

		const Keyframe* getCurrentKeyframe(const Layer* lay) const{
			return getCurrentKeyframe(lay,frame);
		}

		const Cutout* getCutout(unsigned short cut = 1){
			return spriteData->getCutouts()[cut];
		}

		const Cutout* getCutout(const Keyframe* key){
			return spriteData->getCutouts()[key->getCutoutID()];
		}

		void setCurrentModelMat( PG::UTIL::mat4& modelmat, const Keyframe* key){
			assert_Test("Key is nullptr!", !key);

			//modelmat = positionOffsetMat(lay)*PG::UTIL::eulerYXZ(0.f, 0.f, angle)*anchorOffsetMat(lay)*scaleMat(spriteData, lay);
			//modelmat = positionOffsetMat(key)*PG::UTIL::eulerYXZ(0.f, 0.f, angle)*scaleMat(spriteData, key)*anchorOffsetMat(key);

			const float angle = toRad(-key->getRotation());
			assert_Test("CutoutID out of bound!", key->getCutoutID() > spriteData->getCutouts().size());
			const Cutout* cut = spriteData->getCutouts()[key->getCutoutID()];

			modelmat = PG::UTIL::translation(0.0f, 0.0f, 0.2f) * positionOffsetMat(key)*PG::UTIL::eulerYXZ(0.f, 0.f, angle)*scaleMat(cut, key)*anchorOffsetMat(cut,key);
			//PG_INFO_STREAM("x: "<<lay->getOffsetX()<< " y: "<<lay->getOffsetY()<<" = ("<<modelmat[3][0]<<", "<<modelmat[3][1]<<", "<<modelmat[3][2]<<")");
		}

		void setUniforms(GLSpriteWidget::spriteShader& shader, const Keyframe* key){
			assert_Test("Key is nullptr!", !key);
			assert_Test("CutoutID out of bound!", key->getCutoutID() > spriteData->getCutouts().size());
			const Cutout* cut = spriteData->getCutouts()[key->getCutoutID()];


			shader.setUniform(shader.spriteSizeLoc, PG::UTIL::vec2(cut->getWidth(), cut->getHeight()));
			shader.setUniform(shader.startLoc, PG::UTIL::vec2(cut->getX(), cut->getY()));

			 if(cut->isExternalSheet()){
				// shader.setUniform(shader.sizeLoc, PG::UTIL::vec2(cut->getWidth(), cut->getHeight()));
				 shader.setUniform(shader.sizeLoc, PG::UTIL::vec2(externalSheet->getWidth(), externalSheet->getHeight()));
			 }else{
				const SpriteSheet* sheet = spriteData->getSpriteSheet(cut->getSheetID());
				shader.setUniform(shader.sizeLoc, PG::UTIL::vec2(sheet->getWidth(), sheet->getHeight()));
			 }

			 //mirror
			// qDebug()<< "Mirror: "<<key->isMirroredVertically()<<", "<<key->isMirroredHorizontally();
			 shader.setUniform(shader.mirrorLoc, PG::UTIL::vec2( key->isMirroredVertically(), key->isMirroredHorizontally()));
			 //transparency
			 shader.setUniform(shader.alphaMultLoc, key->getTransparency()/128.0f);

			//set colortable
			if(cut->isExternalSheet()){
				shader.setUniform(shader.colorTableStartLoc, (int)0);
			}else{
				if(key->getColortableID() >= spriteData->getNumberOfColortables()){
					shader.setUniform(shader.colorTableStartLoc, (int)0);
				}else{
					//const int colorTableSize = spriteData->getSpriteSheet(cut->getSheetID())->getSizeOfColorTable();
					//qDebug()<<QString::number(__LINE__)<<": sheet "<<QString::number(cut.sheet)<<" size "<<QString::number(spriteSheet.getColorTables().size());
					shader.setUniform(shader.colorTableStartLoc, (int)key->getColortableID()*16);
				}
			}

		}



		PG::GL::Texture* getCurrentIDTexture(const Keyframe* key) const{
			assert_Test("Key is nullptr!", !key);
			const Cutout* cut = spriteData->getCutouts()[key->getCutoutID()];


			if(cut->isExternalSheet()){
				return externalSheet;
			}else{
				assert_Test("Texture index is out of bound!", cut->getSheetID() >= spriteSheetIDTextures.size());
				return spriteSheetIDTextures[cut->getSheetID()];
			}
		}

		PG::GL::Texture* getCurrentColorTable() const{
			return colorTable;
		}

		void apply(const Keyframe* key) const{
			glActiveTexture(GL_TEXTURE0);
			getCurrentIDTexture(key)->apply();
			glActiveTexture(GL_TEXTURE1);
			getCurrentColorTable()->apply();
		}

		operator bool() const{
			return !spriteSheetIDTextures.empty() && colorTable && spriteData->getNumberOfAnimations();
		}

		void setFrame(int frameIn){
			frame = frameIn;
			if(frame > getCurrentAnimation()->getTotalFrames())
				frame = getCurrentAnimation()->getTotalFrames()-1;
			if(frame < 0)
				frame = 0;
		}

		void operator++(){
			frame++;
			if(frame > getCurrentAnimation()->getTotalFrames())
				frame = 0;
		}

		void operator++(int){
			frame++;
			if(frame > getCurrentAnimation()->getTotalFrames())
				frame = 0;
		}

		void operator--(){
			if(frame == 0)
				frame = getCurrentAnimation()->getTotalFrames()-1;
			else
				frame--;
		}

		void clear(){
			for(PG::GL::Texture* t: spriteSheetIDTextures)
				delete t;
			spriteSheetIDTextures.clear();
			if(colorTable) delete colorTable;
			colorTable = nullptr;
			frame = 0;
			//m_spriteSheet->clear();
		}

		void clearAll(){
			if(externalSheet) delete externalSheet;
			externalSheet = nullptr;
			clear();
		}

	} m_animationInfo;

};

#endif /* INCLUDE_GLSPRITEWIDGET_H_ */
