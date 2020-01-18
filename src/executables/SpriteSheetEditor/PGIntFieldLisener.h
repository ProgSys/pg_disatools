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

#include <QObject>
#include <QQmlParserStatus>

class SpriteData;

/// Adds an undo event filter to given object. Used to override default behavior in qml TextFields.
class PGIntFieldLisener : public QObject, public QQmlParserStatus {
	Q_OBJECT

	Q_PROPERTY(SpriteData* spriteData WRITE setSpriteData);
signals:
	void mouseWheel(int delta);
public:
	using QObject::QObject;

	inline void setSpriteData(SpriteData* data) { m_spriteData = data; }

	void componentComplete() final override;
	void classBegin() final override;

	bool eventFilter(QObject* object, QEvent* event) final override;

	SpriteData* m_spriteData = nullptr;

};