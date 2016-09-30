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



TX2EditorModel::TX2EditorModel(QObject *parent){
	image = new PG::FILE::tx2Image();
}

TX2EditorModel::~TX2EditorModel(){
	delete image;
}

unsigned int TX2EditorModel::getWidth() const{
	return image->header.width;
}

unsigned int TX2EditorModel::getHeight() const{
	return image->header.height;
}


bool TX2EditorModel::openTX2(const QString& filepath){
	if(image->open(filepath.toStdString())){
		emit imageChanged();
		return false;
	}
	emit imageChanged();
	return true;
}

bool TX2EditorModel::openImage(const QString& filepath){
	QImage qImg(filepath);
	if(qImg.width() <= 0 || qImg.height() <= 0) {
		emit imageChanged();
		return false;
	}


	std::vector<char> data(qImg.width()*qImg.height()*4);
	for(unsigned int y = 0; y < qImg.height(); y++)
		for(unsigned int x = 0; x < qImg.width(); x++){
			const QRgb& rgb = qImg.pixel(x, y);
			const int alpha =  qAlpha(rgb);
			const QColor color(rgb);
			const unsigned int pos = (x+y*qImg.width())*4;
			data[pos] = color.red();
			data[pos+1] = color.green();
			data[pos+2] = color.blue();
			data[pos+3] = alpha;
		}

	image->setWithRGBA(qImg.width(), qImg.height(), (char*)&data[0]);

	emit imageChanged();
	return true;
}

bool TX2EditorModel::saveTX2(const QString& filepath){
	if(image->header.type == PG::FILE::tx2Type::TX2ERROR ||  image->getWidth() == 0  || image->getHeight() == 0) return false;

	image->save(filepath.toStdString());

	return true;
}


bool TX2EditorModel::saveImage(const QString& filepath){
	if(image->header.type == PG::FILE::tx2Type::TX2ERROR ||  image->getWidth() == 0  || image->getHeight() == 0) return false;

	QImage img(image->getWidth(), image->getHeight(), QImage::Format_RGBA8888);
	image->getRGBAData((char*)img.bits(), 0);
	img.save(filepath);

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

	connect(pushButton_save, SIGNAL( clicked() ),  this, SLOT( save() ));
	connect(pushButton_saveAs, SIGNAL( clicked() ),  this, SLOT( saveAs() ));

	connect(this, SIGNAL( openTX2(const QString&) ),  m_model, SLOT( openTX2(const QString&) ));
	connect(this, SIGNAL( openImage(const QString&) ),  m_model, SLOT( openImage(const QString&) ));


	connect(this, SIGNAL( saveTX2(const QString&) ),  m_model, SLOT( saveTX2(const QString&) ));
	connect(this, SIGNAL( saveImage(const QString&) ),  m_model, SLOT( saveImage(const QString&) ));

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
		if(emit openTX2(filepath)){
			QMainWindow::statusBar()->showMessage(QString("Opened TX2 %1").arg(filepath));
			m_currentOpendFile = filepath;
			pushButton_save->setEnabled(true);
			pushButton_saveAs->setEnabled(true);
		}else{
			QMainWindow::statusBar()->showMessage(QString("Couldn't open TX2 %1").arg(filepath));
			pushButton_save->setEnabled(false);
			pushButton_saveAs->setEnabled(false);
		}
	}else{
		if(emit openImage(filepath)){
			QMainWindow::statusBar()->showMessage(QString("Opened image %1").arg(filepath));
			pushButton_save->setEnabled(true);
			pushButton_saveAs->setEnabled(true);
			m_currentOpendFile = filepath;
		}else{
			QMainWindow::statusBar()->showMessage(QString("Couldn't open image %1").arg(filepath));
			pushButton_save->setEnabled(false);
			pushButton_saveAs->setEnabled(false);
		}
	}
}


void TX2Editor::save(){
	save(m_currentOpendFile);
}

void TX2Editor::saveAs(){
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save image"),
				QFileInfo(m_currentOpendFile).baseName(),
			   tr("TX2 image (*.tx2);;PNG (*.png);;TGA (*.tga);;JPG (*.jpg)"));
	save(fileName);
}
void TX2Editor::save(const QString& filepath){
	if(filepath.isEmpty()) return;

	if(filepath.right(4).toLower() == ".tx2"){
		if(emit saveTX2(filepath)){
			QMainWindow::statusBar()->showMessage(QString("Saved TX2 %1").arg(filepath));
			m_currentOpendFile = filepath;
		}else{
			QMainWindow::statusBar()->showMessage(QString("Couldn't save TX2 %1").arg(filepath));
		}
	}else{
		if(emit saveImage(filepath)){
			QMainWindow::statusBar()->showMessage(QString("Saved image %1").arg(filepath));
			m_currentOpendFile = filepath;
		}else{
			QMainWindow::statusBar()->showMessage(QString("Couldn't save image %1").arg(filepath));
		}
	}

}

void TX2Editor::updateInfo(){
	switch (m_model->image->header.type) {
		case PG::FILE::tx2Type::DXT1:
				value_type->setText("DXT1");
			break;
		case PG::FILE::tx2Type::DXT5:
				value_type->setText("DXT5");
			break;
		case PG::FILE::tx2Type::BGRA:
				value_type->setText("BGRA");
			break;
		case PG::FILE::tx2Type::COLORTABLE_BGRA16:
				value_type->setText("COLORTABLE_BGRA16");
			break;
		case PG::FILE::tx2Type::COLORTABLE_RGBA16:
				value_type->setText("COLORTABLE_RGBA16");
			break;
		case PG::FILE::tx2Type::COLORTABLE_BGRA256:
				value_type->setText("COLORTABLE_BGRA256");
			break;
		case PG::FILE::tx2Type::COLORTABLE_RGBA256:
				value_type->setText("COLORTABLE_RGBA256");
			break;
		default:
			value_type->setText("NONE");
			break;
	}


	value_colors->setText(QString::number(m_model->image->header.colortableSize));
	value_width->setText(QString::number(m_model->getWidth()));
	value_height->setText(QString::number(m_model->getHeight()));

	value_colortables->setText(QString::number(m_model->image->header.colortableSize));
}
