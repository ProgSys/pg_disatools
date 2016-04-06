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

#include <openGL/PG_Shader.h>
#include <openGL/PG_Texture.h>
#include <openGL/PG_Plane.h>

#include <pg/files/PG_SpriteSheet.h>

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
	bool open(const QString& spriteFile);
	bool dump(const QString& filepath);

	///if png is false then tga is used
	int exportSprites(const QString& folder, const QString& type);
	///select animation
	void setAnimation(int index);
	void process();
signals:
	void animationAdded(const QString& name);
private:
    //data
    PG::FILE::SpriteSheet m_SpriteSheet;

    //render
    QTimer m_time;
    PG::UTIL::mat4 modelMatrix;
	PG::UTIL::mat4 viewMatrix;
	PG::UTIL::mat4 perspectiveMatrix;


    PG::GL::Plane m_spriteGeometry;

    PG::GL::Texture m_groundTexture;
    PG::GL::Plane m_groundGeometry;

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

	struct animation{
		unsigned int index = 0;

		std::vector<PG::FILE::keyframe> keyframes;
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

		void setTextures(PG::FILE::SpriteSheet& data){
			clear();
			spriteIDTextures.reserve(data.getNumberOfSpriteSheets());
			PG_INFO_STREAM("Number of sheets: "<<data.getNumberOfSpriteSheets());
			for(unsigned int i = 0; i < data.getNumberOfSpriteSheets(); ++i){
				PG::GL::Texture* t = new PG::GL::Texture();
				t->bind(data.getSpriteSheet(i), PG::GL::Texture::SPRITE);
				spriteIDTextures.push_back(t);
			}

			PG_INFO_STREAM("Number of color tabels: "<<data.getNumberOfColorTables());
			colorTables.reserve(data.getNumberOfColorTables());
			for(unsigned int i = 0; i < data.getNumberOfColorTables(); ++i){
				PG::GL::Texture* t = new PG::GL::Texture();
				t->bind(data.getColorTable(i), PG::GL::Texture::SPRITE);
				colorTables.push_back(t);
			}
		}

		PG::GL::Texture* getCurrentIDTexture() const{
			const PG::FILE::keyframe& key = keyframes[keyframe];
			if(key.external_sheet){
				PG_INFO_STREAM("IS EXTERNAL: "<<key.external_sheet);
				return externalSheet;
			}else
				return spriteIDTextures[key.sheet];
		}


		PG::GL::Texture* getCurrentColorTable() const{
			const PG::FILE::keyframe& key = keyframes[keyframe];
			if(key.external_sheet){
				return colorTables[0];
			}else
				return colorTables[key.colortable];
		}

		void setUniforms(GLWidget::spriteShader& shader, PG::FILE::SpriteSheet& data){
			const PG::FILE::keyframe& key = keyframes[keyframe];

			const PG::UTIL::IDImage& img = data.getSpriteSheet(key.sheet);
			shader.setUniform(shader.spriteSizeLoc, PG::UTIL::vec2(img.getWidth(), img.getHeight()));
			shader.setUniform(shader.startLoc, PG::UTIL::vec2(key.x, key.y));
			shader.setUniform(shader.sizeLoc, PG::UTIL::vec2(key.width, key.height));
			shader.setUniform(shader.scaleLoc, PG::UTIL::vec2(key.scalex, key.scaley));
		}

		void apply() const{
			glActiveTexture(GL_TEXTURE0);
			getCurrentIDTexture()->apply();
			glActiveTexture(GL_TEXTURE1);
			getCurrentColorTable()->apply();
		}

		operator bool() const{
			return !spriteIDTextures.empty() &&  !colorTables.empty() && !keyframes.empty();
		}

		void next(){
			keyframe++;
			//PG_INFO_STREAM(keyframe << " total: "<<keyframes.size());
			if(keyframe >= keyframes.size())
				keyframe = 0;
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
			keyframes.clear();
		}

		void clearAll(){
			if(externalSheet) delete externalSheet;
			externalSheet = nullptr;
			clear();
		}

	} m_currentAnimation;


};

#endif /* INCLUDE_GLWIDGET_H_ */
