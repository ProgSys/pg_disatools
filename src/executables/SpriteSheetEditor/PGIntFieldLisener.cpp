/*
 *  GNU Lesser General Public License (LGPL):
 *
 *	Copyright (C) 2020  ProgSys
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
#include "PGIntFieldLisener.h"

#include <SpriteData.h>
#include <glm/glm.hpp>

void PGIntFieldLisener::componentComplete() {
	if (QObject * p = parent()) {
		p->installEventFilter(this);
	}
}

void PGIntFieldLisener::classBegin() {

}

bool PGIntFieldLisener::eventFilter(QObject* object, QEvent* event) {
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent* e = static_cast<QKeyEvent*>(event);
		if (m_spriteData && e->matches(QKeySequence::Undo)) {
			m_spriteData->undo();
			return false;
		}
	}
	else if (event->type() == QEvent::Wheel) {
		QWheelEvent* e = static_cast<QWheelEvent*>(event);
		emit mouseWheel(glm::sign(e->delta()));
		return false;
	}
	return false;
}