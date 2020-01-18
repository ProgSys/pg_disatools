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
#include "GenericUndo.h"

#include <SpriteData.h>

GenericUndo::GenericUndo(const QString& text, SpriteData* spriteData, const std::function<void(SpriteData*)>& undoFunc, QUndoCommand* parent):
	QUndoCommand(text, parent), m_spriteData(spriteData), m_undoFunc(undoFunc){

}

void GenericUndo::redo() {}
void GenericUndo::undo() {
	if (!m_spriteData || !m_undoFunc) return;
	m_undoFunc(m_spriteData);
};