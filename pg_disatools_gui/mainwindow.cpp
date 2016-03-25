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


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fileinfobox.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QTreeView>
#include <QRegExp>
#include <QProgressDialog>
#include <QFuture>
#include <QtConcurrent/QtConcurrentRun>
#include <QGraphicsPixmapItem>
#include <QModelIndex>
#include <QDesktopServices>
#include <QTemporaryFile>

#include <iostream>

#define WINTITLE "Disa PC File Manager v0.4 alpha"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle(WINTITLE);

    m_tempFile = nullptr;
    m_treeModel = new TreeModel(this);
    //m_treeModel = new TreeModel("C:/Users/ProgSys/Desktop/Disgaea/PC/DATA.DAT",this);

    m_treeSort = new TreeSort(this);
    m_treeSort->setSourceModel(m_treeModel);
    m_treeSort->setDynamicSortFilter(true);
   // m_treeSort->setFilterRegExp(QRegExp(".mpp", Qt::CaseInsensitive, QRegExp::RegExp));


    ui->treeView->setModel(m_treeSort);
    ui->treeView->setAlternatingRowColors(true);
    ui->treeView->setSortingEnabled(true);
    ui->treeView->sortByColumn(0, Qt::AscendingOrder);

    ui->treeView->header()->resizeSection(0,150);
    ui->treeView->header()->resizeSection(1,60);
    ui->treeView->header()->resizeSection(2,60);
    ui->treeView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

    //ui->treeView->addAction(new QAction("Replace",  ui->treeView));

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, SIGNAL(customContextMenuRequested(const QPoint &)),
    		this, SLOT(treeContextMenu(const QPoint &)));

    selectionModel = ui->treeView->selectionModel();

    connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
    		this, SLOT(treeSelectionChanged(const QItemSelection&,const QItemSelection&)));


    /*
    connect(this, SIGNAL(saveFileAs(const QString&)),
    		m_treeModel, SLOT(saveFileAs(const QString&)), Qt::DirectConnection); //Qt::QueuedConnection);
    		*/

    scene = new QGraphicsScene(this);
    ui->imagePreview->setScene(scene);
    QAction* save_image = new QAction("Save Image...",  ui->imagePreview);
    ui->imagePreview->addAction(save_image);

    connect(save_image, SIGNAL(triggered()), this, SLOT(saveSelectedImage()));

    ui->statusBar->showMessage("Please open a .DAT or .MPP.");


    connect(this, SIGNAL(saveImage(const QString&, const QString&)), m_treeModel, SLOT(saveImage(const QString&, const QString&)));
    connect(ui->btnExtractImage, SIGNAL(clicked()), this, SLOT(saveSelectedImage()));
    connect(this, SIGNAL(openFile(const QString&)), m_treeModel, SLOT(open(const QString&)));

}

MainWindow::~MainWindow()
{
    delete ui;
    if(m_tempFile) delete m_tempFile;
}

void MainWindow::on_btnAbout_clicked()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("About");
    //msgBox.setWindowIcon(QMessageBox::Information);
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText(
    			"This gui application allows you to extract and insert files into Disgaea PC .DAT and .MPP files.<br>"\
    			"Note that the position of most files in .DAT is hardcoded,<br> deleting files could break the file order and so the archive itself.<br>"\
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

void MainWindow::on_btnAboutQt_clicked()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::on_btnOpen_clicked()
{
    QFileDialog openDialog(this);
    openDialog.setNameFilter(tr("ARCHIVE (*.dat *.mpp);;DATA (*.dat);;Map File (*.mpp)"));

    QStringList fileNames;
    if (openDialog.exec()){
        fileNames = openDialog.selectedFiles();
        if(fileNames.size() > 0){
        	qInfo() <<"Opening file '"<< fileNames[0] <<"' ";
        	if(emit openFile(fileNames[0])){
            	ui->statusBar->showMessage(QString("Found %1 files.").arg(m_treeModel->rowCount()));
                ui->btnInsert->setEnabled(true);
                ui->btnSaveAs->setEnabled(true);
                QString title(WINTITLE);
                title.append(" - ");
                title.append(fileNames[0]);
                setWindowTitle(title);
        	}else{
            	ui->statusBar->showMessage(QString("Couldn't open %1").arg(fileNames[0]));
                ui->btnInsert->setEnabled(false);
                ui->btnSaveAs->setEnabled(false);
                setWindowTitle(WINTITLE);
        	}
            ui->btnSave->setEnabled(false);
            ui->btnDelete->setEnabled(false);
            ui->btnExtractImage->setEnabled(false);
            ui->btnExtract->setEnabled(false);
        }
    }

}

void MainWindow::on_checkBox_MMP_clicked(bool checked)
{
	m_treeSort->setFilterFileExtention("MPP", !checked);
}

void MainWindow::on_checkBox_ARC_clicked(bool checked)
{
     m_treeSort->setFilterFileExtention("ARC", !checked);
}

void MainWindow::on_checkBox_MPD_clicked(bool checked)
{
     m_treeSort->setFilterFileExtention("MPD", !checked);
}

void MainWindow::on_checkBox_DAT_clicked(bool checked)
{
     m_treeSort->setFilterFileExtention("DAT", !checked);
}

void MainWindow::on_checkBox_FAD_clicked(bool checked)
{
     m_treeSort->setFilterFileExtention("FAD", !checked);
}

void MainWindow::on_checkBox_FFM_clicked(bool checked)
{
     m_treeSort->setFilterFileExtention("FFM", !checked);
}

void MainWindow::on_checkBox_OGG_clicked(bool checked)
{
     m_treeSort->setFilterFileExtention("OGG", !checked);
}

void MainWindow::on_checkBox_PBD_clicked(bool checked)
{
     m_treeSort->setFilterFileExtention("PBD", !checked);
}

void MainWindow::on_checkBox_PNG_clicked(bool checked)
{
     m_treeSort->setFilterFileExtention("PNG", !checked);
}

void MainWindow::on_checkBox_PHD_clicked(bool checked)
{
     m_treeSort->setFilterFileExtention("PHD", !checked);
}

void MainWindow::on_checkBox_TX2_clicked(bool checked)
{
     m_treeSort->setFilterFileExtention("TX2", !checked);
}

void MainWindow::on_checkBox_GEO_clicked(bool checked)
{
    m_treeSort->setFilterFileExtention("GEO", !checked);
}

void MainWindow::treeSelectionChanged (const QItemSelection & sel,const  QItemSelection & desel){
	QModelIndexList selected =  ui->treeView->selectionModel()->selectedRows();
	//qDebug() << selected.size();
	if(selected.isEmpty() || !selected[0].isValid()){
		 ui->btnExtract->setEnabled(false);
		 ui->btnDelete->setEnabled(false);
		 scene->clear();
		 ui->imagePreview->setShowsImage(false);
		return;
	}

	 ui->btnExtract->setEnabled(true);
	 ui->btnDelete->setEnabled(true);


		QMap<int, QVariant> item = m_treeSort->itemData(selected[0]);
		if(item.empty() || !item[0].isValid()) return;
        QString itemName = item[0].toString();
        if( itemName.contains(".TX2")){
            ui->btnExtractImage->setEnabled(true);
            ui->btnExtractImage->setText(QString("Export %1").arg(itemName));
            if(ui->imagePreview->isPreviewEnabled()){
                qDebug() << "Trying to open image preview: "<<itemName;
                m_treeModel->setGraphicsScene(itemName,scene);
                ui->imagePreview->fitInView(scene->itemsBoundingRect() ,Qt::KeepAspectRatio);
                ui->imagePreview->setShowsImage(true);
            }else{
                scene->clear();
            }
		}else{
			scene->clear();
            ui->btnExtractImage->setEnabled(false);
		}

}

void MainWindow::treeContextMenu(const QPoint &pos){

	QPoint globalpos = ui->treeView->mapToGlobal(pos);

	QModelIndex pointedItem = ui->treeView->indexAt(pos);
	pointedItem = m_treeSort->mapToSource(pointedItem);
	PG::FILE::fileInfo *item = static_cast<PG::FILE::fileInfo*>(pointedItem.internalPointer());

	if(pointedItem.isValid() && item){
		QMenu menu(this);
		QAction* action_Info = menu.addAction("File info");
		menu.addSeparator();
		QAction* action_replace = menu.addAction("Replace");
		QAction* action_replaceKeep = menu.addAction("Replace, keep name");
		menu.addSeparator();
		QAction* action_playOGG = menu.addAction("Play");
		if(item->getFileExtension() != "OGG"){
			action_playOGG->setEnabled(false);
		}

		QAction* selectedAction = menu.exec(ui->treeView->viewport()->mapToGlobal(pos));

		if(action_Info == selectedAction){
            FileInfoBox infobox(this);
            m_treeModel->getFileProperties(*item);
            infobox.setModel(item);
            infobox.exec();
		}else if(action_replace == selectedAction || action_replaceKeep == selectedAction){

		    QFileDialog openDialog(this);
		    QStringList fileNames;
		    if (openDialog.exec() ){
		       	setEnabled(false); //disable main window
		        fileNames = openDialog.selectedFiles();
		        m_treeModel->replace(pointedItem, fileNames[0], action_replaceKeep == selectedAction);
		        setEnabled(true);
		    }

			if(m_treeModel->hasDataChanged()){
				ui->btnSave->setEnabled(true);
				ui->btnSaveAs->setEnabled(true);
			}

		}else if(action_playOGG  == selectedAction ){
			QString tempFile = QString::fromStdString(item->name.getName());
			if(m_tempFile) delete m_tempFile;
			m_tempFile = new QTemporaryFile(tempFile+"XXXXXX.OGG", this);
			m_tempFile->setAutoRemove(true);
			m_tempFile->open();

			m_treeModel->extractFileName(pointedItem,m_tempFile->fileName());
			qDebug()<<tempFile;
			//temp.setAutoRemove(true)
			//temp.setAutoRemove(true);
		    QDesktopServices::openUrl(QUrl(m_tempFile->fileName()));
		}

	}
}

void MainWindow::saveSelectedImage(){

	QModelIndexList selected =  ui->treeView->selectionModel()->selectedRows();
	if(selected.isEmpty() || !selected[0].isValid()) return;
	QMap<int, QVariant> item = m_treeSort->itemData(selected[0]);

	QString image = item[0].toString();
	if(!image.contains(".TX2")) return;

	QString imageFileName = image;
	imageFileName.replace(".TX2","");
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"),
									imageFileName,
	                                   tr("PNG (*.png);;JPEG (*.jpg *.jpeg);;TIFF (*.tif);;TGA (*.tga);;PNM (*.pnm)"));

	if (fileName.isEmpty()) return;


	if(emit saveImage(image, fileName)){
		ui->statusBar->showMessage(QString("Image saved to %1").arg(fileName));
	}else{
		ui->statusBar->showMessage("Coudn't export image!");
	}

}

void MainWindow::on_treeView_clicked(const QModelIndex &index)
{}

void MainWindow::on_treeView_activated(const QModelIndex &index)
{}

void MainWindow::on_btnExtract_clicked()
{

    QModelIndexList selected =  ui->treeView->selectionModel()->selectedRows();
    if(selected.empty()){
        ui->btnExtract->setEnabled(false);
        ui->btnDelete->setEnabled(false);
        return;
    }

	QString dir = QFileDialog::getExistingDirectory(this, tr("Extract Directory"), "/home",QFileDialog::ShowDirsOnly
	                                             | QFileDialog::DontResolveSymlinks);

	if(dir.isEmpty() || dir.isNull()){
		ui->statusBar->showMessage("Invalid extract directory.");
		return;
	}

	QModelIndexList selectedSource;
	for(const QModelIndex& index: selected){
		selectedSource.push_back(m_treeSort->mapToSource(index));
	}

	if(selectedSource.size() == 1){
		if(m_treeModel->extractFile(selectedSource[0], dir)){
			ui->statusBar->showMessage(QString("Failed to extract file!"));
		}else{
			ui->statusBar->showMessage(QString("Extracted file to: %1").arg(dir));;
		}
	}else{
    	setEnabled(false); //disable main window

    	QProgressDialog progress;
    	progress.setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    	progress.setWindowTitle("Please wait.");
    	progress.setWindowModality(Qt::WindowModal);
    	progress.setLabelText("Extraction in progress.");
    	progress.setCancelButton(0);
    	progress.setRange(0,100);
    	progress.show();

    	QFuture<int> f1 = QtConcurrent::run(m_treeModel, &TreeModel::extract, selectedSource, dir);
    	while(f1.isRunning()){
    		progress.setValue(m_treeModel->getProgress());
    		QApplication::processEvents();
    	}

    	int result = f1.result();
    	if(result == selectedSource.size()){
    		ui->statusBar->showMessage(QString("Extracted files to: %1").arg(dir));;
    	}else{
    		ui->statusBar->showMessage(QString("Failed to extract %1 files!").arg(selectedSource.size()-result));
        }
    	progress.close();
    	setEnabled(true);

	}

}

void MainWindow::on_btnInsert_clicked()
{
    QFileDialog openDialog(this);
    openDialog.setFileMode(QFileDialog::ExistingFiles); //multiple files
    QStringList fileNames;
    if (openDialog.exec() ){
       	setEnabled(false); //disable main window
        fileNames = openDialog.selectedFiles();


		QProgressDialog progress;
		progress.setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
		progress.setWindowTitle("Please wait.");
		progress.setWindowModality(Qt::WindowModal);
		progress.setLabelText("Extraction in progress.");
		progress.setCancelButton(0);
		progress.setRange(0,100);
		progress.show();

		QFuture<int> f1 = QtConcurrent::run(m_treeModel, &TreeModel::add, fileNames);
		while(f1.isRunning()){
			progress.setValue(m_treeModel->getProgress());
			QApplication::processEvents();
        }


		int added = f1.result();
        if(added <= 0){
        	 ui->statusBar->showMessage("No files added!");
        }else if( added == 1){
        	 ui->statusBar->showMessage(QString("Added %1.").arg(fileNames[0]));
        }else{
        	 ui->statusBar->showMessage(QString("Added %1 files.").arg(added));
        }

        if(m_treeModel->hasDataChanged()){
        	ui->btnSave->setEnabled(true);
        	ui->btnSaveAs->setEnabled(true);
        }
        progress.close();
        setEnabled(true);
    }
}

void MainWindow::on_btnSave_clicked()
{
    if(m_treeModel->hasDataChanged()){
    	if(!checkValid())
    		return;

		QMessageBox::StandardButton reply = QMessageBox::warning(this, "Do you want to continue?",
						"Do you want to save the file?",
					 QMessageBox::Yes|QMessageBox::Cancel);

		if(reply == QMessageBox::No)
			return;

    	setEnabled(false);
    	QProgressDialog progress;
    	progress.setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    	progress.setWindowTitle("Please wait.");
    	progress.setWindowModality(Qt::WindowModal);
    	progress.setLabelText("Saving in progress.");
    	progress.setCancelButton(0);
    	progress.setRange(0,101);
    	progress.show();

    	QFuture<bool> f1 = QtConcurrent::run(m_treeModel, &TreeModel::save);
    	while(f1.isRunning()){
    		progress.setValue(m_treeModel->getProgress());
    		QApplication::processEvents();
    	}
        if(f1.result()){
        	ui->statusBar->showMessage("Saved!");
        	ui->btnSave->setEnabled(false);
        }else{
        	checkValid();
        	ui->statusBar->showMessage("Saving failed, file is maybe broken now.");
        }

        progress.close();
        setEnabled(true);

    }else{
    	ui->btnSave->setEnabled(false);
    }
}

void MainWindow::on_btnSaveAs_clicked()
{
		if(!checkValid())
			return;

		QString fileName;
		if(m_treeModel->getOpenedType() == "DAT"){
	    	fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
	    										m_treeModel->getOpenedFileName(),
	    	                                   tr("Data (*.DAT)"));
		}else if(m_treeModel->getOpenedType() == "MPP"){
	    	fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
	    										m_treeModel->getOpenedFileName(),
	    	                                   tr("MPP (*.MPP)"));
		}else{
			return;
		}



    	if (fileName.isEmpty()) return;

    	setEnabled(false); //disable main window

    	QProgressDialog progress;
    	progress.setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    	progress.setWindowTitle("Please wait.");
    	progress.setWindowModality(Qt::WindowModal);
    	progress.setLabelText("Saving in progress.");
    	progress.setCancelButton(0);
    	progress.setRange(0,101);
    	progress.show();

    	QFuture<bool> f1 = QtConcurrent::run(m_treeModel, &TreeModel::saveAs, fileName);
    	while(f1.isRunning()){
    		progress.setValue(m_treeModel->getProgress());
    		QApplication::processEvents();
    	}


    	if(f1.result()){
    		ui->statusBar->showMessage(QString("Saved to %1.").arg(fileName));
            QString title(WINTITLE);
            title.append(" - ");
            title.append(fileName);
            setWindowTitle(title);
    	}else{
    		checkValid();
    		ui->statusBar->showMessage("Saving failed, file is maybe broken now.");
        }
    	progress.close();
    	setEnabled(true);

}

void MainWindow::on_btnDelete_clicked()
{

	if(m_treeModel->getOpenedType() == "DAT"){
		QMessageBox::StandardButton reply = QMessageBox::warning(this, "You are sure you want to delete the selected files?",
				"The position of most files (like .TX2) is hardcoded ingame, deleting items could destroy the file order and break the archive! Continue?",
				 QMessageBox::Yes|QMessageBox::No);
		if(reply == QMessageBox::No) return;
	}

	QModelIndexList selected =  ui->treeView->selectionModel()->selectedRows();

	QList<PG::FILE::fileInfo*> selectedSource;
	for(const QModelIndex& index: selected){
		PG::FILE::fileInfo *item = static_cast<PG::FILE::fileInfo*>(m_treeSort->mapToSource(index).internalPointer());
		selectedSource.push_back(item);
	}

	unsigned int removed = m_treeModel->remove(selectedSource);

	if(removed == 0){
		 ui->statusBar->showMessage("No files removed!");
	}else if( removed == 1){
		 ui->statusBar->showMessage(QString("One file removed."));
	}else{
		 ui->statusBar->showMessage(QString("%1 files removed.").arg(removed));
	}

	if(m_treeModel->hasDataChanged()){
		ui->btnSave->setEnabled(true);
		ui->btnSaveAs->setEnabled(true);
	}

	ui->btnDelete->setEnabled(false);

}

void MainWindow::on_btnExtractImage_clicked()
{
	//saveSelectedImage();
}

bool MainWindow::checkValid(){
	QString msg;
	if(!m_treeModel->checkIsValid(msg)){
		QMessageBox messageBox(this);
		messageBox.critical(0,"Error",msg);
		messageBox.setFixedSize(500,200);
		return false;
	}
	return true;
}

