/*
 * PG_LinePath.h
 *
 *  Created on: 19.08.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_OPENGL_PG_LINEPATH_H_
#define INCLUDE_OPENGL_PG_LINEPATH_H_

#include <openGL/PG_GLItem.h>
#include <pg/util/PG_Vector.h>
#include <vector>

namespace PG {
namespace GL {

class LinePath : public GLItem  {
public:
	LinePath();

	///Parameterform
	bool bind(const UTIL::vec4& a, const UTIL::vec4& b);
	bool bind(const UTIL::vec4& a, const UTIL::vec4& b, const UTIL::vec4& c);
	bool bind(const UTIL::vec4& a, const UTIL::vec4& b, const UTIL::vec4& c, const UTIL::vec4& d);
	bool bind(const UTIL::vec4& a, const UTIL::vec4& b, const UTIL::vec4& c, const UTIL::vec4& d, const UTIL::vec4& e);
	bool bind(const std::vector<UTIL::vec4>& points);

	void apply() const final;
	void release() const final;

	virtual ~LinePath();
private:
	unsigned int m_bufferID;
	unsigned int m_size = 0;
};

} /* namespace GL */
} /* namespace PG */

#endif /* INCLUDE_OPENGL_PG_LINEPATH_H_ */
