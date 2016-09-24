/*
 * MapEditor.cpp
 *
 *  Created on: 10.09.2016
 *      Author: ProgSys
 */

#include <mapEditor/MapEditor.h>
#include <QFileDialog>
#include <QMessageBox>
#include <qdebug.h>

inline void about(){
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("About");
    //msgBox.setWindowIcon(QMessageBox::Information);
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText(
    			"This gui application allows you to view Disgaea PC *.MPD map files.<br>"\

				"You can find the source code here: <a href='https://github.com/ProgSys/pg_disatools/'>https://github.com/ProgSys/pg_disatools</a><br>"

				"<br><b>GNU Lesser General Public License (LGPL):</b> <br>"
				"<br>Copyright (C) 2016  ProgSys"\
                "<br><br>This program is free software: you can redistribute it and/or modify"\
                "<br>it under the terms of the GNU Lesser General Public License as published by"\
                "<br>the Free Software Foundation, version 3 of the License."\

                "<br><br>This program is distributed in the hope that it will be useful,"\
                "<br>but WITHOUT ANY WARRANTY; without even the implied warranty of"\
                "<br>MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"\
                "<br>GNU Lesser General Public License for more details."\

                "<br><br>You should have received a copy of the GNU Lesser General Public License"\
                "<br>along with this program.  If not, see <a href='http://doc.qt.io/qt-5/lgpl.html'> http://doc.qt.io/qt-5/lgpl.html</a>"\
				"<br>or <a href='http://www.gnu.org/licenses/'> http://www.gnu.org/licenses/ </a>."\
                );

    msgBox.exec();
}

MapEditor::MapEditor(QWidget *parent): QMainWindow(parent) {
	setupUi(this);

	setWindowIcon(QIcon(":/map_editor_icon.ico"));
	setWindowTitle(MapEditorTITLE);

	//About
	connect(actionAbout, &QAction::triggered, this, [this]{
		about();
	});
	connect(actionAbout_Qt, &QAction::triggered, this, [this]{
		 QMessageBox::aboutQt(this);
	} );

	//File
	connect(actionOpen_File, SIGNAL(triggered()), this, SLOT(open()));



	connect(this,  SIGNAL(openMPD(const QString&)), openGLWidget, SLOT(openMap(const QString&)));


}

void MapEditor::open(){
    QFileDialog openDialog(this);
    openDialog.setNameFilter(tr("MPD (*.mpd)"));

    QStringList fileNames;
	if (openDialog.exec()){
		fileNames = openDialog.selectedFiles();
		if(fileNames.size() > 0){
			open(fileNames[0]);
		}
	}
}

void MapEditor::open(const QString& file){
	qDebug()<<"Openning: "<<file;
	if(file.isEmpty() || !(emit openMPD(file))){
		statusbar->showMessage(QString("Failed to opened %1.").arg(file));
		setTitel();
	}else{
		statusbar->showMessage(QString("Opened %1.").arg(file));
		setTitel(file);
	}
}

void MapEditor::setTitel(){
	setWindowTitle(MapEditorTITLE);
}

void MapEditor::setTitel(const QString& filename){
	 if(filename.isEmpty()){
		 setTitel();
		 return;
	 }
	 QString title(MapEditorTITLE);
	 title.append(" - ");
	 title.append(filename);
	 setWindowTitle(title);
}

MapEditor::~MapEditor() {
	// TODO Auto-generated destructor stub
}

