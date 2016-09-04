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
#include <dataEditor/DataEditor.h>
#include <ui_dataEditor.h>
#include <TitleDefine.h>
#include <QMessageBox>
#include <files/DungeonDAT.h>
#include <QFileDialog>
#include <QDebug>

inline void aboutDataEditor(){
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("About");
    //msgBox.setWindowIcon(QMessageBox::Information);
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText(
    			"This gui application allows you view and edit some Disgaea PC .DAT files.<br>"\

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

DataEditor::DataEditor(QWidget *parent):
	QMainWindow(parent){
	setupUi(this);

	setWindowIcon(QIcon("resources/data_editor_icon.ico"));
	setWindowTitle(DATAEditorTITLE);


	//About
	connect(actionAbout, &QAction::triggered, this, [this]{
		aboutDataEditor();
	});
	connect(actionAbout_Qt, &QAction::triggered, this, [this]{
		 QMessageBox::aboutQt(this);
	} );

	m_file = new DungeonDAT(this);

	connect(this, SIGNAL(openFile(const QString&)), m_file, SLOT(open(const QString&) ));
	connect(this, SIGNAL(saveFile(const QString&)), m_file, SLOT(save(const QString&) ));

	treeView->setModel(m_file);

	connect(actionOpen, SIGNAL(triggered()), this, SLOT(open()));
	connect(actionSave, SIGNAL(triggered()), this, SLOT(save()));
	connect(actionSave_As, SIGNAL(triggered()), this, SLOT(saveAs()));
}

DataEditor::~DataEditor() {
	// TODO Auto-generated destructor stub
}


void DataEditor::open(){
    QFileDialog openDialog(this);
    openDialog.setNameFilter(tr("DATA (*.DAT)"));

    QStringList fileNames;
	if (openDialog.exec()){
		fileNames = openDialog.selectedFiles();
		if(fileNames.size() > 0){
			open(fileNames[0]);
		}
	}
}

void DataEditor::open(const QString& file){
	qDebug()<<"Openning: "<<file;
	if(file.isEmpty() || !(emit openFile(file))){
		setTitel();
		m_fileName.clear();
		//statusbar->showMessage(QString("Failed to opened %1.").arg(file));
		actionSave->setEnabled(false);
		actionSave_As->setEnabled(false);
	}else{
		setTitel(file);
		m_fileName = file;
		actionSave->setEnabled(true);
		actionSave_As->setEnabled(true);
		//statusbar->showMessage(QString("Opened %1.").arg(file));
	}
}

void DataEditor::save(){
	if(m_fileName.isEmpty()){
		saveAs();
	}else
		save(m_fileName);
}

void DataEditor::save(const QString& file){
	if(emit saveFile(file)){
		//statusbar->showMessage(QString("Saved to %1.").arg(file));
		m_fileName = file;
	}else{
		//ui->statusbar->showMessage(QString("Failed save to %1.").arg(file));
	}
}

void DataEditor::saveAs(){
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save sprite sheet"),
										QFileInfo(m_fileName).baseName(),
	                                   tr("DATA (*.DAT)"));
	if (fileName.isEmpty()) return;

	save(fileName);
}

void DataEditor::setTitel(){
	setWindowTitle(DATAEditorTITLE);
}

void DataEditor::setTitel(const QString& filename){
	 if(filename.isEmpty()){
		 setTitel();
		 return;
	 }
	 QString title(DATAEditorTITLE);
	 title.append(" - ");
	 title.append(filename);
	 setWindowTitle(title);
}
