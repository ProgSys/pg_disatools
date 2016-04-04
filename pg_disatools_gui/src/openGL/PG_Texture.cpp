/*
 * PG_Texture.cpp
 *
 *  Created on: 31.03.2016
 *      Author: ProgSys
 */

#include <openGL/PG_Texture.h>
#include <pg/util/PG_Base.h>

#include <QOpenGLFunctions>

namespace PG {
namespace GL {

Texture::Texture() {
	// TODO Auto-generated constructor stub
}

void Texture::create(){
	glBindTexture(GL_TEXTURE_2D,0);
	glGenTextures( 1, &m_GLID);
	if(glGetError()){
		//PG_ERROR_STREAM("glGenTextures returns a error! "<<m_GLID<<" "<<m_name);
		return;
	}

	glBindTexture( GL_TEXTURE_2D, m_GLID);

	/*
	if(quality == 2){
		glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	}
	//glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	//glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	//texture settings
	//glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	//glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	//glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	//glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	//glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	//glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	//glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	//glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST );

	//glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	//glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );

	//glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	//glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR );
	 */

	//BEST
	glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
}

void Texture::bind(const PG::UTIL::IDImage& img){

}

void Texture::bind(const PG::UTIL::RGBAImage& img){

}

void Texture::apply() const{

}

void Texture::release() const{

}

Texture::~Texture() {
	// TODO Auto-generated destructor stub
}

} /* namespace GL */
} /* namespace PG */
