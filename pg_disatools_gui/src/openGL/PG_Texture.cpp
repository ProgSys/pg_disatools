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
	if(!create()) return;
	setTexParameter(texType);
	checkGLError();
	setTexture((unsigned char*) &img[0], GL_RED, img.getWidth(), img.getHeight(), texType != SPRITE );
	checkGLError();
}

void Texture::bind(const PG::UTIL::RGBImage& img, Texture::type texType){
	if (m_GLID != INVALID_OGL_VALUE) glDeleteTextures(1, &m_GLID);
	m_GLID = INVALID_OGL_VALUE;
	if(!create()) return;
	setTexParameter(texType);
	checkGLError();
	setTexture((unsigned char*) &img[0], GL_RGB, img.getWidth(), img.getHeight(), texType != SPRITE );
	checkGLError();
}

void Texture::bind(const PG::UTIL::RGBAImage& img, Texture::type texType){
	if (m_GLID != INVALID_OGL_VALUE) glDeleteTextures(1, &m_GLID);
	m_GLID = INVALID_OGL_VALUE;
	if(!create()) return;
	setTexParameter(texType);
	checkGLError();
	setTexture((unsigned char*) &img[0], GL_RGBA, img.getWidth(), img.getHeight(), texType != SPRITE );
	checkGLError();
}

void Texture::bind(const std::vector<PG::UTIL::rgba>& img, Texture::type texType){
	if (m_GLID != INVALID_OGL_VALUE) glDeleteTextures(1, &m_GLID);
	m_GLID = INVALID_OGL_VALUE;
	if(!create()) return;
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

Texture::~Texture() {
	if (m_GLID != INVALID_OGL_VALUE) glDeleteTextures(1, &m_GLID);
}

} /* namespace GL */
} /* namespace PG */
