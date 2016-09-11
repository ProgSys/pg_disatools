/*
 * CameraFreeFly.cpp
 *
 *  Created on: 11.09.2016
 *      Author: ProgSys
 */

#include <openGL/PG_CameraFreeFly.h>
#include <pg/util/PG_MatrixUtil.h>
#include <pg/util/PG_VectorUtil.h>

#define TO_RAD(x) ( x * 3.14159265359 ) / 180.0f

namespace PG {
namespace GL {
namespace CAM {


CameraFreeFly::CameraFreeFly(const PG::UTIL::vec3& cameraPos,
		const PG::UTIL::vec3& cameraLookVec,
		const PG::UTIL::vec3& cameraUp):
			m_cameraPos(cameraPos),m_cameraLookVec(PG::UTIL::normalize(cameraLookVec)), m_cameraUp(cameraUp){
	update();
}

CameraFreeFly::CameraFreeFly(const CameraFreeFly& cam):
		m_cameraPos(cam.m_cameraPos),m_cameraLookVec(cam.m_cameraLookVec), m_cameraUp(cam.m_cameraUp){
	update();
}

CameraFreeFly::~CameraFreeFly() {
	// TODO Auto-generated destructor stub
}


UTIL::mat4 CameraFreeFly::operator*(const PG::UTIL::mat4& tma4) const{
	return m_viewMat * tma4;
}
UTIL::vec4 CameraFreeFly::operator*(const PG::UTIL::vec4& tvec4) const{
	return m_viewMat * tvec4;
}

void CameraFreeFly::mousePressEvent(QMouseEvent * event){
	if(event->buttons() & Qt::LeftButton){
		m_mouseLast.x = event->x();
		m_mouseLast.y = event->y();
	}
}
void CameraFreeFly::mouseMoveEvent(QMouseEvent *event){

    float xoffset = event->x() - m_mouseLast.x;
    float yoffset = m_mouseLast.y - event->y() ;
    m_mouseLast.x = event->x() ;
    m_mouseLast.y = event->y() ;

    float sensitivity = 0.45;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    m_yaw   += xoffset;
    m_pitch += yoffset;

    if(m_pitch > 89.0f)
    	m_pitch = 89.0f;
    if(m_pitch < -89.0f)
    	m_pitch = -89.0f;

    PG::UTIL::vec3 front;
    front.x = cos(TO_RAD(m_yaw)) * cos(TO_RAD(m_pitch));
    front.y = sin(TO_RAD(m_pitch));
    front.z = sin(TO_RAD(m_yaw)) * cos(TO_RAD(m_pitch));
    m_cameraLookVec = PG::UTIL::normalize(front);
    update();
}

void CameraFreeFly::keyPressEvent(QKeyEvent * event){
    if(event->key() == Qt::Key_W)
       m_cameraPos += m_cameraSpeed * m_cameraLookVec;
    else if(event->key() == Qt::Key_S)
    	m_cameraPos -= m_cameraSpeed * m_cameraLookVec;
    else if(event->key() == Qt::Key_A)
    	m_cameraPos -= PG::UTIL::normalize(PG::UTIL::cross(m_cameraLookVec, m_cameraUp)) * m_cameraSpeed;
    else if(event->key() == Qt::Key_D)
    	m_cameraPos += PG::UTIL::normalize(PG::UTIL::cross(m_cameraLookVec, m_cameraUp)) * m_cameraSpeed;

	update();
}

void CameraFreeFly::update(){
	m_viewMat = PG::UTIL::lookAt(m_cameraPos, m_cameraPos + m_cameraLookVec, m_cameraUp);
}

const PG::UTIL::mat4& CameraFreeFly::getVewMatrix() const{
	return m_viewMat;
}

} /* namespace CAM */
} /* namespace GL */
} /* namespace PG */
