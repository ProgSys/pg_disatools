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

#include <pg/files/PG_SpriteSheet.h>

#include <pg/util/PG_MatrixUtil.h>
#include <cmath>

#define PI 3.14159265
#define toRad(x) x * PI / 180.0

class GLWidget : public QOpenGLWidget {
	Q_OBJECT
public:
	explicit GLWidget(QWidget *parent = 0);
	void setUpConnections(QWidget *parent);

    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;

    bool isPlaying() const;

    virtual ~GLWidget();
public slots:
	///returns true on success
	bool open(const QString& spriteFile);
	bool dump(const QString& filepath);

	///if png is false then tga is used
	int exportSprites(const QString& folder, const QString& type);
	///select animation
	void setAnimation(int index);

	void loop();

	void nextFrame();
	void previousFrame();
	void pause();
	void play();

	void displayExternalReferences(bool display);
	void displayGround(bool display);
	void displayShadow(bool display);

	void setBackgroundColor(const QColor& color);

signals:
	void animationAdded(const QString& name);
	void totalFrames(unsigned int frames);
	void currentFrame(unsigned int currFrame);
private:

    //render
    QTimer m_frame;

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
    bool m_playing = true;

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
    	int scaleLoc = -1;

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


	struct spriteInfo{
        //data
        PG::FILE::SpriteSheet spriteSheet;

    	unsigned int index = 0;
    	unsigned int keyframe = 0;

		std::vector<PG::GL::Texture* > spriteIDTextures;
		std::vector<PG::GL::Texture* > colorTables;
		PG::GL::Texture* externalSheet = nullptr;

		void init(){
			if(!externalSheet){
				externalSheet = new PG::GL::Texture();
				externalSheet->bindTGA("resources/materials/external_sheet.tga", PG::GL::Texture::SPRITE);
			}
		}

		bool open(const QString& path){
			clear();
			if(path.isEmpty() || spriteSheet.open(path.toStdString()))
				return false;

			setTextures();

			return true;

		}

		void setTextures(){
			spriteIDTextures.reserve(spriteSheet.getNumberOfSpriteSheets());
			PG_INFO_STREAM("Number of sheets: "<<spriteSheet.getNumberOfSpriteSheets());
			for(unsigned int i = 0; i < spriteSheet.getNumberOfSpriteSheets(); ++i){
				PG::GL::Texture* t = new PG::GL::Texture();
				t->bind(spriteSheet.getSpriteSheet(i), PG::GL::Texture::SPRITE);
				spriteIDTextures.push_back(t);
			}

			colorTables.reserve(spriteSheet.getNumberOfColorTables());
			for(unsigned int i = 0; i < spriteSheet.getNumberOfColorTables(); ++i){
				PG::GL::Texture* t = new PG::GL::Texture();
				t->bind(spriteSheet.getColorTable(i), PG::GL::Texture::SPRITE);
				colorTables.push_back(t);
			}
		}

		unsigned int getNumberOfLayers() const{
			const PG::FILE::animation2D::keyframe& key = spriteSheet.getAnimation(index).keyframes[keyframe];
			return key.layers.size();
		}

		unsigned int getTotalFrames() const{
			return spriteSheet.getAnimation(index).keyframes.size();
		}

		bool isExternalReference(unsigned int layer) const{
			const PG::FILE::animation2D::keyframe& key = spriteSheet.getAnimation(index).keyframes[keyframe];
			const PG::FILE::cutout& cut = key.layers[layer];
			return cut.external_sheet;
		}

		PG::GL::Texture* getCurrentIDTexture(unsigned int layer = 0) const{
			const PG::FILE::animation2D::keyframe& key = spriteSheet.getAnimation(index).keyframes[keyframe];
			const PG::FILE::cutout& cut = key.layers[layer];
			if(cut.external_sheet){
				return externalSheet;
			}else{
				assert_Test("Texture index is out of bound!", cut.sheet >= spriteIDTextures.size());
				return spriteIDTextures[cut.sheet];
			}
		}


		PG::GL::Texture* getCurrentColorTable(unsigned int layer = 0) const{
			const PG::FILE::animation2D::keyframe& key = spriteSheet.getAnimation(index).keyframes[keyframe];
			const PG::FILE::cutout& cut = key.layers[layer];
			if(cut.external_sheet){
				return colorTables[0];
			}else{
				assert_Test("Color table index is out of bound!", cut.colortable >= colorTables.size());
				return colorTables[cut.colortable];
			}
		}

		unsigned int getCurrentDelay() const{
			if(spriteSheet.isOpen())
				return spriteSheet.getAnimation(index).keyframes[keyframe].delay*10;
			else
				return 1000;
		}

		inline PG::UTIL::mat4 scaleMat(const PG::FILE::cutout& cut){
			PG::UTIL::mat4 mat;
			mat[0][0] = (cut.width/50.0) * (cut.scalex/100.0);
			mat[1][1] = (cut.height/50.0) * (cut.scaley/100.0);
			return mat;
		}

		inline PG::UTIL::mat4 mirrorMat(const PG::FILE::cutout& cut){
			PG::UTIL::mat4 mat;
			//PG_INFO_STREAM("mirror: "<<cut.mirror);
			if(cut.unkown0 == 10)
				mat[0][0] = (-1);
			return mat;
		}

		inline PG::UTIL::mat4 anchorOffsetMat(const PG::FILE::cutout& cut){
			PG::UTIL::mat4 mat;
			mat[3][0] = (cut.anchorx/50.0);
			mat[3][1] = (-cut.anchory/50.0);
			return mat;
		}

		inline PG::UTIL::mat4 positionOffsetMat(const PG::FILE::cutout& cut){
			PG::UTIL::mat4 mat;
			mat[3][0] = (cut.offsetx/50.0);
			mat[3][1] = (-cut.offsety/50.0);
			return mat;
		}

		void setCurrentModelMat( PG::UTIL::mat4& modelmat, unsigned int layer = 0){
			const PG::FILE::animation2D::keyframe& key = spriteSheet.getAnimation(index).keyframes[keyframe];
			const PG::FILE::cutout& cut = key.layers[layer];


			//could be multiplied out, but meh fast enogh
			const float angle = toRad(-cut.rotation);
			modelmat = positionOffsetMat(cut)*PG::UTIL::eulerYXZ(0.f, 0.f, angle)*mirrorMat(cut)*anchorOffsetMat(cut)*scaleMat(cut);
			//PG_INFO_STREAM("x: "<<cut.offsetx<< " y: "<<cut.offsety<<" width: "<<((cut.width/50.0)*(cut.scalex/100.0))<<" height: "<<((cut.height/50.0)*(cut.scaley/100.0))<<" = ("<<modelmat[3][0]<<", "<<modelmat[3][1]<<", "<<modelmat[3][2]<<")");
		}

		void setUniforms(GLWidget::spriteShader& shader, unsigned int layer = 0){
			assert_Test("Keyframe has no layers!", spriteSheet.getAnimation(index).keyframes[keyframe].layers.empty());
			const PG::FILE::cutout& cut = spriteSheet.getAnimation(index).keyframes[keyframe].layers[layer];

			const PG::UTIL::IDImage& img = spriteSheet.getSpriteSheet(cut.sheet);
			shader.setUniform(shader.spriteSizeLoc, PG::UTIL::vec2(img.getWidth(), img.getHeight()));
			shader.setUniform(shader.startLoc, PG::UTIL::vec2(cut.x, cut.y));
			shader.setUniform(shader.sizeLoc, PG::UTIL::vec2(cut.width, cut.height));
			shader.setUniform(shader.scaleLoc, PG::UTIL::vec2(cut.scalex, cut.scaley));
		}

		void apply(unsigned int layer = 0 ) const{
			glActiveTexture(GL_TEXTURE0);
			getCurrentIDTexture(layer)->apply();
			glActiveTexture(GL_TEXTURE1);
			getCurrentColorTable(layer)->apply();
		}

		operator bool() const{
			return !spriteIDTextures.empty() &&  !colorTables.empty() && spriteSheet.isOpen();
		}

		void next(){
			if(!spriteSheet.isOpen()) return;
			keyframe++;
			//PG_INFO_STREAM(keyframe << " total: "<<keyframes.size());
			if(keyframe >= spriteSheet.getAnimation(index).keyframes.size())
				keyframe = 0;
		}

		void previous(){
			if(!spriteSheet.isOpen()) return;
			if(keyframe == 0) keyframe = spriteSheet.getAnimation(index).keyframes.size()-1;
			else keyframe--;
		}

		void clear(){
			for(PG::GL::Texture* t: spriteIDTextures)
				delete t;
			spriteIDTextures.clear();
			for(PG::GL::Texture* t: colorTables)
				delete t;
			colorTables.clear();
			index = 0;
			keyframe = 0;
			spriteSheet.clear();
		}

		void clearAll(){
			if(externalSheet) delete externalSheet;
			externalSheet = nullptr;
			clear();
		}

	} m_current;


};

#endif /* INCLUDE_GLWIDGET_H_ */
