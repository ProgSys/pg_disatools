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
#include <openGL/PG_Texture.h>
#include <pg/util/PG_Base.h>

//#include <QOpenGLFunctions>
#include <QDebug>
#include <QString>

// Open GL
#if __APPLE__
	#include <OpenGL/gl3.h>
#else
	#include <GL/glew.h>
#endif

#include <openGL/PG_GLError.h>
#include <pg/files/PG_ImageFiles.h>

#if SUPPORT_QT
//#include <QGLWidget>
#endif

namespace PG {
namespace GL {

Texture::Texture() {
	// TODO Auto-generated constructor stub
}

bool Texture::create(){
	glBindTexture(GL_TEXTURE_2D,0);
	glGenTextures( 1, &m_GLID);
	if(glGetError()){
		qDebug()<<"glGenTextures returns a error! "<<m_GLID;
		if (m_GLID != INVALID_OGL_VALUE) glDeleteTextures(1, &m_GLID);
		m_GLID = INVALID_OGL_VALUE;

		return false;
	}

	glBindTexture( GL_TEXTURE_2D, m_GLID);

	return true;
}

void Texture::setTexParameter(Texture::type texType){
	switch (texType) {
		case NORMAL:
			{
			glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
			}
			break;
		case MEDIUM:
			glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			break;
		case LOW:
			glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			break;
		case SPRITE:
			glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			break;
		default:
			break;
	}
}


bool Texture::setTexture(unsigned char* imagedata, int type, const unsigned int width, const unsigned int height,bool midmap, bool freedata, bool flipYAxis){
	int bytesPerPixel = 0;
	if( type == GL_RED ||  type == GL_GREEN ||  type == GL_BLUE){
		bytesPerPixel = 1;
	}else if(type == GL_RGB || type == GL_BGR || type ==  GL_RGB ){
		bytesPerPixel = 3;
	}else if(type == GL_RGBA || type == GL_BGRA){
		bytesPerPixel = 4;
	}

	m_width = width;
	m_height = height;

	if(bytesPerPixel == 0){
		qDebug()<<"glGenTextures returns a error! "<<QString::number(m_GLID);
		//PG_ERROR_STREAM("Unable to open image, strange number of pixels.");
		return false;
		//free(imagedata);
		//imagedata = loadDefaultTexture(width,height,type);
		//bytesPerPixel = 3;
	}

    if(flipYAxis){
    		int size = width*bytesPerPixel;
    	    unsigned char * tmpBuffer = new unsigned char[size];
    	    for (unsigned int i=0;i<height/2.0;i++){
    	    	// copy row i to tmp
    	    	memcpy(tmpBuffer,imagedata+width*bytesPerPixel*i, size);
    	    	// copy row h-i-1 to i
    	    	memcpy(imagedata+width*bytesPerPixel*i,imagedata+width*bytesPerPixel*(height-i-1),size);
    	    	// copy tmp to row h-i-1
    	    	memcpy(imagedata+width*bytesPerPixel*(height-i-1),tmpBuffer, size);
    	   }
    	   delete [] tmpBuffer;
    }


    glBindTexture( GL_TEXTURE_2D, m_GLID);
    if (bytesPerPixel == 1) {
    	glTexImage2D(GL_TEXTURE_2D, 0,GL_R8, width, height, 0, type, GL_UNSIGNED_BYTE, imagedata); //GL_BGR
    }else if (bytesPerPixel < 3) {
    	qDebug()<<"Unknown data type: "<<QString::number(type)<<" ID: "<<QString::number(m_GLID);
    	if (m_GLID != INVALID_OGL_VALUE) glDeleteTextures(1, &m_GLID);
    	m_GLID = INVALID_OGL_VALUE;
        return false;
    } else if (bytesPerPixel == 3){
        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, type, GL_UNSIGNED_BYTE, imagedata); //GL_BGR
    } else if (bytesPerPixel == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, width, height, 0, type, GL_UNSIGNED_BYTE, imagedata);
    } else {
    	qDebug()<<"Unknown format for bytes per pixel... "<<QString::number(type)<<" ID: "<<QString::number(m_GLID);
        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, width, height, 0, type, GL_UNSIGNED_BYTE, imagedata);
    }

    //ImageTexture settings
    if(midmap && width > 16 && height > 16)
    	glGenerateMipmap(GL_TEXTURE_2D);


	if (freedata) delete[] imagedata;

	return true;
}

void Texture::bind(const PG::UTIL::IDImage& img, Texture::type texType){
	if (m_GLID != INVALID_OGL_VALUE) glDeleteTextures(1, &m_GLID);
	m_GLID = INVALID_OGL_VALUE;
	if(!img.getWidth() || !img.getHeight() || !create()) return;
	setTexParameter(texType);
	checkGLError();
	setTexture((unsigned char*) &img[0], GL_RED, img.getWidth(), img.getHeight(), texType != SPRITE );
	checkGLError();
}

void Texture::bind(const PG::UTIL::RGBImage& img, Texture::type texType){
	if (m_GLID != INVALID_OGL_VALUE) glDeleteTextures(1, &m_GLID);
	m_GLID = INVALID_OGL_VALUE;
	if(!img.getWidth() || !img.getHeight() || !create()) return;
	setTexParameter(texType);
	checkGLError();
	setTexture((unsigned char*) &img[0], GL_RGB, img.getWidth(), img.getHeight(), texType != SPRITE );
	checkGLError();
}

void Texture::bind(const PG::UTIL::RGBAImage& img, Texture::type texType){
	if (m_GLID != INVALID_OGL_VALUE) glDeleteTextures(1, &m_GLID);
	m_GLID = INVALID_OGL_VALUE;
	if(!img.getWidth() || !img.getHeight() || !create()) return;
	setTexParameter(texType);
	checkGLError();
	setTexture((unsigned char*) &img[0], GL_RGBA, img.getWidth(), img.getHeight(), texType != SPRITE );
	checkGLError();
}

void Texture::bind(const PG::FILE::tx2Image& img, Texture::type texType){
	if(img.header.type == PG::FILE::tx2Type::TX2ERROR) return;

	if (m_GLID != INVALID_OGL_VALUE) glDeleteTextures(1, &m_GLID);
	m_GLID = INVALID_OGL_VALUE;
	if(!img.header.width || !img.header.height || !create()) return;
	setTexParameter(texType);
	checkGLError();

	glBindTexture( GL_TEXTURE_2D, m_GLID);
	switch (img.header.type) {
		case  PG::FILE::tx2Type::DXT1:
			glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, img.header.width, img.header.height, 0, img.data.size(), &img.data[0]);
			break;
		case  PG::FILE::tx2Type::DXT5:
			glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, img.header.width, img.header.height, 0, img.data.size(), &img.data[0]);
			break;
		case  PG::FILE::tx2Type::BGRA:
			glTexImage2D(GL_TEXTURE_2D, 0,GL_BGRA, img.header.width, img.header.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &img.data[0]);
			break;
		case  PG::FILE::tx2Type::COLORTABLE_BGRA16:
		case  PG::FILE::tx2Type::COLORTABLE_BGRA256:
		case  PG::FILE::tx2Type::COLORTABLE_RGBA16:
		case  PG::FILE::tx2Type::COLORTABLE_RGBA256:
		{
			PG::UTIL::RGBAImage imgrgba;
			PG::FILE::decompressTX2(img, imgrgba);
			glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, img.header.width, img.header.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &imgrgba[0]);
		}
			break;
		default:
		{
			PG_ERROR_STREAM("Image format not supported! "<<img.header.type);
			throw "Image format not supported!";
		}
			break;
	}

    //ImageTexture settings
    if(texType != SPRITE && img.header.width > 16 && img.header.height > 16)
    	glGenerateMipmap(GL_TEXTURE_2D);

	checkGLError();
}

#if SUPPORT_QT
void Texture::bind(const QList<PG::UTIL::rgba>& colors, Texture::type texType){
	if (m_GLID != INVALID_OGL_VALUE) glDeleteTextures(1, &m_GLID);
	m_GLID = INVALID_OGL_VALUE;
	if(colors.empty() || !create()) return;
	setTexParameter(texType);
	checkGLError();
	setTexture((unsigned char*) &colors[0], GL_RGBA, colors.size(), 1, texType != SPRITE );
	checkGLError();
}

void Texture::bind(const QList<QColor>& colors, Texture::type rgba){
	std::vector<PG::UTIL::rgba> rgbaImg;
	rgbaImg.reserve(colors.size());
	for(const QColor& color: colors){
		rgbaImg.push_back(PG::UTIL::rgba(color.red(),color.green(),color.blue(),color.alpha()));
	}
	bind(rgbaImg, rgba);

}
#endif

void Texture::bind(const std::vector<PG::UTIL::rgba>& img, Texture::type texType){
	if (m_GLID != INVALID_OGL_VALUE) glDeleteTextures(1, &m_GLID);
	m_GLID = INVALID_OGL_VALUE;
	if(img.empty() || !create()) return;
	setTexParameter(texType);
	checkGLError();
	setTexture((unsigned char*) &img[0], GL_RGBA, img.size(), 1, texType != SPRITE );
	checkGLError();
}

void Texture::bindTGA(const std::string& path, Texture::type texType){
	PG::UTIL::RGBAImage img;
	if(PG::FILE::loadTGA(path,img)){
		bind(img, texType);
	}
}

void Texture::bindPNM(const std::string& path, Texture::type texType){
	PG::UTIL::RGBAImage img;
	if(PG::FILE::loadNetPNM(path,img)){
		bind(img, texType);
	}
}

void Texture::update(const PG::UTIL::IDImage& img){
	if (m_GLID != INVALID_OGL_VALUE){
		glBindTexture( GL_TEXTURE_2D, m_GLID);
		glTexImage2D(GL_TEXTURE_2D, 0,GL_R8, img.getWidth(), img.getHeight(), 0, GL_RED, GL_UNSIGNED_BYTE, &(img[0]));
		glBindTexture(GL_TEXTURE_2D, 0);
		checkGLError();
	}
}

void Texture::update(const std::vector<PG::UTIL::rgba>& img){
	if (m_GLID != INVALID_OGL_VALUE){
		glBindTexture( GL_TEXTURE_2D, m_GLID);
		glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, img.size(), 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(img[0]));
		glBindTexture(GL_TEXTURE_2D, 0);
		checkGLError();
	}
}

void Texture::changeType(Texture::type texType){
	if (m_GLID != INVALID_OGL_VALUE){
		glBindTexture( GL_TEXTURE_2D, m_GLID);
		setTexParameter(texType);
		glBindTexture(GL_TEXTURE_2D, 0);
		checkGLError();
	}
}

void Texture::clear(){
	if (m_GLID != INVALID_OGL_VALUE) glDeleteTextures(1, &m_GLID);
	m_GLID = INVALID_OGL_VALUE;
}

void Texture::apply() const{
	if (m_GLID != INVALID_OGL_VALUE)
		glBindTexture( GL_TEXTURE_2D, m_GLID);
}

void Texture::release() const{
	glBindTexture(GL_TEXTURE_2D, 0);
}

unsigned int Texture::getWidth() const{
	return m_width;
}

unsigned int Texture::getHeight() const{
	return m_height;
}

Texture::~Texture() {
	if (m_GLID != INVALID_OGL_VALUE) glDeleteTextures(1, &m_GLID);
}

} /* namespace GL */
} /* namespace PG */
