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
#include <QString>
#include <QTimer>
#include <QColor>

#include <openGL/PG_Shader.h>
#include <openGL/PG_Texture.h>
#include <openGL/PG_Plane.h>

#include <files/SpriteData.h>

#include <pg/util/PG_MatrixUtil.h>
#include <cmath>

#define PI 3.14159265
#define toRad(x) x * PI / 180.0
#define ANIMATION_SPEED 15

inline PG::UTIL::mat4 scaleMat(const SpriteData* ani, const Layer* lay){
	PG::UTIL::mat4 mat;
	const Cutout* cut = ani->getCutouts()[lay->getCutoutID()];
	mat[0][0] = (cut->getWidth()/50.0) * (lay->getScaleX()/100.0);
	mat[1][1] = (cut->getHeight()/50.0) * (lay->getScaleY()/100.0);
	return mat;
}

inline PG::UTIL::mat4 anchorOffsetMat(const Layer* lay){
	PG::UTIL::mat4 mat;
	mat[3][0] = (lay->getAnchorX()/50.0);
	mat[3][1] = (-lay->getAnchorY()/50.0);
	return mat;
}

inline PG::UTIL::mat4 positionOffsetMat(const Layer* lay){
	PG::UTIL::mat4 mat;
	mat[3][0] = (lay->getOffsetX()/50.0);
	mat[3][1] = (-lay->getOffsetY()/50.0);
	return mat;
}

class GLWidget : public QOpenGLWidget {
	Q_OBJECT
public:
	explicit GLWidget(QWidget *parent = 0);
	void setUpConnections(QWidget *parent);

    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;

    virtual ~GLWidget();
public slots:
	///returns true on success
	bool open(const SpriteData* spriteSheet);
	bool dump(const QString& filepath);

	///if png is false then tga is used
	int exportSprites(const QString& folder, const QString& type);

	void renderKeyframe(int index);
	void renderKeyframe();

	void displayExternalReferences(bool display);
	void displayGround(bool display);
	void displayShadow(bool display);

	void setBackgroundColor(const QColor& color);

private:
	//data
	const SpriteData* m_spriteSheet;

    //render

    PG::UTIL::mat4 modelMatrix;
	PG::UTIL::mat4 viewMatrix;
	PG::UTIL::mat4 perspectiveMatrix;

    PG::GL::Plane m_spriteGeometry;

    PG::GL::Texture m_groundTexture;
    PG::GL::Plane m_groundGeometry;

    PG::GL::Texture m_shadowTexture;

    QColor m_clearcolor;

    //settings
    bool m_displayExternalReferences = true;
    bool m_displayGround = true;
    bool m_displayShadow = true;

    //play feedback
    unsigned int m_currentKeyframe = 0;

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

	struct animationInfo{
        //data
    	const SpriteData* spriteData = nullptr;

    	unsigned int keyframe = 0;

		std::vector<PG::GL::Texture* > cutoutIDTextures;
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
			cutoutIDTextures.reserve(spriteData->getNumberOfCutouts());
			PG_INFO_STREAM("Number of cutouts: "<<spriteData->getNumberOfCutouts());
			for(const Cutout* cut: spriteData->getCutouts()){
				assert_Test("Given cutout is nullptr!", cut == nullptr);
				PG::GL::Texture* t = new PG::GL::Texture();
				t->bind(cut->getCutout(), PG::GL::Texture::SPRITE);
				cutoutIDTextures.push_back(t);
			}

			colorTable = new PG::GL::Texture();
			colorTable->bind(spriteData->getColortableGL(), PG::GL::Texture::SPRITE);
		}

		const SpriteAnimation* getCurrentAnimation() const{
			return spriteData->getCurrentAnimation();
		}

		const Keyframe* getCurrentKeyframe() const{
			const SpriteAnimation* spa = spriteData->getCurrentAnimation();
			assert_Test("Current animation is nullptr!", !spa);
			assert_Test("Keyframe out of bound!", keyframe >= spa->getNumberOfKeyframes());
			return spa->getKeyframes()[keyframe];


		}
		unsigned int getCurrentKeyframeID() const{
			return keyframe;
		}

		unsigned int getTotalKeyframes() const{
			return getCurrentAnimation()->getNumberOfKeyframes();
		}

		unsigned int getNumberOfLayers() const{
			return getCurrentKeyframe()->getNumberOfLayers();
		}

		const Cutout* getCutout(unsigned short cut = 1){
			return spriteData->getCutouts()[cut];
		}

		const Cutout* getCutout(const Layer* layer){
			return spriteData->getCutouts()[layer->getCutoutID()];
		}

		void setCurrentModelMat( PG::UTIL::mat4& modelmat, unsigned int layer = 0){
			const Layer* lay = getCurrentKeyframe()->getLayers()[layer];

			//could be multiplied out, but meh fast enogh
			const float angle = toRad(-lay->getRotation());
			modelmat = positionOffsetMat(lay)*PG::UTIL::eulerYXZ(0.f, 0.f, angle)*anchorOffsetMat(lay)*scaleMat(spriteData, lay);
			//PG_INFO_STREAM("x: "<<cut.offsetx<< " y: "<<cut.offsety<<" width: "<<((cut.width/50.0)*(cut.scalex/100.0))<<" height: "<<((cut.height/50.0)*(cut.scaley/100.0))<<" = ("<<modelmat[3][0]<<", "<<modelmat[3][1]<<", "<<modelmat[3][2]<<")");
		}

		void setUniforms(GLWidget::spriteShader& shader, unsigned int layer = 0){
			 const Keyframe* key = getCurrentKeyframe();
			 assert_Test("Keyframe has no layers!", !key->getNumberOfLayers());
			 const Layer* lay = key->getLayers()[layer];
			 const Cutout* cut = spriteData->getCutouts()[lay->getCutoutID()];

			shader.setUniform(shader.spriteSizeLoc, PG::UTIL::vec2(cut->getWidth(), cut->getHeight()));
			shader.setUniform(shader.startLoc, PG::UTIL::vec2(0, 0));
			shader.setUniform(shader.sizeLoc, PG::UTIL::vec2(cut->getWidth(), cut->getHeight()));

			//set colortable
			if(cut->isExternalSheet()){
				shader.setUniform(shader.colorTableStartLoc, (int)0);
			}else{
				if(lay->getColortableID() >= spriteData->getNumberOfColortables()){
					shader.setUniform(shader.colorTableStartLoc, (int)0);
				}else{
					//qDebug()<<QString::number(__LINE__)<<": sheet "<<QString::number(cut.sheet)<<" size "<<QString::number(spriteSheet.getColorTables().size());
					shader.setUniform(shader.colorTableStartLoc, (int)lay->getColortableID());
				}
			}

		}

		PG::GL::Texture* getCurrentIDTexture(unsigned int layer = 0) const{
			 const Keyframe* key = getCurrentKeyframe();
			 assert_Test("Keyframe has no layers!", !key->getNumberOfLayers());
			 const Layer* lay = key->getLayers()[layer];
			 const Cutout* cut = spriteData->getCutouts()[lay->getCutoutID()];


			if(cut->isExternalSheet()){
				return externalSheet;
			}else{
				assert_Test("Texture index is out of bound!", lay->getCutoutID() >= cutoutIDTextures.size());
				return cutoutIDTextures[lay->getCutoutID()];
			}
		}

		PG::GL::Texture* getCurrentColorTable(unsigned int layer = 0) const{
			return colorTable;
		}

		void apply(unsigned int layer = 0 ) const{
			glActiveTexture(GL_TEXTURE0);
			getCurrentIDTexture(layer)->apply();
			glActiveTexture(GL_TEXTURE1);
			getCurrentColorTable(layer)->apply();
		}

		operator bool() const{
			return !cutoutIDTextures.empty() && colorTable && spriteData->getNumberOfAnimations();
		}

		void setKeyframe(int index){
			keyframe = index;
			if(keyframe > getCurrentAnimation()->getNumberOfKeyframes())
				keyframe = 0;
			if(keyframe < 0)
				keyframe = getCurrentAnimation()->getNumberOfKeyframes()-1;
		}

		void operator++(){
			keyframe++;
			if(keyframe > getCurrentAnimation()->getNumberOfKeyframes())
				keyframe = 0;
		}

		void operator++(int){
			keyframe++;
			if(keyframe > getCurrentAnimation()->getNumberOfKeyframes())
				keyframe = 0;
		}

		void operator--(){
			if(keyframe == 0)
				keyframe = getCurrentAnimation()->getNumberOfKeyframes()-1;
			else
				keyframe--;
		}

		void clear(){
			for(PG::GL::Texture* t: cutoutIDTextures)
				delete t;
			cutoutIDTextures.clear();
			if(colorTable) delete colorTable;
			colorTable = nullptr;
			keyframe = 0;
			//m_spriteSheet->clear();
		}

		void clearAll(){
			if(externalSheet) delete externalSheet;
			externalSheet = nullptr;
			clear();
		}

	} m_animationInfo;

};

#endif /* INCLUDE_GLWIDGET_H_ */
