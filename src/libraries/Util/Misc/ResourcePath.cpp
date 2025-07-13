/*
 *  GNU Lesser General Public License (LGPL):
 *
 *	Copyright (C) 2019  ProgSys
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

#include "ResourcePath.h"
#include <QDir>
#include <QDebug>

static QString gResourcePath = "";

void initResourcePath() {
	#if !_DEBUG
	QDir dir("resources");
	if (dir.exists())
		gResourcePath = "resources";
	else {
		gResourcePath = QString(PROJECT_RESOURCES_PATH);
		QDir dir(gResourcePath);
		if (!dir.exists())
			qDebug() << "Failed to resource dir: \""<< PROJECT_RESOURCES_PATH<<"\"";
	}
	#else
	gResourcePath = "resources";
	QDir dir(gResourcePath);
	if (!dir.exists())
		qDebug() << "Failed to find \"resources\\\" folder!";
	#endif

	qDebug() << "ResourcePath: "<< gResourcePath;
}

QString getResourcePath() {
	return gResourcePath;
}