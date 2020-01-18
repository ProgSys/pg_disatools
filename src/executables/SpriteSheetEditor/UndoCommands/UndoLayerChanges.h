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
#pragma once

#include <QUndoCommand>
#include <qvector.h>
#include <chrono>

class SpriteData;
class Layer;

struct BakedKeyframe {
	int start;
	int duration;
	unsigned int cutoutID;
	unsigned int colortableID;
	short anchorx;
	short anchory;
	unsigned short scalex;
	unsigned short scaley;
	short offsetx;
	short offsety;
	short rotation;
	int transparency;
	int mic;
};

class UndoLayerChanges : public QUndoCommand {
public:
	UndoLayerChanges(SpriteData* spriteData, Layer* cut, QUndoCommand* parent = nullptr);

	int id() const final override;
	bool mergeWith(const QUndoCommand* other) final override;
	void redo() final override;
	void undo() final override;

	QObject m_binder;
	std::chrono::system_clock::time_point m_created;
	SpriteData* m_spriteData = nullptr;
	Layer* m_layer;
	QVector<BakedKeyframe> m_keyframes;
};