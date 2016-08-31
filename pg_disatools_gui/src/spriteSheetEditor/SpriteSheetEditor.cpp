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
#include <spriteSheetEditor/SpriteSheetEditor.h>
#include "ui_spritesheeteditor.h"

#include <QFileDialog>
#include <TitleDefine.h>
#include <QMessageBox>
#include <QColorDialog>
#include <QtQml>
#include <spriteSheetEditor/Timeline.h>
#include <files/SpriteData.h>
#include <SpriteSheetEditor/CreateNewAnimation.h>
//#include <QShortcut>

inline void about(){
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("About");
    //msgBox.setWindowIcon(QMessageBox::Information);
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText(
    			"This gui application allows you to view and extract Disgaea PC sprite sheets.<br>"\
    			"Currently it is very basic.<br>"\

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

SpriteSheetEditor::SpriteSheetEditor(QWidget *parent):
	QMainWindow(parent),
	ui(new Ui::SpriteSheetEditorWindow)
{
	ui->setupUi(this);

	m_player = new SpritePlayer(this);
	m_player->connectGLWidget(ui->openGLWidget);
	ui->comboBox->setModel(m_player->getSpriteData());

	setWindowIcon(QIcon("resources/sprite_sheet_editor_icon.ico"));
	setWindowTitle(SpriteSheetEditorTITLE);

	ui->btnPrevious->setIcon(QIcon("resources/materials/icons/previous.png"));
	ui->btnPausePlay->setIcon(QIcon("resources/materials/icons/pause.png"));
	ui->btnNext->setIcon(QIcon("resources/materials/icons/next.png"));

	//keyframes
	//connect(m_player->getTimeline(), SIGNAL(totalKeyframes( int )), this, SLOT(setTotalKeyframes( int)));
	//connect(m_player->getTimeline(), SIGNAL(currentKeyframe( int )), this, SLOT(setCurrentKeyframe( int )));
	connect(m_player->getTimeline(), SIGNAL(totalFrames( int )), this, SLOT(setTotalFrames( int)));
	connect(m_player->getTimeline(), SIGNAL(currentFrame( int )), this, SLOT(setCurrentFrame( int )));

	//ui->openGLWidget->openSprite("C:/Users/ProgSys/Desktop/Disgaea/PC/IMY/LAHARL.SH");
	//files
	connect(ui->actionOpen_File, SIGNAL(triggered()), this, SLOT(open()));
	connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(save()));
	connect(ui->actionSave_As, SIGNAL(triggered()), this, SLOT(saveAs()));

	connect(ui->actionImport_SH, SIGNAL(triggered()), this, SLOT(import()));
	connect(ui->actionExport_SH, SIGNAL(triggered()), this, SLOT(exportIt()));

	//tools
	connect(ui->actionDump, SIGNAL(triggered()), this, SLOT(dump()));
	connect(ui->actionExport_sprites_as_PNG, &QAction::triggered, this, [this]{exportSprites("PNG");} );
	connect(ui->actionExport_sprites_as_TGA, &QAction::triggered, this, [this]{exportSprites("TGA");} );
	connect(ui->actionExport_sprites_IDs, &QAction::triggered, this, [this]{exportSprites("TGA", true);} );

	connect(ui->actionExport_color_table, &QAction::triggered, this, [this]{exportColortable();} );
	connect(ui->actionImport_color_table, &QAction::triggered, this, [this]{importColortable();} );

	//View
	connect(ui->action_Pick_color, SIGNAL(triggered()), this, SLOT(pickBackgroundColor()));
	connect(ui->actionDisplay_external_references, SIGNAL(triggered(bool)), ui->openGLWidget, SLOT(displayExternalReferences(bool)));
	connect(ui->actionDisplay_ground, SIGNAL(triggered(bool)), ui->openGLWidget, SLOT(displayGround(bool)));
	connect(ui->actionDisplay_shadow, SIGNAL(triggered(bool)), ui->openGLWidget, SLOT(displayShadow(bool)));
	connect(ui->actionTimeline, SIGNAL(triggered(bool)), ui->dockTimeline, SLOT(setVisible(bool)));
	connect(ui->actionSprite_sheet_view, SIGNAL(triggered(bool)), ui->spritesView, SLOT(setVisible(bool)));
	connect(ui->dockTimeline, SIGNAL(visibilityChanged(bool)), ui->actionTimeline, SLOT(setChecked(bool)));
	connect(ui->spritesView, SIGNAL(visibilityChanged(bool)), ui->actionSprite_sheet_view, SLOT(setChecked(bool)));

	//About
	connect(ui->actionAbout, &QAction::triggered, this, [this]{
		about();
	});
	connect(ui->actionAbout_Qt, &QAction::triggered, this, [this]{
		 QMessageBox::aboutQt(this);
	} );

	//player functions
	connect(ui->btnPrevious, SIGNAL(clicked()), m_player->getTimeline(), SLOT(previousKeyframe()));
	connect(ui->btnPausePlay, SIGNAL(clicked()), this, SLOT(clickPlayPause()));
	connect(ui->btnNext, SIGNAL(clicked()), m_player->getTimeline(), SLOT(nextKeyframe()));
	connect(ui->comboBox, SIGNAL(currentIndexChanged( int )), m_player, SLOT(setAnimation( int )));
	connect(m_player->getTimeline(), SIGNAL(onPlay()), this, SLOT(setImagePause()));
	connect(m_player->getTimeline(), SIGNAL(onPause()), this, SLOT(setImagePlay()));

	connect(ui->pushButton_newanimation, SIGNAL(clicked()), this,  SLOT(createNewAnimation()) );


	connect(this, SIGNAL(openSH( const QString& )),  m_player, SLOT(open( const QString& )));
	connect(this, SIGNAL(saveSH( const QString& )),  m_player->getSpriteData(), SLOT(save( const QString& )));
	connect(this, SIGNAL(importSH( const QString& )), m_player, SLOT(importSH( const QString& )));
	connect(this, SIGNAL(exportSH( const QString& )), m_player->getSpriteData(), SLOT(exportSH( const QString& )));

	connect(this, SIGNAL( dumpSprite( const QString& )), m_player->getSpriteData(), SLOT(dump( const QString& )));
	connect(this, SIGNAL( exportSprites( const QString& , const QString& ) ), m_player->getSpriteData(), SLOT( exportSprites( const QString& , const QString& ) ) );
	connect(this, SIGNAL( exportSpritesIDs( const QString& , const QString& ) ), m_player->getSpriteData(), SLOT( exportSpritesIDs( const QString& , const QString& ) ) );
	connect(this, SIGNAL( exportColortable( const QString& ) ), m_player->getSpriteData(), SLOT( exportColortable( const QString& ) ) );
	connect(this, SIGNAL( importColortable( const QString& ) ), m_player->getSpriteData(), SLOT( importColortable( const QString& ) ) );


	connect(this, SIGNAL(backgroundColorSelected(  const QColor& )), ui->openGLWidget, SLOT( setBackgroundColor( const QColor& )));

	ui->openGLWidget->setUpConnections(this);

	//timeline
    qmlRegisterType<Timeline>("MyTimeLine",0,1, "Timeline");
    qmlRegisterType<Keyframe>("MyKeyframe",0,1, "Keyframe");
    qmlRegisterType<Cutout>("MyCutout",0,1, "Cutout");
    qmlRegisterType<SpriteData>("MySpriteData",0,1, "SpriteData");
    qmlRegisterType<SpriteSheet>("MySpriteSheet",0,1, "SpriteSheet");
    qmlRegisterType<Marker>("MyMarker",0,1, "Marker");
    qmlRegisterType<MarkersList>("MyMarkersList",0,1, "MarkersList");
    ui->timelineQML->rootContext()->setContextProperty("timeline", m_player->getTimeline());
    ui->timelineQML->rootContext()->setContextProperty("spritedata", m_player->getSpriteData());
    ui->timelineQML->setSource(QUrl::fromLocalFile("QML/Timeline.qml"));



    ui->quickSpriteView->rootContext()->setContextProperty("spritedata", m_player->getSpriteData());


    ui->quickSpriteView->engine()->addImageProvider(QLatin1String("imageprovider"), new SpriteViewImageProvider(m_player->getSpriteData()));
    ui->quickSpriteView->setSource(QUrl::fromLocalFile("QML/SpriteView.qml"));
    ui->timelineQML->engine()->addImageProvider(QLatin1String("previewprovider"), new TimelinePreviewImageProvider(m_player->getSpriteData()));

    //ui->spritesView->close();
    //ui->quickSpriteView->engine()->addImageProvider(QLatin1String("previewprovider"), m_TimelinePreviewImageProvider);

    //QShortcut *shortcut = new QShortcut(QKeySequence("Space"), this);
    // QObject::connect(shortcut, SIGNAL(activated()), receiver, SLOT(yourSlotHere()));
}

void SpriteSheetEditor::open(){
    QFileDialog openDialog(this);
    openDialog.setNameFilter(tr("PG SPRITE (*.spg)"));

    QStringList fileNames;
	if (openDialog.exec()){
		fileNames = openDialog.selectedFiles();
		if(fileNames.size() > 0){
			open(fileNames[0]);
		}
	}
}

void SpriteSheetEditor::open(const QString& file){
	ui->comboBox->clear();

	qDebug()<<"Openning: "<<file;
	if(file.isEmpty() || !(emit openSH(file))){
		ui->statusbar->showMessage(QString("Failed to opened %1.").arg(file));
		setTitel();

		ui->actionSave->setEnabled(false);
		ui->actionSave_As->setEnabled(false);
		ui->actionExport_SH->setEnabled(false);
		ui->actionDump->setEnabled(false);

		ui->actionExport_sprites_as_PNG->setEnabled(false);
		ui->actionExport_sprites_as_TGA->setEnabled(false);
		ui->actionExport_sprites_IDs->setEnabled(false);
		ui->actionExport_color_table->setEnabled(false);
		ui->actionImport_color_table->setEnabled(false);

		ui->pushButton_newanimation->setEnabled(false);
		m_lastOpendFile.clear();
	}else{
		ui->statusbar->showMessage(QString("Opened %1.").arg(file));
		setTitel(file);

		ui->actionSave->setEnabled(true);
		ui->actionSave_As->setEnabled(true);
		ui->actionExport_SH->setEnabled(true);
		ui->actionDump->setEnabled(true);
		ui->actionExport_sprites_as_PNG->setEnabled(true);
		ui->actionExport_sprites_as_TGA->setEnabled(true);
		ui->actionExport_sprites_IDs->setEnabled(true);
		ui->actionExport_color_table->setEnabled(true);
		ui->actionImport_color_table->setEnabled(true);
		ui->pushButton_newanimation->setEnabled(true);
		ui->comboBox->setCurrentIndex(0);
	}

}

void SpriteSheetEditor::save(){
	if(m_lastOpendFile.isEmpty()){
		saveAs();
	}else
		save(m_lastOpendFile);
}

void SpriteSheetEditor::save(const QString& file){
	if(emit saveSH(file)){
		ui->statusbar->showMessage(QString("Saved to %1.").arg(file));
		m_lastOpendFile = file;
	}else{
		ui->statusbar->showMessage(QString("Failed save to %1.").arg(file));
	}

}
void SpriteSheetEditor::saveAs(){
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save sprite sheet"),
										QFileInfo(m_player->getSpriteData()->getLastFileName()).baseName(),
	                                   tr("PG SPRITE (*.spg)"));
	if (fileName.isEmpty()) return;

	save(fileName);
}

void SpriteSheetEditor::import(){
    QFileDialog openDialog(this);
    openDialog.setNameFilter(tr("SPRITE (*.sh);;DATA (*.dat)"));

    QStringList fileNames;
	if (openDialog.exec()){
		fileNames = openDialog.selectedFiles();
		if(fileNames.size() > 0){
			import(fileNames[0]);
		}
	}
}

void SpriteSheetEditor::import(const QString& file){
	ui->comboBox->clear();

	qDebug()<<"Inporting: "<<file;
	if(file.isEmpty() || !(emit importSH(file))){
		ui->statusbar->showMessage(QString("Failed to import %1.").arg(file));
		setTitel();

		ui->actionSave->setEnabled(false);
		ui->actionSave_As->setEnabled(false);
		ui->actionExport_SH->setEnabled(false);
		ui->actionDump->setEnabled(false);

		ui->actionExport_sprites_as_PNG->setEnabled(false);
		ui->actionExport_sprites_as_TGA->setEnabled(false);
		ui->actionExport_sprites_IDs->setEnabled(false);
		ui->actionExport_color_table->setEnabled(false);
		ui->actionImport_color_table->setEnabled(false);

		ui->pushButton_newanimation->setEnabled(false);
	}else{
		ui->statusbar->showMessage(QString("Imported %1.").arg(file));
		setTitel(file);

		ui->actionSave->setEnabled(true);
		ui->actionSave_As->setEnabled(true);
		ui->actionExport_SH->setEnabled(true);
		ui->actionDump->setEnabled(true);
		ui->actionExport_sprites_as_PNG->setEnabled(true);
		ui->actionExport_sprites_as_TGA->setEnabled(true);
		ui->actionExport_sprites_IDs->setEnabled(true);
		ui->actionExport_color_table->setEnabled(true);
		ui->actionImport_color_table->setEnabled(true);
		ui->pushButton_newanimation->setEnabled(true);
		ui->comboBox->setCurrentIndex(0);
	}
}

void SpriteSheetEditor::exportIt(){
	QString fileName = QFileDialog::getSaveFileName(this, tr("Export SH file"),
										QFileInfo(m_player->getSpriteData()->getLastFileName()).baseName()+"_export",
	                                   tr("sh (*.sh)"));

	if (fileName.isEmpty()) return;

	if(emit exportSH(fileName)){
		ui->statusbar->showMessage(QString("SH exported to %1.").arg(fileName));
	}else{
		ui->statusbar->showMessage(QString("Failed to export SH file to %1.").arg(fileName));
		m_lastOpendFile.clear();
	}
}

void SpriteSheetEditor::dump(){
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Dump"),
										"dump",
	                                   tr("txt (*.txt)"));

	if (fileName.isEmpty()) return;

	if(emit dumpSprite(fileName)){
		ui->statusbar->showMessage(QString("Dumped to %1.").arg(fileName));
	}else{
		ui->statusbar->showMessage(QString("Failed to dump file to %1.").arg(fileName));
	}
}

void SpriteSheetEditor::exportSprites(const QString& filetype, bool asID){
	QString dir = QFileDialog::getExistingDirectory(this, tr("Extract Directory"), "/home",QFileDialog::ShowDirsOnly
	                                             | QFileDialog::DontResolveSymlinks);

	if(dir.isEmpty() || dir.isNull()){
		ui->statusbar->showMessage("Invalid extract directory.");
		return;
	}

	if(  (asID)? emit exportSpritesIDs(dir, filetype): emit exportSprites(dir, filetype)   ){
		ui->statusbar->showMessage(QString("Saved sprites to %1.").arg(dir));
	}else{
		ui->statusbar->showMessage(QString("Failed to export sprites to %1.").arg(dir));
	}
}

void SpriteSheetEditor::exportColortable(){
		QString fileName = QFileDialog::getSaveFileName(this, tr("Export color table"),
														"colortable",
													   tr("PNG (*.png);;TGA (*.tga)"));

		if(emit exportColortable(fileName)){
			ui->statusbar->showMessage(QString("Saved color table to %1.").arg(fileName));
		}else{
			ui->statusbar->showMessage(QString("Failed to export color table to %1.").arg(fileName));
		}
}

void SpriteSheetEditor::importColortable(){
    QFileDialog openDialog(this);
    openDialog.setNameFilter(tr("TGA (*.tga);;PNG (*.png)"));

    QStringList fileNames;
	if (openDialog.exec()){
		fileNames = openDialog.selectedFiles();
		if(fileNames.size() > 0){
			if(emit importColortable(fileNames[0])){
				ui->statusbar->showMessage(QString("Color table imported from %1.").arg(fileNames[0]));
			}else{
				ui->statusbar->showMessage(QString("Failed to import color table from %1.").arg(fileNames[0]));
			}

		}
	}


}

void SpriteSheetEditor::createNewAnimation(){
	CreateNewAnimation create(m_player->getSpriteData(), this);
	create.exec();

	if(create.isAccepted()){
		m_player->getSpriteData()->push_backAnimation(create.getName(), create.getID());
	}
}


void SpriteSheetEditor::setCurrentFrame(int currFrame){
	ui->labelCurentFrame->setText(QString::number(currFrame));
}

void SpriteSheetEditor::setTotalFrames(int totalFrames){
	ui->labelTotalFrames->setText(QString::number(totalFrames));
}

void SpriteSheetEditor::setTitel(){
	setWindowTitle(SpriteSheetEditorTITLE);
}

void SpriteSheetEditor::setTitel(const QString& filename){
	 if(filename.isEmpty()){
		 setTitel();
		 return;
	 }
	 QString title(SpriteSheetEditorTITLE);
	 title.append(" - ");
	 title.append(filename);
	 setWindowTitle(title);
}

void SpriteSheetEditor::clickPlayPause(){
	if(m_player->getTimeline()->isPlaying())
		m_player->getTimeline()->pause();
	else
		m_player->getTimeline()->play();
}

void SpriteSheetEditor::setImagePlay(){
	ui->btnPausePlay->setIcon(QIcon("resources/materials/icons/play.png"));
}
void SpriteSheetEditor::setImagePause(){
	ui->btnPausePlay->setIcon(QIcon("resources/materials/icons/pause.png"));
}

void SpriteSheetEditor::pickBackgroundColor(){
	QColor color = QColorDialog::getColor(QColor(5,79,121), this, "Pick a background color!");
	emit backgroundColorSelected(color);
}

SpriteSheetEditor::~SpriteSheetEditor() {
	delete m_player;
	delete ui;
}

