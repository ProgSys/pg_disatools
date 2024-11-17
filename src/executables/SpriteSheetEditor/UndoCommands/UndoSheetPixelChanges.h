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
#pragma once

#include <QUndoCommand>
#include <glm/glm.hpp>
#include <IVec2Hash.h>

class SpriteData;


class UndoSheetPixelChanges : public QUndoCommand {
public:
	UndoSheetPixelChanges(SpriteData* spriteData, unsigned int sheetIndex, QUndoCommand* parent = nullptr);

	void addColor(int x, int y, unsigned char color);

	int id() const final override;
	void redo() final override;
	void undo() final override;

private:
	SpriteData* m_spriteData = nullptr;
	unsigned int m_sheetIndex = 0;
	std::unordered_map<glm::ivec2, unsigned char, IVec2Hash> m_pixelsOld;
	std::unordered_map<glm::ivec2, unsigned char, IVec2Hash> m_pixelsNew;
};