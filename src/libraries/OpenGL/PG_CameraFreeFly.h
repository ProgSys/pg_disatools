/*
 * CameraFreeFly.h
 *
 *  Created on: 11.09.2016
 *      Author: ProgSys
 */

#ifndef SRC_OPENGL_PG_CAMERAFREEFLY_H_
#define SRC_OPENGL_PG_CAMERAFREEFLY_H_
#include <Util/PG_Vector.h>
#include <Util/PG_Matrix.h>

#include <QKeyEvent>
#include <QMouseEvent>

namespace PG {
namespace GL {
namespace CAM {


class CameraFreeFly{
public:
	CameraFreeFly(const PG::UTIL::vec3& cameraPos = PG::UTIL::vec3(1.0f, 1.0f,  1.0f),
			const PG::UTIL::vec3& cameraLookVec = PG::UTIL::vec3(-1.0f, -1.0f, -1.0f),
			const PG::UTIL::vec3& cameraUp = PG::UTIL::vec3(0.0f, 1.0f,  0.0f));
	CameraFreeFly(const PG::GL::CAM::CameraFreeFly& cam);
	virtual ~CameraFreeFly();

	PG::UTIL::mat4 operator*(const PG::UTIL::mat4& tma4) const;
	PG::UTIL::vec4 operator*(const PG::UTIL::vec4& tvec4) const;

    void mousePressEvent(QMouseEvent * event, float deltaTime);
    void mouseMoveEvent(QMouseEvent *event, float deltaTime);
    void keyPressEvent(QKeyEvent * event, float deltaTime);

    void update();

    const PG::UTIL::mat4& getVewMatrix() const;


private:

    PG::UTIL::mat4 m_viewMat;
    PG::UTIL::ivec2 m_mouseLast;
    float m_cameraSpeed = 0.3f;
    float m_yaw = 0.0f;
    float m_pitch = 0.0f;

    PG::UTIL::vec3 m_cameraPos;
    PG::UTIL::vec3 m_cameraLookVec;
    PG::UTIL::vec3 m_cameraUp;
};

} /* namespace CAM */
} /* namespace GL */
} /* namespace PG */

#endif /* SRC_OPENGL_PG_CAMERAFREEFLY_H_ */
