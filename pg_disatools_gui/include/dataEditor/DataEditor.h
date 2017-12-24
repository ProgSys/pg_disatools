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
#include <dataEditor/DataFile.h>
#include <dataEditor/SceneModel.h>

class PreviewDAT: public DataFile  {
	Q_OBJECT
public:
	PreviewDAT(QObject *parent = 0);
	virtual ~PreviewDAT();
};

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

	void exportCSV();
	void exportCSV(const QString& file);

	 void contextMenu(const QPoint &pos);

signals:
	bool openFile(const QString& file);
	bool saveFile(const QString& file);
	bool exportCSVFile(const QString& file);
private:
    void setModel(DataFile* model);
	void setTitel();
	void setTitel(const QString& filename);

	DataFile* m_file = nullptr;
	QString m_fileName;
	SceneModel* mSceneModel;
};

#endif /* INCLUDE_DATAEDITOR_DATAEDITOR_H_ */
