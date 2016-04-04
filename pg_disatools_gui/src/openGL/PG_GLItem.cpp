/*
 * GLItem.cpp
 *
 *  Created on: 31.03.2016
 *      Author: ProgSys
 */

#include <openGL/PG_GLItem.h>

namespace PG {
namespace GL {

GLItem::GLItem(): m_GLID(0) {}

void GLItem::apply() const{

}

void GLItem::release() const{

}

unsigned int GLItem::getGLID() const{
	return m_GLID;
}

GLItem::~GLItem() {
	// TODO Auto-generated destructor stub
}

} /* namespace GL */
} /* namespace PG */
