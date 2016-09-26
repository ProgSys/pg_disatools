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
#include <tx2Editor/TX2Editor.h>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QDir>
#include <QImage>
#include <QtQml>
#include <tx2Editor/TX2ImageProvider.h>
#include <pg/files/PG_TX2.h>


TX2EditorModel::TX2EditorModel(QObject *parent){
	image = new PG::UTIL::RGBAImage();
}

TX2EditorModel::~TX2EditorModel(){
	delete image;
}

unsigned int TX2EditorModel::getWidth() const{
	return image->getWidth();
}

unsigned int TX2EditorModel::getHeight() const{
	return image->getHeight();
}


bool TX2EditorModel::openTX2(const QString& filepath){
	if(PG::FILE::decompressTX2(filepath.toStdString(), *image)){
		image->resize(0,0);
		emit imageChanged();
		return false;
	}
	emit imageChanged();
	return true;
}

bool TX2EditorModel::openImage(const QString& filepath){
	QImage qImg(filepath);
	if(qImg.width() <= 0 || qImg.height() <= 0) {
		image->resize(0,0);
		emit imageChanged();
		return false;
	}

	image->resize(qImg.width(),qImg.height());
	for(unsigned int y = 0; y < qImg.height(); y++)
		for(unsigned int x = 0; x < qImg.width(); x++){
			const QRgb& rgb = qImg.pixel(x, y);
			const int alpha =  qAlpha(rgb);
			const QColor color(rgb);
			image->set(x, y, PG::UTIL::rgba(color.red(), color.green(), color.blue(), alpha));
		}
	emit imageChanged();
	return true;
}




inline void aboutTX2Editor(){
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("About");
    //msgBox.setWindowIcon(QMessageBox::Information);
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText(
    			"This gui application allows you view and edit TX2 image files.<br>"\

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

TX2Editor::TX2Editor(QWidget *parent):
	QMainWindow(parent), m_model(new TX2EditorModel(this)){
	setupUi(this);

	setWindowIcon(QIcon(":/tx2_editor_icon.ico"));
	setWindowTitle(TX2EditorTITLE);


	//About
	connect(actionAbout, &QAction::triggered, this, [this]{
		aboutTX2Editor();
	});
	connect(actionAbout_Qt, &QAction::triggered, this, [this]{
		 QMessageBox::aboutQt(this);
	} );




	qmlRegisterType<TX2EditorModel>("MyTX2Editor",0,1, "TX2EditorModel");
	Ui::TX2EditorUI::quickWidget->rootContext()->setContextProperty("tx2editor", m_model);

	quickWidget->engine()->addImageProvider(QLatin1String("tx2imageprovider"), new TX2ImageProvider(m_model->image));
	quickWidget->setSource(QUrl::fromLocalFile("QML/Tx2View.qml"));

	connect(actionOpen, SIGNAL( triggered() ),  this, SLOT( open() ));
	connect(pushButton_open, SIGNAL( clicked() ),  this, SLOT( open() ));

	connect(this, SIGNAL( openTX2(const QString&) ),  m_model, SLOT( openTX2(const QString&) ));
	connect(this, SIGNAL( openImage(const QString&) ),  m_model, SLOT( openImage(const QString&) ));

	connect(m_model, SIGNAL( imageChanged() ),  this, SLOT( updateInfo() ));
}

TX2Editor::~TX2Editor() {
	delete m_model;
}


void TX2Editor::open(){
    QFileDialog openDialog(this);
    openDialog.setNameFilter(tr("Images (*.tx2;*.png;*.tga;*.jpg);;TX2 image (*.tx2);;PNG (*.png);;TGA (*.tga);;JPG (*.jpg)"));

    QStringList fileNames;
	if (openDialog.exec()){
		fileNames = openDialog.selectedFiles();
		if(fileNames.size() > 0){
			open(fileNames[0]);
		}
	}
}
void TX2Editor::open(const QString& filepath){
	if(filepath.isEmpty()) return;

	if(filepath.right(4).toLower() == ".tx2"){
		if(emit openTX2(filepath))
			QMainWindow::statusBar()->showMessage(QString("Opened TX2 %1").arg(filepath));
		else
			QMainWindow::statusBar()->showMessage(QString("Couldn't open TX2 %1").arg(filepath));
	}else{
		if(emit openImage(filepath))
			QMainWindow::statusBar()->showMessage(QString("Opened image %1").arg(filepath));
		else
			QMainWindow::statusBar()->showMessage(QString("Couldn't open image %1").arg(filepath));
	}
}


void TX2Editor::save(){

}

void TX2Editor::saveAs(){

}
void TX2Editor::save(const QString& filepath){

}

void TX2Editor::updateInfo(){
	value_width->setText(QString::number(m_model->getWidth()));
	value_height->setText(QString::number(m_model->getHeight()));
}
