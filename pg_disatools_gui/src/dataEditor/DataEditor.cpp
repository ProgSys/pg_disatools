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
#include <QFileDialog>
#include <QDebug>
#include <QDir>
#include <QMap>

#include <files/ParserDAT.h>

PreviewDAT::PreviewDAT(QObject *parent): DataFile(parent){
	QList<QVariant> data;
	data << "Definition file" << "Description" << "File starting name";
	m_root = new TreeItem(data);
	QMap<QString,QString> descriptions;

	QFile qfile("resources/dataFiles/descriptions.txt");
	if(qfile.open(QIODevice::ReadOnly)){
		QTextStream in(&qfile);

		while(!in.atEnd()){
			QString line = in.readLine();
			QStringList sep = line.split(':');
			if(sep.size() > 1) descriptions[sep[0]] = sep[1];

		}

		qfile.close();
	}


	QDir myDir("resources/dataFiles/");
	QStringList filters;
	filters.push_back("*.DEF");
	QFileInfoList filesList = myDir.entryInfoList(filters );

	for(const QFileInfo& info: filesList){
		data.clear();
		data << info.fileName();
		if(descriptions.contains(info.baseName())){
			data<< descriptions[info.baseName()];
		}else
			data<< "No description available";
		data <<info.baseName();
		m_root->appendChild(new TreeItem(data, m_root));
	}
}

PreviewDAT::~PreviewDAT(){}

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

	connect(actionOpen, SIGNAL(triggered()), this, SLOT(open()));
	connect(actionSave, SIGNAL(triggered()), this, SLOT(save()));
	connect(actionSave_As, SIGNAL(triggered()), this, SLOT(saveAs()));
	connect(actionExport_as_csv, SIGNAL(triggered()), this, SLOT(exportCSV()));

	m_file = new PreviewDAT(this);
	treeView->setModel(m_file);

	treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeView, SIGNAL(customContextMenuRequested(const QPoint &)),
    		this, SLOT(contextMenu(const QPoint &)));

    actionExport_as_csv->setEnabled(false);
}

DataEditor::~DataEditor() {
	// TODO Auto-generated destructor stub
}

void DataEditor::setModel(DataFile* model){
	if(!model) return;
	if(m_file) {
		disconnect(this, SIGNAL(openFile(const QString&)), m_file, SLOT(open(const QString&) ));
		disconnect(this, SIGNAL(saveFile(const QString&)), m_file, SLOT(save(const QString&) ));
		disconnect(this, SIGNAL(exportCSVFile(const QString&)), m_file, SLOT(exportCSV(const QString&) ));
		treeView->setModel(nullptr);
		delete m_file;
	}
	m_file = model;

	connect(this, SIGNAL(openFile(const QString&)), m_file, SLOT(open(const QString&) ));
	connect(this, SIGNAL(saveFile(const QString&)), m_file, SLOT(save(const QString&) ));
	connect(this, SIGNAL(exportCSVFile(const QString&)), m_file, SLOT(exportCSV(const QString&) ));

	treeView->setModel(m_file);
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
	if(file.isEmpty()) return;

	QString filename = QFileInfo(file).baseName();


	QDir myDir("resources/dataFiles/");
	QStringList filters;
	filters.push_back("*.DEF");
	QFileInfoList filesList = myDir.entryInfoList(filters );

	bool found = false;
	for(const QFileInfo& info: filesList){
		const QString baseFileName = info.baseName();
		if(filename.left(baseFileName.size()).toUpper() == baseFileName.toUpper()){
			qDebug()<<"Parsing: "<<info.absoluteFilePath();
			setModel(new ParserDAT(info.absoluteFilePath(),this));
			found = true;
			break;
		}
	}

	if(!found){
		QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error",
			"File as a unknown name, make sure the file starts with the correct string!",
		 QMessageBox::Ok);
		return;
	}

	if(!(emit openFile(file))){
		setTitel();
		m_fileName.clear();
		//statusbar->showMessage(QString("Failed to opened %1.").arg(file));
		actionSave->setEnabled(false);
		actionSave_As->setEnabled(false);
		actionExport_as_csv->setEnabled(false);
	}else{
		setTitel(file);
		m_fileName = file;
		actionSave->setEnabled(true);
		actionSave_As->setEnabled(true);
		actionExport_as_csv->setEnabled(true);
		//statusbar->showMessage(QString("Opened %1.").arg(file));
	}
}

void DataEditor::save(){
	if(m_fileName.isEmpty()){
		saveAs();
	}else{
		QMessageBox::StandardButton reply = QMessageBox::question(nullptr, "Continue?",
						"Do you want to save and overwrite the file?",
					 QMessageBox::Yes|QMessageBox::Cancel);
		if(reply == QMessageBox::Yes){
			save(m_fileName);
		}
	}
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
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save data sheet"),
										QFileInfo(m_fileName).baseName(),
	                                   tr("DATA (*.DAT)"));
	if (fileName.isEmpty()) return;

	save(fileName);
}

void DataEditor::exportCSV(){
	QString fileName = QFileDialog::getSaveFileName(this, tr("Export data sheet"),
										QFileInfo(m_fileName).baseName(),
	                                   tr("csv (*.csv)"));
	if (fileName.isEmpty()) return;

	exportCSV(fileName);
}

void DataEditor::exportCSV(const QString& file){
	if(emit exportCSVFile(file)){
		//statusbar->showMessage(QString("Saved to %1.").arg(file));
		m_fileName = file;
	}else{
		//ui->statusbar->showMessage(QString("Failed save to %1.").arg(file));
	}
}

void DataEditor::contextMenu(const QPoint &pos){
	if(!m_file) return;
	QPoint globalpos = treeView->mapToGlobal(pos);
	QModelIndex pointedItem = treeView->indexAt(pos);

	if(pointedItem.isValid()){
		QMenu menu(this);
		//QAction* action_info = menu.addAction("Entry "+QString::number(pointedItem.row()+1));
		//action_info->setEnabled(false);

		QAction* action_insert_above = menu.addAction("Insert at "+QString::number(pointedItem.row()+1));
		//QAction* action_insert_below = menu.addAction("Insert below");
		menu.addSeparator();
		QAction* action_insert_top = menu.addAction("Insert top");
		QAction* action_insert_bottom = menu.addAction("Insert bottom");
		menu.addSeparator();
		QAction* action_delete = menu.addAction("Delete");
		globalpos.setY(globalpos.y()+20);
		QAction* selectedAction = menu.exec(globalpos);
		if(nullptr == selectedAction)
			return;
		else if(selectedAction == action_insert_above){
			m_file->insertAt(pointedItem.row());
		}else if(selectedAction == action_insert_top){
			m_file->insertFront();
		}else if(selectedAction == action_insert_bottom){
			m_file->insertBack();
		}else if(selectedAction == action_delete){
			m_file->removeAt(pointedItem.row());
		}
	}

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
