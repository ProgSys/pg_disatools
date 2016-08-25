/*
 *  GNU Lesser General Public License (LGPL):
 *
 *	Copyright (C) 2016  ProgSys
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
#ifndef INCLUDE_SPRITESHEETEDITOR_SPRITEVIEWIMAGEPROVIDER_H_
#define INCLUDE_SPRITESHEETEDITOR_SPRITEVIEWIMAGEPROVIDER_H_

#include <QImage>
#include <QQuickImageProvider>
#include <files/SpriteData.h>

class SpriteViewImageProvider : public QQuickImageProvider{
public:
	SpriteViewImageProvider(SpriteData* data = nullptr);

	QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);

	virtual ~SpriteViewImageProvider();
private:
	SpriteData* m_data;
};

#endif /* INCLUDE_SPRITESHEETEDITOR_SPRITEVIEWIMAGEPROVIDER_H_ */
