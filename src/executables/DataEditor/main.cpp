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
#ifndef DATADITOR_CPP_
#define DATADITOR_CPP_


#include <DataEditor.h>
#include <QApplication>
#include <Util/Misc/ResourcePath.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	initResourcePath();
	a.setStyle("windowsvista");
		/*
	{
		QFile f(getResourcePath() + "/styles/dark/style.qss");
		if (f.open(QFile::ReadOnly | QFile::Text)) {
			a.setStyleSheet(QString(f.readAll()).arg(getResourcePath() + "/styles/dark"));
			f.close();
		}
	}
		*/

	//a.setStyleSheet(getResourcePath()+"/styles/dark/style.qss");
    DataEditor w;
    w.show();
    if(argc == 2) w.open(QString(argv[1]));
    return a.exec();
}



#endif /* DATADITOR_CPP_ */
