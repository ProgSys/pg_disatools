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
#ifndef INCLUDE_DATAEDITOR_DATAEDITOR_H_
#define INCLUDE_DATAEDITOR_DATAEDITOR_H_



#include <QString>
#include <QMainWindow>
#include <ui_dataEditor.h>

class DataEditor: public QMainWindow, public Ui::DataEditorUI {
	Q_OBJECT
public:
	DataEditor(QWidget *parent = 0);
	virtual ~DataEditor();
public slots:
	void open();
	void open(const QString& file);
	void save();
	void save(const QString& file);
	void saveAs();
signals:
	bool openFile(const QString& file);
	bool saveFile(const QString& file);

};

#endif /* INCLUDE_DATAEDITOR_DATAEDITOR_H_ */
