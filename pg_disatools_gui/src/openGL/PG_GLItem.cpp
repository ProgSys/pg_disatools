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
