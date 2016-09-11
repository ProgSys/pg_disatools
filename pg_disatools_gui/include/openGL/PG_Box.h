/*
 * Box.h
 *
 *  Created on: 10.09.2016
 *      Author: ProgSys
 */

#ifndef INCLUDE_OPENGL_PG_BOX_H_
#define INCLUDE_OPENGL_PG_BOX_H_

#include <openGL/PG_GLItem.h>
#include <pg/util/PG_Vector.h>

namespace PG {
namespace GL {
class Box: public GLItem {
public:
	Box();

	/*
	 *   1----2
	 *  /|   /|
	 * 3----4 |
	 * | 5--|-6
	 * |/   |/
	 * 7----8
	 */

	bool bind(const UTIL::vec3& t1, const UTIL::vec3& t2, const UTIL::vec3& t3, const UTIL::vec3& t4,
			const UTIL::vec3& b5, const UTIL::vec3& b6, const UTIL::vec3& b7, const UTIL::vec3& b8,

			const UTIL::vec2& uvTop1   = UTIL::vec2(0,0), const UTIL::vec2& uvTop2   = UTIL::vec2(1,0), const UTIL::vec2& uvTop3   = UTIL::vec2(0,1), const UTIL::vec2& uvTop4 = UTIL::vec2(1,1),
			const UTIL::vec2& uvLeft4  = UTIL::vec2(0,0), const UTIL::vec2& uvLeft2  = UTIL::vec2(1,0), const UTIL::vec2& uvLeft8  = UTIL::vec2(0,1), const UTIL::vec2& uvLeft6 = UTIL::vec2(1,1),
			const UTIL::vec2& uvFront3 = UTIL::vec2(0,0), const UTIL::vec2& uvFront4 = UTIL::vec2(1,0), const UTIL::vec2& uvFront7 = UTIL::vec2(0,1), const UTIL::vec2& uvFront8 = UTIL::vec2(1,1)
	);

	void apply() const final;
	void release() const final;

	virtual ~Box();
private:
	unsigned int m_bufferIDs[3];
};
}
}
#endif /* INCLUDE_OPENGL_PG_BOX_H_ */
