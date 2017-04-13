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

#ifndef FILEMANAGERD2_H
#define FILEMANAGERD2_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QItemSelection>

#include <ui_fileManagerD2.h>

class FileManagerD2 : public QMainWindow, public Ui::FileManagerD2 {
    Q_OBJECT
public:
    explicit FileManagerD2(QWidget *parent = 0);
    ~FileManagerD2();

};

#endif // FILEMANAGERD2_H
