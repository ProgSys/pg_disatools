/*
 *  Copyright (C) 2016  ProgSys
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QTreeView>
#include <QRegExp>
#include <QProgressDialog>
#include <QFuture>
#include <QtConcurrent/QtConcurrentRun>
#include <QGraphicsPixmapItem>

#include <iostream>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


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


    selectionModel = ui->treeView->selectionModel();

    connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
    		this, SLOT(treeSelectionChanged(const QItemSelection&,const QItemSelection&)));


    /*
    connect(this, SIGNAL(saveFileAs(const QString&)),
    		m_treeModel, SLOT(saveFileAs(const QString&)), Qt::DirectConnection); //Qt::QueuedConnection);
    		*/

    scene = new QGraphicsScene(this);
    ui->imagePreview->setScene(scene);
    //ui->imagePreview->fitInView(scene->itemsBoundingRect() ,Qt::KeepAspectRatio);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnAbout_clicked()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("About");
    //msgBox.setWindowIcon(QMessageBox::Information);
    msgBox.setText("About");
    msgBox.setInformativeText(
                "This tool allows you to add and extract files form the Disgaea PC pspfs_v1 archive. \n" \
                "\nCopyright (C) 2016  ProgSys"\
                "\n\nThis program is free software: you can redistribute it and/or modify"\
                "\nit under the terms of the GNU General Public License as published by"\
                "\nthe Free Software Foundation, either version 3 of the License, or"\
                "\n(at your option) any later version."\

                "\n\nThis program is distributed in the hope that it will be useful,"\
                "\nbut WITHOUT ANY WARRANTY; without even the implied warranty of"\
                "\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"\
                "\nGNU General Public License for more details."\

                "\n\nYou should have received a copy of the GNU General Public License"\
                "\nalong with this program.  If not, see <http://www.gnu.org/licenses/>."\
                );
    msgBox.exec();

}

void MainWindow::on_btnOpen_clicked()
{
    QFileDialog openDialog(this);
    openDialog.setNameFilter(tr("DATA (*.dat)"));

    QStringList fileNames;
    if (openDialog.exec()){
        fileNames = openDialog.selectedFiles();
        if(fileNames.size() > 0){
        	std::cout <<"Opening file '"<< fileNames[0].toStdString() <<"' "<< std::endl;
        	m_treeModel->openFile(fileNames[0]);

        	ui->labelInfo->setText( QString("Found %1 files.").arg(m_treeModel->rowCount()) );
            ui->btnInsert->setEnabled(true);
            ui->btnSaveAs->setEnabled(true);
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

void MainWindow::treeSelectionChanged (const QItemSelection & sel,const  QItemSelection & desel){


	QModelIndexList selected =  ui->treeView->selectionModel()->selectedRows();
	//qDebug() << selected.size();
	if(selected.isEmpty() || !selected[0].isValid()){
		 ui->btnExtract->setEnabled(false);
		 ui->btnDelete->setEnabled(false);
		return;
	}

	 ui->btnExtract->setEnabled(true);
	 ui->btnDelete->setEnabled(true);

	QMap<int, QVariant> item = m_treeSort->itemData(selected[0]);
	if( item[0].toString().contains(".TX2")){
		qDebug() << "Trying to open image preview: "<<item[0].toString();
		m_treeModel->setGraphicsScene(item[0].toString(),scene);
		ui->imagePreview->fitInView(scene->itemsBoundingRect() ,Qt::KeepAspectRatio);
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
		ui->labelInfo->setText("Invalid extract directory.");
		return;
	}



    qDebug() <<"Click Extract: "<< selected.size();

    for(const QModelIndex& index: selected){
    		//m_treeModel->extractFile(index);
    	QMap<int, QVariant> item = m_treeSort->itemData(index);
    	qDebug() << item[0].toString();
    	m_treeModel->extractFile(item[0].toString(), dir);
    	ui->labelInfo->setText(QString("Extracted selected to: %1").arg(dir));
    }
}

void MainWindow::on_btnInsert_clicked()
{
    QFileDialog openDialog(this);
    openDialog.setFileMode(QFileDialog::ExistingFiles); //multiple files
    QStringList fileNames;
    if (openDialog.exec() ){
        fileNames = openDialog.selectedFiles();
        int added = m_treeModel->addFiles(fileNames);

        if(added <= 0){
        	 ui->labelInfo->setText( QString("No files added!"));
        }else if( added == 1){
        	 ui->labelInfo->setText( QString("Added %1.").arg(fileNames[0]) );
        }else{
        	 ui->labelInfo->setText( QString("Added %1 files.").arg(added) );
        }

        if(m_treeModel->hasDataChanged()){
        	ui->btnSave->setEnabled(true);
        	ui->btnSaveAs->setEnabled(true);
        }

    }
}

void MainWindow::on_btnSave_clicked()
{
    if(m_treeModel->hasDataChanged()){
    	QProgressDialog progress;
    	progress.setWindowModality(Qt::WindowModal);
    	progress.setLabelText("Saving in progress.");
    	progress.setCancelButton(0);
    	progress.setRange(0,0);
    	progress.show();

    	QFuture<bool> f1 = QtConcurrent::run(m_treeModel, &TreeModel::saveFile);
    	while(f1.isRunning()){
    		QApplication::processEvents();
    	}
        if(f1.result()){
        	ui->statusBar->showMessage("Saved!");
        }else{
        	ui->statusBar->showMessage("Saving failed, file is maybe broken now.");
        }

        progress.close();
        ui->btnSave->setEnabled(false);
    }
}

void MainWindow::on_btnSaveAs_clicked()
{
    	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
    	                                   "",
    	                                   tr("Data (*.DAT)"));

    	if (fileName.isEmpty()) return;

    	QProgressDialog progress;
    	setEnabled(false);
    	progress.setWindowModality(Qt::WindowModal);
    	progress.setLabelText("Saving in progress.");
    	progress.setCancelButton(0);
    	progress.setRange(0,0);
    	progress.show();

    	QFuture<bool> f1 = QtConcurrent::run(m_treeModel, &TreeModel::saveFileAs, fileName);
    	while(f1.isRunning()){
    		QApplication::processEvents();
    	}


    	if(f1.result()){
    		ui->statusBar->showMessage(QString("Saved to %1.").arg(fileName));
    	}else{
    		ui->statusBar->showMessage("Saving failed, file is maybe broken now.");
        }
    	setEnabled(true);
}

void MainWindow::on_btnDelete_clicked()
{
    QModelIndexList selected =  ui->treeView->selectionModel()->selectedRows();

    QStringList fileNames;
    for(const QModelIndex& index: selected){
       		//m_treeModel->extractFile(index);
       	QMap<int, QVariant> item = m_treeSort->itemData(index);
       	fileNames.push_back(item[0].toString());
    }

    int removed = m_treeModel->removeFiles(fileNames);

    if(removed <= 0){
    	 ui->labelInfo->setText( QString("No files removed!"));
    }else if( removed == 1){
    	 ui->labelInfo->setText( QString("Removed %1.").arg(fileNames[0]) );
    }else{
    	 ui->labelInfo->setText( QString("%1 files removed.").arg(removed) );
    }

    if(m_treeModel->hasDataChanged()){
    	ui->btnSave->setEnabled(true);
    	ui->btnSaveAs->setEnabled(true);
    }

    ui->btnDelete->setEnabled(false);
}
