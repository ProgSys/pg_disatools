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
#include <fileManagerD2/FileManagerD2.h>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QDir>
#include <QMap>




FileManagerD2::FileManagerD2(QWidget *parent):
	QMainWindow(parent){
	setupUi(this);

	setWindowIcon(QIcon(":/disa_tools_icon.ico"));
	setWindowTitle(FileManagerD2TITLE);

	mModel = new  SceneModel(this);
}

FileManagerD2::~FileManagerD2() {
	delete mModel;
}
