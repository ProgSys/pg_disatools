/*
 * GLItem.h
 *
 *  Created on: 31.03.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_PG_OPENGL_PG_GLITEM_H_
#define INCLUDE_PG_OPENGL_PG_GLITEM_H_

namespace PG {
namespace GL {

class GLItem {
public:
	GLItem();

	virtual void apply() const;
	virtual void release() const;

	unsigned int getGLID() const;

	virtual ~GLItem();
protected:
	unsigned int m_GLID;
};

} /* namespace GL */
} /* namespace PG */

#endif /* INCLUDE_PG_OPENGL_PG_GLITEM_H_ */
