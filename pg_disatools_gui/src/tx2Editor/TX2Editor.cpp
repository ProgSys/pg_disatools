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
#include <tx2Editor/TX2ConvertToDialog.h>

#include <QProgressDialog>
#include <QFuture>
#include <QtConcurrent/QtConcurrentRun>


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

unsigned int TX2EditorModel::getNumberOfColors() const{
	if(!getNumberOfColorTables()) return 0;
	return image->header.colortableSize;
}

unsigned int TX2EditorModel::getNumberOfColorTables() const{
	return image->header.colortables.size();
}


int TX2EditorModel::getCurrentColorTable() const{
	return m_currentColorTable;
}

void TX2EditorModel::setCurrentColorTable(int current){
	if(current >= getNumberOfColorTables() ) current = getNumberOfColorTables()-1;
	if(current < 0 ) current = 0;
	if(current == m_currentColorTable) return;
	m_currentColorTable = current;
	emit imageChanged();
}


QColor TX2EditorModel::getColor(unsigned int index ) const{
	return getColor(index, m_currentColorTable);
}

QColor TX2EditorModel::getColor(unsigned int index, unsigned int colorTableIndex) const{
	if(colorTableIndex >= getNumberOfColorTables()) return QColor();
	const PG::FILE::ColorTable& table = image->header.colortables[colorTableIndex];
	if(index >= table.size()) return QColor();
	const PG::UTIL::rgba& c = table[index];
	return QColor(c.r,c.g,c.b,c.a);
}

void TX2EditorModel::setColor(const QColor& color, unsigned int index ){
	setColor(color, index, m_currentColorTable);
}

void TX2EditorModel::setColor(const QColor& color, unsigned int index, unsigned int colorTableIndex){
	if(colorTableIndex >= getNumberOfColorTables()) return;
	PG::FILE::ColorTable& table = image->header.colortables[colorTableIndex];
	if(index >= table.size()) return;
	PG::UTIL::rgba& c = table[index];
	c.r = color.red();
	c.g = color.green();
	c.b = color.blue();
	c.a = color.alpha();
	emit imageChanged();
}

void TX2EditorModel::addColorTable(unsigned int atColorTableIndex, bool copy){
	if(!getNumberOfColorTables()){
		std::vector<PG::UTIL::rgba> table(image->header.colortableSize);
		image->header.colortables.push_back(table);
		emit imageChanged();
		return;
	}
	if(atColorTableIndex >= getNumberOfColorTables()) return;
	std::vector<PG::UTIL::rgba> table(image->header.colortableSize);
	if(copy) table =  image->header.colortables[atColorTableIndex];
	image->header.colortables.insert(image->header.colortables.begin()+atColorTableIndex+1, table);
	emit imageChanged();
}

void TX2EditorModel::removeColorTable(unsigned int atColorTableIndex ){
	if(atColorTableIndex >= getNumberOfColorTables()) return;
	image->header.colortables.erase(image->header.colortables.begin()+atColorTableIndex);
	if(m_currentColorTable >= getNumberOfColorTables())
		m_currentColorTable--;
	if(m_currentColorTable < 0) m_currentColorTable = 0;
	emit imageChanged();
}


void TX2EditorModel::exportColorTable(unsigned int colorTableIndex) const{
	if(colorTableIndex >= getNumberOfColorTables()) return;
	QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Export color table"),
								 "colorTable"+QString::number(colorTableIndex+1),
								   tr("PNG (*.png);;TGA (*.tga)"));

	exportColorTable(colorTableIndex, fileName);
}
void TX2EditorModel::exportColorTable(unsigned int colorTableIndex, const QString filePath) const{
	if(colorTableIndex >= getNumberOfColorTables() || filePath.isEmpty()) return;

	const PG::FILE::ColorTable& colortable = image->header.colortables[colorTableIndex];
	QImage img(colortable.size(), 1, QImage::Format_RGBA8888);
	memcpy(img.bits(), &(colortable[0].r) ,colortable.size()*sizeof(PG::UTIL::rgba));

	img.save(filePath, 0, 100);

}
void TX2EditorModel::importColorTable(unsigned int colorTableIndex){
	if(colorTableIndex >= getNumberOfColorTables()) return;
    QFileDialog openDialog(nullptr);
    openDialog.setNameFilter(tr("PNG (*.png);;TGA (*.tga)"));

    QStringList fileNames;
	if (openDialog.exec()){
		fileNames = openDialog.selectedFiles();
		if(fileNames.size() > 0){
			importColorTable(colorTableIndex, fileNames[0]);
		}
	}
}
void TX2EditorModel::importColorTable(unsigned int colorTableIndex, const QString filePath){
	if(colorTableIndex >= getNumberOfColorTables() || filePath.isEmpty()) return;
	QImage img(filePath);
	if(!img.width() || !img.height()) return;

	PG::FILE::ColorTable colortable;
	colortable.reserve(image->header.colortableSize);
	for(int y = 0; y < img.height(); y++)
		for(int x = 0; x < img.width(); x++){
			const QColor& c = QColor(img.pixel(x,y));
			colortable.push_back(PG::UTIL::rgba(c.red(),c.green(), c.blue(), qAlpha(img.pixel(x, y)) ));
		}
	colortable.resize(image->header.colortableSize);
	image->header.colortables[colorTableIndex] = colortable;
	emit imageChanged();
}

bool TX2EditorModel::openTX2(const QString& filepath){
	if(image->open(filepath.toStdString())){
		emit imageChanged();
		return false;
	}
	m_currentColorTable = 0;
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
	m_currentColorTable = 0;
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


void TX2EditorModel::convertTo(PG::FILE::tx2Type type){

	QProgressDialog progress;
	progress.setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
	progress.setWindowTitle("Converting image...");
	progress.setWindowModality(Qt::WindowModal);
	progress.setLabelText("This can take a while.");
	progress.setCancelButton(0);
	progress.setRange(0,0);
	progress.show();

	QFuture<void> f1 = QtConcurrent::run(image, &PG::FILE::tx2Image::convertTo, type,m_currentColorTable );
	while(f1.isRunning()){
		QApplication::processEvents();
	}

	//image->convertTo(type, m_currentColorTable);
	if(image->header.type == PG::FILE::tx2Type::TX2ERROR)
		image->clear();
	qDebug() <<" Convert to: "<<type;
	m_currentColorTable = 0;
	emit imageChanged();
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


	connect(actionOpen, SIGNAL( triggered() ),  this, SLOT( open() ));
	connect(pushButton_open, SIGNAL( clicked() ),  this, SLOT( open() ));

	connect(pushButton_save, SIGNAL( clicked() ),  this, SLOT( save() ));
	connect(pushButton_saveAs, SIGNAL( clicked() ),  this, SLOT( saveAs() ));

	connect(pushButton_convertTo, SIGNAL( clicked() ),  this, SLOT( convertTo() ));

	//View
	connect(actionControls, SIGNAL(triggered(bool)), controlsWidget, SLOT(setVisible(bool)));
	connect(actionColor_Table_View, SIGNAL(triggered(bool)), ColorTableView, SLOT(setVisible(bool)));

	connect(controlsWidget, SIGNAL(visibilityChanged(bool)), actionControls, SLOT(setChecked(bool)));
	connect(ColorTableView, SIGNAL(visibilityChanged(bool)), actionColor_Table_View, SLOT(setChecked(bool)));


	connect(this, SIGNAL( openTX2(const QString&) ),  m_model, SLOT( openTX2(const QString&) ));
	connect(this, SIGNAL( openImage(const QString&) ),  m_model, SLOT( openImage(const QString&) ));

	connect(this, SIGNAL( saveTX2(const QString&) ),  m_model, SLOT( saveTX2(const QString&) ));
	connect(this, SIGNAL( saveImage(const QString&) ),  m_model, SLOT( saveImage(const QString&) ));

	qRegisterMetaType<PG::FILE::tx2Type>("PG::FILE::tx2Type");
	connect(this, SIGNAL( convertTo(PG::FILE::tx2Type) ),  m_model, SLOT( convertTo(PG::FILE::tx2Type) ));

	connect(m_model, SIGNAL( imageChanged() ),  this, SLOT( updateInfo() ));

	//QML
	qmlRegisterType<TX2EditorModel>("MyTX2Editor",0,1, "TX2EditorModel");
	Ui::TX2EditorUI::quickWidget->rootContext()->setContextProperty("tx2editor", m_model);
	Ui::TX2EditorUI::quickWidget_2->rootContext()->setContextProperty("tx2editor", m_model);

	quickWidget->engine()->addImageProvider(QLatin1String("tx2imageprovider"), new TX2ImageProvider(m_model));
	quickWidget->setSource(QUrl::fromLocalFile("QML/Tx2View.qml"));

	quickWidget_2->setSource(QUrl::fromLocalFile("QML/Tx2ColorTableView.qml"));

	ColorTableView->close();
}

TX2Editor::~TX2Editor() {
	delete m_model;
}


void TX2Editor::open(){
    QFileDialog openDialog(this);
    openDialog.setNameFilter(tr("Images (*.tx2;*.txp;*.png;*.tga;*.jpg);;TX2 image (*.tx2);;PNG (*.png);;TGA (*.tga);;JPG (*.jpg)"));

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

	if(filepath.right(4).toLower() == ".tx2" || filepath.right(4).toLower() == ".txp"){
		if(emit openTX2(filepath)){
			QMainWindow::statusBar()->showMessage(QString("Opened TX2 %1").arg(filepath));
			m_currentOpendFile = filepath;
			setTitel(m_currentOpendFile);
			buttonsEnable(true);
		}else{
			QMainWindow::statusBar()->showMessage(QString("Couldn't open TX2 %1").arg(filepath));
			buttonsEnable(false);
			setTitel();
		}
	}else{
		if(emit openImage(filepath)){
			QMainWindow::statusBar()->showMessage(QString("Opened image %1").arg(filepath));
			m_currentOpendFile = filepath;
			//QFileInfo info(m_currentOpendFile);
			//m_currentOpendFile = info.path() + info.completeBaseName() + ".tx2";
			setTitel(m_currentOpendFile);
			buttonsEnable(true);
			pushButton_save->setEnabled(false);
		}else{
			QMainWindow::statusBar()->showMessage(QString("Couldn't open image %1").arg(filepath));
			buttonsEnable(false);
			setTitel();
		}
	}
}


void TX2Editor::save(){
	save(m_currentOpendFile);
}

void TX2Editor::saveAs(){
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save image"),
				QFileInfo(m_currentOpendFile).baseName(),
			   tr("TX2 image (*.tx2;*.txp);;PNG (*.png);;TGA (*.tga);;JPG (*.jpg)"));
	save(fileName);
}
void TX2Editor::save(const QString& filepath){
	if(filepath.isEmpty()) return;

	if(filepath.right(4).toLower() == ".tx2"){
		if(emit saveTX2(filepath)){
			QMainWindow::statusBar()->showMessage(QString("Saved TX2 %1").arg(filepath));
			m_currentOpendFile = filepath;
			setTitel(m_currentOpendFile);
			pushButton_save->setEnabled(true);
		}else{
			QMainWindow::statusBar()->showMessage(QString("Couldn't save TX2 %1").arg(filepath));
		}
	}else{
		if(emit saveImage(filepath)){
			QMainWindow::statusBar()->showMessage(QString("Saved image %1").arg(filepath));
		}else{
			QMainWindow::statusBar()->showMessage(QString("Couldn't save image %1").arg(filepath));
		}
	}

}

inline bool hasToManyColors(PG::FILE::tx2Image* img, unsigned int maxColors, unsigned int colorTable = 0){
	std::vector<PG::UTIL::rgba> colors;
	PG::UTIL::RGBAImage rgbaImg(img->getWidth(), img->getHeight());
	img->getRGBAData((char*)&rgbaImg[0].r, colorTable);

	for(const PG::UTIL::rgba& imgColor: rgbaImg){
		bool found = false;
		for(const PG::UTIL::rgba& c: colors){
			if(imgColor == c){
				found = true;
				break;
			}
		}
		if(found) continue;
		colors.push_back(imgColor);
		if(colors.size() > maxColors) return true;
	}

	return false;
}

void TX2Editor::convertTo(){
	TX2ConvertToDialog dialog(this);
	dialog.exec();

	if(dialog.isAccepted()){
		QMainWindow::statusBar()->showMessage(QString("Please wait this can take a while!"));
		if(dialog.getResult() == "DXT1"){
			if(m_model->getWidth() % 4 != 0 || m_model->getHeight() % 4 != 0 ){
				QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error",
											"To convert a image to DXT1 it must be divisible by 4!",
									 QMessageBox::Ok);
				return;
			}
			setEnabled(false);
			emit convertTo( PG::FILE::tx2Type::DXT1);
		}else if(dialog.getResult() == "DXT5"){
			if(m_model->getWidth() % 4 != 0 || m_model->getHeight() % 4 != 0 ){
				QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Error",
											"To convert a image to DXT5 it must be divisible by 4!",
									 QMessageBox::Ok);
				return;
			}
			emit convertTo( PG::FILE::tx2Type::DXT5);
		}else if(dialog.getResult() == "BGRA8888")
			emit convertTo( PG::FILE::tx2Type::BGRA);
		else if(dialog.getResult() == "COLORTABLE RGBA16"){
			if(hasToManyColors(m_model->image, 16, m_model->getCurrentColorTable())){
				QMessageBox::StandardButton reply = QMessageBox::warning(nullptr, "Continue?",
							"Image has more than 16 colors! Convert may cause errors if you continue!",
					 QMessageBox::Yes|QMessageBox::Cancel);
				if(reply == QMessageBox::Cancel)
					return;
			}
			setEnabled(false);
			emit convertTo( PG::FILE::tx2Type::COLORTABLE_RGBA16);
		}else if(dialog.getResult() == "COLORTABLE BGRA16"){
			if(hasToManyColors(m_model->image, 16, m_model->getCurrentColorTable())){
				QMessageBox::StandardButton reply = QMessageBox::warning(nullptr, "Continue?",
							"Image has more than 16 colors! Convert may cause errors if you continue!",
					 QMessageBox::Yes|QMessageBox::Cancel);
				if(reply == QMessageBox::Cancel)
					return;
			}
			setEnabled(false);
			emit convertTo( PG::FILE::tx2Type::COLORTABLE_BGRA16);
		}else if(dialog.getResult() == "COLORTABLE RGBA256"){
			if(hasToManyColors(m_model->image, 256, m_model->getCurrentColorTable())){
				QMessageBox::StandardButton reply = QMessageBox::warning(nullptr, "Continue?",
							"Image has more than 256 colors! Convert may cause errors if you continue!",
					 QMessageBox::Yes|QMessageBox::Cancel);
				if(reply == QMessageBox::Cancel)
					return;
			}
			setEnabled(false);
			emit convertTo( PG::FILE::tx2Type::COLORTABLE_RGBA256);
		}else if(dialog.getResult() == "COLORTABLE BGRA256"){
			if(hasToManyColors(m_model->image, 256, m_model->getCurrentColorTable())){
				QMessageBox::StandardButton reply = QMessageBox::warning(nullptr, "Continue?",
							"Image has more than 256 colors! Convert may cause errors if you continue!",
					 QMessageBox::Yes|QMessageBox::Cancel);
				if(reply == QMessageBox::Cancel)
					return;
			}
			setEnabled(false);
			emit convertTo( PG::FILE::tx2Type::COLORTABLE_BGRA256);
		}else
			QMainWindow::statusBar()->showMessage(QString("Unknown target '%1'!").arg(dialog.getResult()));
		QMainWindow::statusBar()->showMessage(QString("Convert done!"));
	}else{
		QMainWindow::statusBar()->showMessage(QString("Convert canceled."));
	}

	setEnabled(true);
}


void TX2Editor::buttonsEnable(bool enable){
	pushButton_save->setEnabled(enable);
	pushButton_saveAs->setEnabled(enable);
	actionSave->setEnabled(enable);
	actionSave_As->setEnabled(enable);
	pushButton_convertTo->setEnabled(enable);
}

void TX2Editor::setTitel(){
	setWindowTitle(TX2EditorTITLE);
}

void TX2Editor::setTitel(const QString& filename){
	 if(filename.isEmpty()){
		 setTitel();
		 return;
	 }
	 QString title(TX2EditorTITLE);
	 title.append(" - ");
	 title.append(filename);
	 setWindowTitle(title);
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

	value_colortables->setText(QString::number(m_model->image->header.colortables.size()));
}
