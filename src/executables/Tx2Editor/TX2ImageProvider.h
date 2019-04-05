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
#include <QImage>
#include <QQuickImageProvider>
#include <Files/PG_TX2.h>
#include <TX2Editor.h>

#ifndef INCLUDE_TX2EDITOR_TX2IMAGEPROVIDER_H_
#define INCLUDE_TX2EDITOR_TX2IMAGEPROVIDER_H_

class TX2ImageProvider: public QQuickImageProvider {
public:
	TX2ImageProvider(TX2EditorModel* image = nullptr);
	QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
	virtual ~TX2ImageProvider();

	void setModel(TX2EditorModel* image = nullptr);

private:
	TX2EditorModel* m_image;
};

#endif /* INCLUDE_TX2EDITOR_TX2IMAGEPROVIDER_H_ */
