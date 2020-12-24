/*
mapEditor *  GNU Lesser General Public License (LGPL):
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
#pragma once


#include <SpriteSheetEditor.h>
#include <QApplication>
#include <Util/Misc/ResourcePath.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	initResourcePath();
	a.setStyle("windowsvista");
    SpriteSheetEditor w;
    w.show();
    if(argc == 2) w.import(QString(argv[1]));
    return a.exec();
}

