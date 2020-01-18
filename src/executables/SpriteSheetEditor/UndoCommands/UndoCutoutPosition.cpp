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
#include "UndoCutoutPosition.h"

#include <SpriteData.h>
#include <glm/ext.hpp>

UndoCutoutPosition::UndoCutoutPosition(SpriteData* spriteData, Cutout* cut, QUndoCommand* parent) :
	QUndoCommand("UndoCutoutPosition", parent), m_spriteData(spriteData), 
	m_cutout(cut), m_x(cut->getX()), m_y(cut->getY()), m_width(cut->getWidth()), m_height(cut->getHeight())
{
	m_created = std::chrono::system_clock::now();
	QObject::connect(cut, &QObject::destroyed, &m_binder, [this]() { m_cutout = nullptr; });
}

int UndoCutoutPosition::id() const {
	return 1;
}

bool UndoCutoutPosition::mergeWith(const QUndoCommand* other) {
	if (other->id() != id()) // make sure other is also an AppendText command
		return false;
	auto command = dynamic_cast<const UndoCutoutPosition*>(other);
	if (!command || command->m_cutout != m_cutout) return false;

	const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(command->m_created - m_created).count();
	if (diff < 1000) {
		m_created = command->m_created;
		return true;
	}
	else {
		return false;
	}
}

void UndoCutoutPosition::redo() {}
void UndoCutoutPosition::undo() {
	if (!m_cutout) return;
	if (!glm::isnan(m_x) && !glm::isnan(m_y)) {
		m_cutout->setX(m_x);
		m_cutout->setY(m_y);
	}
	if (!glm::isnan(m_width) && !glm::isnan(m_height)) {
		m_cutout->setWidth(m_width);
		m_cutout->setHeight(m_height);
	}
	emit m_spriteData->updateSpriteSheetImage();
};