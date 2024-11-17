/*
 *  GNU Lesser General Public License (LGPL):
 *
 *	Copyright (C) 2024  ProgSys
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
#include "UndoSheetPixelChanges.h"

#include <SpriteData.h>
#include <glm/ext.hpp>


UndoSheetPixelChanges::UndoSheetPixelChanges(SpriteData* spriteData, unsigned int sheetIndex, QUndoCommand* parent):
	QUndoCommand("UndoSheetPixelChanges", parent), m_spriteData(spriteData), m_sheetIndex(sheetIndex) {

}

void UndoSheetPixelChanges::addColor(int x, int y, unsigned char color) {
	const glm::ivec2 at(x, y);
	if(m_pixelsOld.count(at) == 0){
		m_pixelsOld[at] = m_spriteData->getColorIndex(m_sheetIndex, x, y);
	}
	m_pixelsNew[at] = color;
}

int UndoSheetPixelChanges::id() const {
	return 3;
}
void UndoSheetPixelChanges::redo() {
	if (!m_spriteData) return;

	m_spriteData->setColorIndex(m_sheetIndex, m_pixelsNew);
}

void UndoSheetPixelChanges::undo() {
	if (!m_spriteData) return;

	m_spriteData->setColorIndex(m_sheetIndex, m_pixelsOld);
}
